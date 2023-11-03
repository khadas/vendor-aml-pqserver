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
    mpPQcontrol->setObserver(this);
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

        //LOGD("%s mPqCommand[%d]:%s\n", token, cmd_size, mPqCommand[cmd_size].c_str());
        cmd_size++;
        token = strtok(NULL, delimitation);
    }

    //LOGD("%s: cmd_size = %d\n", __FUNCTION__, cmd_size);

    return cmd_size;
}

int PqService::SetCmd(pq_moudle_param_t param)
{
    int ret = 0;
    int moduleId = param.moduleId;
    bool enable = false;

    if (((moduleId >= PQ_MODULE_CMD_START) && (moduleId <= PQ_MODULE_CMD_MAX))
        || ((moduleId >= PQ_FACTORY_CMD_START) || (moduleId <= PQ_FACTORY_CMD_MAX))) {
        int paramData[32] = {0};
        int i = 0;
        source_input_param_t source_input_param;
        tvin_cutwin_t overscanParam;
        tcon_rgb_ogo_t tcon_rgbogo;
        memset(&source_input_param, 0, sizeof(source_input_param_t));
        memset(&overscanParam, 0, sizeof(tvin_cutwin_t));
        memset(&tcon_rgbogo, 0, sizeof(tcon_rgb_ogo_t));

        for (i = 0; i < param.paramLength; i++) {
            paramData[i] = param.paramBuf[i];
        }

        switch (moduleId) {
        case PQ_SET_PICTURE_MODE:
            ret = mpPQcontrol->SetPQMode(paramData[0], paramData[1]);
            break;
        case PQ_SET_COLOR_TEMPERATURE_MODE:
            if (paramData[0] == VPP_COLOR_TEMPERATURE_MODE_USER) {
                ret = mpPQcontrol->SetColorTempParams(paramData[0], (rgb_ogo_type_t)paramData[2], paramData[3]);
            } else {
                ret = mpPQcontrol->SetColorTemperature(paramData[0], paramData[1]);
            }
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
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

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
            ret = mpPQcontrol->SetDemoSquitoMode((vpp_demosquito_mode_t)paramData[0], paramData[1]);
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
        case PQ_SET_COLOR_CUSTOMIZE:
            ret = mpPQcontrol->SetColorCustomize((vpp_cms_color_t)paramData[0], (vpp_cms_type_t)paramData[1], paramData[2], paramData[3]);
            break;
        case PQ_SET_COLOR_CUSTOMIZE_3DLUT:
            ret = mpPQcontrol->SetColorCustomizeBy3DLut((vpp_cms_6color_t)paramData[0], (vpp_cms_type_t)paramData[1], paramData[2], paramData[3]);
            break;
        case PQ_RESET_COLOR_CUSTOMIZE:
            ret = mpPQcontrol->ResetColorCustomize((vpp_cms_method_t)paramData[0]);
            break;
        case PQ_SET_WB_GAMMA_DATA:
            ret = mpPQcontrol->SetWhitebalanceGamma(paramData[0], paramData[1], paramData[2]);
            break;
        case PQ_SET_SUPERRESOLUTION:
            ret = mpPQcontrol->SetSuperResolution(paramData[0], paramData[1]);
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
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Brightness(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_CONTRAST:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Contrast(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_SATUATION:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Saturation(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_HUE:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Hue(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_SHARPNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetPQMode_Sharpness(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_OVERSCAN:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

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
        case PQ_FACTORY_SET_NOLINE_PARAMS: {
            noline_params_t noline_params;
            int type;
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            type = (int)paramData[3];
            noline_params.osd0 = (int)paramData[4];
            noline_params.osd25 = (int)paramData[5];
            noline_params.osd50 = (int)paramData[6];
            noline_params.osd75 = (int)paramData[7];
            noline_params.osd100 = (int)paramData[8];
            ret = mpPQcontrol->FactorySetNolineParams(source_input_param, type, noline_params);
        }
            break;
        case PQ_FACTORY_SET_COLOR_PARAMS:
            tcon_rgbogo.en = paramData[1];
            tcon_rgbogo.r_pre_offset = paramData[2];
            tcon_rgbogo.g_pre_offset = paramData[3];
            tcon_rgbogo.b_pre_offset = paramData[4];
            tcon_rgbogo.r_gain = paramData[5];
            tcon_rgbogo.g_gain = paramData[6];
            tcon_rgbogo.b_gain = paramData[7];
            tcon_rgbogo.r_post_offset = paramData[8];
            tcon_rgbogo.g_post_offset = paramData[9];
            tcon_rgbogo.b_post_offset = paramData[10];
            ret = mpPQcontrol->FactorySetRGBGainOffset((vpp_color_temperature_mode_t)paramData[0], tcon_rgbogo);
            break;
        case PQ_FACTORY_SET_DEC_LUMA_PARAMS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactorySetDecodeLumaParams(source_input_param, paramData[3], paramData[4]);
            break;
        case PQ_FACTORY_SET_Sharpness_Params:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactorySetSharpnessParams(source_input_param,(sharpness_timing_t)paramData[3],paramData[4],paramData[5]);
            break;

        case PQ_FACTORY_SET_PQ_ENABLE: {
            ret = mpPQcontrol->SetHdmiColorRangeMode(paramData[0],    paramData[1]);
             break;
            }

        case PQ_FACTORY_SET_WB_GAMMA_DATA:
            ret = mpPQcontrol->FactorySetWhitebalanceGamma(paramData[0], paramData[1], paramData[2], paramData[3]);
            break;
        default:
            break;
        }
    } else {
        LOGE("%s: invalid PQ cmd: %d\n", __FUNCTION__, moduleId);
        ret = -1;
    }

    return ret;
}

char* PqService::GetCmd(pq_moudle_param_t param)
{
    int ret = 0;
    int moduleId = param.moduleId;
    bool enable = false;

    if (((moduleId >= PQ_MODULE_CMD_START) && (moduleId <= PQ_MODULE_CMD_MAX))
        || ((moduleId >= PQ_FACTORY_CMD_START) || (moduleId <= PQ_FACTORY_CMD_MAX))) {
        int paramData[32] = {0};
        int i = 0;
        int mode = 0;
        source_input_param_t source_input_param;
        tvin_cutwin_t overscanParam;
        tvpq_rgb_ogo_t rgbogo;
        tcon_rgb_ogo_t tcon_rgbogo;
        vpp_single_color_param_cm_t cms_cm;
        vpp_single_color_param_3dlut_t cms_3dlut;
        memset(&source_input_param, 0, sizeof(source_input_param_t));
        memset(&overscanParam, 0, sizeof(tvin_cutwin_t));
        memset(&rgbogo, 0, sizeof(tvpq_rgb_ogo_t));
        memset(&tcon_rgbogo, 0, sizeof(tcon_rgb_ogo_t));
        memset(&cms_cm, 0, sizeof(vpp_single_color_param_cm_t));
        memset(&cms_3dlut, 0, sizeof(vpp_single_color_param_3dlut_t));

        for (i = 0; i < param.paramLength; i++) {
            paramData[i] = param.paramBuf[i];
        }

        switch (moduleId) {
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
            mode = mpPQcontrol->GetColorTemperature();
            rgbogo = mpPQcontrol->GetColorTempParams(mode);
            sprintf(mRetBuf, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", rgbogo.en, rgbogo.r_pre_offset, rgbogo.g_pre_offset, rgbogo.b_pre_offset,
                    rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain, rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset);
            break;
        case PQ_GET_COLOR_CUSTOMIZE:
            cms_cm = mpPQcontrol->GetColorCustomize((vpp_cms_color_t)paramData[0]);
            sprintf(mRetBuf, "%d.%d.%d", cms_cm.sat, cms_cm.hue, cms_cm.luma);
            break;
        case PQ_GET_COLOR_CUSTOMIZE_3DLUT:
            cms_3dlut = mpPQcontrol->GetColorCustomizeBy3DLut((vpp_cms_6color_t)paramData[0]);
            sprintf(mRetBuf, "%d.%d.%d", cms_3dlut.red, cms_3dlut.green, cms_3dlut.blue);
            break;
        case PQ_GET_WB_GAMMA_DATA:
            ret = mpPQcontrol->GetWhitebalanceGamma(paramData[0], paramData[1]);
            break;
        case PQ_GET_SUPERRESOLUTION:
            ret = mpPQcontrol->GetSuperResolution();
            break;

        //Factory cmd
        case PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPQcontrol->GetColorTemperature();
            break;
        case PQ_FACTORY_GET_BRIGHTNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Brightness(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_CONTRAST:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Contrast(source_input_param, paramData[3]);
            break;

        case PQ_FACTORY_GET_SATUATION:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Saturation(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_HUE:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Hue(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_SHARPNESS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetPQMode_Sharpness(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_OVERSCAN:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

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
        case PQ_FACTORY_GET_NOLINE_PARAMS: {
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            int type = (enum tvin_trans_fmt)paramData[3];
            noline_params_t noline_params = mpPQcontrol->FactoryGetNolineParams(source_input_param, type);
            sprintf(mRetBuf, "%d.%d.%d.%d.%d",
                    noline_params.osd0,
                    noline_params.osd25,
                    noline_params.osd50,
                    noline_params.osd75,
                    noline_params.osd100);
            return mRetBuf;
        }
        case PQ_FACTORY_GET_COLOR_PARAMS:
            tcon_rgbogo.en = paramData[1];
            tcon_rgbogo.r_pre_offset = paramData[2];
            tcon_rgbogo.g_pre_offset = paramData[3];
            tcon_rgbogo.b_pre_offset = paramData[4];
            tcon_rgbogo.r_gain = paramData[5];
            tcon_rgbogo.g_gain = paramData[6];
            tcon_rgbogo.b_gain = paramData[7];
            tcon_rgbogo.r_post_offset = paramData[8];
            tcon_rgbogo.g_post_offset = paramData[9];
            tcon_rgbogo.b_post_offset = paramData[10];
            ret = mpPQcontrol->FactoryGetRGBGainOffset((vpp_color_temperature_mode_t)paramData[0], &tcon_rgbogo);

            sprintf(mRetBuf, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", ret, tcon_rgbogo.r_pre_offset, tcon_rgbogo.g_pre_offset, tcon_rgbogo.b_pre_offset,
                tcon_rgbogo.r_gain, tcon_rgbogo.g_gain, tcon_rgbogo.b_gain, tcon_rgbogo.r_post_offset, tcon_rgbogo.g_post_offset, tcon_rgbogo.b_post_offset);
            return mRetBuf;
        case PQ_FACTORY_GET_DEC_LUMA_PARAMS:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];

            ret = mpPQcontrol->FactoryGetDecodeLumaParams(source_input_param, paramData[3]);
            break;
        case PQ_FACTORY_GET_Sharpness_Params:
            source_input_param.source_input = (tv_source_input_t)paramData[0];
            source_input_param.sig_fmt      = (enum tvin_sig_fmt_e)paramData[1];
            source_input_param.trans_fmt    = (enum tvin_trans_fmt)paramData[2];
            ret = mpPQcontrol->FactoryGetSharpnessParams(source_input_param,(sharpness_timing_t)paramData[3],paramData[4]);
            break;

        case PQ_FACTORY_GET_PQ_ENABLE: {
            ret = mpPQcontrol->GetHdmiColorRangeMode(paramData[0]);
            break;
            }

        case PQ_FACTORY_GET_WB_GAMMA_DATA:
            ret = mpPQcontrol->FactoryGetWhitebalanceGamma(paramData[0], paramData[1], paramData[2]);
            break;

        default:
            break;
        }
    } else {
        LOGE("%s: invalid PQ cmd: %d!\n", __FUNCTION__, moduleId);
        ret = -1;
    }

    if ((moduleId != PQ_GET_SOURCE_CHANNEL)
        && (moduleId != PQ_FACTORY_GET_OVERSCAN)
        && (moduleId != PQ_GET_COLORTEMP_USER_PARAM)
        && (moduleId != PQ_GET_COLOR_CUSTOMIZE)
        && (moduleId != PQ_GET_COLOR_CUSTOMIZE_3DLUT)) {
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
    pqParam.moduleId = atoi(mPqCommand[2].c_str());
    pqParam.paramLength = cmd_size - 3;

    for (i = 0; i < pqParam.paramLength; i++)
    {
        pqParam.paramBuf[i] = atoi(mPqCommand[i + 3].c_str());
    }

    if ((strcmp(mPqCommand[0].c_str(), "pq") == 0) || (strcmp(mPqCommand[0].c_str(), "pqFactory") == 0)) {
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

    //LOGD("%s: mRetBuf %s\n", __FUNCTION__, mRetBuf);
}

status_t PqService::onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags) {
    //LOGD("%s: code is %u\n", __FUNCTION__, code);

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
        case CMD_SET_PQ_CB: {
            int pqServiceCallBackID = SetClientProxyToServer(data.readStrongBinder());
            reply->writeInt32(pqServiceCallBackID);
            LOGD("%s %s pqServiceCallBackID %d\n", __FUNCTION__, "CMD_SET_PQ_CB", pqServiceCallBackID);
            break;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return (0);
}

//for callback to upper client
int PqService::SetClientProxyToServer(sp<IBinder> callBack)
{
    LOGD("%s the main process pid %p\n", __FUNCTION__, getpid());
    int ret = -1;

    if (callBack != nullptr) {
        LOGD("%s callBack is %p\n", __FUNCTION__, callBack);
        int cookie = -1;
        int clientSize = mPqServiceCallBack.size();
        LOGD("%s clientSize is %d\n", __FUNCTION__, clientSize);

        for (int i = 0; i < clientSize; i++) {
            if (mPqServiceCallBack[i] == NULL) {
                cookie = i;
                mPqServiceCallBack[i] = callBack;
                LOGD("%s mPqServiceCallBack[%d] %p\n", __FUNCTION__, i, mPqServiceCallBack[i]);
                break;
            } else {
                LOGD("%s mPqServiceCallBack[%d] has been register\n", __FUNCTION__, i);
            }
        }

        if (cookie < 0) {
            cookie = clientSize;
            mPqServiceCallBack[clientSize] = callBack;
            LOGD("%s mPqServiceCallBack[clientSize] %p\n", __FUNCTION__, mPqServiceCallBack[clientSize]);
        }
        ret = cookie;
    } else {
        LOGE("%s callBack is NULL\n", __FUNCTION__);
    }

    return ret;
}

void PqService::GetCbDataFromLibpq(CPQControlCb &cb_data)
{
    int cbType = cb_data.getCbType();
    LOGD("%s: cbType: %d\n", __FUNCTION__, cbType);

    switch (cbType) {
        case CPQControlCb::PQ_CB_TYPE_HDRTYPE:
            Sethdrtype(cb_data);
            break;
        default :
            LOGE("%s invalie callback type\n", __FUNCTION__);
            break;
    }

    return;
}

void PqService::Sethdrtype(CPQControlCb &cb_data)
{
    LOGD("%s the main process's pid %p\n", __FUNCTION__, getpid());
    Parcel send, reply;

    PQControlCb::HdrTypeCb *hdrTypeCb = (PQControlCb::HdrTypeCb *)(&cb_data);
    int clientSize = mPqServiceCallBack.size();
    LOGD("%s now has %d pqclient\n", __FUNCTION__, clientSize);

    for (int i = 0; i < clientSize; i++) {
        if (mPqServiceCallBack[i] != NULL) {
            LOGD("%s send callback data from server to client by binder i %d\n", __FUNCTION__, i);
            send.writeInt32(hdrTypeCb->mHdrType);
            mPqServiceCallBack[i]->transact(CMD_HDR_DT_CB, send, &reply);
        } else {
            LOGE("%s mPqServiceCallBack is NULL\n");
        }
    }

    return;
}
#ifdef __cplusplus
}
#endif
