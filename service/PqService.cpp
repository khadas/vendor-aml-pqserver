/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "PqService"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "PqService.h"
#include "PQType.h"
#include "CPQLog.h"
#include "CPQControl.h"
#include "pqcmd.h"

#ifdef __cplusplus
extern "C" {
#endif

PqService *mInstance = NULL;
PqService *PqService::GetInstance() {
    if (mInstance == NULL) {
        mInstance = new PqService();
    }

    return mInstance;
}

PqService::PqService() {
    init_pq_logging();
    mpPQcontrol = CPQControl::GetInstance();
    mpPQcontrol->CPQControlInit();
}

PqService::~PqService() {
    if (mpPQcontrol != NULL) {
        mpPQcontrol->CPQControlUnInit();
        delete mpPQcontrol;
        mpPQcontrol = NULL;
    }
}

int PqService::PqServiceHandleMessage()
{
    while (1) {
        usleep(500*1000);
    }
    return 0;
}

int PqService::SplitCommand(const char *commandData)
{
    char cmdbuff[1024] = {'\0'};
    char *token;
    int   cmd_size = 0;
    const char *delimitation = ".";

    if (strlen(commandData) <= sizeof(cmdbuff)/sizeof(char)) {
        strcpy(cmdbuff, commandData);
    }

    /* get first str*/
    token = strtok(cmdbuff, delimitation);

    /* continue get str*/
    while (token != NULL)
    {
        mPqCommand[cmd_size].assign(token);

        LOGD("%s mPqCommand[%d]:%s\n", token, cmd_size, mPqCommand[cmd_size].c_str());
        cmd_size++;
        token = strtok(NULL, delimitation);
    }

    LOGD("%s: cmd_size = %d\n", __FUNCTION__, cmd_size);

    return cmd_size;
}

int PqService::SetCmd(pq_moudle_param_t param)
{
    int ret = 0;
    int moudleId = param.moudleId;
    bool enable = false;

    if (((moudleId >= PQ_MOUDLE_CMD_START) && (moudleId <= PQ_MOUDLE_CMD_MAX))
        || ((moudleId >= PQ_FACTORY_CMD_START) || (moudleId <= PQ_FACTORY_CMD_MAX))) {
        int paramData[32] = {0};
        int i = 0;
        source_input_param_t source_input_param;
        tvin_cutwin_t overscanParam;
        memset(&source_input_param, 0, sizeof(source_input_param_t));
        memset(&overscanParam, 0, sizeof(tvin_cutwin_t));

        for (i = 0; i < param.paramLength; i++) {
            paramData[i] = param.paramBuf[i];
        }

        switch (moudleId) {
        case PQ_SET_PICTURE_MODE:
            ret = mpPQcontrol->SetPQMode(paramData[0], paramData[1]);
            break;
        case PQ_SET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->SetColorTemperature(paramData[0], paramData[1], (rgb_ogo_type_t)paramData[2], paramData[3]);
            break;
        case PQ_SET_BRIGHTNESS:
            ret = mpPQcontrol->SetBrightness(paramData[0], paramData[1]);
            break;
        case PQ_SET_CONTRAST:
            ret = mpPQcontrol->SetContrast(paramData[0], paramData[1]);
            break;
        case PQ_SET_SATUATION:
            ret = mpPQcontrol->SetSaturation(paramData[0], paramData[1]);
            break;
        case PQ_SET_HUE:
            ret = mpPQcontrol->SetHue(paramData[0], paramData[1]);
            break;
        case PQ_SET_SHARPNESS:
            ret = mpPQcontrol->SetSharpness(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_SET_NOISE_REDUCTION_MODE:
            ret = mpPQcontrol->SetNoiseReductionMode(paramData[0], paramData[1]);
            break;
        case PQ_SET_EYE_PROTECTION_MODE:
            ret = mpPQcontrol->SetEyeProtectionMode((tv_source_input_t)paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_SET_GAMMA:
            ret = mpPQcontrol->SetGammaValue((vpp_gamma_curve_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_DISPLAY_MODE:
            ret = mpPQcontrol->SetDisplayMode((vpp_display_mode_t)paramData[1], paramData[2]);
            break;
        case PQ_SET_BACKLIGHT:
            ret = mpPQcontrol->SetBacklight(paramData[0], paramData[1]);
            break;
        case PQ_SET_DYNAMICBACKLIGHT:
            ret = mpPQcontrol->SetDynamicBacklight((Dynamic_backlight_status_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_LOCALCONTRAST:
            ret = mpPQcontrol->SetLocalContrastMode((local_contrast_mode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_CM:
            ret = mpPQcontrol->SetColorBaseMode((vpp_color_basemode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_SOURCE_CHANNEL:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->SetCurrentSource((tv_source_input_t)paramData[0]);
            break;
        case PQ_SET_COLORGAMUT:
            ret = mpPQcontrol->SetColorGamutMode((vpp_colorgamut_mode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_DYNAMICCONTRAST:
            ret = mpPQcontrol->SetDnlpMode((Dynamic_contrast_mode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_RECOVERYPQ:
            ret = mpPQcontrol->SSMRecovery();
            break;
        case PQ_SET_MEMCMODE:
            ret = mpPQcontrol->SetMemcMode(paramData[0], paramData[1]);
            break;
        case PQ_SET_MEMC_DEBLUR:
            ret = mpPQcontrol->SetMemcDeBlurLevel(paramData[0], paramData[1]);
            break;
        case PQ_SET_MEMC_DEJUDDER:
            ret = mpPQcontrol->SetMemcDeJudderLevel(paramData[0], paramData[1]);
            break;
        case PQ_SET_DECONTOUR:
            ret = mpPQcontrol->SetSmoothPlusMode(paramData[0], paramData[1]);
            break;
        case PQ_SET_DEBLOCK:
            ret = mpPQcontrol->SetDeblockMode((vpp_deblock_mode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_DEMOSQUITO:
            ret = mpPQcontrol->SetDemoSquitoMode((vpp_DemoSquito_mode_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_BLACKSTRETCH:
            ret = mpPQcontrol->SetBlackStretch(paramData[0], paramData[1]);
            break;
        case PQ_SET_AIPQ:
            ret = mpPQcontrol->SetAipqEnable(paramData[0]);
            break;
        case PQ_SET_AISR:
            enable = (paramData[0] == 0) ? false : true;
            ret = mpPQcontrol->SetAiSrEnable(enable);
            break;
        case PQ_SET_LDIM:
            ret = mpPQcontrol->SetLocalDimming(paramData[0], paramData[1]);
            break;
        case PQ_SET_MPEGNR:
            ret = mpPQcontrol->SetMpegNr((vpp_pq_level_t)paramData[0], paramData[1]);
            break;
        case PQ_SET_PICTURE_UI_CLEAR:
            mpPQcontrol->resetCurSrcPqUiSetting();
            break;

        //Factory cmd
        case PQ_FACTORY_RESET_PICTURE_MODE:
            ret = mpPQcontrol->FactoryResetPQMode();
            break;
        case PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->FactoryResetColorTemp();
            break;
        case PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->SetColorTemperature(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_SET_BRIGHTNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Brightness(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_CONTRAST:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Contrast(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_SATUATION:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Saturation(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_HUE:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Hue(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_SHARPNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Sharpness(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_OVERSCAN:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            overscanParam.hs = (unsigned short)paramData[4];
            overscanParam.he = (unsigned short)paramData[5];
            overscanParam.vs = (unsigned short)paramData[6];
            overscanParam.ve = (unsigned short)paramData[7];

            ret = mpPQcontrol->FactorySetOverscanParam(source_input_param, (vpp_display_mode_t)paramData[3], overscanParam);
            break;
        case PQ_FACTORY_SET_WB_RED_GAIN:
            ret = mpPQcontrol->FactorySetColorTemp_Rgain(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_WB_GREEN_GAIN:
            ret = mpPQcontrol->FactorySetColorTemp_Ggain(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_WB_BLUE_GAIN:
            ret = mpPQcontrol->FactorySetColorTemp_Bgain(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_WB_RED_OFFSET:
            ret = mpPQcontrol->FactorySetColorTemp_Roffset(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_WB_GREEN_OFFSET:
            ret = mpPQcontrol->FactorySetColorTemp_Goffset(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_WB_BLUE_OFFSET:
            ret = mpPQcontrol->FactorySetColorTemp_Boffset(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_RGB_PATTERN:
            ret = mpPQcontrol->SetRGBPattern(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_FACTORY_SET_GRAY_PATTERN:
            ret = mpPQcontrol->SetGrayPattern(paramData[0]);
            break;

        default:
            break;
        }
    } else {
        LOGE("%s: invalid PQ cmd: %d\n", __FUNCTION__, moudleId);
        ret = -1;
    }

    return ret;
}

char* PqService::GetCmd(pq_moudle_param_t param)
{
    int ret = 0;
    int moudleId = param.moudleId;
    bool enable = false;

    if (((moudleId >= PQ_MOUDLE_CMD_START) && (moudleId <= PQ_MOUDLE_CMD_MAX))
        || ((moudleId >= PQ_FACTORY_CMD_START) || (moudleId <= PQ_FACTORY_CMD_MAX))) {
        int paramData[32] = {0};
        int i = 0;
        source_input_param_t source_input_param;
        tvin_cutwin_t overscanParam;
        tvpq_rgb_ogo_t rgbogo;
        memset(&source_input_param, 0, sizeof(source_input_param_t));
        memset(&overscanParam, 0, sizeof(tvin_cutwin_t));
        memset(&rgbogo, 0, sizeof(tvpq_rgb_ogo_t));

        for (i = 0; i < param.paramLength; i++) {
            paramData[i] = param.paramBuf[i];
        }

        switch (moudleId) {
        case PQ_GET_PICTURE_MODE:
            ret = mpPQcontrol->GetPQMode();
            break;
        case PQ_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->GetColorTemperature();
            break;
        case PQ_GET_BRIGHTNESS:
            ret = mpPQcontrol->GetBrightness();
            break;
        case PQ_GET_CONTRAST:
            ret = mpPQcontrol->GetContrast();
            break;
        case PQ_GET_SATUATION:
            ret = mpPQcontrol->GetSaturation();
            break;
        case PQ_GET_HUE:
            ret = mpPQcontrol->GetHue();
            break;
        case PQ_GET_SHARPNESS:
            ret = mpPQcontrol->GetSharpness();
            break;
        case PQ_GET_NOISE_REDUCTION_MODE:
            ret = mpPQcontrol->GetNoiseReductionMode();
            break;
        case PQ_GET_EYE_PROTECTION_MODE:
            ret = mpPQcontrol->GetEyeProtectionMode((tv_source_input_t)paramData[0]);
            break;
        case PQ_GET_GAMMA:
            ret = mpPQcontrol->GetGammaValue();
            break;
        case PQ_GET_DISPLAY_MODE:
            ret = mpPQcontrol->GetDisplayMode();
            break;
        case PQ_GET_BACKLIGHT:
            ret = mpPQcontrol->GetBacklight();
            break;
        case PQ_GET_DYNAMICBACKLIGHT:
            ret = mpPQcontrol->GetDynamicBacklight();
            break;
        case PQ_GET_LOCALCONTRAST:
            ret = mpPQcontrol->GetLocalContrastMode();
            break;
        case PQ_GET_CM:
            ret = mpPQcontrol->GetColorBaseMode();
            break;
        case PQ_GET_SOURCE_CHANNEL:
            source_input_param = mpPQcontrol->GetCurrentSourceInputInfo();
            sprintf(mRetBuf, "%d.%d.%d", source_input_param.source_input, source_input_param.sig_fmt, source_input_param.trans_fmt);
            break;
        case PQ_GET_COLORGAMUT:
            ret = mpPQcontrol->GetColorGamutMode();
            break;
        case PQ_GET_SOURCE_HDR_TYPE:
            ret = mpPQcontrol->GetSourceHDRType();
            break;
        case PQ_GET_DYNAMICCONTRAST:
            ret = mpPQcontrol->GetDnlpMode();
            break;
        case PQ_GET_HAS_MEMC:
            enable = mpPQcontrol->hasMemcFunc();
            ret = (enable == true) ? 1: 0;
            break;
        case PQ_GET_MEMCMODE:
            ret = mpPQcontrol->GetMemcMode();
            break;
        case PQ_GET_MEMC_DEBLUR:
            ret = mpPQcontrol->GetMemcDeBlurLevel();
            break;
        case PQ_GET_MEMC_DEJUDDER:
            ret = mpPQcontrol->GetMemcDeJudderLevel();
            break;
        case PQ_GET_DECONTOUR:
            ret = mpPQcontrol->GetSmoothPlusMode();
            break;
        case PQ_GET_DEBLOCK:
            ret = mpPQcontrol->GetDeblockMode();
            break;
        case PQ_GET_DEMOSQUITO:
            ret = mpPQcontrol->GetDemoSquitoMode();
            break;
        case PQ_GET_BLACKSTRETCH:
            ret = mpPQcontrol->GetBlackStretch();
            break;
        case PQ_GET_BLUESTRETCH:
            ret = mpPQcontrol->GetBlueStretch();
            break;
        case PQ_GET_HAS_AIPQ:
            ret = 0;
            break;
        case PQ_GET_AIPQ:
            ret = mpPQcontrol->GetAipqEnable();
            break;
        case PQ_GET_HAS_AISR:
            enable = mpPQcontrol->hasAisrFunc();
            ret = (enable == true) ? 1 : 0;
            break;
        case PQ_GET_AISR:
            ret = mpPQcontrol->GetAiSrEnable();
            break;
        case PQ_GET_LDIM:
            ret = mpPQcontrol->GetLocalDimming();
            break;
        case PQ_GET_MPEGNR:
            ret = mpPQcontrol->GetMpegNr();
            break;
        case PQ_GET_COLORTEMP_USER_PARAM:
            rgbogo = mpPQcontrol->GetColorTemperatureUserParam();
            sprintf(mRetBuf, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", rgbogo.en, rgbogo.r_pre_offset, rgbogo.g_pre_offset, rgbogo.b_pre_offset,
                    rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain, rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset);
            break;

        //Factory cmd
        case PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->GetColorTemperature();
            break;
        case PQ_FACTORY_GET_BRIGHTNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Brightness(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_CONTRAST:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Contrast(source_input_param, paramData[3]);
            break;

        case PQ_FACTORY_GET_SATUATION:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Saturation(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_HUE:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Hue(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_SHARPNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Sharpness(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_OVERSCAN:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (tvin_sig_fmt_t)paramData[1];
            source_input_param.trans_fmt    = (tvin_trans_fmt_t)paramData[2];

            overscanParam = mpPQcontrol->FactoryGetOverscanParam(source_input_param, (vpp_display_mode_t)paramData[3]);
            sprintf(mRetBuf, "%d.%d.%d.%d", overscanParam.hs, overscanParam.he, overscanParam.vs, overscanParam.ve);
            break;
        case PQ_FACTORY_GET_WB_RED_GAIN:
            ret = mpPQcontrol->FactoryGetColorTemp_Rgain(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_WB_GREEN_GAIN:
            ret = mpPQcontrol->FactoryGetColorTemp_Ggain(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_WB_BLUE_GAIN:
            ret = mpPQcontrol->FactoryGetColorTemp_Bgain(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_WB_RED_OFFSET:
            ret = mpPQcontrol->FactoryGetColorTemp_Roffset(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_WB_GREEN_OFFSET:
            ret = mpPQcontrol->FactoryGetColorTemp_Goffset(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_WB_BLUE_OFFSET:
            ret = mpPQcontrol->FactoryGetColorTemp_Boffset(paramData[0], paramData[1]);
            break;
        case PQ_FACTORY_GET_RGB_PATTERN:
            ret = mpPQcontrol->GetRGBPattern();
            break;
        case PQ_FACTORY_GET_GRAY_PATTERN:
            ret = mpPQcontrol->GetGrayPattern();
            break;

        default:
            break;
        }
    } else {
        LOGE("%s: invalid PQ cmd: %d!\n", __FUNCTION__, moudleId);
        ret = -1;
    }

    if ((moudleId != PQ_GET_SOURCE_CHANNEL)
        && (moudleId != PQ_FACTORY_GET_OVERSCAN)
        && (moudleId != PQ_GET_COLORTEMP_USER_PARAM)) {
        sprintf(mRetBuf, "%d", ret);
    }

    return mRetBuf;
}

void PqService::ParserPqCommand(const char *commandData)
{
    LOGD("%s: cmd data is %s\n", __FUNCTION__, commandData);

    int  cmd_size = 0;
    int  ret      = 0;
    int  i        = 0;
    char* ret_char;

    //split command
    cmd_size = SplitCommand(commandData);

    //parse command
    pq_moudle_param_t pqParam;
    memset(&pqParam, 0, sizeof(pq_moudle_param_t));
    pqParam.moudleId = atoi(mPqCommand[2].c_str());
    pqParam.paramLength = cmd_size - 3;

    for (i = 0; i < pqParam.paramLength; i++)
    {
        pqParam.paramBuf[i] = atoi(mPqCommand[i + 3].c_str());
    }

    if ((strcmp(mPqCommand[0].c_str(), "pq") == 0)
        || (strcmp(mPqCommand[0].c_str(), "pqFactory") == 0)) {
        if (strcmp(mPqCommand[1].c_str(), "set") == 0) {
            ret = SetCmd(pqParam);
            sprintf(mRetBuf, "%d", ret);
        } else if (strcmp(mPqCommand[1].c_str(), "get") == 0) {
            ret_char = GetCmd(pqParam);
            if (ret_char != NULL) {
                LOGD("%s: ret_char %s\n", __FUNCTION__, ret_char);
            }
            //sprintf(mRetBuf, "%s", ret_char);
        } else {
            LOGD("%s: invalid cmd\n", __FUNCTION__);
            ret = 0;
            sprintf(mRetBuf, "%d", ret);
        }
    } else {
        LOGD("%s: invalie cmdType\n", __FUNCTION__);
    }

    LOGD("%s: mRetBuf %s\n", __FUNCTION__, mRetBuf);
}

status_t PqService::onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags) {
    LOGD("%s: code is %u\n", __FUNCTION__, code);

    switch (code) {
        case CMD_PQ_ACTION: {
            const char* command = data.readCString();
            ParserPqCommand(command);
            reply->writeCString(mRetBuf);
            break;
        }
        case CMD_PQ_SET_DDR_SSC: {
            int step = data.readInt32();
            reply->writeInt32(mpPQcontrol->FactorySetDDRSSC(step));
            break;
        }
        case CMD_PQ_GET_DDR_SSC: {
            reply->writeInt32(mpPQcontrol->FactoryGetDDRSSC());
            break;
        }
        case CMD_PQ_SET_LVDS_SSC_LEVEL: {
            int level = data.readInt32();
            reply->writeInt32(mpPQcontrol->FactorySetLVDSSSCLevel(level));
            break;
        }
        case CMD_PQ_GET_LVDS_SSC_LEVEL: {
            reply->writeInt32(mpPQcontrol->FactoryGetLVDSSSCLevel());
            break;
        }
        case CMD_PQ_SET_LVDS_SSC_FREP: {
            int step = data.readInt32();
            reply->writeInt32(mpPQcontrol->FactorySetLVDSSSCFrep(step));
            break;
        }
        case CMD_PQ_GET_LVDS_SSC_FREP: {
            reply->writeInt32(mpPQcontrol->FactoryGetLVDSSSCFrep());
            break;
        }
        case CMD_PQ_SET_LVDS_SSC_MODE: {
            int mode = data.readInt32();
            reply->writeInt32(mpPQcontrol->FactorySetLVDSSSCMode(mode));
            break;
        }
        case CMD_PQ_GET_LVDS_SSC_MODE: {
            reply->writeInt32(mpPQcontrol->FactoryGetLVDSSSCMode());
            break;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return (0);
}
#ifdef __cplusplus
}
#endif
