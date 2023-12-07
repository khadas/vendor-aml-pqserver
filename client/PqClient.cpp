/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "PqClient"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

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
        LOGD("%s mInstance is null\n", __FUNCTION__);
        mInstance = new PqClient();
    } else {
        LOGD("%s mInstance is not null\n", __FUNCTION__);
    }

    LOGD("%s: getid %p\n", __FUNCTION__, getpid());
    return mInstance;
}

PqClient::PqClient() {
    sp<ProcessState> proc(ProcessState::self());
    proc->startThreadPool();
    Parcel send, reply;
    sp<IServiceManager> serviceManager = defaultServiceManager();
    init_pq_logging();
    do {
        mpqServicebinder = serviceManager->getService(String16("pqservice"));
        LOGD("%s mpqServicebinder is %p\n", __FUNCTION__, mpqServicebinder);
        if (mpqServicebinder != 0) break;
        LOGD("PqClient: Waiting pqservice published.\n");
        usleep(500000);
    } while(true);
    LOGD("Connected to pqservice.\n");
}

PqClient::~PqClient() {
    mInstance = NULL;
    Parcel send, reply;
    mpqServicebinder->transact(CMD_CLR_PQ_CB, send, &reply);
    mpqServicebinder = NULL;
}

void PqClient::SendMethodCall(char *CmdString)
{
    LOGD("%s\n", __FUNCTION__);

    Parcel send, reply;
    memset(mRetBuf, 0, sizeof(mRetBuf)/sizeof(char));

    if (mpqServicebinder != NULL) {
        send.writeCString(CmdString);
        if (mpqServicebinder->transact(CMD_PQ_ACTION, send, &reply) != 0) {
            LOGE("PqClient: call %s failed\n", CmdString);
            const char* tmp;
            tmp = reply.readCString();
            if (strlen(tmp) <= sizeof(mRetBuf)/sizeof(char)) {
                strcpy(mRetBuf, tmp);
            }
        } else {
            const char* tmp;
            tmp = reply.readCString();
            if (strlen(tmp) <= sizeof(mRetBuf)/sizeof(char)) {
                strcpy(mRetBuf, tmp);
            }
        }
    }

    LOGE("PqClient: mRetBuf %s\n", mRetBuf);
}

int PqClient::SplitRetBuf(const char *commandData)
{
    char cmdbuff[1024] = {'\0'};
    char *token;
    int  cmd_size = 0;
    const char *delimitation = ".";

    if (strlen(commandData) <= sizeof(cmdbuff)/sizeof(char)) {
        strcpy(cmdbuff, commandData);
    }

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

int PqClient::GetColorTemperature(void)
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

int PqClient::SetColorTemperatureUserParam(tvpq_rgb_ogo_t *pData)
{
    LOGD("%s\n", __FUNCTION__);
    if (mpqServicebinder == NULL) {
        return -1;
    }

    if (pData == NULL) {
        LOGE("%s pData is null\n", __FUNCTION__);
        return -1;
    }

    Parcel send, reply;
    send.writeInt32(pData->en);

    send.writeInt32(pData->r_gain);
    send.writeInt32(pData->g_gain);
    send.writeInt32(pData->b_gain);

    send.writeInt32(pData->r_post_offset);
    send.writeInt32(pData->g_post_offset);
    send.writeInt32(pData->b_post_offset);

    send.writeInt32(pData->r_pre_offset);
    send.writeInt32(pData->g_pre_offset);
    send.writeInt32(pData->b_pre_offset);

    if (mpqServicebinder->transact(CMD_PQ_SET_COLORTEMPERATURE_USER_PARAM, send, &reply) != 0) {
        LOGE("PqClient: call %d failed\n", CMD_PQ_SET_COLORTEMPERATURE_USER_PARAM);
        return -1;
    }

    return reply.readInt32();
}

tvpq_rgb_ogo_t PqClient::GetColorTemperatureUserParam(void)
{
    LOGD("%s\n", __FUNCTION__);
    tvpq_rgb_ogo_t Data;
    memset(&Data, 0, sizeof(tvpq_rgb_ogo_t));

    if (mpqServicebinder == NULL) {
        return Data;
    }

    Parcel send, reply;
    if (mpqServicebinder->transact(CMD_PQ_GET_COLORTEMPERATURE_USER_PARAM, send, &reply) != 0) {
        LOGE("PqClient: call %d failed\n", CMD_PQ_GET_COLORTEMPERATURE_USER_PARAM);
        return Data;
    }

    Data.en = reply.readInt32();

    Data.r_gain = reply.readInt32();
    Data.g_gain = reply.readInt32();
    Data.b_gain = reply.readInt32();

    Data.r_post_offset = reply.readInt32();
    Data.g_post_offset = reply.readInt32();
    Data.b_post_offset = reply.readInt32();

    Data.r_pre_offset = reply.readInt32();
    Data.g_pre_offset = reply.readInt32();
    Data.b_pre_offset = reply.readInt32();

    return Data;
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

int PqClient::SetColorGamutMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_COLORGAMUT, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetColorGamutMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_COLORGAMUT);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetSourceHDRType()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_SOURCE_HDR_TYPE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetDynamicContrastMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_DYNAMICCONTRAST, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetDynamicContrastMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DYNAMICCONTRAST);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SSMRecovery(void)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d", PQ_SET_RECOVERYPQ);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::HasMemcFunc()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_HAS_MEMC);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;

}

