/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "CDolbyVision"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "CPQLog.h"
#include "CAmdolbyVision.h"

CDolbyVision::CDolbyVision(const char *binFilePath, const char *cfgFilePath) {
    //open dolby vision module
    mDVDevFd = DV_OpenModule();

    //load dv cfg file
    SetAmdolbyCfgFile(binFilePath, cfgFilePath);
}

CDolbyVision::~CDolbyVision() {
    DV_CloseModule();
}

int CDolbyVision::ReadSysfs(const char *path, char *buf, int count)
{
    int fd, len;

    if ( NULL == buf ) {
        LOGE("%s buf is NULL.\n", __FUNCTION__);
        return -1;
    }

    if ((fd = open(path, O_RDONLY)) < 0) {
        LOGE("%s open %s error(%s).\n", __FUNCTION__, path, strerror (errno));
        return -1;
    }

    len = read(fd, buf, count);
    if (len < 0) {
        LOGE("%s read %s error(%s).\n", __FUNCTION__, path, strerror(errno));
        close(fd);
        return len;
    }

    int i , j;
    for (i = 0, j = 0; i <= len -1; i++) {
        //change '\0' to 0x20(spacing), otherwise the string buffer will be cut off ,if the last char is '\0' should not replace it
        if (0x0 == buf[i] && i < len - 1) {
            buf[i] = 0x20;
        }
        /* delete all the character of '\n' */
        if (0x0a != buf[i]) {
            buf[j++] = buf[i];
        }
    }
    buf[j] = 0x0;

    close(fd);
    return len;
}

int CDolbyVision::DV_OpenModule()
{
    int fd = open(DV_DEV_PATH, O_RDWR);

    if ( fd < 0 ) {
        LOGE("Open %s error(%s)!\n", DV_DEV_PATH, strerror(errno));
        return -1;
    } else {
        LOGD("Open amdolby_vision module success!\n");
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

int CDolbyVision::SetAmdolbyCfgFile(const char *binFilePath, const char *cfgFilePath) {
    struct dv_config_file_s amdolbyCfgFile;
    memset(&amdolbyCfgFile, 0, sizeof(struct dv_config_file_s));
    if ((binFilePath == NULL) || (cfgFilePath == NULL)) {
        LOGD("%s: file path invalid\n", __FUNCTION__);
        return -1;
    } else {
        strncpy((char*)amdolbyCfgFile.bin_name, binFilePath, 255);
        strncpy((char*)amdolbyCfgFile.cfg_name, cfgFilePath, 255);
    }

    LOGD("%s: bin_name=%s, cfg_name=%s.\n", __FUNCTION__, amdolbyCfgFile.bin_name, amdolbyCfgFile.cfg_name);

    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_CONFIG_FILE, &amdolbyCfgFile);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::SetAmdolbyPQMode(dolby_pq_mode_t mode) {
    LOGD("%s mode is %d\n", __FUNCTION__, mode);

    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_PIC_MODE_ID, &mode);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

dolby_pq_mode_t CDolbyVision::GetAmdolbyPQMode(void) {
    dolby_pq_mode_t amdolbyPQmode = DOLBY_PQ_MODE_BRIGHT_DV;

    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_PIC_MODE_ID, &amdolbyPQmode);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
        amdolbyPQmode = DOLBY_PQ_MODE_BRIGHT_DV;
    } else {
        LOGD("%s success, mode is %d!\n", __FUNCTION__, amdolbyPQmode);
    }

    return amdolbyPQmode;
}

