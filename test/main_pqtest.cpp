/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#define LOG_MOUDLE_TAG "PQ"
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
        LOGD("%s: cmd is %d\n", __FUNCTION__, cmdID);
        int ret = -1;
        switch (cmdID) {
        case PQ_SET_PICTURE_MODE:
            ret = mpPqClient->SetPQMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_PICTURE_MODE:
            ret = mpPqClient->GetPQMode();
            LOGD("%s: curent pq mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->SetColorTemperature(setValue[0], setValue[1], setValue[2], setValue[3]);
            break;
        case PQ_GET_COLOR_TEMPERATURE_MODE:
            ret = mpPqClient->GetColorTemperature();
            LOGD("%s: curent colortempture mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BRIGHTNESS:
            ret = mpPqClient->SetBrightness(setValue[0], setValue[1]);
            break;
        case PQ_GET_BRIGHTNESS:
            ret = mpPqClient->GetBrightness();
            LOGD("%s: curent Brightness value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_CONTRAST:
            ret = mpPqClient->SetContrast(setValue[0], setValue[1]);
            break;
        case PQ_GET_CONTRAST:
            ret = mpPqClient->GetContrast();
            LOGD("%s: curent contrast value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SATUATION:
            ret = mpPqClient->SetSaturation(setValue[0], setValue[1]);
            break;
        case PQ_GET_SATUATION:
            ret = mpPqClient->GetSaturation();
            LOGD("%s: curent saturation value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_HUE:
            ret = mpPqClient->SetHue(setValue[0], setValue[1]);
            break;
        case PQ_GET_HUE:
            ret = mpPqClient->GetHue();
            LOGD("%s: curent hue value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SHARPNESS:
            ret = mpPqClient->SetSharpness(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_SHARPNESS:
            ret = mpPqClient->GetSharpness();
            LOGD("%s: curent sharpness value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_NOISE_REDUCTION_MODE:
            ret = mpPqClient->SetNoiseReductionMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_NOISE_REDUCTION_MODE:
            ret = mpPqClient->GetNoiseReductionMode();
            LOGD("%s: curent NoiseReduction value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_EYE_PROTECTION_MODE:
            ret = mpPqClient->SetEyeProtectionMode(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_EYE_PROTECTION_MODE:
            ret = mpPqClient->GetEyeProtectionMode();
            LOGD("%s: curent Protection mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_GAMMA:
            ret = mpPqClient->SetGammaValue(setValue[0], setValue[1]);
            break;
        case PQ_GET_GAMMA:
            ret = mpPqClient->GetGammaValue();
            LOGD("%s: curent Protection mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DISPLAY_MODE:
            ret = mpPqClient->SetDisplayMode(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_DISPLAY_MODE:
            ret = mpPqClient->GetDisplayMode();
            LOGD("%s: curent Display mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_BACKLIGHT:
            ret = mpPqClient->SetBacklight(setValue[0], setValue[1]);
            break;
        case PQ_GET_BACKLIGHT:
            ret = mpPqClient->GetBacklight();
            LOGD("%s: curent backlight value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_DYNAMICBACKLIGHT:
            ret = mpPqClient->SetDynamicBacklight(setValue[0], setValue[1]);
            break;
        case PQ_GET_DYNAMICBACKLIGHT:
            ret = mpPqClient->GetDynamicBacklight();
            LOGD("%s: curent backlight value is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_LOCALCONTRAST:
            ret = mpPqClient->SetLocalContrastMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_LOCALCONTRAST:
            ret = mpPqClient->GetLocalContrastMode();
            LOGD("%s: curent Local contrast mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_CM:
            ret = mpPqClient->SetColorBaseMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_CM:
            ret = mpPqClient->GetColorBaseMode();
            LOGD("%s: curent Color base mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_SET_SOURCE_CHANNEL:
            ret = mpPqClient->setCurrentSourceInfo(setValue[0], setValue[1], setValue[2]);
            break;
        case PQ_GET_SOURCE_CHANNEL:
            source_input_param_t source_param;
            source_param = mpPqClient->getCurrentSourceInfo();
            LOGD("%s: curent source info: source_input:%d sig_fmt:%d trans_fmt:%d.\n",
                __FUNCTION__, source_param.source_input, source_param.sig_fmt, source_param.trans_fmt);
            break;
        case PQ_SET_COLORGAMUT:
            ret = mpPqClient->SetColorBaseMode(setValue[0], setValue[1]);
            break;
        case PQ_GET_COLORGAMUT:
            ret = mpPqClient->GetColorBaseMode();
            LOGD("%s: curent Color base mode is %d.\n", __FUNCTION__, ret);
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
            LOGD("%s: curent colortempture mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_BRIGHTNESS:
            ret = mpPqClient->FactorySetPQMode_Brightness(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_BRIGHTNESS:
            ret = mpPqClient->FactoryGetPQMode_Brightness(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: curent brightness mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_CONTRAST:
            ret = mpPqClient->FactorySetPQMode_Contrast(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_CONTRAST:
            ret = mpPqClient->FactoryGetPQMode_Contrast(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: curent contrast mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_SATUATION:
            ret = mpPqClient->FactorySetPQMode_Saturation(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_SATUATION:
            ret = mpPqClient->FactoryGetPQMode_Saturation(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: curent saturation mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_HUE:
            ret = mpPqClient->FactorySetPQMode_Hue(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_HUE:
            ret = mpPqClient->FactoryGetPQMode_Hue(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: curent hue mode is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_SHARPNESS:
            ret = mpPqClient->FactorySetPQMode_Sharpness(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_SHARPNESS:
            ret = mpPqClient->FactoryGetPQMode_Sharpness(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: curent sharpness mode is %d.\n", __FUNCTION__, ret);
            break;
        case PQ_FACTORY_SET_OVERSCAN:
            tvin_cutwin_t overscanParam;
            memset(&overscanParam, 0, sizeof(tvin_cutwin_t));
            overscanParam.he = (unsigned short)setValue[3];
            overscanParam.hs = (unsigned short)setValue[4];
            overscanParam.ve = (unsigned short)setValue[5];
            overscanParam.vs = (unsigned short)setValue[6];
            ret = mpPqClient->FactorySetOverscanParams(setValue[0], setValue[1], setValue[2], overscanParam);
            break;
        case PQ_FACTORY_GET_OVERSCAN:
            tvin_cutwin_t cutwin_t;
            cutwin_t = mpPqClient->FactoryGetOverscanParams(setValue[0], setValue[1], setValue[2]);
            LOGD("%s: cutwin_t: he:%d hs:%d ve:%d vs:%d.\n",
                __FUNCTION__, cutwin_t.he, cutwin_t.hs, cutwin_t.ve, cutwin_t.vs);
            break;
        case PQ_FACTORY_SET_WB_RED_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceRedGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;
        case PQ_FACTORY_GET_WB_RED_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceRedGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: red gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_GREEN_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceRedGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_GREEN_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceGreenGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: green gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_BLUE_GAIN:
            ret = mpPqClient->FactorySetWhiteBalanceBlueGain(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_BLUE_GAIN:
            ret = mpPqClient->FactoryGetWhiteBalanceBlueGain(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: blue gain is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_RED_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceRedPostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_RED_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceRedPostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: red offset is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_GREEN_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceGreenPostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_GREEN_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceGreenPostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: gree offset is %d.\n", __FUNCTION__, ret);
            break;

        case PQ_FACTORY_SET_WB_BLUE_OFFSET:
            ret = mpPqClient->FactorySetWhiteBalanceBluePostOffset(setValue[0], setValue[1], setValue[2], setValue[3], setValue[4]);
            break;

        case PQ_FACTORY_GET_WB_BLUE_OFFSET:
            ret = mpPqClient->FactoryGetWhiteBalanceBluePostOffset(setValue[0], setValue[1], setValue[2], setValue[3]);
            LOGD("%s: blue offset is %d.\n", __FUNCTION__, ret);
            break;

        default:
            break;
            }

        return 0;
    }

    PqClient *mpPqClient;
    pqcmd_t cmdID = PQ_MOUDLE_CMD_MAX;
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

    LOGD("#### please select cmd####\n");
    LOGD("#### select 201 to set pq mode ####\n");
    LOGD("#### select 202 to get pq mode ####\n");
    LOGD("#### select 203 to set ColorTemperature mode ####\n");
    LOGD("#### select 204 to get ColorTemperature mode ####\n");
    LOGD("#### select 205 to set Brightness ####\n");
    LOGD("#### select 206 to get Brightness ####\n");
    LOGD("#### select 207 to set Contrast ####\n");
    LOGD("#### select 208 to get Contrast ####\n");
    LOGD("#### select 209 to set Saturatioin ####\n");
    LOGD("#### select 210 to get Saturatioin ####\n");
    LOGD("#### select 211 to set Hue ####\n");
    LOGD("#### select 212 to get Hue ####\n");
    LOGD("#### select 213 to set Sharpness ####\n");
    LOGD("#### select 214 to get Sharpness ####\n");
    LOGD("#### select 215 to set NoiseRedution ####\n");
    LOGD("#### select 216 to get NoiseRedution ####\n");
    LOGD("#### select 217 to set EyeProtection ####\n");
    LOGD("#### select 218 to get EyeProtection ####\n");
    LOGD("#### select 219 to set Gamma ####\n");
    LOGD("#### select 220 to get Gamma ####\n");
    LOGD("#### select 221 to set displaymode ####\n");
    LOGD("#### select 222 to get displaymode ####\n");
    LOGD("#### select 223 to set backlight ####\n");
    LOGD("#### select 224 to get backlight ####\n");
    LOGD("#### select 225 to set DynamicBacklight ####\n");
    LOGD("#### select 226 to get DynamicBacklight ####\n");
    LOGD("#### select 227 to set LocalContrast ####\n");
    LOGD("#### select 228 to get LocalContrast ####\n");
    LOGD("#### select 229 to set CM ####\n");
    LOGD("#### select 230 to get CM ####\n");
    LOGD("#### select 231 to set source channel ####\n");
    LOGD("#### select 232 to get source channel ####\n");
    LOGD("#### select 233 to set ColorGamut ####\n");
    LOGD("#### select 234 to get ColorGamut ####\n");

    LOGD("#### below is factory cmd####\n");
    LOGD("#### select 301 to reset pq mode ####\n");
    LOGD("#### select 302 to reset ColorTemperature ####\n");
    LOGD("#### select 303 to set ColorTemperature ####\n");
    LOGD("#### select 304 to get ColorTemperature ####\n");
    LOGD("#### select 305 to set Brightness ####\n");
    LOGD("#### select 306 to get Brightness ####\n");
    LOGD("#### select 307 to set Contrast ####\n");
    LOGD("#### select 308 to get Contrast ####\n");
    LOGD("#### select 309 to set Saturatioin ####\n");
    LOGD("#### select 310 to get Saturatioin ####\n");
    LOGD("#### select 311 to set Hue ####\n");
    LOGD("#### select 312 to get Hue ####\n");
    LOGD("#### select 313 to set Sharpness ####\n");
    LOGD("#### select 314 to get Sharpness ####\n");
    LOGD("#### select 315 to set overscan ####\n");
    LOGD("#### select 316 to get overscan ####\n");
    LOGD("#### select 317 to set WB R Gain ####\n");
    LOGD("#### select 318 to get WB R Gain ####\n");
    LOGD("#### select 319 to set WB G Gain ####\n");
    LOGD("#### select 320 to get WB G Gain ####\n");
    LOGD("#### select 321 to set WB B Gain ####\n");
    LOGD("#### select 322 to get WB B Gain ####\n");
    LOGD("#### select 323 to set WB R OFFSET ####\n");
    LOGD("#### select 324 to get WB R OFFSET ####\n");
    LOGD("#### select 325 to set WB G OFFSET ####\n");
    LOGD("#### select 326 to get WB G OFFSET ####\n");
    LOGD("#### select 327 to set WB B OFFSET ####\n");
    LOGD("#### select 328 to get WB B OFFSET ####\n");

    LOGD("#### select 299 to exit####\n");
    LOGD("##########################\n");
    while (run) {
        char Command[10];
        int conmmand;

        scanf("%s", Command);
        conmmand = atoi(Command);
        LOGD("#### Command: %s %d\n", Command, conmmand);

        switch (conmmand) {
          case 299: {
            test->cmdID = PQ_MOUDLE_CMD_MAX;
            run = 0;
            break;
          }
          case 201: {
              LOGD("please input pq_mode value:(0~10)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_PICTURE_MODE;
              break;
          }
          case 202: {
              test->cmdID = PQ_GET_PICTURE_MODE;
              break;
          }
          case 203: {
              LOGD("please input ColorTemperature mode value:(0~3)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              LOGD("please input rgb_ogo_type_t:(0~5)(-1:only change ColorTemperature mode)\n");
              int rgb_type = 0;
              scanf("%d", &rgb_type);
              test->setValue[2] = rgb_type;

              LOGD("please input value:(0~5)\n");
              int value = 0;
              scanf("%d", &value);
              test->setValue[3] = value;

              test->cmdID = PQ_SET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 204: {
              test->cmdID = PQ_GET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 205: {
              LOGD("please input Brightness value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_BRIGHTNESS;
              break;
          }
          case 206: {
              test->cmdID = PQ_GET_BRIGHTNESS;
              break;
          }
          case 207: {
              LOGD("please input Contrast value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_CONTRAST;
              break;
          }
          case 208: {
              test->cmdID = PQ_GET_CONTRAST;
              break;
          }
          case 209: {
              LOGD("please input Saturatioin value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_SATUATION;
              break;
          }
          case 210: {
              test->cmdID = PQ_GET_SATUATION;
              break;
          }
          case 211: {
              LOGD("please input Hue value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_HUE;
              break;
          }
          case 212: {
              test->cmdID = PQ_GET_HUE;
              break;
          }
          case 213: {
              LOGD("please input Sharpness value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[2] = is_save;

              test->cmdID = PQ_SET_SHARPNESS;
              break;
          }
          case 214: {
              test->cmdID = PQ_GET_SHARPNESS;
              break;
          }
          case 215: {
              LOGD("please input NoiseRedution value:(0~4)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_NOISE_REDUCTION_MODE;
              break;
          }
          case 216: {
              test->cmdID = PQ_GET_NOISE_REDUCTION_MODE;
              break;
          }
          case 217: {
              LOGD("please input EyeProtection value:(0~1)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[1] = mode;

              test->cmdID = PQ_SET_EYE_PROTECTION_MODE;
              break;
          }
          case 218: {
              test->cmdID = PQ_GET_EYE_PROTECTION_MODE;
              break;
          }
          case 219: {
              LOGD("please input Gamma value:(0~11)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_GAMMA;
              break;
          }
          case 220: {
              test->cmdID = PQ_GET_GAMMA;
              break;
          }
          case 221: {
              LOGD("please input displaymode value:(0/4/5/6)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[1] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[2] = is_save;

              test->cmdID = PQ_SET_DISPLAY_MODE;
              break;
          }
          case 222: {
              test->cmdID = PQ_GET_DISPLAY_MODE;
              break;
          }
          case 223: {
              LOGD("please input backlight value:(0~100)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_BACKLIGHT;
              break;
          }
          case 224: {
              test->cmdID = PQ_GET_BACKLIGHT;
              break;
          }
          case 225: {
              LOGD("please input DynamicBacklight value:(0~2)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_DYNAMICBACKLIGHT;
              break;
          }
          case 226: {
              test->cmdID = PQ_GET_DYNAMICBACKLIGHT;
              break;
          }
          case 227: {
              LOGD("please input LocalContrast value:(0~3)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_LOCALCONTRAST;
              break;
          }
          case 228: {
              test->cmdID = PQ_GET_LOCALCONTRAST;
              break;
          }
          case 229: {
              LOGD("please input CM value:(0~3)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_CM;
              break;
          }
          case 230: {
              test->cmdID = PQ_GET_CM;
              break;
          }
          case 231: {
              LOGD("please input source value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              test->cmdID = PQ_SET_SOURCE_CHANNEL;
              break;
          }
          case 232: {
              test->cmdID = PQ_GET_SOURCE_CHANNEL;
              break;
          }
          case 233: {
              LOGD("please input ColorGamut value:(0~2)\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[0] = mode;

              LOGD("please input is save:(0~1)\n");
              int is_save = 0;
              scanf("%d", &is_save);
              test->setValue[1] = is_save;

              test->cmdID = PQ_SET_COLORGAMUT;
              break;
          }
          case 234: {
              test->cmdID = PQ_GET_COLORGAMUT;
              break;
          }

          case 301: {
              test->cmdID = PQ_FACTORY_RESET_PICTURE_MODE;
              break;
          }
          case 302: {
              test->cmdID = PQ_FACTORY_RESET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 303: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input color mode value:\n");
              int color_mode = 0;
              scanf("%d", &color_mode);
              test->setValue[3] = color_mode;

              test->cmdID = PQ_FACTORY_SET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 304: {
              test->cmdID = PQ_FACTORY_GET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 305: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              LOGD("please input brightness mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_BRIGHTNESS;
              break;
          }
          case 306: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              test->cmdID = PQ_FACTORY_GET_BRIGHTNESS;
              break;
          }
          case 307: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;


              test->cmdID = PQ_FACTORY_SET_CONTRAST;
              break;
          }
          case 308: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              test->cmdID = PQ_FACTORY_GET_CONTRAST;
              break;
          }
          case 309: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;
              LOGD("please input saturation mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[3] = mode;

              test->cmdID = PQ_FACTORY_SET_SATUATION;
              break;
          }
          case 310: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              test->cmdID = PQ_FACTORY_GET_SATUATION;
              break;
          }
          case 311: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              LOGD("please input hue mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[3] = mode;

              test->cmdID = PQ_FACTORY_SET_HUE;
              break;
          }
          case 312: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              test->cmdID = PQ_FACTORY_GET_HUE;
              break;
          }
          case 313: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              LOGD("please input sharpness mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[3] = mode;

              test->cmdID = PQ_FACTORY_SET_SHARPNESS;
              break;
          }
          case 314: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input pq mode value:(0~10)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              test->cmdID = PQ_FACTORY_GET_SHARPNESS;
              break;
          }
          case 315: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              LOGD("please input he:\n");
              int he = 0;
              scanf("%d", &he);
              test->setValue[3] = he;
              LOGD("please input hs:\n");
              int hs = 0;
              scanf("%d", &hs);
              test->setValue[4] = hs;
              LOGD("please input ve:\n");
              int ve = 0;
              scanf("%d", &ve);
              test->setValue[5] = ve;
              LOGD("please input vs:\n");
              int vs = 0;
              scanf("%d", &vs);
              test->setValue[6] = vs;

              test->cmdID = PQ_FACTORY_SET_OVERSCAN;
              break;
          }
          case 316: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;

              test->cmdID = PQ_FACTORY_GET_OVERSCAN;
              break;
          }
          case 317: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int pq_mode = 0;
              scanf("%d", &pq_mode);
              test->setValue[3] = pq_mode;

              LOGD("please input red gain value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_RED_GAIN;
              break;
          }
          case 318: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_RED_GAIN;
              break;
          }
          case 319: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              LOGD("please input green gain value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_GREEN_GAIN;
              break;
          }
          case 320: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_GREEN_GAIN;
              break;
          }
          case 321: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              LOGD("please input blue gain value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_BLUE_GAIN;
              break;
          }
          case 322: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_BLUE_GAIN;
              break;
          }
          case 323: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              LOGD("please input red offset value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_RED_OFFSET;
              break;
          }
          case 324: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_RED_OFFSET;
              break;
          }
          case 325: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              LOGD("please input green offset value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_GREEN_OFFSET;
              break;
          }
          case 326: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_GREEN_OFFSET;
              break;
          }
          case 327: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              LOGD("please input blue offset value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->setValue[4] = mode;

              test->cmdID = PQ_FACTORY_SET_WB_BLUE_OFFSET;
              break;
          }
          case 328: {
              LOGD("please input source value:\n");
              int source = 0;
              scanf("%d", &source);
              test->setValue[0] = source;
              LOGD("please input sig_fmt value:\n");
              int sig_fmt = 0;
              scanf("%d", &sig_fmt);
              test->setValue[1] = sig_fmt;
              LOGD("please input 3d_fmt value:\n");
              int fmt_3d = 0;
              scanf("%d", &fmt_3d);
              test->setValue[2] = fmt_3d;
              LOGD("please input colortemp_mode:(0~3)\n");
              int colortemp_mode = 0;
              scanf("%d", &colortemp_mode);
              test->setValue[3] = colortemp_mode;

              test->cmdID = PQ_FACTORY_GET_WB_BLUE_OFFSET;
              break;
          }

          default: {
              test->cmdID = PQ_MOUDLE_CMD_MAX;
              run = 0;
              break;
          }
        }

        test->SendCmd();

        LOGD("#### please input cmd again####\n");

        fflush (stdout);
    }

    return 0;
}