int PqClient::SetMemcMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_MEMCMODE, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetMemcMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_MEMCMODE);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetMemcDeBlurLevel(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_MEMC_DEBLUR, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetMemcDeBlurLevel()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_MEMC_DEBLUR);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetMemcDeJudderLevel(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_MEMC_DEJUDDER, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetMemcDeJudderLevel()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_MEMC_DEJUDDER);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetDecontourMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_DECONTOUR, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetDecontourMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DECONTOUR);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetDeBlockMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_DEBLOCK, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetDeBlockMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DEBLOCK);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetDeMosquitoMode(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_DEMOSQUITO, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetDeMosquitoMode()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_DEMOSQUITO);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetBlackStretch(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_BLACKSTRETCH, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetBlackStretch()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_BLACKSTRETCH);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetBlueStretch(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_BLUESTRETCH, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetBlueStretch()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_BLUESTRETCH);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::HasAipqFunc()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_HAS_AIPQ);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetAipqEnable(int mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d", PQ_SET_AIPQ, mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetAipqEnable()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_AIPQ);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::HasAisrFunc()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_HAS_AISR);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetAiSrEnable(int mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d", PQ_SET_AISR, mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetAiSrEnable()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_AISR);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetLdim(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_LDIM, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetLdim()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_GET_LDIM);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetMpegNr(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_MPEGNR, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetMpegNr()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_SET_MPEGNR);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

noline_params_t PqClient::FactoryGetNolineParams(source_input_param_t source_input_param, int type)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d",
            PQ_FACTORY_GET_NOLINE_PARAMS,
            source_input_param.source_input,
            source_input_param.sig_fmt,
            source_input_param.trans_fmt,
            type);
    SendMethodCall(buf);

    SplitRetBuf(mRetBuf);

    noline_params_t noline_params;
    noline_params.osd0   = atoi(mRet[0].c_str());
    noline_params.osd25  = atoi(mRet[1].c_str());
    noline_params.osd50  = atoi(mRet[2].c_str());
    noline_params.osd75  = atoi(mRet[3].c_str());
    noline_params.osd100 = atoi(mRet[4].c_str());
    LOGE("PqClient: [osd0:%d][osd25:%d][osd50:%d][osd75:%d][osd100:%d].\n",
            noline_params.osd0,
            noline_params.osd25,
            noline_params.osd50,
            noline_params.osd75,
            noline_params.osd100);

    return noline_params;
}

int PqClient::FactorySetNolineParams(source_input_param_t source_input_param, int type, noline_params_t noline_params)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",
            PQ_FACTORY_SET_NOLINE_PARAMS,
            source_input_param.source_input,
            source_input_param.sig_fmt,
            source_input_param.trans_fmt,
            type,
            noline_params.osd0,
            noline_params.osd25,
            noline_params.osd50,
            noline_params.osd75,
            noline_params.osd100
    );
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetBrightnessNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_BRIGHTNESS, noline_params);
}