int CDolbyVision::SetAmdolbyPQParam(dolby_pq_mode_t mode, enum pq_item_e iteamID, int value) {
    struct dv_pq_info_s amdolbyPQInfo;
    memset(&amdolbyPQInfo, 0, sizeof(amdolbyPQInfo));
    amdolbyPQInfo.pic_mode_id = mode;
    amdolbyPQInfo.item              = iteamID;
    amdolbyPQInfo.value             = value;

    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_SINGLE_PQ_VALUE, &amdolbyPQInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::GetAmdolbyPQParam(dolby_pq_mode_t mode, enum pq_item_e iteamID) {
    struct dv_pq_info_s amdolbyPQInfo;
    memset(&amdolbyPQInfo, 0, sizeof(amdolbyPQInfo));
    amdolbyPQInfo.pic_mode_id = mode;
    amdolbyPQInfo.item              = iteamID;

    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_SINGLE_PQ_VALUE, &amdolbyPQInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::SetAmdolbyPQFullParam(struct dv_full_pq_info_s fullInfo) {
    int ret = DV_DeviceIOCtl(DV_IOC_GET_DV_FULL_PQ_VALUE, &fullInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CDolbyVision::GetAmdolbyPQFullParam(struct dv_full_pq_info_s *fullInfo) {
    int ret = 0;
    struct dv_full_pq_info_s amdolbyPQFullInfo;
    memset(&amdolbyPQFullInfo, 0, sizeof(amdolbyPQFullInfo));
    amdolbyPQFullInfo.pic_mode_id = fullInfo->pic_mode_id;

    ret = DV_DeviceIOCtl(DV_IOC_GET_DV_FULL_PQ_VALUE, &amdolbyPQFullInfo);
    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success, brightness:%d, contrast:%d, saturation:%d, hue:%d.\n",
                 __FUNCTION__,
                 amdolbyPQFullInfo.brightness,
                 amdolbyPQFullInfo.contrast,
                 amdolbyPQFullInfo.saturation,
                 amdolbyPQFullInfo.colorshift);
    }

    memcpy(fullInfo, &amdolbyPQFullInfo, sizeof(amdolbyPQFullInfo));
    return ret;
}

bool CDolbyVision::isSourceCallAmdolbyCore(enum hdr_type_e hdrType)
{
    bool ret = false;
    char temp[8];

    memset(temp, 0, sizeof(temp));
    int readRet = ReadSysfs(HDR10_POLICY_PATH, temp, sizeof(temp));

    LOGD("%s: hdr10PolicyValue is %s.\n", __FUNCTION__, temp);

    if (readRet > 0) {
        int hdr10PolicyValue = atoi(temp);
        switch (hdrType) {
        case HDRTYPE_HDR10:
            if (((hdr10PolicyValue & HDR_BY_DV_F_SINK) == HDR_BY_DV_F_SINK)
                || ((hdr10PolicyValue & HDR_BY_DV_F_SRC) == HDR_BY_DV_F_SRC)) {
                ret = true;
            }
            break;
        case HDRTYPE_HDR10PLUS:
            if ((hdr10PolicyValue & HDRP_BY_DV) == HDRP_BY_DV) {
                ret = true;
            }
            break;
        case HDRTYPE_HLG:
            if (((hdr10PolicyValue & HLG_BY_DV_F_SINK) == HLG_BY_DV_F_SINK)
                || ((hdr10PolicyValue & HLG_BY_DV_F_SRC) == HLG_BY_DV_F_SRC)) {
                ret = true;
            }
            break;
        case HDRTYPE_DOVI:
            ret = true;
            break;
        case HDRTYPE_SDR:
            if ((hdr10PolicyValue & SDR_BY_DV) == SDR_BY_DV) {
                ret = true;
            }
            break;
        case HDRTYPE_MVC:
        case HDRTYPE_PRIMESL:
        case HDRTYPE_NONE:
        default:
            ret = false;
            break;
        }

    } else {
        LOGD("%s: read %s failed!\n", __FUNCTION__, HDR10_POLICY_PATH);
        ret = false;
    }

    return ret;
}

dolby_pq_mode_t CDolbyVision::MappingPQModeToAmdolbyVisionPQMode(enum hdr_type_e hdrType, vpp_picture_mode_t pq_mode)
{
    dolby_pq_mode_t amdolbyMode = DOLBY_PQ_MODE_INVALID;
/*
    switch (hdrType) {
    case HDRTYPE_SDR:
        if (pq_mode == VPP_PICTURE_MODE_DYNAMIC) {
            amdolbyMode = DOLBY_PQ_MODE_DYNAMIC_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_STANDARD) {
            amdolbyMode = DOLBY_PQ_MODE_STANDARD_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_HDR) {
            amdolbyMode = DOLBY_PQ_MODE_DARK_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_NORMAL) {
            amdolbyMode = DOLBY_PQ_MODE_BRIGHT_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_PC) {
            amdolbyMode = DOLBY_PQ_MODE_PC_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_CUSTOMER) {
            amdolbyMode = DOLBY_PQ_MODE_CUSTOMER_SDR;
        } else if (pq_mode == VPP_PICTURE_MODE_STORE) {
            amdolbyMode = DOLBY_PQ_MODE_STORE_SDR;
        } else {
            LOGD("%s: not supported mode.\n", __FUNCTION__);
            amdolbyMode = DOLBY_PQ_MODE_MAX;
        }
        break;

    case HDRTYPE_HDR10:
        if (pq_mode == VPP_PICTURE_MODE_DYNAMIC) {
            amdolbyMode = DOLBY_PQ_MODE_DYNAMIC_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_STANDARD) {
            amdolbyMode = DOLBY_PQ_MODE_STANDARD_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_HDR) {
            amdolbyMode = DOLBY_PQ_MODE_DARK_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_NORMAL) {
            amdolbyMode = DOLBY_PQ_MODE_BRIGHT_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_PC) {
            amdolbyMode = DOLBY_PQ_MODE_PC_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_CUSTOMER) {
            amdolbyMode = DOLBY_PQ_MODE_CUSTOMER_HDR10;
        } else if (pq_mode == VPP_PICTURE_MODE_STORE) {
            amdolbyMode = DOLBY_PQ_MODE_STORE_HDR10;
        } else {
            LOGD("%s: not supported mode.\n", __FUNCTION__);
            amdolbyMode = DOLBY_PQ_MODE_MAX;
        }
        break;

    case HDRTYPE_HLG:
        if (pq_mode == VPP_PICTURE_MODE_DYNAMIC) {
            amdolbyMode = DOLBY_PQ_MODE_DYNAMIC_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_STANDARD) {
            amdolbyMode = DOLBY_PQ_MODE_STANDARD_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_HDR) {
            amdolbyMode = DOLBY_PQ_MODE_DARK_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_NORMAL) {
            amdolbyMode = DOLBY_PQ_MODE_BRIGHT_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_PC) {
            amdolbyMode = DOLBY_PQ_MODE_PC_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_CUSTOMER) {
            amdolbyMode = DOLBY_PQ_MODE_CUSTOMER_HLG;
        } else if (pq_mode == VPP_PICTURE_MODE_STORE) {
            amdolbyMode = DOLBY_PQ_MODE_STORE_HLG;
        } else {
            LOGD("%s: not supported mode.\n", __FUNCTION__);
            amdolbyMode = DOLBY_PQ_MODE_MAX;
        }
        break;

    case HDRTYPE_DOVI:
        if (pq_mode == VPP_PICTURE_MODE_DYNAMIC) {
            amdolbyMode = DOLBY_PQ_MODE_DYNAMIC_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_STANDARD) {
            amdolbyMode = DOLBY_PQ_MODE_STANDARD_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_HDR) {
            amdolbyMode = DOLBY_PQ_MODE_DARK_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_NORMAL) {
            amdolbyMode = DOLBY_PQ_MODE_BRIGHT_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_PC) {
            amdolbyMode = DOLBY_PQ_MODE_PC_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_CUSTOMER) {
            amdolbyMode = DOLBY_PQ_MODE_CUSTOMER_DV;
        } else if (pq_mode == VPP_PICTURE_MODE_STORE) {
            amdolbyMode = DOLBY_PQ_MODE_STORE_DV;
        } else {
            LOGD("%s: not supported mode.\n", __FUNCTION__);
            amdolbyMode = DOLBY_PQ_MODE_MAX;
        }
        break;
    case HDRTYPE_HDR10PLUS:
    case HDRTYPE_MVC:
    case HDRTYPE_PRIMESL:
    case HDRTYPE_NONE:
    default:
        LOGD("%s: not supported mode.\n", __FUNCTION__);
        amdolbyMode = DOLBY_PQ_MODE_MAX;
        break;
    }

    LOGD("%s: current hdr type is %d, PQmode is %d, amdolby PQmode is %d.\n",
             __FUNCTION__, hdrType, pq_mode, amdolbyMode);
*/
    return amdolbyMode;
}

int CDolbyVision::SetAmdolbyPQDarkDetail(int mode) {
    int ret = DV_DeviceIOCtl(DV_IOC_SET_DV_DARK_DETAIL, &mode);

    if (ret < 0) {
        LOGD("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}
