/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "PqClient"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "PqClient.h"
#include "CPqClientLog.h"
#include "pqcmd.h"

const int RET_SUCCESS = 0;
const int RET_FAILED  = -1;

const int EVENT_SIGLE_DETECT = 4;
const int EVENT_SOURCE_CONNECT = 10;

PqClient *mInstance = NULL;
PqClient *PqClient::GetInstance() {
    if (mInstance == NULL) {
        mInstance = new PqClient();
    }

    return mInstance;
}

PqClient::PqClient() {
#if (TV_IPC_TYPE == TV_BINDER)
    Parcel send, reply;
    sp<IServiceManager> serviceManager = defaultServiceManager();
    do {
        mpqServicebinder = serviceManager->getService(String16("pqservice"));
        if (mpqServicebinder != 0) break;
        LOGD("PqClient: Waiting pqservice published.\n");
        usleep(500000);
    } while(true);
    LOGD("Connected to pqservice.\n");
#else
    if (mpDBusConnection == NULL) {
        mpDBusConnection = ClientBusInit();
    }
#endif
}

PqClient::~PqClient() {
#if (TV_IPC_TYPE == TV_DBUS)
    if (mpDBusConnection != NULL) {
        dbus_connection_unref(mpDBusConnection);
        mpDBusConnection = NULL;
    }
#else //(TV_IPC_TYPE == TV_BINDER)
    Parcel send, reply;
    mpqServicebinder->transact(CMD_CLR_PQ_CB, send, &reply);
    mpqServicebinder = NULL;
#endif
}

#if (TV_IPC_TYPE == TV_DBUS)
DBusConnection *PqClient::ClientBusInit()
{
    DBusConnection *connection;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        LOGE("PqClient: connection error: :%s -- %s\n", err.name, err.message);
        dbus_error_free(&err);
        return NULL;
    }

    ret = dbus_bus_request_name(connection, "aml.tv.client", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        LOGE("PqClient: Name error: %s -- %s\n", err.name, err.message);
        dbus_error_free(&err);
        return NULL;
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        return NULL;
    }

    dbus_bus_add_match(connection, "type='signal'", &err);

    dbus_connection_flush(connection);
    if (dbus_error_is_set(&err)) {
        LOGE("PqClient: add Match Error %s--%s\n", err.name, err.message);
        dbus_error_free(&err);
        return connection;
    }

    return connection;
}
#endif

void PqClient::SendMethodCall(char *CmdString)
{
    LOGD("%s.\n", __FUNCTION__);

#if (TV_IPC_TYPE == TV_DBUS)
    DBusMessage *msg;
    DBusMessageIter arg;
    DBusPendingCall *pending;

    msg = dbus_message_new_method_call("aml.tv.service", "/aml/tv", "aml.tv", "cmd");
    if (msg == NULL) {
        LOGE("PqClient: no memory\n");
        return RET_FAILED;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &CmdString, DBUS_TYPE_INVALID)) {
        LOGE("PqClient: add args failed!\n");
        dbus_message_unref(msg);
        return RET_FAILED;
    }

    if (!dbus_connection_send_with_reply (mpDBusConnection, msg, &pending, -1)) {
        LOGE("PqClient: no memeory!");
        dbus_message_unref(msg);
        return RET_FAILED;
    }

    if (pending == NULL) {
        LOGE("PqClient: Pending is NULL, may be disconnect...\n");
        dbus_message_unref(msg);
        return RET_FAILED;
    }

    dbus_connection_flush(mpDBusConnection);
    dbus_message_unref(msg);

    dbus_pending_call_block (pending);
    msg = dbus_pending_call_steal_reply (pending);
    if (msg == NULL) {
        LOGE("PqClient: reply is null. error\n");
        return RET_FAILED;
    }

    dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &arg)) {
        LOGE("PqClient: no argument, error\n");
    }

    if (dbus_message_iter_get_arg_type(&arg) != DBUS_TYPE_INT32) {
        LOGE("PqClient: paramter type error\n");
    }

    dbus_message_iter_get_basic(&arg, &ReturnVal);
    LOGE("PqClient: ret = %d\n",ReturnVal);
    dbus_message_unref(msg);
#else //(TV_IPC_TYPE == TV_BINDER)
    Parcel send, reply;
    memset(mRetBuf, 0, sizeof(mRetBuf)/sizeof(char));

    if (mpqServicebinder != NULL) {
        send.writeCString(CmdString);
        if (mpqServicebinder->transact(CMD_PQ_ACTION, send, &reply) != 0) {
            LOGE("PqClient: call %s failed.\n", CmdString);
            const char* tmp;
            tmp = reply.readCString();
            strcpy(mRetBuf, tmp);
        } else {
            const char* tmp;
            tmp = reply.readCString();
            strcpy(mRetBuf, tmp);
        }
    }