int PqClient::FactorySetContrastNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_CONTRAST, noline_params);
}
int PqClient::FactorySetSaturationNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_SATURATION, noline_params);
}
int PqClient::FactorySetHueNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_HUE, noline_params);
}
int PqClient::FactorySetSharpnessNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_SHARPNESS, noline_params);
}
int PqClient::FactorySetVolumeNolineParams(source_input_param_t source_input_param, noline_params_t noline_params)
{
    return FactorySetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_VOLUME, noline_params);
}

noline_params_t PqClient::FactoryGetBrightnessNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_BRIGHTNESS);
}

noline_params_t PqClient::FactoryGetContrastNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_CONTRAST);
}

noline_params_t PqClient::FactoryGetSaturationNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_SATURATION);
}

noline_params_t PqClient::FactoryGetHueNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_HUE);
}

noline_params_t PqClient::FactoryGetSharpnessNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_SHARPNESS);
}

noline_params_t PqClient::FactoryGetVolumeNolineParams(source_input_param_t source_input_param)
{
    return FactoryGetNolineParams(source_input_param, NOLINE_PARAMS_TYPE_VOLUME);
}

int PqClient::FactorySetColorParams(int tempMode, tvpq_rgb_ogo_t rgbogo)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[128] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_COLOR_PARAMS, tempMode, rgbogo.en, rgbogo.r_pre_offset, rgbogo.g_pre_offset,
        rgbogo.b_pre_offset, rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain, rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetColorParams(int tempMode, tvpq_rgb_ogo_t *rgbogo)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[128] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", PQ_FACTORY_GET_COLOR_PARAMS, tempMode, rgbogo->en, rgbogo->r_pre_offset, rgbogo->g_pre_offset,
        rgbogo->b_pre_offset, rgbogo->r_gain, rgbogo->g_gain, rgbogo->b_gain, rgbogo->r_post_offset, rgbogo->g_post_offset, rgbogo->b_post_offset);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    //rgbogo->en = atoi(mRet[0].c_str());
    rgbogo->r_pre_offset = atoi(mRet[1].c_str());
    rgbogo->g_pre_offset = atoi(mRet[2].c_str());
    rgbogo->b_pre_offset = atoi(mRet[3].c_str());
    rgbogo->r_gain = atoi(mRet[4].c_str());
    rgbogo->g_gain = atoi(mRet[5].c_str());
    rgbogo->b_gain = atoi(mRet[6].c_str());
    rgbogo->r_post_offset = atoi(mRet[7].c_str());
    rgbogo->g_post_offset = atoi(mRet[8].c_str());
    rgbogo->b_post_offset = atoi(mRet[9].c_str());

    ret = atoi(mRet[0].c_str());
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetDecodeLumaParams(int sourceInput, int sigFmt, int transFmt, int paramType, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_DEC_LUMA_PARAMS, sourceInput, sigFmt, transFmt, paramType, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetDecodeLumaParams(int sourceInput, int sigFmt, int transFmt, int paramType)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_DEC_LUMA_PARAMS, sourceInput, sigFmt, transFmt, paramType);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::ResetPictureUiSetting(void)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d", PQ_SET_PICTURE_UI_CLEAR);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetColorCustomize(int color, int type, int value, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d", PQ_SET_COLOR_CUSTOMIZE, color, type, value, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

vpp_single_color_param_cm_t PqClient::GetColorCustomize(int color)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d.%d", PQ_GET_COLOR_CUSTOMIZE, color);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    vpp_single_color_param_cm_t color_param;
    color_param.sat = atoi(mRet[0].c_str());
    color_param.hue = atoi(mRet[1].c_str());
    color_param.luma = atoi(mRet[2].c_str());

    return color_param;
}

int PqClient::SetColorCustomizeBy3DLut(int color, int type, int value, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d", PQ_SET_COLOR_CUSTOMIZE_3DLUT, color, type, value, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

vpp_single_color_param_3dlut_t PqClient::GetColorCustomizeBy3DLut(int color)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d.%d", PQ_GET_COLOR_CUSTOMIZE_3DLUT, color);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    vpp_single_color_param_3dlut_t color_param;
    color_param.red = atoi(mRet[0].c_str());
    color_param.green = atoi(mRet[1].c_str());
    color_param.blue = atoi(mRet[2].c_str());

    return color_param;
}

