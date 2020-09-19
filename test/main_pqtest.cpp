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
    int fd;
    char  bcmd[16];
    fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0777);

    if (fd >= 0) {
        sprintf(bcmd,"%d",value);
        write(fd,bcmd,strlen(bcmd));
        close(fd);
        return 0;
    }

    return -1;
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
        LOGD("%s: cmd is %d.\n", __FUNCTION__, cmdID);
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
            ret = mpPqClient->SetColorTemperature(setValue[0], setValue[1]);
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
#if (TV_IPC_TYPE == TV_BINDER)
    sp<ProcessState> proc(ProcessState::self());
    proc->startThreadPool();
#endif

    PqTest *test = new PqTest();
    int run = 1;
    DisplayInit();

    LOGD("#### please select cmd####\n");
    LOGD("#### select 1 to set pq mode ####\n");
    LOGD("#### select 2 to get pq mode ####\n");
    LOGD("#### select 3 to set ColorTemperature mode ####\n");
    LOGD("#### select 4 to get ColorTemperature mode ####\n");
    LOGD("#### select 5 to set Brightness ####\n");
    LOGD("#### select 6 to get Brightness ####\n");
    LOGD("#### select 7 to set Contrast ####\n");
    LOGD("#### select 8 to get Contrast ####\n");
    LOGD("#### select 9 to set Saturatioin ####\n");
    LOGD("#### select 10 to get Saturatioin ####\n");
    LOGD("#### select 11 to set Hue ####\n");
    LOGD("#### select 12 to get Hue ####\n");
    LOGD("#### select 13 to set Sharpness ####\n");
    LOGD("#### select 14 to get Sharpness ####\n");
    LOGD("#### select 15 to set NoiseRedution ####\n");
    LOGD("#### select 16 to get NoiseRedution ####\n");
    LOGD("#### select 17 to set EyeProtection ####\n");
    LOGD("#### select 18 to get EyeProtection ####\n");
    LOGD("#### select 19 to set Gamma ####\n");
    LOGD("#### select 20 to get Gamma ####\n");
    LOGD("#### select 21 to set displaymode ####\n");
    LOGD("#### select 22 to get displaymode ####\n");
    LOGD("#### select 23 to set backlight ####\n");
    LOGD("#### select 24 to get backlight ####\n");
    LOGD("#### select 25 to set DynamicBacklight ####\n");
    LOGD("#### select 26 to get DynamicBacklight ####\n");
    LOGD("#### select 27 to set LocalContrast ####\n");
    LOGD("#### select 28 to get LocalContrast ####\n");
    LOGD("#### select 29 to set CM ####\n");
    LOGD("#### select 30 to get CM ####\n");
    LOGD("#### select 31 to set source channel ####\n");
    LOGD("#### select 32 to get source channel ####\n");

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
          case 1: {
              LOGD("please input pq_mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_PICTURE_MODE;
              test->setValue[0] = mode;
              break;
          }
          case 2: {
              test->cmdID = PQ_GET_PICTURE_MODE;
              break;
          }
          case 3: {
              LOGD("please input ColorTemperature mode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_COLOR_TEMPERATURE_MODE;
              test->setValue[0] = mode;
              break;
          }
          case 4: {
              test->cmdID = PQ_GET_COLOR_TEMPERATURE_MODE;
              break;
          }
          case 5: {
              LOGD("please input Brightness value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_BRIGHTNESS;
              test->setValue[0] = mode;
              break;
          }
          case 6: {
              test->cmdID = PQ_GET_BRIGHTNESS;
              break;
          }
          case 7: {
              LOGD("please input Contrast value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_CONTRAST;
              test->setValue[0] = mode;
              break;
          }
          case 8: {
              test->cmdID = PQ_GET_CONTRAST;
              break;
          }
          case 9: {
              LOGD("please input Saturatioin value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_SATUATION;
              test->setValue[0] = mode;
              break;
          }
          case 10: {
              test->cmdID = PQ_GET_SATUATION;
              break;
          }
          case 11: {
              LOGD("please input Hue value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_HUE;
              test->setValue[0] = mode;
              break;
          }
          case 12: {
              test->cmdID = PQ_GET_HUE;
              break;
          }
          case 13: {
              LOGD("please input Sharpness value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_SHARPNESS;
              test->setValue[0] = mode;
              break;
          }
          case 14: {
              test->cmdID = PQ_GET_SHARPNESS;
              break;
          }
          case 15: {
              LOGD("please input NoiseRedution value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_NOISE_REDUCTION_MODE;
              test->setValue[0] = mode;
              break;
          }
          case 16: {
              test->cmdID = PQ_GET_NOISE_REDUCTION_MODE;
              break;
          }
          case 17: {
              LOGD("please input EyeProtection value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_EYE_PROTECTION_MODE;
              test->setValue[1] = mode;
              break;
          }
          case 18: {
              test->cmdID = PQ_GET_EYE_PROTECTION_MODE;
              break;
          }
          case 19: {
              LOGD("please input Gamma value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_GAMMA;
              test->setValue[0] = mode;
              break;
          }
          case 20: {
              test->cmdID = PQ_GET_GAMMA;
              break;
          }
          case 21: {
              LOGD("please input displaymode value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_DISPLAY_MODE;
              test->setValue[1] = mode;
              break;
          }
          case 22: {
              test->cmdID = PQ_GET_DISPLAY_MODE;
              break;
          }
          case 23: {
              LOGD("please input backlight value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_BACKLIGHT;
              test->setValue[0] = mode;
              break;
          }
          case 24: {
              test->cmdID = PQ_GET_BACKLIGHT;
              break;
          }
          case 25: {
              LOGD("please input DynamicBacklight value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_DYNAMICBACKLIGHT;
              test->setValue[0] = mode;
              break;
          }
          case 26: {
              test->cmdID = PQ_GET_DYNAMICBACKLIGHT;
              break;
          }
          case 27: {
              LOGD("please input LocalContrast value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_LOCALCONTRAST;
              test->setValue[0] = mode;
              break;
          }
          case 28: {
              test->cmdID = PQ_GET_LOCALCONTRAST;
              break;
          }
          case 29: {
              LOGD("please input CM value:\n");
              int mode = 0;
              scanf("%d", &mode);
              test->cmdID = PQ_SET_CM;
              test->setValue[0] = mode;
              break;
          }
          case 30: {
              test->cmdID = PQ_GET_CM;
              break;
          }
          case 31: {
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
          case 32: {
              test->cmdID = PQ_GET_SOURCE_CHANNEL;
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
