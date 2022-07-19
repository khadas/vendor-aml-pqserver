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
    Parcel send, reply;
    sp<IServiceManager> serviceManager = defaultServiceManager();
    do {
        mpqServicebinder = serviceManager->getService(String16("pqservice"));
        if (mpqServicebinder != 0) break;
        LOGD("PqClient: Waiting pqservice published.\n");
        usleep(500000);
    } while(true);
    LOGD("Connected to pqservice.\n");
}

PqClient::~PqClient() {
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
            strcpy(mRetBuf, tmp);
        } else {
            const char* tmp;
            tmp = reply.readCString();
            strcpy(mRetBuf, tmp);
        }
    }

    LOGE("PqClient: mRetBuf %s\n", mRetBuf);
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

int PqClient::SetColorTemperature(int colorTemperatureValue, int isSave, int rgb_type, int value)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d", PQ_SET_COLOR_TEMPERATURE_MODE, colorTemperatureValue, isSave, rgb_type, value);
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

tvpq_rgb_ogo_t PqClient::GetColorTemperatureUserParam()
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d", PQ_GET_COLORTEMP_USER_PARAM);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    tvpq_rgb_ogo_t rgbogo;
    rgbogo.en = atoi(mRet[0].c_str());
    rgbogo.r_pre_offset = atoi(mRet[1].c_str());
    rgbogo.g_pre_offset = atoi(mRet[2].c_str());
    rgbogo.b_pre_offset = atoi(mRet[3].c_str());
    rgbogo.r_gain = atoi(mRet[4].c_str());
    rgbogo.g_gain = atoi(mRet[5].c_str());
    rgbogo.b_gain = atoi(mRet[6].c_str());
    rgbogo.r_post_offset = atoi(mRet[7].c_str());
    rgbogo.g_post_offset = atoi(mRet[8].c_str());
    rgbogo.b_post_offset = atoi(mRet[9].c_str());

    return rgbogo;
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

int PqClient::FactorySetOverscanParams(int sourceInput, int sigFmt, int transFmt, tvin_cutwin_t cutwin_t)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};
    int  ret     = -1;

    sprintf(buf, "pq.set.%d.%d.%d.%d.%d.%d.%d.%d.%d", PQ_FACTORY_SET_OVERSCAN, sourceInput, sigFmt, transFmt, cutwin_t.he, cutwin_t.hs, cutwin_t.ve, cutwin_t.vs);
    SendMethodCall(buf);

    ret = atoi(mRetBuf);
    LOGE("PqClient: ret %d.\n", ret);

    return ret;
}

tvin_cutwin_t PqClient::FactoryGetOverscanParams(int sourceInput, int sigFmt, int transFmt)
{
    LOGD("%s\n", __FUNCTION__);

    char buf[32] = {0};

    sprintf(buf, "pq.get.%d.%d.%d.%d", PQ_FACTORY_GET_OVERSCAN, sourceInput, sigFmt, transFmt);
    SendMethodCall(buf);
    SplitRetBuf(mRetBuf);

    tvin_cutwin_t cutwin_t;
    cutwin_t.he = atoi(mRet[0].c_str());
    cutwin_t.hs = atoi(mRet[1].c_str());
    cutwin_t.ve = atoi(mRet[2].c_str());
    cutwin_t.vs = atoi(mRet[3].c_str());

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
