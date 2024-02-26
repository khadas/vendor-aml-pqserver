/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "PqTest"

#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <PqClient.h>
#include "CPqClientLog.h"
#include "pqcmd.h"
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

static int WriteSysfs(const char *path, const char *cmd)
{
    int fd;
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0777);

    if (fd >= 0) {
        write(fd, cmd, strlen(cmd));
        close(fd);
        return 0;
    }

    return -1;
}

static int WriteSysfs(const char *path, int value)
{
    char cmd[16] = {0};
    sprintf(cmd, "%d", value);

    return WriteSysfs(path, cmd);
}

class PqTest {
public:
    PqTest()
    {
        mpPqClient = PqClient::GetInstance();
    }

    ~PqTest()
    {

    }

    int SendCmd() {
        printf("%s: cmd is %d\n", __FUNCTION__, cmdID);
        int ret = -1;
        switch (cmdID) {
        case PQ_SET_PICTURE_MODE:
            ret = mpPqClient->SetPQMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_PICTURE_MODE:
            ret = mpPqClient->GetPQMode();
            printf("%s: current pq mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_GET_LAST_PICTURE_MODE:
            ret = mpPqClient->GetLastPQMode();
            printf("%s: current last pq mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->SetColorTemperature(setValue[0], setValue[1]);
            break;
        case PQ_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->GetColorTemperature();
            printf("%s: current colortempture mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BRIGHTNESS:
            ret = mpPqClient->SetBrightness(setValue[0], setValue[1]);
            break;
        case PQ_GET_BRIGHTNESS:
            ret = mpPqClient->GetBrightness();
            printf("%s: current Brightness value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_CONTRAST:
            ret = mpPqClient->SetContrast(setValue[0], setValue[1]);
            break;
        case PQ_GET_CONTRAST:
            ret = mpPqClient->GetContrast();
            printf("%s: current contrast value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SATUATION:
            ret = mpPqClient->SetSaturation(setValue[0], setValue[1]);
            break;
        case PQ_GET_SATUATION:
            ret = mpPqClient->GetSaturation();
            printf("%s: current saturation value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_HUE:
            ret = mpPqClient->SetHue(setValue[0], setValue[1]);
            break;
        case PQ_GET_HUE:
            ret = mpPqClient->GetHue();
            printf("%s: current hue value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SHARPNESS:
            ret = mpPqClient->SetSharpness(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_SHARPNESS:
            ret = mpPqClient->GetSharpness();
            printf("%s: current sharpness value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_NOISE_REDUCTION_MODE:
            ret = mpPqClient->SetNoiseReductionMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_NOISE_REDUCTION_MODE:
            ret = mpPqClient->GetNoiseReductionMode();
            printf("%s: current NoiseReduction value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_EYE_PROTECTION_MODE:
            ret = mpPqClient->SetEyeProtectionMode(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_EYE_PROTECTION_MODE:
            ret = mpPqClient->GetEyeProtectionMode();
            printf("%s: current Protection mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_GAMMA:
            ret = mpPqClient->SetGammaValue(setValue[0], setValue[1]);
            break;
        case PQ_GET_GAMMA:
            ret = mpPqClient->GetGammaValue();
            printf("%s: current Protection mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DISPLAY_MODE:
            ret = mpPqClient->SetDisplayMode(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_DISPLAY_MODE:
            ret = mpPqClient->GetDisplayMode();
            printf("%s: current Display mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BACKLIGHT:
            ret = mpPqClient->SetBacklight(setValue[0], setValue[1]);
            break;
        case PQ_GET_BACKLIGHT:
            ret = mpPqClient->GetBacklight();
            printf("%s: current backlight value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DYNAMICBACKLIGHT:
            ret = mpPqClient->SetDynamicBacklight(setValue[0], setValue[1]);
            break;
        case PQ_GET_DYNAMICBACKLIGHT:
            ret = mpPqClient->GetDynamicBacklight();
            printf("%s: current backlight value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_LOCALCONTRAST:
            ret = mpPqClient->SetLocalContrastMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_LOCALCONTRAST:
            ret = mpPqClient->GetLocalContrastMode();
            printf("%s: current Local contrast mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_CM:
            ret = mpPqClient->SetColorBaseMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_CM:
            ret = mpPqClient->GetColorBaseMode();
            printf("%s: current Color base mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SOURCE_CHANNEL:
            ret = mpPqClient->setCurrentSourceInfo(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_SOURCE_CHANNEL:
            source_input_param_t source_param;
            source_param = mpPqClient->getCurrentSourceInfo();
            printf("%s: current source info: source_input:%d sig_fmt:%d trans_fmt:%d.\n",
                __FUNCTION__, source_param.source_input, source_param.sig_fmt, source_param.trans_fmt);
            break;
        case PQ_SET_COLORGAMUT:
            ret = mpPqClient->SetColorGamutMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_COLORGAMUT:
            ret = mpPqClient->GetColorGamutMode();
            printf("%s: current Color gamut mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_GET_SOURCE_HDR_TYPE:
            ret = mpPqClient->GetSourceHDRType();
            printf("%s: current HDR type is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DYNAMICCONTRAST:
            ret = mpPqClient->SetDynamicContrastMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_DYNAMICCONTRAST:
            ret = mpPqClient->GetDynamicContrastMode();
            printf("%s: current Dynamic contrast mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_RECOVERYPQ:
            ret = mpPqClient->SSMRecovery();
            break;
        case PQ_GET_HAS_MEMC:
            ret = mpPqClient->HasMemcFunc();
            printf("%s: current hasmemc is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_MEMCMODE:
            ret = mpPqClient->SetMemcMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_MEMCMODE:
            ret = mpPqClient->GetMemcMode();
            printf("%s: current Memc mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_MEMC_DEBLUR:
            ret = mpPqClient->SetMemcDeBlurLevel(setValue[0], setValue[1]);
            break;
        case PQ_GET_MEMC_DEBLUR:
            ret = mpPqClient->GetMemcDeBlurLevel();
            printf("%s: current Memc DeBlurLevel level is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_MEMC_DEJUDDER:
            ret = mpPqClient->SetMemcDeJudderLevel(setValue[0], setValue[1]);
            break;
        case PQ_GET_MEMC_DEJUDDER:
            ret = mpPqClient->GetMemcDeJudderLevel();
            printf("%s: current Memc DeJudderLevel level is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DECONTOUR:
            ret = mpPqClient->SetDecontourMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_DECONTOUR:
            ret = mpPqClient->GetDecontourMode();
            printf("%s: current Memc Decontour Mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DEBLOCK:
            ret = mpPqClient->SetDeBlockMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_DEBLOCK:
            ret = mpPqClient->GetDeBlockMode();
            printf("%s: current DeBlock Mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DEMOSQUITO:
            ret = mpPqClient->SetDeMosquitoMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_DEMOSQUITO:
            ret = mpPqClient->GetDeMosquitoMode();
            printf("%s: current DeMosquito Mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BLACKSTRETCH:
            ret = mpPqClient->SetBlackStretch(setValue[0], setValue[1]);
            break;
        case PQ_GET_BLACKSTRETCH:
            ret = mpPqClient->GetBlackStretch();
            printf("%s: current BlackStretch Mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BLUESTRETCH:
            ret = mpPqClient->SetBlueStretch(setValue[0], setValue[1]);
            break;
        case PQ_GET_BLUESTRETCH:
            ret = mpPqClient->GetBlueStretch();
            printf("%s: current BlueStretch Mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_GET_HAS_AIPQ:
            ret = mpPqClient->HasAipqFunc();
            break;
        case PQ_SET_AIPQ:
            ret = mpPqClient->SetAipqEnable(setValue[0]);
            break;
        case PQ_GET_AIPQ:
            ret = mpPqClient->GetAipqEnable();
            printf("%s: current Aipq Enable is %d\n", __FUNCTION__, ret);
            break;
        case PQ_GET_HAS_AISR:
            ret = mpPqClient->HasAisrFunc();
            break;
        case PQ_SET_AISR:
            ret = mpPqClient->SetAiSrEnable(setValue[0]);
            break;
        case PQ_GET_AISR:
            ret = mpPqClient->GetAiSrEnable();
            printf("%s: current AiSr Enable is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_LDIM:
            ret = mpPqClient->SetLdim(setValue[0], setValue[1]);
            break;
        case PQ_GET_LDIM:
            ret = mpPqClient->GetLdim();
            printf("%s: current LDIM mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_MPEGNR:
            ret = mpPqClient->SetMpegNr(setValue[0], setValue[1]);
            break;
        case PQ_GET_MPEGNR:
            ret = mpPqClient->GetMpegNr();
            printf("%s: current MPEG NR mode is %d\n", __FUNCTION__, ret);
            break;
        case PQ_GET_COLORTEMP_USER_PARAM:
            tvpq_rgb_ogo_t rgbogo;
            rgbogo = mpPqClient->GetColorTemperatureUserParam();
            printf("%s: current RGB GAIN&OFFSET is %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", __FUNCTION__,
                    rgbogo.en, rgbogo.r_pre_offset, rgbogo.g_pre_offset, rgbogo.b_pre_offset,
                    rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain, rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset);
            break;
        case PQ_SET_PICTURE_UI_CLEAR:
            ret = mpPqClient->ResetPictureUiSetting();
            printf("%s: reset picture ui setting result is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_COLOR_CUSTOMIZE:
            ret = mpPqClient->SetColorCustomize(setValue[0], setValue[1], setValue[2], setValue[3]);
            break;
        case PQ_GET_COLOR_CUSTOMIZE:
            vpp_single_color_param_cm_t cms_cm;
            cms_cm = mpPqClient->GetColorCustomize(setValue[0]);
            printf("%s: current color customize(CMS):%d %d %d\n", __FUNCTION__,
                cms_cm.sat, cms_cm.hue, cms_cm.luma);
            break;
         case PQ_SET_COLOR_CUSTOMIZE_3DLUT:
            ret = mpPqClient->SetColorCustomizeBy3DLut(setValue[0], setValue[1], setValue[2], setValue[3]);
            break;
        case PQ_GET_COLOR_CUSTOMIZE_3DLUT:
            vpp_single_color_param_3dlut_t cms_3dlut;
            cms_3dlut = mpPqClient->GetColorCustomizeBy3DLut(setValue[0]);
            printf("%s: current color customize(3DLUT):%d %d %d\n", __FUNCTION__,
                cms_3dlut.red, cms_3dlut.green, cms_3dlut.blue);
            break;
        case PQ_RESET_COLOR_CUSTOMIZE:
            ret = mpPqClient->ResetColorCustomize(setValue[0]);
            break;
        case PQ_SET_WB_GAMMA_DATA:
            ret = mpPqClient->SetWhitebalanceGamma(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_WB_GAMMA_DATA:
            ret = mpPqClient->GetWhitebalanceGamma(setValue[0], setValue[1]);
            printf("%s: offset is %d\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SUPERRESOLUTION:
            ret = mpPqClient->SetSuperResolution(setValue[0], setValue[1]);
            break;
        case PQ_GET_SUPERRESOLUTION:
            ret = mpPqClient->GetSuperResolution();
            printf("%s: current Super Resolution is %d\n", __FUNCTION__, ret);
            break;
        case PQ_GET_DB_VERSION_INFO:
            tvpq_databaseinfo_t ver_info;
            memset(&ver_info, 0, sizeof(tvpq_databaseinfo_t));
            ver_info = mpPqClient->GetDbVersionInfo(setValue[0], setValue[1]);
            printf("%s: ver_info.version:%s\n", __FUNCTION__, ver_info.version);
            break;

        //factory API
        case PQ_FACTORY_RESET_PICTURE_MODE:
            ret = mpPqClient->FactoryResetPQMode();
            break;
        case PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->FactoryResetColorTemp();
            break;
        case PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->FactoryWhiteBalanceSetColorTemperature(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->FactoryWhiteBalanceGetColorTemperature();
            printf("%s: current colortempture mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_BRIGHTNESS:
            ret = mpPqClient->FactorySetPQMode_Brightness(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_BRIGHTNESS:
            ret = mpPqClient->FactoryGetPQMode_Brightness(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: current brightness mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_CONTRAST:
            ret = mpPqClient->FactorySetPQMode_Contrast(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_CONTRAST:
            ret = mpPqClient->FactoryGetPQMode_Contrast(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: current contrast mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_SATUATION:
            ret = mpPqClient->FactorySetPQMode_Saturation(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_SATUATION:
            ret = mpPqClient->FactoryGetPQMode_Saturation(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: current saturation mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_HUE:
            ret = mpPqClient->FactorySetPQMode_Hue(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_HUE:
            ret = mpPqClient->FactoryGetPQMode_Hue(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: current hue mode is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_SHARPNESS:
            ret = mpPqClient->FactorySetPQMode_Sharpness(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_SHARPNESS:
            ret = mpPqClient->FactoryGetPQMode_Sharpness(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: current sharpness mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_OVERSCAN:
            tvin_cutwin_t overscanParam;
            memset(&overscanParam, 0, sizeof(tvin_cutwin_t));
            overscanParam.hs = (unsigned short)setValue[4];
            overscanParam.he = (unsigned short)setValue[5];
            overscanParam.vs = (unsigned short)setValue[6];
            overscanParam.ve = (unsigned short)setValue[7];
            ret = mpPqClient->FactorySetOverscanParams(setValue[0], setValue[1], setValue[2], setValue[3], overscanParam);
            break;
        case PQ_FACTORY_GET_OVERSCAN:
            tvin_cutwin_t cutwin_t;
            cutwin_t = mpPqClient->FactoryGetOverscanParams(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: cutwin_t: hs:%d he:%d vs:%d ve:%d.\n",
                __FUNCTION__, cutwin_t.hs, cutwin_t.he, cutwin_t.vs, cutwin_t.ve);
            break;
        case PQ_FACTORY_SET_WB_RED_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceRedGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_WB_RED_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceRedGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: red gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_GREEN_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceGreenGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_GREEN_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceGreenGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: green gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_BLUE_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceBlueGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_BLUE_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceBlueGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: blue gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_RED_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceRedPostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_RED_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceRedPostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: red offset is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_GREEN_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceGreenPostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_GREEN_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceGreenPostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: gree offset is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_BLUE_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceBluePostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_BLUE_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceBluePostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            printf("%s: blue offset is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_COLOR_PARAMS:
            tvpq_rgb_ogo_t params;
            memset(&params, 0, sizeof(tvpq_rgb_ogo_t));
            params.en = setValue[1];
            params.r_gain = setValue[2];
            params.g_gain = setValue[3];
            params.b_gain = setValue[4];
            params.r_post_offset = setValue[5];
            params.g_post_offset = setValue[6];
            params.b_post_offset = setValue[7];
            ret = mpPqClient->FactorySetColorParams(setValue[0], params);
            break;

        case PQ_FACTORY_GET_COLOR_PARAMS:
            tvpq_rgb_ogo_t data;
            memset(&data, 0, sizeof(tvpq_rgb_ogo_t));
            ret = mpPqClient->FactoryGetColorParams(setValue[0], &data);
            printf("%s: en %d\n", __FUNCTION__, data.en);
            printf("%s: rgain %d\n", __FUNCTION__, data.r_gain);
            printf("%s: ggai %d\n", __FUNCTION__, data.g_gain);
            printf("%s: bgain %d\n", __FUNCTION__, data.b_gain);
            printf("%s: roffset %d\n", __FUNCTION__, data.b_post_offset);
            printf("%s: goffset %d\n", __FUNCTION__, data.g_post_offset);
            printf("%s: boffset %d\n", __FUNCTION__, data.b_post_offset);
            break;

        case PQ_FACTORY_SET_WB_GAMMA_DATA:
            ret = mpPqClient->FactorySetWhitebalanceGamma(setValue[0], setValue[1], setValue[2], setValue[3]);
            break;
        case PQ_FACTORY_GET_WB_GAMMA_DATA:
            ret = mpPqClient->FactoryGetWhitebalanceGamma(setValue[0], setValue[1], setValue[2]);
            printf("%s: offset is %d\n", __FUNCTION__, ret);
            break;
        default:
            break;
            }

        return 0;
    }

    PqClient *mpPqClient;
    pqcmd_t cmdID = PQ_MODULE_CMD_MAX;
    int setValue[10] = {0};
};

static int SetOsdBlankStatus(const char *path, int cmd)
{
    return WriteSysfs(path, cmd);
}

static int DisplayInit()
{
    SetOsdBlankStatus("/sys/class/graphics/fb0/osd_display_debug", 1);
    SetOsdBlankStatus("/sys/class/graphics/fb0/blank", 1);
    return 0;
}

int main(int argc, char **argv) {
    unsigned char read_buf[256];
    memset(read_buf, 0, sizeof(read_buf));

    sp<ProcessState> proc(ProcessState::self());
    proc->startThreadPool();

    PqTest *test = new PqTest();
    int run = 1;
    DisplayInit();

    printf("#### please select cmd####\n");
    printf("#### select 201 to set pq mode ####\n");
    printf("#### select 202 to get pq mode ####\n");
    printf("#### select 203 to set ColorTemperature mode ####\n");
    printf("#### select 204 to get ColorTemperature mode ####\n");
    printf("#### select 205 to set Brightness ####\n");
    printf("#### select 206 to get Brightness ####\n");
    printf("#### select 207 to set Contrast ####\n");
    printf("#### select 208 to get Contrast ####\n");
    printf("#### select 209 to set Saturatioin ####\n");
    printf("#### select 210 to get Saturatioin ####\n");
    printf("#### select 211 to set Hue ####\n");
    printf("#### select 212 to get Hue ####\n");
    printf("#### select 213 to set Sharpness ####\n");
    printf("#### select 214 to get Sharpness ####\n");
    printf("#### select 215 to set NoiseRedution ####\n");
    printf("#### select 216 to get NoiseRedution ####\n");
    printf("#### select 217 to set EyeProtection ####\n");
    printf("#### select 218 to get EyeProtection ####\n");
    printf("#### select 219 to set Gamma ####\n");
    printf("#### select 220 to get Gamma ####\n");
    printf("#### select 221 to set displaymode ####\n");
    printf("#### select 222 to get displaymode ####\n");
    printf("#### select 223 to set backlight ####\n");
    printf("#### select 224 to get backlight ####\n");
    printf("#### select 225 to set DynamicBacklight ####\n");
    printf("#### select 226 to get DynamicBacklight ####\n");
    printf("#### select 227 to set LocalContrast ####\n");
    printf("#### select 228 to get LocalContrast ####\n");
    printf("#### select 229 to set CM ####\n");
    printf("#### select 230 to get CM ####\n");
    printf("#### select 231 to set source channel ####\n");
    printf("#### select 232 to get source channel ####\n");
    printf("#### select 233 to set ColorGamut ####\n");
    printf("#### select 234 to get ColorGamut ####\n");
    printf("#### select 235 to get source fmt type ####\n");
    printf("#### select 236 to set DynamicContrast ####\n");
    printf("#### select 237 to get DynamicContrast ####\n");
    printf("#### select 238 to recovery ssm ####\n");
    printf("#### select 239 to get has memc ####\n");
    printf("#### select 240 to set Memc ####\n");
    printf("#### select 241 to get Memc ####\n");
    printf("#### select 242 to set Memc DeBlur ####\n");
    printf("#### select 243 to get Memc DeBlur ####\n");
    printf("#### select 244 to set Memc DeJudder ####\n");
    printf("#### select 245 to get Memc DeJudder ####\n");
    printf("#### select 246 to set DeContour ####\n");
    printf("#### select 247 to get DeContour ####\n");
    printf("#### select 248 to set DeBlock ####\n");
    printf("#### select 249 to get DeBlock ####\n");
    printf("#### select 250 to set DeMosquito ####\n");
    printf("#### select 251 to get DeMosquito ####\n");
    printf("#### select 252 to set BlackStretch ####\n");
    printf("#### select 253 to get BlackStretch ####\n");
    printf("#### select 254 to set BlueStretch ####\n");
    printf("#### select 255 to get BlueStretch ####\n");
    printf("#### select 256 to get has aipq ####\n");
    printf("#### select 257 to set aipq ####\n");
    printf("#### select 258 to get aipq ####\n");
    printf("#### select 259 to get has aisr ####\n");
    printf("#### select 260 to set aisr ####\n");
    printf("#### select 261 to get aisr ####\n");
    printf("#### select 262 to set LocalDimming ####\n");
    printf("#### select 263 to get LocalDimming ####\n");
    printf("#### select 264 to set MpegNr ####\n");
    printf("#### select 265 to get MpegNr ####\n");
    printf("#### select 266 to get RGB Gain/Offset ####\n");
    printf("#### select 267 to reset picture ui setting ####\n");
    printf("#### select 268 to set color customize ####\n");
    printf("#### select 269 to get color customize ####\n");
    printf("#### select 270 to set color customize by 3dlut ####\n");
    printf("#### select 271 to get color customize by 3d lut ####\n");
    printf("#### select 272 to reset color customize ####\n");
    printf("#### select 273 to set Whitebalance Gamma ####\n");
    printf("#### select 274 to get whitebalance Gamma ####\n");
    printf("#### select 275 to set SuperResolution ####\n");
    printf("#### select 276 to get SuperResolution ####\n");
    printf("#### select 277 to get last pq mode ####\n");
    printf("#### select 280 to get db version info ####\n");

    printf("#### below is factory cmd####\n");
    printf("#### select 301 to reset pq mode ####\n");
    printf("#### select 302 to reset ColorTemperature ####\n");
    printf("#### select 303 to set ColorTemperature ####\n");
    printf("#### select 304 to get ColorTemperature ####\n");
    printf("#### select 305 to set Brightness ####\n");
    printf("#### select 306 to get Brightness ####\n");
    printf("#### select 307 to set Contrast ####\n");
    printf("#### select 308 to get Contrast ####\n");
    printf("#### select 309 to set Saturatioin ####\n");
    printf("#### select 310 to get Saturatioin ####\n");
    printf("#### select 311 to set Hue ####\n");
    printf("#### select 312 to get Hue ####\n");
    printf("#### select 313 to set Sharpness ####\n");
    printf("#### select 314 to get Sharpness ####\n");
    printf("#### select 315 to set overscan ####\n");
    printf("#### select 316 to get overscan ####\n");
    printf("#### select 317 to set WB R Gain ####\n");
    printf("#### select 318 to get WB R Gain ####\n");
    printf("#### select 319 to set WB G Gain ####\n");
    printf("#### select 320 to get WB G Gain ####\n");
    printf("#### select 321 to set WB B Gain ####\n");
    printf("#### select 322 to get WB B Gain ####\n");
    printf("#### select 323 to set WB R OFFSET ####\n");
    printf("#### select 324 to get WB R OFFSET ####\n");
    printf("#### select 325 to set WB G OFFSET ####\n");
    printf("#### select 326 to get WB G OFFSET ####\n");
    printf("#### select 327 to set WB B OFFSET ####\n");
    printf("#### select 328 to get WB B OFFSET ####\n");
    printf("#### select 335 to set CRI colortemp params####\n");
    printf("#### select 336 to get CRI colortemp params ####\n");
    printf("#### select 343 to set CRI Whitebalance Gamma ####\n");
    printf("#### select 344 to get CRI Whitebalance Gamma ####\n");

    printf("#### select 999 to exit####\n");
    printf("##########################\n");
    while (run) {
        char Command[10] = {'\0'};
        int conmmand;
        int mode = 0, is_save = 0;
        int source = 0, sig_fmt = 0, fmt_3d = 0, pq_mode = 0;
        int colortemp_mode = 0;
        int color = 0, type = 0, value = 0;
        int scanf_ret = 0;

        int ret = scanf("%10s", Command);
        fflush(stdin);
        if (ret <= 0 ) {
           LOGE("scanf faile\n");
        }
        conmmand = atoi(Command);
        printf("#### Command: %s %d\n", Command, conmmand);

        switch (conmmand) {
          case 299: {
            test->cmdID = PQ_MODULE_CMD_MAX;
            run = 0;
            break;
          }
          case 201: {
              printf("PQ_SET_PICTURE_MODE\n");
              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;

              if (scanf_ret != 2)
                break;

              test->cmdID = PQ_SET_PICTURE_MODE;
              break;
          }
          case 202: {
              printf("PQ_GET_PICTURE_MODE\n");
              test->cmdID = PQ_GET_PICTURE_MODE;
              break;
          }
          case 203: {
              printf("PQ_SET_COLOR_TEMPERATURE_MODE\n");
              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;

              if (scanf_ret != 2) break;
              test->cmdID = PQ_SET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 204: {
              printf("PQ_GET_COLOR_TEMPERATURE_MODE\n");
              test->cmdID = PQ_GET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 205: {
              printf("PQ_SET_BRIGHTNESS\n");
              printf("Brightness value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;

              if (scanf_ret != 2) break;
              test->cmdID = PQ_SET_BRIGHTNESS;
              break;
          }
          case 206: {
              printf("PQ_GET_BRIGHTNESS\n");
              test->cmdID = PQ_GET_BRIGHTNESS;
              break;
          }
          case 207: {
              printf("PQ_SET_CONTRAST\n");
              printf("Contrast value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;

              if (scanf_ret != 2) break;
              test->cmdID = PQ_SET_CONTRAST;
              break;
          }
          case 208: {
              printf("PQ_GET_CONTRAST\n");
              test->cmdID = PQ_GET_CONTRAST;
              break;
          }
          case 209: {
              printf("PQ_SET_SATURATION\n");
              printf("Saturatioin value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_SATUATION;
              break;
          }
          case 210: {
              printf("PQ_GET_SATURATION\n");
              test->cmdID = PQ_GET_SATUATION;
              break;
          }
          case 211: {
              printf("PQ_SET_HUE\n");
              printf("Hue value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_HUE;
              break;
          }
          case 212: {
              printf("PQ_GET_HUE\n");
              test->cmdID = PQ_GET_HUE;
              break;
          }
          case 213: {
              printf("PQ_SET_SHARPNESS\n");
              printf("Sharpness value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_SHARPNESS;
              break;
          }
          case 214: {
              printf("PQ_GET_SHARPNESS\n");
              test->cmdID = PQ_GET_SHARPNESS;
              break;
          }
          case 215: {
              printf("PQ_SET_NOISE_REDUCTION_MODE\n");
              printf("NoiseReduction: OFF(0), LOW(1), MID(2), HIGH(3), AUTO(4)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_NOISE_REDUCTION_MODE;
              break;
          }
          case 216: {
              printf("PQ_GET_NOISE_REDUCTION_MODE\n");
              test->cmdID = PQ_GET_NOISE_REDUCTION_MODE;
              break;
          }
          case 217: {
              printf("PQ_SET_EYE_PROTECTION_MODE\n");
              printf("EyeProtection(0: OFF, 1: ON)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_SET_EYE_PROTECTION_MODE;
              break;
          }
          case 218: {
              printf("PQ_GET_EYE_PROTECTION_MODE\n");
              test->cmdID = PQ_GET_EYE_PROTECTION_MODE;
              break;
          }
          case 219: {
              printf("PQ_SET_GAMMA\n");
              printf("Gamma(0:[2.2], 1:[1.7], 2:[1.8], 3:[1.9], 4:[2.0], 5:[2.2], 6:[2.2], 7:[2.3], 8:[2.4], 9:[2.5], 10:[2.6], 11:[2.7], 12:[BT1886])\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_GAMMA;
              break;
          }
          case 220: {
              printf("PQ_GET_GAMMA\n");
              test->cmdID = PQ_GET_GAMMA;
              break;
          }
          case 221: {
            int is_used = 0;
              printf("PQ_SET_DISPLAY_MODE\n");
              printf("is used:(0 ~ 1)\n");
              scanf_ret = scanf("%d", &is_used);
              test->setValue[0] = is_used;

              printf("displaymode(0:[16:9], 1:[PERSON], 2:[MOVIE], 3:[CAPTION], 4:[4:3], 5:[FULL], 6:[NORMAL], 7:[NOSCALEUP], 8:[CROP_FULL], 9:[CROP], 10:[ZOOM])\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[1] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[2] = is_save;
              if (scanf_ret != 3) break;

              test->cmdID = PQ_SET_DISPLAY_MODE;
              break;
          }
          case 222: {
              printf("PQ_GET_DISPLAY_MODE\n");
              test->cmdID = PQ_GET_DISPLAY_MODE;
              break;
          }
          case 223: {
              printf("PQ_SET_BACKLIGHT\n");
              printf("backlight value:(0 ~ 100)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_BACKLIGHT;
              break;
          }
          case 224: {
              printf("PQ_GET_BACKLIGHT\n");
              test->cmdID = PQ_GET_BACKLIGHT;
              break;
          }
          case 225: {
              printf("PQ_SET_DYNAMICBACKLIGHT\n");
              printf("DynamicBacklight: OFF(0), LOW(1), HIGH(2)\n");

              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_DYNAMICBACKLIGHT;
              break;
          }
          case 226: {
              printf("PQ_GET_DYNAMICBACKLIGHT\n");
              test->cmdID = PQ_GET_DYNAMICBACKLIGHT;
              break;
          }
          case 227: {
              printf("PQ_SET_LOCALCONTRAST\n");
              printf("LocalContrast: OFF(0), LOW(1), MID(2), HIGH(3)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_LOCALCONTRAST;
              break;
          }
          case 228: {
              printf("PQ_GET_LOCALCONTRAST\n");
              test->cmdID = PQ_GET_LOCALCONTRAST;
              break;
          }
          case 229: {
              printf("PQ_SET_CM\n");
              printf("CM value: OFF(0), 1: OPTIMIZE(1), ENHANCE(2), DEMO(3)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_CM;
              break;
          }
          case 230: {
              printf("PQ_GET_CM\n");
              test->cmdID = PQ_GET_CM;
              break;
          }
          case 231: {
              int sig_fmt = 0, fmt_3d = 0;
              printf("PQ_SET_SOURCE_CHANNEL\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              if (scanf_ret != 3) break;

              test->cmdID = PQ_SET_SOURCE_CHANNEL;
              break;
          }
          case 232: {
              printf("PQ_GET_SOURCE_CHANNEL\n");
              test->cmdID = PQ_GET_SOURCE_CHANNEL;
              break;
          }
          case 233: {
              printf("PQ_SET_COLORGAMUT\n");
              printf("ColorGamut(SRC(0), AUTO(1), NATIVE(2)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_COLORGAMUT;
              break;
          }
          case 234: {
              printf("PQ_GET_COLORGAMUT\n");
              test->cmdID = PQ_GET_COLORGAMUT;
              break;
          }
          case 235: {
              printf("PQ_GET_SOURCE_HDR_TYPE\n");
              test->cmdID = PQ_GET_SOURCE_HDR_TYPE;
              break;
          }
          case 236: {
              printf("PQ_SET_DYNAMICCONTRAST\n");
              printf("DynamicContrast: OFF(0), LOW(1), MID(2), HIGH(3)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_DYNAMICCONTRAST;
              break;
          }
          case 237: {
              printf("PQ_GET_DYNAMICCONTRAST\n");
              test->cmdID = PQ_GET_DYNAMICCONTRAST;
              break;
          }
          case 238: {
              printf("PQ_SET_RECOVERYPQ\n");
              test->cmdID = PQ_SET_RECOVERYPQ;
              break;
          }
          case 239: {
              printf("PQ_GET_HAS_MEMC\n");
              test->cmdID = PQ_GET_HAS_MEMC;
              break;
          }
          case 240: {
              printf("PQ_SET_MEMCMODE\n");
              printf("Memc mode: OFF(0), LOW(1), MID(2), HIGH(3)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_MEMCMODE;
              break;
          }
          case 241: {
              printf("PQ_GET_MEMCMODE\n");
              test->cmdID = PQ_GET_MEMCMODE;
              break;
          }
          case 248: {
              printf("PQ_SET_DEBLOCK\n");
              printf("DeBlock: OFF(0), LOW(1), MID(2), HIGH(3), AUTO(4)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0~1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_DEBLOCK;
              break;
          }
          case 249: {
              printf("PQ_GET_DEBLOCK\n");
              test->cmdID = PQ_GET_DEBLOCK;
              break;
          }
          case 250: {
              printf("PQ_SET_DEMOSQUITO\n");
              printf("DeMosquito: OFF(0), LOW(1), MID(2), HIGH(3), AUTO(4)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0~1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_DEMOSQUITO;
              break;
          }
          case 251: {
              printf("PQ_GET_DEMOSQUITO\n");
              test->cmdID = PQ_GET_DEMOSQUITO;
              break;
          }
          case 256: {
              printf("PQ_GET_HAS_AIPQ\n");
              test->cmdID = PQ_GET_HAS_AIPQ;
              break;
          }
          case 257: {
              printf("PQ_SET_AIPQ\n");
              printf("AIPQ enable: OFF(0), ON(1)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_SET_AIPQ;
              break;
          }
          case 258: {
              printf("PQ_GET_AIPQ\n");
              test->cmdID = PQ_GET_AIPQ;
              break;
          }
          case 259: {
              printf("PQ_GET_HAS_AISR\n");
              test->cmdID = PQ_GET_HAS_AISR;
              break;
          }
          case 260: {
              printf("PQ_SET_AISR\n");
              printf("AISR enable: 0: OFF, 1: ON\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_SET_AISR;
              break;
          }
          case 261: {
              printf("PQ_GET_AISR\n");
              test->cmdID = PQ_GET_AISR;
              break;
          }
          case 268: {
              printf("PQ_SET_COLOR_CUSTOMIZE\n");
              printf("color: PURPLE(0), RED(1), SKIN(2), YELLOW(3), YELLOW_GREEN(4), GREEN(5), BLUE_GREEN(6), CYAN(7), BLUE(8)\n");
              scanf_ret= scanf("%d", &color);
              test->setValue[0] = color;

              printf("type: Saturation(0), Hue(1), Luminance(2)\n");
              scanf_ret += scanf("%d", &type);
              test->setValue[1] = type;

              printf("value: -50 ~ 50\n");
              scanf_ret += scanf("%d", &value);
              test->setValue[2] = value;

              printf("is save:(0~1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[3] = is_save;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_SET_COLOR_CUSTOMIZE;
              break;
          }
          case 269: {
              printf("PQ_SET_COLOR_CUSTOMIZE\n");
              printf("color: PURPLE(0), RED(1), SKIN(2), YELLOW(3), YELLOW_GREEN(4), GREEN(5), BLUE_GREEN(6), CYAN(7), BLUE(8)\n");
              scanf_ret = scanf("%d", &color);
              test->setValue[0] = color;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_GET_COLOR_CUSTOMIZE;
              break;
          }
          case 270: {
              printf("PQ_SET_COLOR_CUSTOMIZE_3DLUT\n");
              printf("color: RED(0), GREEN(1), BLUE(2), CYAN(3), MAGENTA(4), YELLOW(5)\n");
              scanf_ret = scanf("%d", &color);
              test->setValue[0] = color;

              printf("type: (RED(3), GREEN(4), BLUE(5)\n");
              scanf_ret += scanf("%d", &type);
              test->setValue[1] = type;

              printf("value: (-100 ~ 100)\n");
              scanf_ret += scanf("%d", &value);
              test->setValue[2] = value;

              printf("is save:(0~1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[3] = is_save;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_SET_COLOR_CUSTOMIZE_3DLUT;
              break;
          }
          case 271: {
              printf("PQ_GET_COLOR_CUSTOMIZE_3DLUT\n");
              printf("color: RED(0), GREEN(1), BLUE(2), CYAN(3), MAGENTA(4), YELLOW(5)\n");
              scanf_ret = scanf("%d", &color);
              test->setValue[0] = color;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_GET_COLOR_CUSTOMIZE_3DLUT;
              break;
          }
          case 272: {
              printf("PQ_RESET_COLOR_CUSTOMIZE\n");
              printf("Module: CMS(0), 3DLUT(1)\n");
              scanf_ret = scanf("%d", &value);
              test->setValue[0] = value;
              if (scanf_ret != 1) break;

              test->cmdID = PQ_RESET_COLOR_CUSTOMIZE;
            break;
          }
          case 273: {
              int offset = 0;
              printf("PQ_SET_WB_GAMMA_DATA\n");
              printf("color: RED CHANNEL(0), GREEN CHANNEL(1), BLUE CHANNEL(2)\n");
              scanf_ret = scanf("%d", &color);
              test->setValue[0] = color;

              printf("point: 10 percent per step(0%(0) ~ 100%(10)\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[1] = mode;

              printf("offset: -1023 ~ 1023\n");
              scanf_ret += scanf("%d", &offset);
              test->setValue[2] = offset;

              if (scanf_ret != 3) break;

              test->cmdID = PQ_SET_WB_GAMMA_DATA;
              break;
          }
          case 274: {
              printf("PQ_GET_WB_GAMMA_DATA\n");
              printf("color: RED CHANNEL(0), GREEN CHANNEL(1), BLUE CHANNEL(2)\n");
              scanf_ret = scanf("%d", &color);
              test->setValue[0] = color;

              printf("mode: 10 percent per step(0 percent(0) ~ 100 percent(11)\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[1] = mode;

              if (scanf_ret != 2) break;

              test->cmdID = PQ_GET_WB_GAMMA_DATA;
            break;
          }
          case 275: {
              printf("PQ_SET_SUPERRESOLUTION\n");
              printf("mode: OFF(0), MID(1), HIGH(2)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("is save:(0 ~ 1)\n");
              scanf_ret += scanf("%d", &is_save);
              test->setValue[1] = is_save;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_SET_SUPERRESOLUTION;
              break;
          }
          case 276: {
              printf("PQ_GET_SUPERRESOLUTION\n");
              test->cmdID = PQ_GET_SUPERRESOLUTION;
            break;
          }
          case 277: {
              printf("PQ_GET_LAST_PICTURE_MODE\n");
              test->cmdID = PQ_GET_LAST_PICTURE_MODE;
              break;
          }
          case 280: {
              printf("PQ_GET_DB_VERSION_INFO\n");
              printf("mode: pq.db(0), overscan.db(1)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("type: tool_ver(0), project_ver(1), generate_time(2), chip_ver(3), db_ver(4)\n");
              scanf_ret += scanf("%d", &type);
              test->setValue[1] = type;
              if (scanf_ret != 2) break;

              test->cmdID = PQ_GET_DB_VERSION_INFO;
              break;
          }

          //factory cmd
          case 301: {
              printf("PQ_FACTORY_RESET_PICTURE_MODE\n");
              test->cmdID = PQ_FACTORY_RESET_PICTURE_MODE;
              break;
          }
          case 302: {
              printf("PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE\n");
              test->cmdID = PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 303: {
              printf("PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 304: {
              printf("PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE\n");
              test->cmdID = PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 305: {
              printf("PQ_FACTORY_SET_BRIGHTNESS\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              printf("brightness value: 0 ~ 100\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_BRIGHTNESS;
              break;
          }
          case 306: {
              printf("PQ_FACTORY_GET_BRIGHTNESS\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_BRIGHTNESS;
              break;
          }
          case 307: {
              printf("PQ_FACTORY_SET_CONTRAST\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              printf("contrast value: 0 ~ 100\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_CONTRAST;
              break;
          }
          case 308: {
              printf("PQ_FACTORY_GET_CONTRAST\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_CONTRAST;
              break;
          }
          case 309: {
              printf("PQ_FACTORY_SET_SATURATION\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              printf("Saturation value: 0 ~ 100\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_SATUATION;
              break;
          }
          case 310: {
              printf("PQ_FACTORY_GET_SATURATION\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_SATUATION;
              break;
          }
          case 311: {
              printf("PQ_FACTORY_SET_HUE\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              printf("Hue value: 0 ~ 100\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_HUE;
              break;
          }
          case 312: {
              printf("PQ_FACTORY_GET_HUE\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_HUE;
              break;
          }
          case 313: {
              printf("PQ_FACTORY_SET_SHARPNESS\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              printf("sharpness value: 0 ~ 100\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_SHARPNESS;
              break;
          }
          case 314: {
              printf("PQ_FACTORY_GET_SHARPNESS\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("pq_mode value: STANDARD(0), BRIGHT(1), SOFT(2), USER(3), MOVIE(4), COLORFUL(5), MONITOR(6), GAME(7), SPORTS(8), SONY(9), SAMSUNG(10), SHARP(11), DV_BRIGHT(12), DV_DARK(13)\n");
              scanf_ret += scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_SHARPNESS;
              break;
          }
          case 315: {
              int dmode = 0;
              int he = 0, hs = 0, ve = 0, vs = 0;
              printf("PQ_FACTORY_SET_OVERSCAN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("displaymode(0:[16:9], 1:[PERSON], 2:[MOVIE], 3:[CAPTION], 4:[4:3], 5:[FULL], 6:[NORMAL], 7:[NOSCALEUP], 8:[CROP_FULL], 9:[CROP], 10:[ZOOM])\n");
              scanf_ret += scanf("%d", &dmode);
              test->setValue[3] = dmode;

              printf("hs value: \n");
              scanf_ret += scanf("%d", &hs);
              test->setValue[4] = hs;

              printf("he value: \n");
              scanf_ret += scanf("%d", &he);
              test->setValue[5] = he;

              printf("vs value: \n");
              scanf_ret += scanf("%d", &vs);
              test->setValue[6] = vs;

              printf("ve value: \n");
              scanf_ret += scanf("%d", &ve);
              test->setValue[7] = ve;
              if (scanf_ret != 8) break;

              test->cmdID = PQ_FACTORY_SET_OVERSCAN;
              break;
          }
          case 316: {
              int dmode = 0;
              printf("PQ_FACTORY_GET_OVERSCAN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("displaymode(0:[16:9], 1:[PERSON], 2:[MOVIE], 3:[CAPTION], 4:[4:3], 5:[FULL], 6:[NORMAL], 7:[NOSCALEUP], 8:[CROP_FULL], 9:[CROP], 10:[ZOOM])\n");
              scanf_ret += scanf("%d", &dmode);
              test->setValue[3] = dmode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_OVERSCAN;
              break;
          }
          case 317: {
              printf("PQ_FACTORY_SET_WB_RED_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("red gain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_RED_GAIN;
              break;
          }
          case 318: {
              printf("PQ_FACTORY_GET_WB_RED_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_RED_GAIN;
              break;
          }
          case 319: {
              printf("PQ_FACTORY_SET_WB_GREEN_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("green gain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_GREEN_GAIN;
              break;
          }
          case 320: {
              printf("PQ_FACTORY_GET_WB_GREEN_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_GREEN_GAIN;
              break;
          }
          case 321: {
              printf("PQ_FACTORY_SET_WB_BLUE_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("blue gain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_BLUE_GAIN;
              break;
          }
          case 322: {
              printf("PQ_FACTORY_GET_WB_BLUE_GAIN\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_BLUE_GAIN;
              break;
          }
          case 323: {
              printf("PQ_FACTORY_SET_WB_RED_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("red offset: -1024 ~ 1023\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_RED_OFFSET;
              break;
          }
          case 324: {
              printf("PQ_FACTORY_GET_WB_RED_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_RED_OFFSET;
              break;
          }
          case 325: {
              printf("PQ_FACTORY_SET_WB_GREEN_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("green offset: -1024 ~ 1023\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_GREEN_OFFSET;
              break;
          }
          case 326: {
              printf("PQ_FACTORY_GET_WB_GREEN_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_GREEN_OFFSET;
              break;
          }
          case 327: {
              printf("PQ_FACTORY_SET_WB_BLUE_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              printf("blue offset: -1024 ~ 1023\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[4] = mode;
              if (scanf_ret != 5) break;

              test->cmdID = PQ_FACTORY_SET_WB_BLUE_OFFSET;
              break;
          }
          case 328: {
              printf("PQ_FACTORY_GET_WB_BLUE_OFFSET\n");
              printf("source: INVALID(-1), TV(0), AV1(1), AV2(2), YPBPR1(3), YPBPR2(4), HDMI1(5), HDMI2(6), HDMI3(7), HDMI4(8), VGA(9), MPEG(10), DTV(11), SVIDEO(12), IPTV(13), DUMMY(14), SPDIF(15), ADTV(16)\n");
              scanf_ret = scanf("%d", &source);
              test->setValue[0] = source;

              printf("sig_fmt: TVIN_SIG_FMT_NULL(0) ~ TVIN_SIG_FMT_MAX(0xc01)\n");
              scanf_ret += scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;

              printf("fmt_3d: TVIN_TFMT_2D(0) ~ TVIN_TFMT_3D_DET_CHESSBOARD(15)\n");
              scanf_ret += scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret += scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;
              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_GET_WB_BLUE_OFFSET;
              break;
          }
          case 335: {
              int en = 0, rgain = 0, ggain = 0, bgain = 0, roffset = 0, goffset = 0, boffset = 0;
              printf("PQ_FACTORY_SET_COLOR_PARAMS\n");
              printf("ColorTemperature mode :(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              printf("en: 0 ~ 1\n");
              scanf_ret += scanf("%d", &en);
              test->setValue[1] = en;

              printf("rgain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &rgain);
              test->setValue[2] = rgain;

              printf("ggain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &ggain);
              test->setValue[3] = ggain;

              printf("bgain: 0 ~ 2047\n");
              scanf_ret += scanf("%d", &bgain);
              test->setValue[4] = bgain;

              printf("roffset: -1023 ~ 1024\n");
              scanf_ret += scanf("%d", &roffset);
              test->setValue[5] = roffset;

              printf("goffset: -1023 ~ 1024\n");
              scanf_ret += scanf("%d", &goffset);
              test->setValue[6] = goffset;

              printf("boffset: -1023 ~ 1024\n");
              scanf_ret += scanf("%d", &boffset);
              test->setValue[7] = boffset;

              if (scanf_ret != 8) break;

              test->cmdID = PQ_FACTORY_SET_COLOR_PARAMS;
              break;
          }
          case 336: {
              printf("PQ_FACTORY_GET_COLOR_PARAMS\n");
              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret = scanf("%d", &mode);
              test->setValue[0] = mode;

              if (scanf_ret != 1) break;

              test->cmdID = PQ_FACTORY_GET_COLOR_PARAMS;
            break;
          }

          case 343: {
              int offset = 0;
              int level = 0;
              printf("PQ_FACTORY_SET_WB_GAMMA_DATA\n");
              printf("ColorTemperature mode value:(0:Sandard, 1:warm, 2:cool, 3:user, 4:warmer, 5:cooler)\n");
              scanf_ret = scanf("%d", &level);
              test->setValue[0] = level;

              printf("color: RED CHANNEL(0), GREEN CHANNEL(1), BLUE CHANNEL(2)\n");
              scanf_ret += scanf("%d", &color);
              test->setValue[1] = color;

              printf("mode: 10 percent per step(0 per(0) ~ 100 per(11)\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[2] = mode;

              printf("offset: -1023 ~ 1023\n");
              scanf_ret += scanf("%d", &offset);
              test->setValue[3] = offset;

              if (scanf_ret != 4) break;

              test->cmdID = PQ_FACTORY_SET_WB_GAMMA_DATA;
              break;
          }
          case 344: {
              int level = 0;
              printf("PQ_FACTORY_GET_WB_GAMMA_DATA\n");
              printf("ColorTemperature mode value:(0: Sandard, 1: warm, 2: cool, 3: user, 4: warmer, 5: cooler)\n");
              scanf_ret = scanf("%d", &level);
              test->setValue[0] = level;

              printf("color: RED CHANNEL(0), GREEN CHANNEL(1), BLUE CHANNEL(2)\n");
              scanf_ret += scanf("%d", &color);
              test->setValue[1] = color;

              printf("mode: 10 percent per step(0 per(0) ~ 100 per(11)\n");
              scanf_ret += scanf("%d", &mode);
              test->setValue[2] = mode;

              if (scanf_ret != 3) break;

              test->cmdID = PQ_FACTORY_GET_WB_GAMMA_DATA;
            break;
          }
          default: {
              test->cmdID = PQ_MODULE_CMD_MAX;
              run = 0;
              break;
          }
        }

        test->SendCmd();

        printf("#### please input cmd again####\n");

        fflush (stdout);
    }

    return 0;
}