int PqClient::ResetColorCustomize(int mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d", PQ_RESET_COLOR_CUSTOMIZE, mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::SetWhitebalanceGamma(int channel, int point, int offset)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d", PQ_SET_WB_GAMMA_DATA, channel, point, offset);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetWhitebalanceGamma(int channel, int point)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d.%d.%d", PQ_GET_WB_GAMMA_DATA, channel, point);
    SendMethodCall(buf);

    int ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::SetSuperResolution(int mode, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_SET_SUPERRESOLUTION, mode, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::GetSuperResolution(void)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d", PQ_GET_WB_GAMMA_DATA);
    SendMethodCall(buf);

    int ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

//PQ Factory cmd
int PqClient::FactoryResetPQMode(void)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d", PQ_FACTORY_RESET_PICTURE_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryResetColorTemp(void)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d", PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryWhiteBalanceSetColorTemperature(int sourceInput, int sigFmt, int transFmt, int colorTemperatureValue, int isSave)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d", PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE, colorTemperatureValue, isSave);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryWhiteBalanceGetColorTemperature()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d", PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetPQMode_Brightness(int sourceInput, int sigFmt, int transFmt, int pq_mode, int brightness)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_BRIGHTNESS, sourceInput, sigFmt, transFmt, pq_mode, brightness);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetPQMode_Brightness(int sourceInput, int sigFmt, int transFmt, int pq_mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_BRIGHTNESS, sourceInput, sigFmt, transFmt, pq_mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetPQMode_Contrast(int sourceInput, int sigFmt, int transFmt, int pq_mode, int contrast)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_CONTRAST, sourceInput, sigFmt, transFmt, pq_mode, contrast);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetPQMode_Contrast(int sourceInput, int sigFmt, int transFmt, int pq_mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_CONTRAST, sourceInput, sigFmt, transFmt, pq_mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetPQMode_Saturation(int sourceInput, int sigFmt, int transFmt, int pq_mode, int saturation)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_SATUATION, sourceInput, sigFmt, transFmt, pq_mode, saturation);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetPQMode_Saturation(int sourceInput, int sigFmt, int transFmt, int pq_mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_SATUATION, sourceInput, sigFmt, transFmt, pq_mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetPQMode_Hue(int sourceInput, int sigFmt, int transFmt, int pq_mode, int hue)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_HUE, sourceInput, sigFmt, transFmt, pq_mode, hue);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetPQMode_Hue(int sourceInput, int sigFmt, int transFmt, int pq_mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_HUE, sourceInput, sigFmt, transFmt, pq_mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetPQMode_Sharpness(int sourceInput, int sigFmt, int transFmt, int pq_mode, int sharpness)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_SHARPNESS, sourceInput, sigFmt, transFmt, pq_mode, sharpness);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetPQMode_Sharpness(int sourceInput, int sigFmt, int transFmt, int pq_mode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_SHARPNESS, sourceInput, sigFmt, transFmt, pq_mode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetOverscanParams(int sourceInput, int sigFmt, int transFmt, int dmode, tvin_cutwin_t cutwin_t)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[256] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_OVERSCAN, sourceInput, sigFmt, transFmt, dmode, cutwin_t.hs, cutwin_t.he, cutwin_t.vs, cutwin_t.ve);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

tvin_cutwin_t PqClient::FactoryGetOverscanParams(int sourceInput, int sigFmt, int transFmt, int dmode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[256] = {0};

    sprintf(buf, "pq.get.%d.%d.%d.%d.%d", PQ_FACTORY_GET_OVERSCAN, sourceInput, sigFmt, transFmt, dmode);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    tvin_cutwin_t cutwin_t;
    cutwin_t.hs = atoi(mRet[0].c_str());
    cutwin_t.he = atoi(mRet[1].c_str());
    cutwin_t.vs = atoi(mRet[2].c_str());
    cutwin_t.ve = atoi(mRet[3].c_str());

    //LOGD("%s %d %d %d %d\n", __FUNCTION__, cutwin_t.hs, cutwin_t.he, cutwin_t.vs, cutwin_t.ve);

    return cutwin_t;
}

int PqClient::FactorySetWhiteBalanceRedGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_RED_GAIN, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceRedGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_RED_GAIN, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceGreenGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_GREEN_GAIN, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceGreenGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_GREEN_GAIN, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceBlueGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_BLUE_GAIN, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceBlueGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_BLUE_GAIN, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceRedPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_RED_OFFSET, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceRedPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_RED_OFFSET, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceGreenPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_GREEN_OFFSET, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceGreenPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_GREEN_OFFSET, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetWhiteBalanceBluePostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.set.%d.%d.%d.%d", PQ_FACTORY_SET_WB_BLUE_OFFSET, sourceInput, colortemptureMode, value);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetWhiteBalanceBluePostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d", PQ_FACTORY_GET_WB_BLUE_OFFSET, sourceInput, colortemptureMode);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
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

int PqClient::FactoryGetSharpnessParams(int sourceInput, int sigFmt, int transFmt, int SDHD,int param_type)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d.%d.%d.%d", PQ_FACTORY_GET_Sharpness_Params, sourceInput,sigFmt,transFmt, SDHD,param_type);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactorySetSharpnessParams(int sourceInput, int sigFmt, int transFmt, int SDHD,int param_type,int val)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pqFactory.get.%d.%d.%d.%d.%d.%d.%d", PQ_FACTORY_GET_Sharpness_Params, sourceInput,sigFmt,transFmt, SDHD,param_type,val);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}
int PqClient::FactorySetHdmiColorRangeMode(int pq_type, int isEnable)
{
    LOGD("%s\n", __FUNCTION__);
    char buf[32] = {0};
    int  ret     = -1;
    sprintf(buf, "pq.set.%d.%d", pq_type, isEnable);
    SendMethodCall(buf);
    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

int PqClient::FactoryGetHdmiColorRangeMode(int pq_type)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    sprintf(buf, "pq.get.%d", pq_type);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);
    int isEnable = atoi(mRet[0].c_str());
    LOGE("PqClient: [isEnable:%d].\n", isEnable);

    return isEnable;
}