#endif

    LOGE("PqClient: mRetBuf %s.\n", mRetBuf);
}

int PqClient::SplitRetBuf(const char *commandData)
{
    char cmdbuff[1024];
    char *token;
    int  cmd_size = 0;
    const char *delimitation = ".";

    strcpy(cmdbuff, commandData);

    /* get first str*/
    token = strtok(cmdbuff, delimitation);

    /* continue get str*/
    while (token != NULL)
    {
        mRet[cmd_size].assign(token);

        LOGD("%s mRet[%d]:%s\n", token, cmd_size, mRet[cmd_size].c_str());
        cmd_size++;
        token = strtok(NULL, delimitation);
    }

    LOGD("%s: cmd_size = %d\n", __FUNCTION__, cmd_size);

    return cmd_size;
}

int PqClient::SetPQMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_PICTURE_MODE, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetPQMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_PICTURE_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetColorTemperature(int colorTemperatureValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_COLOR_TEMPERATURE_MODE, colorTemperatureValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetColorTemperature()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_COLOR_TEMPERATURE_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetBrightness(int brightnessValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_BRIGHTNESS, brightnessValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetBrightness()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_BRIGHTNESS);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetContrast(int contrastValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_CONTRAST, contrastValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetContrast()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_CONTRAST);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetSaturation(int saturationValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_SATUATION, saturationValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetSaturation()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_SATUATION);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetHue(int hueValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_HUE, hueValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetHue()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_HUE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetSharpness(int sharpnessValue, int is_enable, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d", PQ_SET_SHARPNESS, sharpnessValue, is_enable, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetSharpness()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_SHARPNESS);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetNoiseReductionMode(int nr_mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_NOISE_REDUCTION_MODE, nr_mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetNoiseReductionMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_NOISE_REDUCTION_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetEyeProtectionMode(int source_input, int enable, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d", PQ_SET_EYE_PROTECTION_MODE, source_input, enable, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetEyeProtectionMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_EYE_PROTECTION_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetGammaValue(int gamma_curve, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_GAMMA, gamma_curve, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetGammaValue()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_GAMMA);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetDisplayMode(int isUsed, int DisplayMode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d", PQ_SET_DISPLAY_MODE, isUsed, DisplayMode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetDisplayMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DISPLAY_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetBacklight(int backlightValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_BACKLIGHT, backlightValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetBacklight()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_BACKLIGHT);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetDynamicBacklight(int backlightValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_DYNAMICBACKLIGHT, backlightValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetDynamicBacklight()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DYNAMICBACKLIGHT);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetLocalContrastMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_LOCALCONTRAST, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetLocalContrastMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_LOCALCONTRAST);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetColorBaseMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_CM, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::GetColorBaseMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_CM);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::setCurrentSourceInfo(int sourceInput, int sigFmt, int transFmt)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d", PQ_SET_SOURCE_CHANNEL, sourceInput, sigFmt, transFmt);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

source_input_param_t PqClient::getCurrentSourceInfo()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d", PQ_GET_SOURCE_CHANNEL);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    source_input_param_t source_input_param;
    source_input_param.source_input = atoi(mRet[0].c_str());
    source_input_param.sig_fmt      = atoi(mRet[1].c_str());
    source_input_param.trans_fmt    = atoi(mRet[2].c_str());

    return source_input_param;
}

int PqClient::FactorySetRGBPattern(int r, int g, int b)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_RGB_PATTERN, r, g, b);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetRGBPattern()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d", PQ_FACTORY_GET_RGB_PATTERN);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetGrayPattern(int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d", PQ_FACTORY_SET_GRAY_PATTERN, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetGrayPattern()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d", PQ_FACTORY_GET_GRAY_PATTERN);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceRedGain(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_RED_GAIN, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceRedGain(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_RED_GAIN, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceGreenGain(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_GREE_GAIN, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceGreenGain(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_GREE_GAIN, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceBlueGain(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_BLUE_GAIN, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceBlueGain(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_BLUE_GAIN, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceRedPostOffset(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_RED_POSTOFFSET, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceRedPostOffset(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_RED_POSTOFFSET, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceGreenPostOffset(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_GREE_POSTOFFSET, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceGreenPostOffset(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_GREE_POSTOFFSET, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceBluePostOffset(int source, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_BLUE_POSTOFFSET, source, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceBluePostOffset(int source, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_BLUE_POSTOFFSET, source, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

#if (TV_IPC_TYPE == TV_BINDER)
status_t PqClient::onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags) {
    LOGD("PqClient get tanscode: %u\n", code);
    switch (code) {
        case CMD_START:
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return (0);
}
#endif
