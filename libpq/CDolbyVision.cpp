/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "CDolbyVision"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "CPQLog.h"
#include "CDolbyVision.h"

CDolbyVision::CDolbyVision(const char *binFilePath, const char *cfgFilePath) {
    //open dolby vision module
    mDVDevFd = DV_OpenModule();

    //load dv cfg file
    SetDolbyCfgFile(binFilePath, cfgFilePath);
}

CDolbyVision::~CDolbyVision() {
    DV_CloseModule();
}

int CDolbyVision::DV_OpenModule()
{
    int fd = open(DV_DEV_PATH, O_RDWR);

    if ( fd < 0 ) {
        LOGE("Open %s error(%s)!\n", DV_DEV_PATH, strerror(errno));
        return -1;
    } else {
        LOGD("Open dobly_vision module success!\n");
    }

    return fd;
}

int CDolbyVision::DV_CloseModule()
{
    if (mDVDevFd != -1) {
        close ( mDVDevFd );
        mDVDevFd = -1;
    }

    return 0;
}

int CDolbyVision::DV_DeviceIOCtl( int request, ... )
{
    int ret = -1;
    va_list ap;
    void *arg;

    if (mDVDevFd < 0) {
        mDVDevFd = DV_OpenModule();
    }

    if (mDVDevFd >= 0) {
        va_start(ap, request);
        arg = va_arg(ap, void *);
        va_end(ap);

        ret = ioctl(mDVDevFd, request, arg );
        if (ret < 0) {
            LOGD("%s failed(%s)!\n", __FUNCTION__, strerror(errno));
        } else {
            LOGD("%s success!\n", __FUNCTION__);
        }
    }

    return ret;
}

int CDolbyVision::SetDolbyCfgFile(const char *binFilePath, const char *cfgFilePath) {
    dolby_config_file_t dolbyCfgFile;
    memset(&dolbyCfgFile, 0, sizeof(dolby_config_file_t));

    char filePath0[256] = {0};
    sprintf(filePath0, "%s/dv_config.bin", binFilePath);
    strncpy((char*)dolbyCfgFile.bin_name, filePath0, 255);

    char filePath1[256] = {0};
    sprintf(filePath1, "%s/Amlogic_dv.cfg", cfgFilePath);
    strncpy((char*)dolbyCfgFile.cfg_name, filePath1, 255);

    LOGD("%s: bin_name=%s, cfg_name=%s.\n", __FUNCTION__, dolbyCfgFile.bin_name, dolbyCfgFile.cfg_name);

    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_CONFIG_FILE, &dolbyCfgFile);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::SetDolbyPQMode(dolby_pq_mode_t mode) {
    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_PIC_MODE_ID, &mode);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;

}

dolby_pq_mode_t CDolbyVision::GetDolbyPQMode(void) {
    dolby_pq_mode_t dolbyPQmode = DOLBY_PQ_MODE_BRIGHT_DV;

    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_PIC_MODE_ID, &dolbyPQmode);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
        dolbyPQmode = DOLBY_PQ_MODE_BRIGHT_DV;
    } else {
        LOGD("%s success, mode is %d!\n", __FUNCTION__, dolbyPQmode);
    }

    return dolbyPQmode;
}

int CDolbyVision::SetDolbyPQParam(dolby_pq_mode_t mode, dolby_pq_item_t iteamID, int value) {
    dolby_pq_info_t dolbyPQInfo;
    memset(&dolbyPQInfo, 0, sizeof(dolbyPQInfo));
    dolbyPQInfo.dolby_pic_mode_id = mode;
    dolbyPQInfo.item              = iteamID;
    dolbyPQInfo.value             = value;

    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_SINGLE_PQ_VALUE, &dolbyPQInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::GetDolbyPQParam(dolby_pq_mode_t mode, dolby_pq_item_t iteamID) {
    dolby_pq_info_t dolbyPQInfo;
    memset(&dolbyPQInfo, 0, sizeof(dolbyPQInfo));
    dolbyPQInfo.dolby_pic_mode_id = mode;
    dolbyPQInfo.item              = iteamID;

    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_SINGLE_PQ_VALUE, &dolbyPQInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::SetDolbyPQFullParam(dolby_full_pq_info_t fullInfo) {
    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_FULL_PQ_VALUE, &fullInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::GetDolbyPQFullParam(dolby_full_pq_info_t *fullInfo) {
    int ret = 0;
    dolby_full_pq_info_t dolbyPQFullInfo;
    memset(&dolbyPQFullInfo, 0, sizeof(dolbyPQFullInfo));
    dolbyPQFullInfo.pic_mode_id = fullInfo->pic_mode_id;

    ret = DV_DeviceIOCtl(DV_IOC_GET_DV_FULL_PQ_VALUE, &dolbyPQFullInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success, brightness:%d, contrast:%d, saturation:%d, hue:%d.\n",
                 __FUNCTION__,
                 dolbyPQFullInfo.brightness,
                 dolbyPQFullInfo.contrast,
                 dolbyPQFullInfo.saturation,
                 dolbyPQFullInfo.colorshift);
    }

    memcpy(fullInfo, &dolbyPQFullInfo, sizeof(dolbyPQFullInfo));
    return ret;
}