//ddr
int PqClient::FactorySetDDRSSC(int step)
{
    Parcel send, reply;

    send.writeInt32(step);

    if (mpqServicebinder->transact(CMD_PQ_SET_DDR_SSC, send, &reply) != 0) {
        LOGD("%s CMD_PQ_SET_DDR_SSC step = %d fail\n", __FUNCTION__, step);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactoryGetDDRSSC(void)
{
    Parcel send, reply;

    if (mpqServicebinder->transact(CMD_PQ_GET_DDR_SSC, send, &reply) != 0) {
        LOGD("%s CMD_PQ_GET_DDR_SSC fail\n", __FUNCTION__);
        return -1;
    }

    return reply.readInt32();
}

//lcd
int PqClient::FactorySetLVDSSSCLevel(int level)
{
    Parcel send, reply;

    send.writeInt32(level);

    if (mpqServicebinder->transact(CMD_PQ_SET_LVDS_SSC_LEVEL, send, &reply) != 0) {
        LOGD("%s CMD_PQ_SET_LVDS_SSC_LEVEL level = %d fail\n", __FUNCTION__, level);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactoryGetLVDSSSCLevel(void)
{
    Parcel send, reply;

    if (mpqServicebinder->transact(CMD_PQ_GET_LVDS_SSC_LEVEL, send, &reply) != 0) {
        LOGD("%s CMD_PQ_GET_LVDS_SSC_LEVEL fail\n", __FUNCTION__);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactorySetLVDSSSCFrep(int step)
{
    Parcel send, reply;

    send.writeInt32(step);

    if (mpqServicebinder->transact(CMD_PQ_SET_LVDS_SSC_FREP, send, &reply) != 0) {
        LOGD("%s CMD_PQ_SET_LVDS_SSC_FREP step = %d fail\n", __FUNCTION__, step);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactoryGetLVDSSSCFrep(void)
{
    Parcel send, reply;

    if (mpqServicebinder->transact(CMD_PQ_GET_LVDS_SSC_FREP, send, &reply) != 0) {
        LOGD("%s CMD_PQ_GET_LVDS_SSC_FREP fail\n", __FUNCTION__);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactorySetLVDSSSCMode(int mode)
{
    Parcel send, reply;

    send.writeInt32(mode);

    if (mpqServicebinder->transact(CMD_PQ_SET_LVDS_SSC_MODE, send, &reply) != 0) {
        LOGD("%s CMD_PQ_SET_LVDS_SSC_MODE mode = %d fail\n", __FUNCTION__, mode);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactoryGetLVDSSSCMode(void)
{
    Parcel send, reply;

    if (mpqServicebinder->transact(CMD_PQ_GET_LVDS_SSC_MODE, send, &reply) != 0) {
        LOGD("%s CMD_PQ_GET_LVDS_SSC_MODE fail\n", __FUNCTION__);
        return -1;
    }

    return reply.readInt32();
}

int PqClient::FactorySetWhitebalanceGamma(int level, int channel, int point, int offset)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d", PQ_FACTORY_SET_WB_GAMMA_DATA, level, channel, point, offset);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d\n", ret);

    return ret;
}

int PqClient::FactoryGetWhitebalanceGamma(int level, int channel, int point)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d.%d.%d.%d", PQ_FACTORY_GET_WB_GAMMA_DATA, level, channel, point);
    SendMethodCall(buf);

    int ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

status_t PqClient::onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags) {
    LOGD("PqClient get tanscode: %u\n", code);
    switch (code) {
        case CMD_HDR_DT_CB: {
            GetHdrTypeFromPqserver(&data);
            break;
        }
        case CMD_START:
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return (0);
}

//for callback method
int PqClient::RegisterObserverToPqClient(PqClientIObserver *observer)
{
    LOGD("%s the main process pid %p\n", __FUNCTION__, getpid());

    //send client binder proxy object to server
    if (mpqServicebinder != 0) {
        Parcel send, reply;
        send.writeStrongBinder(sp<IBinder>(this));
        mpqServicebinder->transact(CMD_SET_PQ_CB, send, &reply);
        mpqServicebinderId = reply.readInt32();
        LOGD("%s mpqServicebinderId %d\n", __FUNCTION__, mpqServicebinderId);
    }

    //register upper client observer
    if (observer != nullptr) {
        LOGD("%s observer is %p\n", __FUNCTION__, observer);
        int cookie = -1;
        int clientSize = mPqClientObserver.size();
        LOGD("%s clientSize is %d\n", __FUNCTION__, clientSize);

        for (int i = 0; i < clientSize; i++) {
            if (mPqClientObserver[i] == NULL) {
                cookie = i;
                mPqClientObserver[i] = observer;
                LOGD("%s mPqClientObserver[%d] %p\n", __FUNCTION__, i, mPqClientObserver[i]);
                break;
            } else {
                LOGD("%s mPqClientObserver[%d] has been register\n", __FUNCTION__, i);
            }
        }

        if (cookie < 0) {
            cookie = clientSize;
            mPqClientObserver[clientSize] = observer;
            LOGD("%s mPqClientObserver[clientSize] %p\n", __FUNCTION__, mPqClientObserver[clientSize]);
        }
    } else {
        LOGE("%s observer is NULL\n", __FUNCTION__);
    }

    return 0;
}

int PqClient::TransactCbData(CPqClientCb &cb_data)
{
    int clientSize = mPqClientObserver.size();
    LOGD("%s now has %d pqclient\n", __FUNCTION__, clientSize);

    int i = 0;
    for (i = 0; i < clientSize; i++) {
        if (mPqClientObserver[i] != NULL) {
            mPqClientObserver[i]->GetPqCbData(cb_data);
            LOGD("%s mPqClientObserver[%d] %p\n", __FUNCTION__, i, mPqClientObserver[i]);
        } else {
            LOGD("%s mPqClientObserver[%d] is NULL\n", __FUNCTION__, i, mPqClientObserver[i]);
        }
    }

    LOGD("%s send event for %d count PqClientObserver!\n", __FUNCTION__, i);
    return 0;
}

int PqClient::GetHdrTypeFromPqserver(const void* param)
{
    LOGD("%s the main process pid %p\n", __FUNCTION__, getpid());
    Parcel *parcel = (Parcel *) param;

    PqClientCb::HdrTypeCb cb_data;
    cb_data.mHdrType = parcel->readInt32();
    LOGD("%s cb_data.mHdrType %d\n", __FUNCTION__, cb_data.mHdrType);

    mInstance->TransactCbData(cb_data);

    return 0;
}


