/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "CPQControl"

#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <dlfcn.h>

#include "CPQControl.h"

#define PI 3.14159265358979
CPQControl *CPQControl::mInstance = NULL;
pthread_mutex_t PqControlMutex = PTHREAD_MUTEX_INITIALIZER;

CPQControl *CPQControl::GetInstance()
{
    if (NULL == mInstance)
        mInstance = new CPQControl();
    return mInstance;
}

CPQControl::CPQControl()
{

}

CPQControl::~CPQControl()
{

}

void CPQControl::CPQControlInit()
{
    mInitialized   = false;
    mAmvideoFd     = -1;
    mDiFd          = -1;
    mMemcFd        = -1;
    mLdFd          = -1;
    mLcdFd         = -1;
    mbDtvKitEnable = false;

    //open vpp module
    mAmvideoFd = VPPOpenModule();
    if (mAmvideoFd < 0) {
        LOGE("Open amvecm module failed!\n");
    } else {
        LOGD("Open amvecm module success!\n");
    }

    //open DI module
    mDiFd = DIOpenModule();
    if (mDiFd < 0) {
        LOGE("Open DI module failed!\n");
    } else {
        LOGD("Open DI module success!\n");
    }

    // open LD module
    mLdFd = LCDLdimOpenModule();
    if (mLdFd < 0) {
        LOGE("Open ld failed!\n");
    } else {
        LOGD("Open ld ldim success!\n");
    }

    //open MEMC module
    mMemcFd = MEMCOpenModule();
    if (mMemcFd < 0) {
        LOGE("Open MEMC module failed!\n");
    } else {
        LOGD("Open MEMC module success!\n");
    }

    //open LCD module
    if (mbCpqCfg_lcd_hdrinfo_enable) {
        mLcdFd = LCDOpenModule();
        if (mLcdFd < 0) {
            LOGE("Open lcd failed\n");
        } else {
            LOGD("Open lcd success\n");
        }
    } else {
        LOGD("lcd hdr info disable\n");
    }

    //open vdin module
    mpVdin = CVdin::getInstance();

    //Load config file
    mPQConfigFile = CConfigFile::GetInstance();

    const char* pqConfigFilePath = getenv("PQ_CONFIG_FILE_PATH");
    if (!pqConfigFilePath) {
        LOGD("%s: read pqconfig file path failed,use default path:%s\n", __FUNCTION__, PQ_CONFIG_DEFAULT_PATH);
        pqConfigFilePath = PQ_CONFIG_DEFAULT_PATH;
    } else {
        LOGD("%s: pqconfig file path is %s\n", __FUNCTION__, pqConfigFilePath);
    }
    mPQConfigFile->LoadFromFile(pqConfigFilePath);
    SetFlagByCfg();

    //load DV config file
    char dvbinpath[128] = {0};
    char dvcfgpath[128] = {0};
    mPQConfigFile->GetDvFilePath(dvbinpath, dvcfgpath);
    mDolbyVision = new CDolbyVision(dvbinpath, dvcfgpath);

    //open DB
    //for read file path
    const char *config_value;
    char filePath1[64] = {0};
    char filePath2[64] = {0};

    mPQdb = new CPQdb();

    memset(filePath1, 0, sizeof(filePath1));
    memset(filePath2, 0, sizeof(filePath2));
    config_value = mPQConfigFile->GetDatabaseFilePath(CFG_SECTION_PQ, CFG_PQ_DB_CONFIG_PATH, NULL);
    if (!config_value) {
        LOGD("%s: read pqconfig file path failed\n", __FUNCTION__);
        sprintf(filePath1, "%s", PQ_DB_DEFAULT_PATH);
        sprintf(filePath2, "%s", OVERSCAN_DB_DEFAULT_PATH);
    } else {
        LOGD("%s: pqconfig file path is %s\n", __FUNCTION__, config_value);
        sprintf(filePath1, "%s/pq.db", config_value);
        sprintf(filePath2, "%s/overscan.db", config_value);
    }
    int ret = mPQdb->openPqDB(filePath1);
    if (ret != 0) {
        mbDatabaseMatchChipStatus = false;
        LOGE("open pq DB failed!\n");
    } else {
        LOGD("open pq DB success!\n");
        mbDatabaseMatchChipStatus = isPqDatabaseMachChip();
    }
    //init source
    mCurentSourceInputInfo.source_input = SOURCE_MPEG;
    mCurentSourceInputInfo.sig_fmt      = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
    mCurentSourceInputInfo.trans_fmt    = TVIN_TFMT_2D;
    mCurrentHdrType                     = HDR_TYPE_SDR;
    mSourceInputForSaveParam            = SOURCE_MPEG;
    mCurentPqSource.pq_source_input     = SOURCE_MPEG;
    mCurentPqSource.pq_sig_fmt          = PQ_FMT_DEFAUT;

    //check output mode
    // must do before load pq param
    mPQdb->mOutPutType  = CheckOutPutMode(SOURCE_MPEG);

    //open overscan DB
    if (mbCpqCfg_seperate_db_enable) {
        mpOverScandb = new COverScandb();
        ret = mpOverScandb->openOverScanDB(filePath2);
        if (ret != 0) {
            LOGE("open overscan DB failed\n");
        } else {
            LOGD("open overscan DB success\n");
        }
    }

    //SSM file check
    memset(filePath1, 0, sizeof(filePath1));
    memset(filePath2, 0, sizeof(filePath2));
    config_value = mPQConfigFile->GetSettingDataFilePath(CFG_SECTION_PQ, CFG_PQ_SETTINGDATA_FILE_PATH, NULL);
    if (!config_value) {
        LOGD("%s: read setting data file path failed,use default path:%s\n", __FUNCTION__, PQ_SETTINGDATA_DEFAULT_PATH);
        sprintf(filePath1, "%s", PQ_SETTINGDATA_DEFAULT_PATH);
    } else {
        LOGD("%s: setting data file path is %s\n", __FUNCTION__, config_value);
        sprintf(filePath1, "%s", config_value);
    }

    config_value = mPQConfigFile->GetWhiteBalanceFilePath(CFG_SECTION_PQ, CFG_PQ_WHITEBALANCE_FILE_PATH, NULL);
    if (!config_value) {
        LOGD("%s: read whitebalance data file path failed,use default path:%s\n", __FUNCTION__, PQ_WHITEBALANCE_DEFAULT_PATH);
        sprintf(filePath2, "%s", PQ_WHITEBALANCE_DEFAULT_PATH);
    } else {
        LOGD("%s: whitebalance data file path is %s!\n", __FUNCTION__, config_value);
        sprintf(filePath2, "%s", config_value);
    }

    mSSMAction = SSMAction::getInstance();
    mSSMAction->setObserver(this);
    mSSMAction->init(filePath1, filePath2);

    //Set DNLP
    if (mbCpqCfg_dnlp_enable) {
        Cpq_SetDNLPStatus(VE_DNLP_STATE_ON);
    } else {
        Cpq_SetDNLPStatus(VE_DNLP_STATE_OFF);
    }

    //load display mode setting
    vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
    SetDisplayMode(display_mode, 1);

    //Load PQ seting
    if (LoadPQSettings() < 0) {
        LOGE("Load PQ failed\n");
    } else {
        LOGD("Load PQ success\n");
    }

    //set backlight
    BacklightInit();

    //set DynamicBackLight
    DynamicBackLightInit();

    AipqInit();
    //cabc pq
    SetCabc();
    //aad pq
    SetAad();

    //Vframe size
    mCDevicePollCheckThread.setObserver(this);
    mCDevicePollCheckThread.StartCheck();

    mUEventObserver.setUeventObserverCallBack(this);
    mUEventObserver.StartUEventThead();

    mInitialized = true;

}

void CPQControl::CPQControlUnInit()
{
    //close moduel
    VPPCloseModule();
    //close DI module
    DICloseModule();
    //close lcd ldim
    LCDLdimCloseModule();
    //close lcd
    LCDCloseModule();
    //close vdin
    if (mpVdin != NULL) {
        delete mpVdin;
        mpVdin = NULL;
    }

    //close dobly vision
    if (mDolbyVision != NULL) {
        delete mDolbyVision;
        mDolbyVision = NULL;
    }

    if (mSSMAction!= NULL) {
        delete mSSMAction;
        mSSMAction = NULL;
    }

    if (mPQdb != NULL) {
        //closed DB
        mPQdb->closeDb();

        delete mPQdb;
        mPQdb = NULL;
    }

    if (mpOverScandb != NULL) {
        mpOverScandb->closeDb();

        delete mpOverScandb;
        mpOverScandb = NULL;
    }

    if (mPQConfigFile != NULL) {
        delete mPQConfigFile;
        mPQConfigFile = NULL;
    }
}

int CPQControl::VPPOpenModule(void)
{
    if (mAmvideoFd < 0) {
        mAmvideoFd = open(VPP_DEV_PATH, O_RDWR);
        LOGD("amvecm module path: %s.\n", VPP_DEV_PATH);
        if (mAmvideoFd < 0) {
            LOGE("Open amvecm module, error(%s)!\n", strerror(errno));
            return -1;
        }
    } else {
        LOGD("amvecm module has been opened before!\n");
    }

    return mAmvideoFd;
}

int CPQControl::VPPCloseModule(void)
{
    if (mAmvideoFd >= 0) {
        close ( mAmvideoFd);
        mAmvideoFd = -1;
    }
    return 0;
}

int CPQControl::VPPDeviceIOCtl(int request, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mAmvideoFd, request, arg);
    return ret;
}

int CPQControl::DIOpenModule(void)
{
    if (mDiFd < 0) {
        mDiFd = open(DI_DEV_PATH, O_RDWR);

        LOGD("DI Module path: %s.\n", DI_DEV_PATH);

        if (mDiFd < 0) {
            LOGE("Open DI module, error(%s)!\n", strerror(errno));
            return -1;
        }
    }

    return mDiFd;
}

int CPQControl::DICloseModule(void)
{
    if (mDiFd>= 0) {
        close ( mDiFd);
        mDiFd = -1;
    }
    return 0;
}

int CPQControl::DIDeviceIOCtl(int request, ...)
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    tmp_ret = ioctl(mDiFd, request, arg);
    return tmp_ret;
}

int CPQControl::AFEDeviceIOCtl ( int request, ... )
{
    int tmp_ret = -1;
    int afe_dev_fd = -1;
    va_list ap;
    void *arg;

    afe_dev_fd = open ( AFE_DEV_PATH, O_RDWR );

    if ( afe_dev_fd >= 0 ) {
        va_start ( ap, request );
        arg = va_arg ( ap, void * );
        va_end ( ap );

        tmp_ret = ioctl ( afe_dev_fd, request, arg );

        close(afe_dev_fd);
        return tmp_ret;
    } else {
        LOGE ( "Open tvafe module error(%s).\n", strerror ( errno ));
        return -1;
    }
}

int CPQControl::LCDLdimOpenModule(void)
{
    if (mLdFd < 0) {
        mLdFd = open(LDIM_PATH, O_RDWR);
        LOGD("lcd ldim module path: %s.\n", LDIM_PATH);
        if (mLdFd < 0) {
            LOGE("Open lcd ldim module, error(%s)!\n", strerror(errno));
            return -1;
        }
    } else {
        LOGD("lcd ldim module has been opened before!\n");
    }

    return mLdFd;
}

int CPQControl::LCDLdimCloseModule(void)
{
    if (mLdFd >= 0) {
        close( mLdFd);
        mLdFd = -1;
    }
    return 0;
}

int CPQControl::LCDLdimDeviceIOCtl(int request, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mLdFd, request, arg);
    return ret;
}

int CPQControl::MEMCOpenModule(void)
{
    if (mMemcFd < 0) {
        mMemcFd = open(CPQ_MEMC_SYSFS, O_RDWR);

        LOGD("MEMC OpenModule path: %s", CPQ_MEMC_SYSFS);

        if (mMemcFd < 0) {
            LOGE("Open MEMC module, error(%s)!\n", strerror(errno));
            return -1;
        }
    }

    return mMemcFd;
}

int CPQControl::MEMCCloseModule(void)
{
    if (mMemcFd>= 0) {
        close ( mMemcFd);
        mMemcFd = -1;
    }
    return 0;
}

int CPQControl::MEMCDeviceIOCtl(int request, ...)
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    tmp_ret = ioctl(mMemcFd, request, arg);
    return tmp_ret;
}

int CPQControl::LCDOpenModule(void)
{
    if (mLcdFd < 0) {
        mLcdFd = open(LCD_PATH, O_RDWR);
        LOGD("lcd module path: %s\n", LCD_PATH);
        if (mLcdFd < 0) {
            LOGE("Open lcd module, error(%s)\n", strerror(errno));
            return -1;
        }
    } else {
        LOGD("lcd module has been opened before\n");
    }

    return mLcdFd;
}

int CPQControl::LCDCloseModule(void)
{
    if (mLcdFd >= 0) {
        close( mLcdFd);
        mLcdFd = -1;
    }
    return 0;
}

int CPQControl::LCDDeviceIOCtl(int request, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;
    va_start(ap, request);
    arg = va_arg ( ap, void * );
    va_end(ap);
    ret = ioctl(mLcdFd, request, arg);
    return ret;
}

void CPQControl::onVframeSizeChange()
{
    char temp[8];
    memset(temp, 0, sizeof(temp));
    int ret = pqReadSys(SYSFS_VIDEO_EVENT_PATH, temp, sizeof(temp));
    if (ret > 0) {
        int eventFlagValue = strtol(temp, NULL, 16);
        LOGD("%s: event value = %d(0x%x)\n", __FUNCTION__, eventFlagValue, eventFlagValue);
        int frameszieEventFlag      = (eventFlagValue & 0x1) >> 0;
        //int hdrTypeEventFlag        = (eventFlagValue & 0x2) >> 1;
        int videoPlayStartEventFlag = (eventFlagValue & 0x4) >> 2;
        int videoPlayStopEventFlag  = (eventFlagValue & 0x8) >> 3;
        //int videoPlayAxisEventFlag = (eventFlagValue & 0x10) >> 4;
        /*LOGD("%s: frameszieEventFlag = %d,hdrTypeEventFlag = %d,videoPlayStartEventFlag = %d,videoPlayStopEventFlag = %d,videoPlayAxisEventFlag = %d!\n",
                 __FUNCTION__, frameszieEventFlag, hdrTypeEventFlag, videoPlayStartEventFlag, videoPlayStopEventFlag,
                 videoPlayAxisEventFlag);*/
        if ((videoPlayStartEventFlag == 1) && (videoPlayStopEventFlag == 0)) {
            mbVideoIsPlaying = true;
        } else if ((videoPlayStartEventFlag == 0) && (videoPlayStopEventFlag == 1)) {
            mbVideoIsPlaying = false;
        } else {
            LOGD("%s: not videoplay on/off event\n", __FUNCTION__);
        }

        if (((mCurentSourceInputInfo.source_input == SOURCE_DTV)
            || (mCurentSourceInputInfo.source_input == SOURCE_MPEG))
            && (frameszieEventFlag == 1)) {
            source_input_param_t new_source_input_param;
            new_source_input_param.source_input = mCurentSourceInputInfo.source_input;
            new_source_input_param.trans_fmt    = mCurentSourceInputInfo.trans_fmt;
            new_source_input_param.sig_fmt      = getVideoResolutionToFmt();
            SetCurrentSourceInputInfo(new_source_input_param);
        }
    } else {
        LOGE("%s: read video event failed!\n", __FUNCTION__);
    }
}

int CPQControl::GetWindowStatus(void)
{
    window_mode_t newWindowStatus = WINDOW_FULL;

    int width = 0, height = 0;
    char axisValue[32] = {0};
    char resolutionVal[32] = {0};
    int readRet =  pqReadSys(SYSFS_VIDEO_AXIS_PATH, axisValue, sizeof(axisValue));
    if (readRet == -1) {
        LOGD("%s: read videoAxis failed!\n",__FUNCTION__);
    } else {
        char *buf = strtok(axisValue, " ");
        int position[4] = {0};
        int i = 0;
        while (buf != NULL) {
            position[i++] = atoi(buf);
            buf = strtok(NULL, " ");
        }
        pqReadSys(SYSFS_DEVICE_RESOLUTION, resolutionVal, sizeof(resolutionVal));
        if (strncasecmp(resolutionVal, "1366x768", strlen ("1366x768")) == 0) {
            width = 1366;
            height = 768;
        } else if (strncasecmp(resolutionVal, "3840x2160", strlen ("3840x2160")) == 0) {
            width = 3840;
            height = 2160;
        } else if (strncasecmp(resolutionVal, "1920x1080", strlen ("1920x1080")) == 0) {
            width = 1920;
            height = 1080;
        } else {
            LOGD("video display resolution is = (%s) not define , default it", resolutionVal);
            width = 1366;
            height = 768;
        }
        if ((position[0] == 0) && (position[1] == 0) &&(position[2] == -1) && (position[3] == -1)) {//device first on
            newWindowStatus = WINDOW_LUNCHER;
        } else {
            if (GetVideoPlayStatus() == 1) {
                if (((width - (position[2] - position[0]) <= 40) && (height - (position[3] - position[1]) <= 40))
                    || ((width - (position[2] - position[0]) >= width) && (height - (position[3] - position[1]) >= height))) {
                    newWindowStatus = WINDOW_FULL;
                } else {
                    newWindowStatus = WINDOW_PREVIEW;
                }
            } else {
                newWindowStatus = WINDOW_LUNCHER;
            }
        }
    }
    LOGD("%s: newWindowStatus = %d!\n",__FUNCTION__, newWindowStatus);
    return (int)newWindowStatus;
}

tvin_sig_fmt_t CPQControl::getVideoResolutionToFmt()
{
    int fd = -1;
    char buf[32] = {0};
    tvin_sig_fmt_t sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;

    fd = open(SYS_VIDEO_FRAME_HEIGHT, O_RDONLY);
    if (fd < 0) {
        LOGE("[%s] open: %s error!\n", __FUNCTION__, SYS_VIDEO_FRAME_HEIGHT);
        return sig_fmt;
    }

    if (read(fd, buf, sizeof(buf)) >0) {
        int height = atoi(buf);
        if (height <= 576) {
            sig_fmt = TVIN_SIG_FMT_HDMI_720X480P_60HZ;
        } else if (height > 576 && height <= 720) {
            sig_fmt = TVIN_SIG_FMT_HDMI_1280X720P_60HZ;
        } else if (height > 720 && height <= 1088) {
            sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
        } else {
            sig_fmt = TVIN_SIG_FMT_HDMI_3840_2160_00HZ;
        }
    } else {
        LOGE("[%s] read error!\n", __FUNCTION__);
    }
    close(fd);

    return sig_fmt;
}

int CPQControl::SetCurrenSourceInfo(tvin_parm_t sig_info)
{
    if (mCurrentSignalInfo.info.trans_fmt  != sig_info.info.trans_fmt
        || mCurrentSignalInfo.info.fmt     != sig_info.info.fmt
        || mCurrentSignalInfo.info.status  != sig_info.info.status
        || mCurrentSignalInfo.port         != sig_info.port
        || mCurrentSignalInfo.info.hdr_info != sig_info.info.hdr_info) {
        mCurrentSignalInfo.info.trans_fmt    = sig_info.info.trans_fmt;
        mCurrentSignalInfo.info.fmt          = sig_info.info.fmt;
        mCurrentSignalInfo.info.status       = sig_info.info.status;
        mCurrentSignalInfo.info.cfmt         = sig_info.info.cfmt;
        mCurrentSignalInfo.info.hdr_info     = sig_info.info.hdr_info;
        mCurrentSignalInfo.info.fps          = sig_info.info.fps;
        mCurrentSignalInfo.info.is_dvi       = sig_info.info.is_dvi;
        mCurrentSignalInfo.info.aspect_ratio = sig_info.info.aspect_ratio;

        mCurrentSignalInfo.port              = sig_info.port;

        tv_source_input_t SourceInput;
        SourceInput = mpVdin->Tvin_PortToSourceInput(sig_info.port);

        if ((SourceInput == SOURCE_MPEG)
            || (SourceInput != SOURCE_MPEG && mCurrentSignalInfo.info.status == TVIN_SIG_STATUS_STABLE)) {
            tvin_latency_t allmInfo;
            memset(&allmInfo, 0x0, sizeof(tvin_latency_t));
            mpVdin->Tvin_GetAllmInfo(&allmInfo);
            mCurrentTvinInfo.allmInfo.allm_mode  = allmInfo.allm_mode;
            mCurrentTvinInfo.allmInfo.it_content = allmInfo.it_content;
            mCurrentTvinInfo.allmInfo.cn_type    = allmInfo.cn_type;
            mCurrentTvinInfo.is_dvi              = mCurrentSignalInfo.info.is_dvi;
            mCurrentTvinInfo.hdr_info            = mCurrentSignalInfo.info.hdr_info;

            source_input_param_t source_input_param;
            source_input_param.source_input = SourceInput;
            source_input_param.sig_fmt      = mCurrentSignalInfo.info.fmt;
            source_input_param.trans_fmt    = mCurrentSignalInfo.info.trans_fmt;
            SetCurrentSourceInputInfo(source_input_param);
        }
    }

    return 0;
}

void CPQControl::onSigStatusChange(void)
{
    tvin_parm_s tempSignalInfo;
    int ret = mpVdin->Tvin_GetVdinParam(&tempSignalInfo);

    if (ret < 0) {
        LOGD("%s Get Signal Info error\n", __FUNCTION__);
    } else {
        SetCurrenSourceInfo(tempSignalInfo);
        LOGD("source_input is %d, port is %d, sig_fmt is %d, status is %d, isDVI is %d, hdr_info is 0x%x\n",
            mCurentSourceInputInfo.source_input, mCurrentSignalInfo.port, mCurrentSignalInfo.info.fmt, mCurrentSignalInfo.info.status,
            mCurrentSignalInfo.info.is_dvi, mCurrentSignalInfo.info.hdr_info);
    }

    return;
}

void CPQControl::stopVdin(void)
{
    SetCurrentSource(SOURCE_MPEG);

    tvin_parm_s tempSignalInfo;
    tempSignalInfo.info.trans_fmt    = TVIN_TFMT_2D;
    tempSignalInfo.info.fmt          = TVIN_SIG_FMT_NULL;
    tempSignalInfo.info.status       = TVIN_SIG_STATUS_NULL;
    tempSignalInfo.info.cfmt         = TVIN_COLOR_FMT_MAX;
    tempSignalInfo.info.hdr_info     = 0;
    tempSignalInfo.info.fps          = 60;
    tempSignalInfo.info.is_dvi       = 0;
    tempSignalInfo.info.aspect_ratio = TVIN_ASPECT_NULL;
    tempSignalInfo.port              = TVIN_PORT_MPEG0;

    SetCurrenSourceInfo(tempSignalInfo);
}

void CPQControl::onUevent(uevent_data_t ueventData)
{
    LOGD("%s matchName:%s\n", __FUNCTION__, ueventData.matchName);
    int ret = -1;

    vdin_event_info_s SignalEventInfo;
    memset(&SignalEventInfo, 0, sizeof(vdin_event_info_s));
    ret = mpVdin->Tvin_GetSignalEventInfo(&SignalEventInfo);

    if (ret < 0) {
        stopVdin();
        LOGD("Get vidn event error!\n");
    } else {
        tv_source_input_type_t source_type   = mpVdin->Tvin_SourceInputToSourceInputType(mCurentSourceInputInfo.source_input);
        tvin_sig_change_flag_t vdinEventType = (tvin_sig_change_flag_t)SignalEventInfo.event_sts;
        switch (vdinEventType) {
        case TVIN_SIG_CHG_SDR2HDR:
        case TVIN_SIG_CHG_HDR2SDR:
        case TVIN_SIG_CHG_DV2NO:
        case TVIN_SIG_CHG_NO2DV: {
            LOGD("%s: hdr info change!\n", __FUNCTION__);
            tvin_info_s vdinSignalInfo;
            memset(&vdinSignalInfo, 0, sizeof(vdin_info_t));
            ret = mpVdin->Tvin_GetSignalInfo(&vdinSignalInfo);
            if (ret < 0) {
                LOGD("%s: Get vidn event error!\n", __FUNCTION__);
            } else {
                if ((mCurrentSignalInfo.info.status == TVIN_SIG_STATUS_STABLE)
                    && (mCurrentSignalInfo.info.hdr_info != vdinSignalInfo.hdr_info)) {
                    if (source_type == SOURCE_TYPE_HDMI) {
                        SetCurrentHdrInfo(vdinSignalInfo.hdr_info);
                    }
                    mCurrentSignalInfo.info.hdr_info = vdinSignalInfo.hdr_info;
                } else {
                    LOGD("%s: hdmi signal don't stable!\n", __FUNCTION__);
                }
            }
            break;
        }
        case TVIN_SIG_CHG_COLOR_FMT:
            LOGD("%s: no need do any thing for colorFmt change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_RANGE:
            LOGD("%s: no need do any thing for colorRange change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_BIT:
            LOGD("%s: no need do any thing for color bit deepth change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_VS_FRQ:
            LOGD("%s: no need do any thing for VS_FRQ change!\n", __FUNCTION__);
            break;
        case TVIN_SIG_CHG_STS:
            LOGD("%s: vdin signal status change!\n", __FUNCTION__);
            onSigStatusChange();
            break;
        case TVIN_SIG_CHG_AFD: {
            LOGD("%s: AFD info change!\n", __FUNCTION__);
            /*
            if (source_type == SOURCE_TYPE_HDMI) {
                tvin_info_t newSignalInfo;
                memset(&newSignalInfo, 0, sizeof(tvin_info_t));
                int ret = mpTvin->Tvin_GetSignalInfo(&newSignalInfo);
                if (ret < 0) {
                    LOGD("%s: Get Signal Info error!\n", __FUNCTION__);
                } else {
                    if ((newSignalInfo.status == TVIN_SIG_STATUS_STABLE)
                        && (mCurrentSignalInfo.aspect_ratio != newSignalInfo.aspect_ratio)) {
                        mCurrentSignalInfo.aspect_ratio = newSignalInfo.aspect_ratio;
                        //tvSetCurrentAspectRatioInfo(newSignalInfo.aspect_ratio);
                    } else {
                        LOGD("%s: signal not stable or same AFD info!\n", __FUNCTION__);
                    }
                }
            }
            */
            break;
        }
        case TVIN_SIG_CHG_DV_ALLM:
            LOGD("%s: allm info change!\n", __FUNCTION__);
            /*
            if (source_type == SOURCE_TYPE_HDMI) {
                //setPictureModeBySignal(PQ_MODE_SWITCH_TYPE_AUTO);
            } else {
                LOGD("%s: not hdmi source!\n", __FUNCTION__);
            }
            */
            break;
        case TVIN_SIG_CHG_CLOSE_FE:
            stopVdin();
            break;
        default:
            LOGD("%s: invalid vdin event!\n", __FUNCTION__);
            break;
        }
    }
}

void CPQControl::onTXStatusChange()
{
    LOGD("%s!\n", __FUNCTION__);

    SetCurrentSourceInputInfo(mCurentSourceInputInfo);
}

int CPQControl::LoadPQSettings()
{
    int ret = 0;

    if (!mbCpqCfg_pq_enable) {
        LOGD("All PQ moudle disabled\n");
        pq_ctrl_t pqControlVal;
        memset(&pqControlVal, 0, sizeof(pq_ctrl_t));
        vpp_pq_ctrl_t amvecmConfigVal;
        amvecmConfigVal.length = 14;//this is the count of pq_ctrl_s option
        amvecmConfigVal.ptr    = &pqControlVal;
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_PQ_CTRL, &amvecmConfigVal);
        if (ret < 0) {
            LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
        }
    } else {
        LOGD("%s source_input: %d, sig_fmt: 0x%x(%d), trans_fmt: 0x%x\n", __FUNCTION__,
            mCurentSourceInputInfo.source_input,
            mCurentSourceInputInfo.sig_fmt, mCurentSourceInputInfo.sig_fmt,
            mCurentSourceInputInfo.trans_fmt);


        LOGD("pq_source_input: %d, timming: %d, \n", mCurentPqSource.pq_source_input,
                 mCurentPqSource.pq_sig_fmt);

        if (mbCpqCfg_new_picture_mode_enable) {
            ret |= LoadPQUISettings();
            ret |= LoadPQTableSettings();
            return ret;
        }

        ret |= Cpq_SetXVYCCMode(VPP_XVYCC_MODE_STANDARD, mCurentSourceInputInfo);

        ret |= Cpq_SetDIModuleParam(mCurentSourceInputInfo);

        int cur_mode  = GetPQMode();
        //save cur_mode
        SaveLastPQMode(cur_mode);
        //save new mode for pq setting
        SavePQMode(cur_mode);

        //load pq setting
        ret |= Cpq_SetPQMode((vpp_picture_mode_t)cur_mode, mCurentSourceInputInfo);
    }

    return ret;
}

int CPQControl::LoadPQUISettings()
{
    int ret = 0;

    //picture mode
    vpp_picture_mode_t pqmode = (vpp_picture_mode_t)GetPQMode();
    ret = Set_PictureMode (pqmode, mCurentPqSource);

    return ret;
}

int CPQControl::LoadPQTableSettings()
{
    int ret = 0;

    ret |= Cpq_SetXVYCCMode(VPP_XVYCC_MODE_STANDARD, mCurentSourceInputInfo);

    ret |= Cpq_SetDIModuleParam(mCurentSourceInputInfo);

    int LDLevel = GetLocalDimming();
    ret |= SetLocalDimming(LDLevel, 1);

    vpp_mcdi_mode_t mcdimode =(vpp_mcdi_mode_t)GetMcDiMode();
    ret |= Cpq_SetMcDiMode(mcdimode, mCurentSourceInputInfo);

    if (mInitialized) {//don't load gamma in device turn on
        vpp_gamma_curve_t GammaLevel = (vpp_gamma_curve_t)GetGammaValue();
        ret |= SetGammaValue(GammaLevel, 1);
    }

    vpp_color_basemode_t baseMode = GetColorBaseMode();
    ret |= SetColorBaseMode(baseMode, 1);

    //memc
    int MemcMode = GetMemcMode();
    ret |= SetMemcMode(MemcMode, 1);

    //display
    vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
    ret |= SetDisplayMode(display_mode, 1);

    //load hdr tmo
    //int HdrTmoMode = GetHDRTMOMode();
    ret |= SetHDRTMOMode(HDR_TMO_DYNAMIC, 1);

    ret |= AiParamLoad();

    int aisr_enable = GetAiSrEnable();
    ret |= SetAiSrEnable((aisr_enable > 0)? true : false);

    vpp_smooth_plus_mode_t smoothplus_mode = VPP_SMOOTH_PLUS_MODE_OFF;
    ret |= Cpq_SetSmoothPlusMode(smoothplus_mode, mCurentSourceInputInfo);

    return ret;
}

void CPQControl::resetPQUiSetting(void)
{
    int i = 0, j = 0, k = 0, config_val = 0;
    vpp_pictur_mode_para_t picture;
    pq_src_param_t src;

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        for (j = PQ_FMT_DEFAUT; j < PQ_FMT_MAX; j++) {
            src.pq_source_input = (tv_source_input_t)i;
            src.pq_sig_fmt = (pq_sig_fmt_t)j;

            //picture
            if (j == PQ_FMT_DOBLY) {
                config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DV_PICTUREMODE_DEF, VPP_PICTURE_MODE_DV_BRIGHT);
            } else {
                config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_PICTUREMODE_DEF, VPP_PICTURE_MODE_STANDARD);
            }
            mSSMAction->SSMSavePictureMode(i * PQ_FMT_MAX + j, config_val);
            mSSMAction->SSMSaveLastPictureMode(i * PQ_FMT_MAX + j, config_val);

            //picture mode params
            for (k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
                if (mPQdb->PQ_GetPictureModeParams(src, vpp_picture_mode_t(k), &picture) == 0) {
                    SetPictureModeData(src, vpp_picture_mode_t(k), &picture);
                } else {
                    RsetPictureModeData(src, vpp_picture_mode_t(k));
                }
            }
        }
    }

    //user colortemp
    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_R_DEF, 0);
    mSSMAction->SSMSaveRGBGainRStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_G_DEF, 0);
    mSSMAction->SSMSaveRGBGainGStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_B_DEF, 0);
    mSSMAction->SSMSaveRGBGainBStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_R_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetRStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_G_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetGStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_B_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetBStart(0, config_val);

    return;
}

void CPQControl::resetPQTableSetting(void)
{
    int i = 0, j = 0, config_val = 0;
    const char *buf = NULL;

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_GAMMALEVEL_DEF, VPP_GAMMA_CURVE_DEFAULT);
        mSSMAction->SSMSaveGammaValue(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_MCDI_DEF, VPP_MCDI_MODE_STANDARD);
        mSSMAction->SSMSaveMcDiMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_LOCALDIMMING_DEF, VPP_PQ_LV_OFF);
        mSSMAction->SSMSaveLocalDimming(i, config_val);

        //MEMC
        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_MEMCMODE_DEF, VPP_MEMC_MODE_HIGH);
        mSSMAction->SSMSaveMemcMode(i, config_val);

        buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMCDEBLURLEVEL_DEF, NULL);
        int Deblur_para[VPP_MEMC_MODE_MAX] = { 0, 3, 6, 10 };
        pqTransformStringToInt(buf, Deblur_para);
        for (j = VPP_MEMC_MODE_OFF; j < VPP_MEMC_MODE_MAX; j++) {
            mSSMAction->SSMSaveMemcDeblurLevel(i * VPP_MEMC_MODE_MAX + j, Deblur_para[j]);
        }

        buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMCDEJUDDERLEVEL_DEF, NULL);
        int DeJudder_para[VPP_MEMC_MODE_MAX] = { 0, 3, 6, 10 };
        pqTransformStringToInt(buf, DeJudder_para);
        for (j = VPP_MEMC_MODE_OFF; j < VPP_MEMC_MODE_MAX; j++) {
            mSSMAction->SSMSaveMemcDeJudderLevel(i * VPP_MEMC_MODE_MAX + j, DeJudder_para[j]);
        }

        //DISPLAY MODE
        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DISPLAYMODE_DEF, VPP_DISPLAY_MODE_NORMAL);
        mSSMAction->SSMSaveDisplayMode(i, config_val);
    }

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DYNAMICBACKLIGHT_DEF, DYNAMIC_BACKLIGHT_OFF);
    mSSMAction->SSMSaveDynamicBacklightMode(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORDEMOMODE_DEF, VPP_COLOR_DEMO_MODE_ALLON);
    mSSMAction->SSMSaveColorDemoMode(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORBASEMODE_DEF, VPP_COLOR_BASE_MODE_ENHANCE);
    mSSMAction->SSMSaveColorBaseMode (VPP_COLOR_BASE_MODE_OPTIMIZE);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DDRSSC_DEF, 0);
    mSSMAction->SSMSaveDDRSSC(config_val);

    buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LVDSSSC_DEF, NULL);
    int tmp[2] = {0, 0};
    pqTransformStringToInt(buf, tmp);
    mSSMAction->SSMSaveLVDSSSC(tmp);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_EDID_VERSION_DEF, 0);
    mSSMAction->SSMEdidRestoreDefault(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_HDCP_SWITCHER_DEF, 0);
    mSSMAction->SSMHdcpSwitcherRestoreDefault(0);

    buf = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_COLOR_RANGE_MODE_DEF, NULL);
    if (strcmp(buf, "full") == 0) {
        mSSMAction->SSMSColorRangeModeRestoreDefault(1);
    } else if (strcmp(buf, "limit") == 0) {
        mSSMAction->SSMSColorRangeModeRestoreDefault(2);
    } else {
        mSSMAction->SSMSColorRangeModeRestoreDefault(0);
    }
    //static frame
    Cpq_SSMWriteNTypes(SSM_RW_BLACKOUT_ENABLE_START, 1, 1, 0);
    //screen color for signal
    Cpq_SSMWriteNTypes(CUSTOMER_DATA_POS_SCREEN_COLOR_START, 1, 0, 0);

    //EyeProtection
    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_EYEPROJECTMODE_DEF, 0);
    mSSMAction->SSMSaveEyeProtectionMode(config_val);

    //ai pq/sr
    mSSMAction->SSMSaveAipqEnableVal(0);
    mSSMAction->SSMSaveAiSrEnable(1);

    return;
}

int CPQControl::SetPictureModeData(pq_src_param_t pq_source_input, vpp_picture_mode_t picmode, vpp_pictur_mode_para_t *params)
{
    int ret = -1;
    int isValid = -1;
    int size = sizeof(vpp_pictur_mode_para_t);
    int offset = 0;
    if (size > MAX_PICTUREMODE_PARAM_SIZE) {
        LOGE("%s error size: %d > ssmdata param len : %d!\n", __FUNCTION__, size, MAX_PICTUREMODE_PARAM_SIZE);
        return -1;
    }

    offset = pq_source_input.pq_source_input * PQ_FMT_MAX * VPP_PICTURE_MODE_MAX + pq_source_input.pq_sig_fmt * VPP_PICTURE_MODE_MAX + picmode;
    isValid = 1;

    ret = mSSMAction->SSMSavePictureModeParams(offset * MAX_PICTUREMODE_PARAM_SIZE, size, (int *)params);
    ret |= mSSMAction->SSMSavePictureModeParamsFlag(offset, isValid);

    if (ret < 0)
        LOGE("%s offset: %d, SetPictureModeData fail!\n", __FUNCTION__, offset);

    return ret;
}

int CPQControl::RsetPictureModeData(pq_src_param_t pq_source_input, vpp_picture_mode_t picmode)
{
    int ret = -1;
    int isValid = -1;
    int offset = 0;

    offset = pq_source_input.pq_source_input * PQ_FMT_MAX * VPP_PICTURE_MODE_MAX + pq_source_input.pq_sig_fmt * VPP_PICTURE_MODE_MAX + picmode;
    isValid = 0;

    ret = mSSMAction->SSMSavePictureModeParamsFlag(offset, isValid);

    if (ret < 0)
        LOGE("%s RsetPictureModeData fail!\n", __FUNCTION__);

    return ret;
}

int CPQControl::GetPictureModeData(pq_src_param_t pq_source_intput, vpp_picture_mode_t picmode, vpp_pictur_mode_para_t *params)
{
    int ret = 0;
    int isValid = -1;;
    int size = sizeof(vpp_pictur_mode_para_t);
    int Offset = 0, OffsetRetry = 0, OffsetDef = 0;
    if (size > MAX_PICTUREMODE_PARAM_SIZE) {
        LOGE("%s error size: %d > ssmdata param len : %d!\n", __FUNCTION__, size, MAX_PICTUREMODE_PARAM_SIZE);
        return -1;
    }

    Offset =      pq_source_intput.pq_source_input * PQ_FMT_MAX * VPP_PICTURE_MODE_MAX + pq_source_intput.pq_sig_fmt * VPP_PICTURE_MODE_MAX + picmode;

    OffsetRetry = pq_source_intput.pq_source_input * PQ_FMT_MAX * VPP_PICTURE_MODE_MAX + PQ_FMT_DEFAUT * VPP_PICTURE_MODE_MAX + picmode;

    OffsetDef =   SOURCE_TV * PQ_FMT_MAX * VPP_PICTURE_MODE_MAX + PQ_FMT_DEFAUT * VPP_PICTURE_MODE_MAX + picmode;

    if (mSSMAction->SSMReadPictureModeParamsFlag(Offset, &isValid) == 0) {
        if (isValid == 1) {
            ret = mSSMAction->SSMReadPictureModeParams(Offset * MAX_PICTUREMODE_PARAM_SIZE, size, (int *)params);
        } else {
            if (mSSMAction->SSMReadPictureModeParamsFlag(OffsetRetry, &isValid) == 0) {
                if (isValid == 1) {
                    ret = mSSMAction->SSMReadPictureModeParams(OffsetRetry * MAX_PICTUREMODE_PARAM_SIZE, size, (int *)params);
                } else {
                    if (mSSMAction->SSMReadPictureModeParamsFlag(OffsetDef, &isValid) == 0) {
                        if (isValid == 1) {
                            ret = mSSMAction->SSMReadPictureModeParams(OffsetDef * MAX_PICTUREMODE_PARAM_SIZE, size, (int *)params);
                        } else {
                            LOGD("%s  Offset:%d  OffsetRetry: %d, OffsetDef: %d, all NULL!!! check XML DB\n", __FUNCTION__, Offset, OffsetRetry, OffsetDef);
                            ret = -1;
                            return ret;
                        }
                    }
                }
            }
        }
    }

    return ret;
}

int CPQControl::Set_PictureMode(vpp_picture_mode_t pq_mode, pq_src_param_t source_input_param)
{
    int ret = -1;
    vpp_pictur_mode_para_t pq_para;

    LOGD("%s: src: %d, timming: %d, pq_mode: %d!\n", __FUNCTION__,mCurentPqSource.pq_source_input, mCurentPqSource.pq_sig_fmt, pq_mode);

    SetPcGameMode(pq_mode);

    SetFacColorParams(mCurentSourceInputInfo, pq_mode);

    ret = GetPictureModeData(source_input_param, pq_mode, &pq_para);

    if (ret == 0) {
        ret |= Cpq_SetBrightness(pq_para.Brightness, mCurentSourceInputInfo);
        ret |= Cpq_SetContrast(pq_para.Contrast, mCurentSourceInputInfo);
        ret |= Cpq_SetSaturation(pq_para.Saturation, mCurentSourceInputInfo);
        ret |= Cpq_SetHue(pq_para.Hue, mCurentSourceInputInfo);
        ret |= Cpq_SetSharpness(pq_para.Sharpness, mCurentSourceInputInfo);
        ret |= Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)pq_para.Nr, mCurentSourceInputInfo);
        ret |= Cpq_SetDnlpMode((Dynamic_contrast_mode_t)pq_para.DynamicContrast, mCurentSourceInputInfo);
        ret |= Cpq_SetLocalContrastMode((local_contrast_mode_t)pq_para.LocalContrast);
        ret |= Cpq_SetColorGamutMode((vpp_colorgamut_mode_t)pq_para.ColorGamut, mCurentSourceInputInfo);
        ret |= Cpq_BlackStretch(pq_para.BlackStretch, mCurentSourceInputInfo);
        ret |= Cpq_BlueStretch(pq_para.BlueStretch, mCurentSourceInputInfo);
        ret |= Cpq_ChromaCoring(pq_para.ChromaCoring, mCurentSourceInputInfo);
        ret |= Cpq_SetMpegNr((vpp_pq_level_t)pq_para.MpegNr, mCurentSourceInputInfo);

        //dobly mode
        if (pq_para.DolbyMode >= 0) {
            ret |= mDolbyVision->SetDolbyPQMode((dolby_pq_mode_t) pq_para.DolbyMode);
        }
        //dolby dark Detail
        if (pq_para.DolbyDarkDetail >= 0) {
            ret |= Cpq_SetDolbyDarkDetail(pq_para.DolbyDarkDetail);
        }
        //colortemp
        if (mbCpqCfg_whitebalance_enable) {
            if (pq_para.ColorTemperature != VPP_COLOR_TEMPERATURE_MODE_USER) {
                Cpq_CheckColorTemperatureParamAlldata(mCurentSourceInputInfo);
                ret |= Cpq_SetColorTemperatureWithoutSave((vpp_color_temperature_mode_t)pq_para.ColorTemperature, mSourceInputForSaveParam);
            } else {
                tcon_rgb_ogo_t param;
                memset(&param, 0, sizeof(tcon_rgb_ogo_t));
                if (Cpq_GetColorTemperatureUser(mCurentSourceInputInfo.source_input, &param) == 0) {
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, R_GAIN, 1, param.r_gain);
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, G_GAIN, 1, param.g_gain);
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, B_GAIN, 1, param.b_gain);
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, R_POST_OFFSET, 1, param.r_post_offset);
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, G_POST_OFFSET, 1, param.g_post_offset);
                    ret |= Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, B_POST_OFFSET, 1, param.b_post_offset);
                }
            }
        } else {
            LOGD("%s: whitebalance moudle disabled!\n", __FUNCTION__);
        }
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

void CPQControl::SetPcGameMode(vpp_picture_mode_t pq_mode)
{
    //enable or disable game/pc mode
    if ((mCurentSourceInputInfo.source_input == SOURCE_HDMI1) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI2) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI3) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI4)) {//HDMI source;

        int last_pq_mode = GetLastPQMode();
        if (last_pq_mode == VPP_PICTURE_MODE_GAME) {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        } else if (last_pq_mode == VPP_PICTURE_MODE_MONITOR) {
            if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        } else {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        }
    } else {//other source, game mode off and monitor mode off;
        if (mInitialized) {
            Cpq_SetGameMode(MODE_OFF);
            Cpq_SetPCMode(MODE_OFF);
        }
    }

    return;
}


int CPQControl::SetFacColorParams(source_input_param_t source_input_param, vpp_picture_mode_t pqMode)
{
    int ret = 0;

    if (mbCpqCfg_sharpness0_enable) {
        ret |= Cpq_SetSharpness0FixedParam(source_input_param);
        ret |= Cpq_SetSharpness0VariableParam(source_input_param);
    } else {
        LOGD("%s: Sharpness0 moudle disabled!\n", __FUNCTION__);
    }

    if (mbCpqCfg_sharpness1_enable) {
        ret |= Cpq_SetSharpness1FixedParam(source_input_param);
        ret |= Cpq_SetSharpness1VariableParam(source_input_param);
    } else {
        LOGD("%s: Sharpness1 moudle disabled!\n", __FUNCTION__);
    }

    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        ret |= Cpq_SetBrightnessBasicParam(source_input_param);
        ret |= Cpq_SetContrastBasicParam(source_input_param);
        ret |= Cpq_SetSaturationBasicParam(source_input_param);
        ret |= Cpq_SetHueBasicParam(source_input_param);
    } else {
        LOGD("%s: brightness contrast saturation hue moudle disabled!\n", __FUNCTION__);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGE("%s success\n",__FUNCTION__);
    }
    return ret;
}

pq_sig_fmt_t CPQControl::CheckPQTimming(hdr_type_t hdr_type)
{
    pq_sig_fmt_t timming = PQ_FMT_DEFAUT;
    switch (hdr_type) {
    case HDR_TYPE_HDR10:
        timming = PQ_FMT_HDR;
        break;
    case HDR_TYPE_HDR10PLUS:
        timming = PQ_FMT_HDRP;
        break;
    case HDR_TYPE_DOVI:
        timming = PQ_FMT_DOBLY;
        break;
    case HDR_TYPE_HLG:
        timming = PQ_FMT_HLG;
        break;
    case HDR_TYPE_SDR:
        timming = PQ_FMT_SDR;
        break;
    default:
        timming = PQ_FMT_DEFAUT;
        break;
    }

    return timming;
}

int CPQControl::SetDolbyDarkDetail(int mode, int is_save)
{
    int ret =0;
    LOGD("%s, mode = %d\n", __FUNCTION__, mode);
    ret = Cpq_SetDolbyDarkDetail(mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDolbyDarkDetail(mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return 0;
}

int CPQControl::GetDolbyDarkDetail(void)
{
    int mode = -1;

    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            mode = para.DolbyDarkDetail;
        }
    }

    LOGD("%s, source: %d, timming: %d, mode = %d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, mode);
    return mode;
}

int CPQControl::SaveDolbyDarkDetail(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.DolbyDarkDetail = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDolbyDarkDetail(int mode)
{
    int ret = -1;

    ret = mDolbyVision->SetDolbyPQDarkDetail(mode);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_LoadRegs(am_regs_t regs)
{
    if (regs.length == 0) {
        LOGD("%s--Regs is NULL\n", __FUNCTION__);
        return -1;
    }

    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_LOAD_REG, &regs);
        if (ret < 0) {
            LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                usleep(16*1000);
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::Cpq_LoadDisplayModeRegs(ve_pq_load_t regs)
{
    if (regs.length == 0) {
        LOGD("%s--Regs is NULL!\n", __FUNCTION__);
        return -1;
    }

    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_SET_OVERSCAN, &regs);
        if (ret < 0) {
            LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::DI_LoadRegs(am_pq_param_t di_regs)
{
    int count_retry = 20;
    int ret = 0;
    while (count_retry) {
        ret = DIDeviceIOCtl(AMDI_IOC_SET_PQ_PARM, &di_regs);
        if (ret < 0) {
            LOGE("%s, error(%s), errno(%d)\n", __FUNCTION__, strerror(errno), errno);
            if (errno == EBUSY) {
                LOGE("%s, %s, retry...\n", __FUNCTION__, strerror(errno));
                count_retry--;
                continue;
            }
        }
        break;
    }

    return ret;
}

int CPQControl::LoadCpqLdimRegs()
{
    bool ret = 0;
    int ldFd = -1;

    if (!isFileExist(LDIM_PATH)) {
        LOGD("Don't have ldim module!\n");
    } else {
        ldFd = open(LDIM_PATH, O_RDWR);

        if (ldFd < 0) {
            LOGE("Open ldim module, error(%s)!\n", strerror(errno));
            ret = -1;
        } else {
            vpu_ldim_param_s *ldim_param_temp = new vpu_ldim_param_s();

            if (ldim_param_temp) {
                if (!mPQdb->PQ_GetLDIM_Regs(ldim_param_temp) || ioctl(ldFd, LDIM_IOC_PARA, ldim_param_temp) < 0) {
                   LOGE("LoadCpqLdimRegs, error(%s)!\n", strerror(errno));
                   ret = -1;
                }

                delete ldim_param_temp;
            }
                close (ldFd);
        }
    }

    return ret;
}

int CPQControl::Cpq_LoadBasicRegs(source_input_param_t source_input_param, vpp_picture_mode_t pqMode)
{
    int ret = 0;
    int level = 0;

    if (pqMode != VPP_PICTURE_MODE_MONITOR) {
        if (mbCpqCfg_seperate_black_blue_chorma_db_enable) {
            level = GetBlackStretch();
            ret |= Cpq_BlackStretch(level, source_input_param);

            level = GetBlueStretch();
            ret |= Cpq_BlueStretch(level, source_input_param);

            level = GetChromaCoring();
            ret |= Cpq_ChromaCoring(level, source_input_param);
        } else {
            if (mbCpqCfg_blackextension_enable) {
                ret |= SetBlackExtensionParam(source_input_param);
            } else {
                LOGD("%s: BlackExtension moudle disabled!\n", __FUNCTION__);
            }
        }

        if (mbCpqCfg_sharpness0_enable) {
            ret |= Cpq_SetSharpness0FixedParam(source_input_param);
            ret |= Cpq_SetSharpness0VariableParam(source_input_param);
        } else {
            LOGD("%s: Sharpness0 moudle disabled!\n", __FUNCTION__);
        }

        if (mbCpqCfg_sharpness1_enable) {
            ret |= Cpq_SetSharpness1FixedParam(source_input_param);
            ret |= Cpq_SetSharpness1VariableParam(source_input_param);
        } else {
            LOGD("%s: Sharpness1 moudle disabled!\n", __FUNCTION__);
        }
    }

    if (mbCpqCfg_amvecm_basic_enable) {
        ret |= Cpq_SetBrightnessBasicParam(source_input_param);
        ret |= Cpq_SetContrastBasicParam(source_input_param);
        ret |= Cpq_SetSaturationBasicParam(source_input_param);
        ret |= Cpq_SetHueBasicParam(source_input_param);
    } else {
        LOGD("%s: brightness contrast saturation hue moudle disabled\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::BacklightInit(void)
{
    int ret = 0;
    int backlight = GetBacklight();
    ret = SetBacklight(backlight, 1);

    return ret;
}

int CPQControl::Cpq_SetDIModuleParam(source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0, sizeof(am_pq_param_t));
    if (mbCpqCfg_di_enable) {
        if (mPQdb->PQ_GetDIParams(source_input_param, &regs) == 0) {
            di_regs.table_name |= TABLE_NAME_DI;
            if (regs.length != 0) {
                di_regs.table_len = regs.length;
                am_reg_t tmp_buf[regs.length];
                for (unsigned int i=0;i<regs.length;i++) {
                      tmp_buf[i].addr = regs.am_reg[i].addr;
                      tmp_buf[i].mask = regs.am_reg[i].mask;
                      tmp_buf[i].type = regs.am_reg[i].type;
                      tmp_buf[i].val  = regs.am_reg[i].val;
                }
                di_regs.table_ptr = tmp_buf;
                ret = DI_LoadRegs(di_regs);
            } else {
                LOGE("%s: get DI Module Param failed!\n",__FUNCTION__);
            }
        } else {
            LOGE("%s GetDIParams failed!\n",__FUNCTION__);
        }
    } else {
        LOGD("DI moudle disabled!\n");
    }


    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::SetPQMode(int pq_mode, int is_save)
{
    LOGD("%s, source: %d, timming: %d, pq_mode: %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, mCurentPqSource.pq_sig_fmt, pq_mode);
    int ret = -1;

    int cur_mode = GetPQMode();
    if (cur_mode == pq_mode) {
        LOGD("Same PQ mode,no need set again!\n");
        ret = 0;
        return ret;
    }

    if (is_save == 1) {
        SavePQMode(pq_mode);
    }

    if (mbCpqCfg_new_picture_mode_enable) {
        ret = Set_PictureMode((vpp_picture_mode_t)pq_mode, mCurentPqSource);
    } else {
        ret = Cpq_SetPQMode((vpp_picture_mode_t)pq_mode, mCurentSourceInputInfo);
    }

    if ((ret == 0) && (is_save == 1)) {
        if ((mCurentSourceInputInfo.source_input >= SOURCE_HDMI1) &&
            (mCurentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
            vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
            ret = SetDisplayMode(display_mode, 1);
        }
    }

    if (ret < 0) {
        LOGE("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}



int CPQControl::GetPQMode(void)
{
    int mode = VPP_PICTURE_MODE_STANDARD;
    int offset = 0;
    if ( mbCpqCfg_new_picture_mode_enable) {
        offset =  mCurentPqSource.pq_source_input * PQ_FMT_MAX + mCurentPqSource.pq_sig_fmt;
    } else {
        offset = mSourceInputForSaveParam;
    }
    mSSMAction->SSMReadPictureMode(offset, &mode);

    if (mode < VPP_PICTURE_MODE_STANDARD || mode >= VPP_PICTURE_MODE_MAX) {
        mode = VPP_PICTURE_MODE_STANDARD;
    }

    //LOGD("%s, source: %d, timming: %d, mode: %d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, mode);
    return mode;
}


int CPQControl::SavePQMode(int pq_mode)
{
    int ret = -1;
    LOGD("%s, source:%d, mode:%d\n", __FUNCTION__, mCurentPqSource.pq_source_input, pq_mode);
    int offset =  mCurentPqSource.pq_source_input * PQ_FMT_MAX + mCurentPqSource.pq_sig_fmt;

    ret = mSSMAction->SSMSavePictureMode(offset, pq_mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::GetLastPQMode(void)
{
    int mode = VPP_PICTURE_MODE_STANDARD;
    int offset =  mCurentPqSource.pq_source_input * PQ_FMT_MAX + mCurentPqSource.pq_sig_fmt;

    mSSMAction->SSMReadLastPictureMode(offset, &mode);

    if (mode < VPP_PICTURE_MODE_STANDARD || mode >= VPP_PICTURE_MODE_MAX) {
        mode = VPP_PICTURE_MODE_STANDARD;
    }

    LOGD("%s, source: %d, mode: %d\n", __FUNCTION__, mCurentPqSource.pq_source_input, mode);
    return mode;

}


int CPQControl::SaveLastPQMode(int pq_mode)
{
    int ret = -1;
    LOGD("%s, source: %d, mode: %d\n", __FUNCTION__, mCurentPqSource.pq_source_input, pq_mode);
    int offset =  mCurentPqSource.pq_source_input * PQ_FMT_MAX + mCurentPqSource.pq_sig_fmt;

    ret = mSSMAction->SSMSaveLastPictureMode(offset, pq_mode);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;

}

int CPQControl::Cpq_SetVppPCMode(game_pc_mode_t pcStatus)
{
    char val[64] = {0};

    //0:enablepc mode,1:disable pc mode for amvecm
    unsigned int status = 0;
    if (pcStatus == MODE_ON) {
        status = 0;
    } else {
        status = 1;
    }

    sprintf(val, "%d", status);
    return pqWriteSys(AMVECM_PC_MODE_PATH, val);
}

int CPQControl::Cpq_SetVdinPCMode(game_pc_mode_t pcStatus)
{
    return mpVdin->Tvin_SetPCMode(pcStatus);
}

int CPQControl::Cpq_SetPCMode(game_pc_mode_t pcStatus)
{
    LOGD("%s: pcStatus:%d\n", __FUNCTION__, pcStatus);

    int ret = 0;

    //pc mode
    if ((pcStatus == MODE_ON)
        || (pcStatus == MODE_OFF)) {
        Cpq_SetVdinPCMode(pcStatus);
        Cpq_SetVppPCMode(pcStatus);
    }

    return ret;
}

int CPQControl::Cpq_SetVdinGameMode(game_pc_mode_t gameStatus)
{
    return mpVdin->Tvin_SetGameMode(gameStatus);
}

int CPQControl::Cpq_SetGameMode(game_pc_mode_t gameStatus)
{
    LOGD("%s: gameStatus:%d\n", __FUNCTION__, gameStatus);

    int ret = 0;

    //game mode
    if (gameStatus != mGamemode) {
        mGamemode = gameStatus;
        Cpq_SetVdinGameMode(gameStatus);
    }

    return ret;
}

int CPQControl::Cpq_SetPQMode(vpp_picture_mode_t pq_mode, source_input_param_t source_input_param)
{
    int ret = 0;
    vpp_pq_para_t pq_para;

    //enable or disable game/pc mode
    if ((mCurentSourceInputInfo.source_input == SOURCE_HDMI1) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI2) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI3) ||
        (mCurentSourceInputInfo.source_input == SOURCE_HDMI4)) {//HDMI source;
        int last_pq_mode = GetLastPQMode();
        if (last_pq_mode == VPP_PICTURE_MODE_GAME) {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        } else if (last_pq_mode == VPP_PICTURE_MODE_MONITOR) {
            if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        } else {
            if (pq_mode == VPP_PICTURE_MODE_GAME) {//game mode on and monitor mode off;
                Cpq_SetGameMode(MODE_ON);
                Cpq_SetPCMode(MODE_OFF);
            } else if (pq_mode == VPP_PICTURE_MODE_MONITOR) {//game mode off and monitor mode on;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_ON);
            } else {//game mode off and monitor mode off;
                Cpq_SetGameMode(MODE_OFF);
                Cpq_SetPCMode(MODE_OFF);
            }
        }
    } else {//other source, game mode off and monitor mode off;
        if (mInitialized) {
            Cpq_SetGameMode(MODE_OFF);
            Cpq_SetPCMode(MODE_OFF);
        }
    }

    vpp_noise_reduction_mode_t nrMode = VPP_NOISE_REDUCTION_MODE_OFF;
    if (mbCpqCfg_nr_enable) {
        if ((pq_mode == VPP_PICTURE_MODE_MONITOR) || (pq_mode == VPP_PICTURE_MODE_GAME)) {
            nrMode = VPP_NOISE_REDUCTION_MODE_OFF;
        } else {
            nrMode = (vpp_noise_reduction_mode_t)GetNoiseReductionMode();
        }
        Cpq_SetNoiseReductionMode(nrMode, source_input_param);
    } else {
        LOGD("%s: nr2 moudle disabled!\n", __FUNCTION__);
    }

    Cpq_LoadBasicRegs(source_input_param, pq_mode);
    ret = GetPQParams(source_input_param, pq_mode, &pq_para);
    if (ret < 0) {
        LOGE("%s: Get PQ Params failed!\n", __FUNCTION__);
    } else {
        ret = SetPQParams(source_input_param, pq_mode, pq_para);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetPQParams(source_input_param_t source_input_param, vpp_picture_mode_t pq_mode, vpp_pq_para_t pq_para)
{
    int ret = 0;

    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        int hue_level = 0, hue = 50, saturation = 50;
        if (((source_input_param.source_input == SOURCE_TV) ||
              (source_input_param.source_input == SOURCE_AV1) ||
              (source_input_param.source_input == SOURCE_AV2)) &&
            ((source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_M) ||
             (source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_443))) {
            hue_level = 100 - pq_para.hue;
        } else {
            hue_level = 50;
        }

        ret = mPQdb->PQ_GetHueParams(source_input_param, hue_level, &hue);
        if (ret == 0) {
            ret = mPQdb->PQ_GetSaturationParams(source_input_param, pq_para.saturation, &saturation);
            if (ret == 0) {
                ret = Cpq_SetVideoSaturationHue(saturation, hue);
            } else {
                LOGE("%s: PQ_GetSaturationParams failed!\n", __FUNCTION__);
            }
        } else {
            LOGE("%s: PQ_GetHueParams failed!\n", __FUNCTION__);
        }
    }

    if (pq_mode != VPP_PICTURE_MODE_MONITOR) {
        ret |= Cpq_SetSharpness(pq_para.sharpness, source_input_param);
    }
    ret |= Cpq_SetBrightness(pq_para.brightness, source_input_param);
    ret |= Cpq_SetContrast(pq_para.contrast, source_input_param);

    return ret;
}


int CPQControl::GetPQParams(source_input_param_t source_input_param, vpp_picture_mode_t pq_mode, vpp_pq_para_t *pq_para)
{
    int ret = -1;
    if (pq_para == NULL) {
        LOGD("%s: pq_para is NULL!\n", __FUNCTION__);
    } else {
        if (pq_mode == VPP_PICTURE_MODE_USER) {
            mSSMAction->SSMReadBrightness(mSourceInputForSaveParam, &pq_para->brightness);
            mSSMAction->SSMReadContrast(mSourceInputForSaveParam, &pq_para->contrast);
            mSSMAction->SSMReadSaturation(mSourceInputForSaveParam, &pq_para->saturation);
            mSSMAction->SSMReadHue(mSourceInputForSaveParam, &pq_para->hue);
            mSSMAction->SSMReadSharpness(mSourceInputForSaveParam, &pq_para->sharpness);
            ret = 0;
        } else {
            if (mbCpqCfg_seperate_db_enable) {
                ret = mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, pq_mode, pq_para);
            } else {
                ret = mPQdb->PQ_GetPQModeParams(source_input_param.source_input, pq_mode, pq_para);
            }
        }
    }

    if (ret != 0) {
        LOGE("%s failed!\n", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}

//color temperature gamma
int CPQControl::SetUserGammaValue(int level, vpp_color_temperature_mode_t color_mode)
{
    int ret = 0;
    tcon_gamma_table_t target_gamma_r, target_gamma_g, target_gamma_b;

    LOGD("%s, customer_gamma_level is %d color_mode:%d\n", __FUNCTION__, level, color_mode);

    //1. read curve gamma from db(data is unsigned int)
    if (color_mode == VPP_COLOR_TEMPERATURE_MODE_STANDARD) {
        mPQdb->mColorTemperatureMode = LVDS_STD;
    } else if (color_mode == VPP_COLOR_TEMPERATURE_MODE_WARM) {
        mPQdb->mColorTemperatureMode = LVDS_WARM;
    } else if (color_mode == VPP_COLOR_TEMPERATURE_MODE_COLD) {
        mPQdb->mColorTemperatureMode = LVDS_COOL;
    } else if (color_mode == VPP_COLOR_TEMPERATURE_MODE_USER) {
        mPQdb->mColorTemperatureMode = LVDS_USER;
    }

    ret = mPQdb->PQ_GetGammaParams(mCurentSourceInputInfo, (vpp_gamma_curve_t)level, "Red", &target_gamma_r);
    ret |= mPQdb->PQ_GetGammaParams(mCurentSourceInputInfo, (vpp_gamma_curve_t)level, "Green", &target_gamma_g);
    ret |= mPQdb->PQ_GetGammaParams(mCurentSourceInputInfo, (vpp_gamma_curve_t)level, "Blue", &target_gamma_b);

    //2. read cri_data gamma or cool/standard/warm gamma from db(data is unsigned short and 10bit valid)

    //3. blend gamma(unsigned int->unsigned short and 10bit valid)

    //4. load gamma to driver
    if (ret == 0) {
        Cpq_SetGammaTbl_R(target_gamma_r.data);
        Cpq_SetGammaTbl_G(target_gamma_g.data);
        Cpq_SetGammaTbl_B(target_gamma_b.data);
    }

    return ret;
}

//color temperature
int CPQControl::SetColorTemperature(int temp_mode, int is_save, rgb_ogo_type_t rgb_ogo_type, int value)
{
    int ret = -1;
    LOGD("%s: source:%d, mode: %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, temp_mode);
    if (mbCpqCfg_whitebalance_enable) {
        if (temp_mode == VPP_COLOR_TEMPERATURE_MODE_USER) {
            ret = Cpq_SetColorTemperatureUser(mCurentSourceInputInfo.source_input, rgb_ogo_type, is_save, value);
        } else {
            ret = Cpq_SetColorTemperatureWithoutSave((vpp_color_temperature_mode_t)temp_mode, mCurentSourceInputInfo.source_input);
        }

        if ((ret == 0) && (is_save == 1)) {
            ret = SaveColorTemperature((vpp_color_temperature_mode_t)temp_mode);
        }
    } else {
        LOGD("whitebalance moudle disabled!\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}


int CPQControl::GetColorTemperature(void)
{
    int mode = VPP_COLOR_TEMPERATURE_MODE_STANDARD;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            mode = para.ColorTemperature;
        }
    } else {
        mSSMAction->SSMReadColorTemperature(mSourceInputForSaveParam, &mode);
    }

    if (mode < VPP_COLOR_TEMPERATURE_MODE_STANDARD || mode > VPP_COLOR_TEMPERATURE_MODE_USER) {
        mode = VPP_COLOR_TEMPERATURE_MODE_STANDARD;
    }

    LOGD("%s: source: %d, timming: %d mode: %d!\n",__FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, mode);
    return mode;
}

int CPQControl::SaveColorTemperature(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.ColorTemperature = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveColorTemperature(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

tcon_rgb_ogo_t CPQControl::GetColorTemperatureUserParam(void) {
    tcon_rgb_ogo_t param;
    memset(&param, 0, sizeof(tcon_rgb_ogo_t));
    Cpq_GetColorTemperatureUser(mCurentSourceInputInfo.source_input, &param);
    return param;
}

int CPQControl::Cpq_SetColorTemperatureWithoutSave(vpp_color_temperature_mode_t Tempmode, tv_source_input_t tv_source_input __unused)
{
    tcon_rgb_ogo_t rgbogo;

    GetColorTemperatureParams(Tempmode, &rgbogo);

    if (GetEyeProtectionMode(mCurentSourceInputInfo.source_input))//if eye protection mode is enable, b_gain / 2.
        rgbogo.b_gain /= 2;

    return Cpq_SetRGBOGO(&rgbogo);
}

int CPQControl::Cpq_CheckColorTemperatureParamAlldata(source_input_param_t source_input_param)
{
    int ret= -1;
    unsigned short ret1 = 0, ret2 = 0;

    //check color temperature data lable(254:0x55,255:0XAA)
    ret = Cpq_CheckTemperatureDataLable();
    //calculate checksum
    ret1 = Cpq_CalColorTemperatureParamsChecksum();
    //read checksum(256/257)
    ret2 = Cpq_GetColorTemperatureParamsChecksum();

    if (ret && (ret1 == ret2)) {
        LOGD("%s, color temperature param lable & checksum ok\n", __FUNCTION__);
        if (Cpq_CheckColorTemperatureParams() == 0) {
            LOGD("%s, color temperature params check failed\n", __FUNCTION__);
            Cpq_RestoreColorTemperatureParamsFromDB(source_input_param);
         }
    } else {
        LOGD("%s, color temperature param data error\n", __FUNCTION__);
        Cpq_SetTemperatureDataLable();
        Cpq_RestoreColorTemperatureParamsFromDB(source_input_param);
    }

    return 0;
}

unsigned short CPQControl::Cpq_CalColorTemperatureParamsChecksum(void)
{
    unsigned char data_buf[SSM_CR_RGBOGO_LEN];
    unsigned short sum = 0;
    int cnt;

    mSSMAction->SSMReadRGBOGOValue(0, SSM_CR_RGBOGO_LEN, data_buf);

    for (cnt = 0; cnt < SSM_CR_RGBOGO_LEN; cnt++) {
        sum += data_buf[cnt];
    }

    LOGD("%s, sum = 0x%X\n", __FUNCTION__, sum);

    return sum;
}

int CPQControl::Cpq_SetColorTemperatureParamsChecksum(void)
{
    int ret = 0;
    USUC usuc;

    usuc.s = Cpq_CalColorTemperatureParamsChecksum();

    LOGD("%s, sum = 0x%X\n", __FUNCTION__, usuc.s);

    ret |= mSSMAction->SSMSaveRGBOGOValue(SSM_CR_RGBOGO_LEN, SSM_CR_RGBOGO_CHKSUM_LEN, usuc.c);

    return ret;
}

unsigned short CPQControl::Cpq_GetColorTemperatureParamsChecksum(void)
{
    USUC usuc;

    mSSMAction->SSMReadRGBOGOValue(SSM_CR_RGBOGO_LEN, SSM_CR_RGBOGO_CHKSUM_LEN, usuc.c);

    LOGD("%s, sum = 0x%X\n", __FUNCTION__, usuc.s);

    return usuc.s;
}

int CPQControl::Cpq_SetColorTemperatureUser(tv_source_input_t source_input, rgb_ogo_type_t rgb_ogo_type, int is_save, int value)
{
    LOGD("%s: type: %d, value: %u\n", __FUNCTION__, rgb_ogo_type, value);
    int ret = -1;
    tcon_rgb_ogo_t rgbogo;
    memset(&rgbogo, 0, sizeof(tcon_rgb_ogo_t));
    ret = Cpq_GetColorTemperatureUser(source_input, &rgbogo);

    switch (rgb_ogo_type)
    {
        case R_GAIN:
            rgbogo.r_gain = (unsigned)value;
        break;
        case G_GAIN:
            rgbogo.g_gain = (unsigned)value;
        break;
        case B_GAIN:
            rgbogo.b_gain = (unsigned)value;
        break;
        case R_POST_OFFSET:
            rgbogo.r_post_offset = value;
        break;
        case G_POST_OFFSET:
            rgbogo.g_post_offset = value;
        break;
        case B_POST_OFFSET:
            rgbogo.b_post_offset = value;
        break;
        default:
            ret = -1;
        break;
    }

    if (GetEyeProtectionMode(source_input) == 1) {
        LOGD("eye protection mode is enable\n");
        rgbogo.b_gain /= 2;
    }

    if (ret != -1) {
       ret = Cpq_SetRGBOGO(&rgbogo);
    }

    if ((ret != -1) && (is_save == 1)) {
        ret = Cpq_SaveColorTemperatureUser(source_input, rgb_ogo_type, value);
    }

    if (ret < 0) {
        LOGD("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::Cpq_GetColorTemperatureUser(tv_source_input_t source_input __unused, tcon_rgb_ogo_t* p_tcon_rgb_ogo)
{
    int ret = 0;
    if (p_tcon_rgb_ogo != NULL) {
        p_tcon_rgb_ogo->en = 1;
        p_tcon_rgb_ogo->r_pre_offset = 0;
        p_tcon_rgb_ogo->g_pre_offset = 0;
        p_tcon_rgb_ogo->b_pre_offset = 0;
        ret |= mSSMAction->SSMReadRGBGainRStart(0, &p_tcon_rgb_ogo->r_gain);
        ret |= mSSMAction->SSMReadRGBGainGStart(0, &p_tcon_rgb_ogo->g_gain);
        ret |= mSSMAction->SSMReadRGBGainBStart(0, &p_tcon_rgb_ogo->b_gain);
        ret |= mSSMAction->SSMReadRGBPostOffsetRStart(0, &p_tcon_rgb_ogo->r_post_offset);
        ret |= mSSMAction->SSMReadRGBPostOffsetGStart(0, &p_tcon_rgb_ogo->g_post_offset);
        ret |= mSSMAction->SSMReadRGBPostOffsetBStart(0, &p_tcon_rgb_ogo->b_post_offset);
    } else {
        LOGD("%s: buf is null\n", __FUNCTION__);
        ret = -1;
    }

    if (ret < 0) {
        LOGD("%s failed\n", __FUNCTION__);
        ret = -1;
    }

    return ret;
}

int CPQControl::Cpq_SaveColorTemperatureUser(tv_source_input_t source_input __unused, rgb_ogo_type_t rgb_ogo_type, int value)
{
    LOGD("%s: rgb_ogo_type[%d]:[%d]", __FUNCTION__, rgb_ogo_type, value);

    int ret = 0;
    switch (rgb_ogo_type)
    {
        case R_GAIN:
            ret |= mSSMAction->SSMSaveRGBGainRStart(0, (unsigned)value);
        break;
        case G_GAIN:
            ret |= mSSMAction->SSMSaveRGBGainGStart(0, (unsigned)value);
        break;
        case B_GAIN:
            ret |= mSSMAction->SSMSaveRGBGainBStart(0, (unsigned)value);
        break;
        case R_POST_OFFSET:
            ret |= mSSMAction->SSMSaveRGBPostOffsetRStart(0, value);
        break;
        case G_POST_OFFSET:
            ret |= mSSMAction->SSMSaveRGBPostOffsetGStart(0, value);
        break;
        case B_POST_OFFSET:
            ret |= mSSMAction->SSMSaveRGBPostOffsetBStart(0, value);
        break;
        default:
            ret = -1;
        break;
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_RestoreColorTemperatureParamsFromDB(source_input_param_t source_input_param)
{
    int i = 0;
    tcon_rgb_ogo_t rgbogo;

    for (i = 0; i < 3; i++) {
        mPQdb->PQ_GetColorTemperatureParams((vpp_color_temperature_mode_t) i, source_input_param, &rgbogo);
        SaveColorTemperatureParams((vpp_color_temperature_mode_t) i, rgbogo);
    }

    Cpq_SetColorTemperatureParamsChecksum();

    return 0;
}

int CPQControl::Cpq_CheckTemperatureDataLable(void)
{
    USUC usuc;
    USUC ret;

    mSSMAction->SSMReadRGBOGOValue(SSM_CR_RGBOGO_LEN - 2, 2, ret.c);

    usuc.c[0] = 0x55;
    usuc.c[1] = 0xAA;

    if ((usuc.c[0] == ret.c[0]) && (usuc.c[1] == ret.c[1])) {
        LOGD("%s, lable ok\n", __FUNCTION__);
        return 1;
    } else {
        LOGE("%s, lable error\n", __FUNCTION__);
        return 0;
    }
}

int CPQControl::Cpq_SetTemperatureDataLable(void)
{
    USUC usuc;
    int ret = 0;

    usuc.c[0] = 0x55;
    usuc.c[1] = 0xAA;

    ret = mSSMAction->SSMSaveRGBOGOValue(SSM_CR_RGBOGO_LEN - 2, 2, usuc.c);

    return ret;
}

int CPQControl::SetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, tcon_rgb_ogo_t params)
{
    SaveColorTemperatureParams(Tempmode, params);
    Cpq_SetColorTemperatureParamsChecksum();

    return 0;
}

int CPQControl::GetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, tcon_rgb_ogo_t *params)
{
    SUC suc;
    USUC usuc;
    int ret = 0;
    if (VPP_COLOR_TEMPERATURE_MODE_STANDARD == Tempmode) { //standard
        ret |= mSSMAction->SSMReadRGBOGOValue(0, 2, usuc.c);
        params->en = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(2, 2, suc.c);
        params->r_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(4, 2, suc.c);
        params->g_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(6, 2, suc.c);
        params->b_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(8, 2, usuc.c);
        params->r_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(10, 2, usuc.c);
        params->g_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(12, 2, usuc.c);
        params->b_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(14, 2, suc.c);
        params->r_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(16, 2, suc.c);
        params->g_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(18, 2, suc.c);
        params->b_post_offset = suc.s;
    } else if (VPP_COLOR_TEMPERATURE_MODE_WARM == Tempmode) { //warm
        ret |= mSSMAction->SSMReadRGBOGOValue(20, 2, usuc.c);
        params->en = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(22, 2, suc.c);
        params->r_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(24, 2, suc.c);
        params->g_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(26, 2, suc.c);
        params->b_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(28, 2, usuc.c);
        params->r_gain = usuc.s;
        ret |= mSSMAction->SSMReadRGBOGOValue(30, 2, usuc.c);
        params->g_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(32, 2, usuc.c);
        params->b_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(34, 2, suc.c);
        params->r_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(36, 2, suc.c);
        params->g_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(38, 2, suc.c);
        params->b_post_offset = suc.s;
    } else if (VPP_COLOR_TEMPERATURE_MODE_COLD == Tempmode) { //cool
        ret |= mSSMAction->SSMReadRGBOGOValue(40, 2, usuc.c);
        params->en = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(42, 2, suc.c);
        params->r_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(44, 2, suc.c);
        params->g_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(46, 2, suc.c);
        params->b_pre_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(48, 2, usuc.c);
        params->r_gain = usuc.s;
        ret |= mSSMAction->SSMReadRGBOGOValue(50, 2, usuc.c);
        params->g_gain = usuc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(52, 2, usuc.c);
        params->b_gain = usuc.s;
        ret |= mSSMAction->SSMReadRGBOGOValue(54, 2, suc.c);
        params->r_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(56, 2, suc.c);
        params->g_post_offset = suc.s;

        ret |= mSSMAction->SSMReadRGBOGOValue(58, 2, suc.c);
        params->b_post_offset = suc.s;
    } else if (VPP_COLOR_TEMPERATURE_MODE_USER == Tempmode) { //cool
        Cpq_GetColorTemperatureUser(mCurentSourceInputInfo.source_input, params);
    }

    //use default value when read param from cri_data
    //and restore cri_data from db
    if (ret) {
        params->r_pre_offset = 0;
        params->g_pre_offset = 0;
        params->b_pre_offset = 0;
        params->r_gain = 1024;
        params->g_gain = 1024;
        params->b_gain = 1024;
        params->r_post_offset = 0;
        params->g_post_offset = 0;
        params->b_post_offset = 0;

        Cpq_RestoreColorTemperatureParamsFromDB(mCurentSourceInputInfo);
    }

    LOGD("%s, rgain[%d], ggain[%d],bgain[%d],roffset[%d],goffset[%d],boffset[%d]  ret[%d]\n", __FUNCTION__,
         params->r_gain, params->g_gain, params->b_gain, params->r_post_offset,
         params->g_post_offset, params->b_post_offset, ret);

    return ret;
}

int CPQControl::SaveColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, tcon_rgb_ogo_t params)
{
    SUC suc;
    USUC usuc;
    int ret = 0;

    if (VPP_COLOR_TEMPERATURE_MODE_STANDARD == Tempmode) { //standard
        usuc.s = params.en;
        ret |= mSSMAction->SSMSaveRGBOGOValue(0, 2, usuc.c);

        suc.s = params.r_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(2, 2, suc.c);

        suc.s = params.g_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(4, 2, suc.c);

        suc.s = params.b_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(6, 2, suc.c);

        usuc.s = params.r_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(8, 2, usuc.c);

        usuc.s = params.g_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(10, 2, usuc.c);

        usuc.s = params.b_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(12, 2, usuc.c);

        suc.s = params.r_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(14, 2, suc.c);

        suc.s = params.g_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(16, 2, suc.c);

        suc.s = params.b_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(18, 2, suc.c);
    } else if (VPP_COLOR_TEMPERATURE_MODE_WARM == Tempmode) { //warm
        usuc.s = params.en;
        ret |= mSSMAction->SSMSaveRGBOGOValue(20, 2, usuc.c);

        suc.s = params.r_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(22, 2, suc.c);

        suc.s = params.g_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(24, 2, suc.c);
        suc.s = params.b_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(26, 2, suc.c);

        usuc.s = params.r_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(28, 2, usuc.c);

        usuc.s = params.g_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(30, 2, usuc.c);

        usuc.s = params.b_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(32, 2, usuc.c);

        suc.s = params.r_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(34, 2, suc.c);

        suc.s = params.g_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(36, 2, suc.c);

        suc.s = params.b_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(38, 2, suc.c);
    } else if (VPP_COLOR_TEMPERATURE_MODE_COLD == Tempmode) { //cool
        usuc.s = params.en;
        ret |= mSSMAction->SSMSaveRGBOGOValue(40, 2, usuc.c);

        suc.s = params.r_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(42, 2, suc.c);

        suc.s = params.g_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(44, 2, suc.c);

        suc.s = params.b_pre_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(46, 2, suc.c);

        usuc.s = params.r_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(48, 2, usuc.c);

        usuc.s = params.g_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(50, 2, usuc.c);

        usuc.s = params.b_gain;
        ret |= mSSMAction->SSMSaveRGBOGOValue(52, 2, usuc.c);

        suc.s = params.r_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(54, 2, suc.c);

        suc.s = params.g_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(56, 2, suc.c);

        suc.s = params.b_post_offset;
        ret |= mSSMAction->SSMSaveRGBOGOValue(58, 2, suc.c);
    }

    LOGD("%s, rgain[%d], ggain[%d],bgain[%d],roffset[%d],goffset[%d],boffset[%d]\n", __FUNCTION__,
         params.r_gain, params.g_gain, params.b_gain, params.r_post_offset,
         params.g_post_offset, params.b_post_offset);
    return ret;
}

int CPQControl::Cpq_CheckColorTemperatureParams(void)
{
    int i = 0;
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    for (i = 0; i < 3; i++) {
        GetColorTemperatureParams((vpp_color_temperature_mode_t) i, &rgbogo);

        if (rgbogo.r_gain > 2047 || rgbogo.b_gain > 2047 || rgbogo.g_gain > 2047) {
            if (rgbogo.r_post_offset > 1023 || rgbogo.g_post_offset > 1023 || rgbogo.b_post_offset > 1023 ||
                rgbogo.r_post_offset < -1024 || rgbogo.g_post_offset < -1024 || rgbogo.b_post_offset < -1024) {
                return 0;
            }
        }
    }

    return 1;
}

//Brightness
int CPQControl::SetBrightness(int value, int is_save)
{
    int ret =0;
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetBrightness(value, mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveBrightness(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return 0;
}

int CPQControl::GetBrightness(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;

    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Brightness;
        }
    } else {
        if (GetPQParams(mCurentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.brightness;
        }
    }
    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, data);
    return data;
}

int CPQControl::SaveBrightness(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Brightness = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveBrightness(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}


int CPQControl::Cpq_SetBrightnessBasicParam(source_input_param_t source_input_param)
{
    int ret = -1;
    ret = mPQdb->LoadVppBasicParam(TVPQ_DATA_BRIGHTNESS, source_input_param);

    if (ret < 0)
        LOGE("%s failed\n",__FUNCTION__);


    return ret;
}

int CPQControl::Cpq_SetBrightness(int value, source_input_param_t source_input_param)
{
    int ret = -1;
    int params;
    int level;
    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        if (value >= 0 && value <= 100) {
            level = value;
            if (mPQdb->PQ_GetBrightnessParams(source_input_param, level, &params) == 0) {
                if (Cpq_SetVideoBrightness(params) == 0) {
                    return 0;
                } else {
                    LOGE("%s:Cpq_SetVideoBrightness failed\n", __FUNCTION__);
                }
            } else {
                LOGE("%s:PQ_GetBrightnessParams failed\n", __FUNCTION__);
            }
        }
    } else {
        LOGD("%s: brightness disabled\n", __FUNCTION__);
        ret = 0;
    }
    return ret;
}

int CPQControl::Cpq_SetVideoBrightness(int value)
{
    LOGD("%s brightness:%d\n", __FUNCTION__, value);
    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));
    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= 0x1;
        params.brightness = value;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1<<1);
        params.brightness2 = value;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

//Contrast
int CPQControl::SetContrast(int value, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    int ret = Cpq_SetContrast(value, mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveContrast(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetContrast(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Contrast;
        }
    } else {
        if (GetPQParams(mCurentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.contrast;
        }
    }
    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, data);
    return data;
}


int CPQControl::SaveContrast(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Contrast = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveContrast(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}


int CPQControl::Cpq_SetContrastBasicParam(source_input_param_t source_input_param)
{
    int ret = -1;
    ret = mPQdb->LoadVppBasicParam(TVPQ_DATA_CONTRAST, source_input_param);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetContrast(int value, source_input_param_t source_input_param)
{
    int ret = -1;
    int params;
    int level;
    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        if (value >= 0 && value <= 100) {
            level = value;
            if (mPQdb->PQ_GetContrastParams(source_input_param, level, &params) == 0) {
                if (Cpq_SetVideoContrast(params) == 0) {
                    return 0;
                } else {
                    LOGE("%s: Cpq_SetVideoContrast failed\n", __FUNCTION__);
                }
            } else {
                LOGE("%s: PQ_GetContrastParams failed\n", __FUNCTION__);
            }
        }
    } else {
        LOGD("%s: contrast disabled\n", __FUNCTION__);
        ret = 0;
    }

    return ret;
}

int CPQControl::Cpq_SetVideoContrast(int value)
{
    LOGD("%s Contrast:%d", __FUNCTION__, value);

    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));

    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= (0x1<<4);
        params.contrast = value;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1<<5);
        params.contrast2 = value;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

//Saturation
int CPQControl::SetSaturation(int value, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, value);
    int ret = Cpq_SetSaturation(value, mCurentSourceInputInfo);
    if ((ret == 0) && (is_save == 1)) {
        ret = SaveSaturation(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSaturation(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Saturation;
        }
    } else {
        if (GetPQParams(mCurentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.saturation;
        }
    }

    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, data);
    return data;
}

int CPQControl::SaveSaturation(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Saturation = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveSaturation(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetSaturationBasicParam(source_input_param_t source_input_param)
{
    int ret = -1;
    ret = mPQdb->LoadVppBasicParam(TVPQ_DATA_SATURATION, source_input_param);

    if (ret < 0)
        LOGE("%s failed\n",__FUNCTION__);


    return ret;
}

int CPQControl::Cpq_SetSaturation(int value, source_input_param_t source_input_param)
{
    int ret = -1;
    int saturation = 0, hue = 0;
    int satuation_level = 0, hue_level = 0;
    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        if (value >= 0 && value <= 100) {
            satuation_level = value;
            if (((source_input_param.source_input == SOURCE_TV) ||
                (source_input_param.source_input == SOURCE_AV1) ||
                (source_input_param.source_input == SOURCE_AV2)) &&
                ((source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_M) ||
                (source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_443))) {
                hue_level = 100 - GetHue();
            } else {
                hue_level = 50;
            }
            ret = mPQdb->PQ_GetHueParams(source_input_param, hue_level, &hue);
            if (ret == 0) {
                ret = mPQdb->PQ_GetSaturationParams(source_input_param, satuation_level, &saturation);
                if (ret == 0) {
                    ret = Cpq_SetVideoSaturationHue(saturation, hue);
                } else {
                    LOGE("%s: PQ_GetSaturationParams failed\n", __FUNCTION__);
                }
            } else {
                LOGE("%s: PQ_GetHueParams failed\n", __FUNCTION__);
            }
        }
    }else {
        LOGD("%s: satuation disabled\n", __FUNCTION__);
        ret = 0;
    }

    return ret;
}

//Hue
int CPQControl::SetHue(int value, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, value);
    int ret = Cpq_SetHue(value, mCurentSourceInputInfo);
    if ((ret == 0) && (is_save == 1)) {
        ret = SaveHue(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}


int CPQControl::GetHue(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Hue;
        }
    } else {
        if (GetPQParams(mCurentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.hue;
        }
    }

    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, timming: %d value = %d\n", __FUNCTION__, mSourceInputForSaveParam,mCurentPqSource.pq_sig_fmt, data);
    return data;
}

int CPQControl::SaveHue(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Hue = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveHue(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetHueBasicParam(source_input_param_t source_input_param)
{
    int ret = -1;
    ret = mPQdb->LoadVppBasicParam(TVPQ_DATA_HUE, source_input_param);

    if (ret < 0)
        LOGE("%s failed\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetHue(int value, source_input_param_t source_input_param)
{
    int ret = -1;
    int hue_params = 0, saturation_params = 0;
    int hue_level = 0, saturation_level = 0;
    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        if (value >= 0 && value <= 100) {
            hue_level = 100 - value;
            ret = mPQdb->PQ_GetHueParams(source_input_param, hue_level, &hue_params);
            if (ret == 0) {
                saturation_level = GetSaturation();
                ret = mPQdb->PQ_GetSaturationParams(source_input_param, saturation_level, &saturation_params);
                if (ret == 0) {
                    ret = Cpq_SetVideoSaturationHue(saturation_params, hue_params);
                } else {
                    LOGE("%s:PQ_GetSaturationParams failed\n", __FUNCTION__);
                }
            } else {
                LOGE("%s:PQ_GetHueParams failed\n", __FUNCTION__);
            }
        }
    } else {
        LOGD("%s: hue disabled\n", __FUNCTION__);
        ret = 0;
    }

    return ret;
}

int CPQControl::Cpq_SetVideoSaturationHue(int satVal, int hueVal)
{
    signed long temp;
    LOGD("%s: satVal:%d hueVal:%d\n", __FUNCTION__, satVal, hueVal);
    am_pic_mode_t params;
    memset(&params, 0, sizeof(params));
    video_set_saturation_hue(satVal, hueVal, &temp);

    if (mbCpqCfg_amvecm_basic_enable) {
        params.flag |= (0x1<<2);
        params.saturation_hue = temp;
    }

    if (mbCpqCfg_amvecm_basic_withOSD_enable) {
        params.flag |= (0x1<<3);
        params.saturation_hue_post = temp;
    }

    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
    if (ret < 0) {
        LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

void CPQControl::video_set_saturation_hue(signed char saturation, signed char hue, signed long *mab)
{
    signed short ma = (signed short) (cos((float) hue * PI / 128.0) * ((float) saturation / 128.0
                                      + 1.0) * 256.0);
    signed short mb = (signed short) (sin((float) hue * PI / 128.0) * ((float) saturation / 128.0
                                      + 1.0) * 256.0);

    if (ma > 511) {
        ma = 511;
    }

    if (ma < -512) {
        ma = -512;
    }

    if (mb > 511) {
        mb = 511;
    }

    if (mb < -512) {
        mb = -512;
    }

    *mab = ((ma & 0x3ff) << 16) | (mb & 0x3ff);
}

void CPQControl::video_get_saturation_hue(signed char *sat, signed char *hue, signed long *mab)
{
    signed long temp = *mab;
    signed int ma = (signed int) ((temp << 6) >> 22);
    signed int mb = (signed int) ((temp << 22) >> 22);
    signed int sat16 = (signed int) ((sqrt(
                                          ((float) ma * (float) ma + (float) mb * (float) mb) / 65536.0) - 1.0) * 128.0);
    signed int hue16 = (signed int) (atan((float) mb / (float) ma) * 128.0 / PI);

    if (sat16 > 127) {
        sat16 = 127;
    }

    if (sat16 < -128) {
        sat16 = -128;
    }

    if (hue16 > 127) {
        hue16 = 127;
    }

    if (hue16 < -128) {
        hue16 = -128;
    }

    *sat = (signed char) sat16;
    *hue = (signed char) hue16;
}

//sharpness
int CPQControl::SetSharpness(int value, int is_enable __unused, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentPqSource.pq_source_input, value);
    int ret = Cpq_SetSharpness(value, mCurentSourceInputInfo);

    if ((ret== 0) && (is_save == 1)) {
        ret = SaveSharpness(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSharpness(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Sharpness;
        }
    } else {
        if (GetPQParams(mCurentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.sharpness;
        }
    }

    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, data);
    return data;
}

int CPQControl::SaveSharpness(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Sharpness = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveSharpness(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::Cpq_SetSharpness(int value, source_input_param_t source_input_param)
{
    int ret = -1;
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        ret = 0;
    } else {
        am_regs_t regs;
        memset(&regs, 0, sizeof(am_regs_t));
        int level;
        if (value >= 0 && value <= 100) {
            level = value;
            if (mbCpqCfg_sharpness0_enable) {
                ret = mPQdb->PQ_GetSharpness0Params(source_input_param, level, &regs);
                if (ret == 0) {
                    ret = Cpq_LoadRegs(regs);
                } else {
                    LOGE("%s: PQ_GetSharpness0Params failed\n", __FUNCTION__);
                }
            } else {
                LOGD("%s: sharpness0 disabled\n", __FUNCTION__);
                ret = 0;
            }

            if (mbCpqCfg_sharpness1_enable) {
                ret = mPQdb->PQ_GetSharpness1Params(source_input_param, level, &regs);
                if (ret == 0) {
                    ret = Cpq_LoadRegs(regs);
                } else {
                    LOGE("%s: PQ_GetSharpness1Params failed\n", __FUNCTION__);
                }
            } else {
                LOGD("%s: sharpness1 disabled\n", __FUNCTION__);
                ret = 0;
            }
        }else {
            LOGE("%s: invalid value\n", __FUNCTION__);
        }
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0FixedParam(source_input_param_t source_input_param)
{
    int ret = -1;

    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        ret = 0;
    } else {
        am_regs_t regs;
        memset(&regs, 0, sizeof(am_regs_t));
        ret = mPQdb->PQ_GetSharpness0FixedParams(source_input_param, &regs);
        if (ret < 0) {
            LOGE("%s: PQ_GetSharpness0FixedParams failed\n", __FUNCTION__);
        } else {
            ret = Cpq_LoadRegs(regs);
        }
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetSharpness0VariableParam(source_input_param_t source_input_param)
{
    int ret = -1;

    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        ret = 0;
    } else {
        ret = mPQdb->PQ_SetSharpness0VariableParams(source_input_param);
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetSharpness1FixedParam(source_input_param_t source_input_param)
{
    int ret = -1;
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        ret = 0;
    } else {
        am_regs_t regs;
        memset(&regs, 0, sizeof(am_regs_t));
        ret = mPQdb->PQ_GetSharpness1FixedParams(source_input_param, &regs);
        if (ret < 0) {
            LOGE("%s: PQ_GetSharpness1FixedParams failed\n", __FUNCTION__);
        } else {
            ret = Cpq_LoadRegs(regs);
        }
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetSharpness1VariableParam(source_input_param_t source_input_param)
{
    int ret = -1;
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        ret = 0;
    } else {
        ret = mPQdb->PQ_SetSharpness1VariableParams(source_input_param);
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

//NoiseReductionMode
int CPQControl::SetNoiseReductionMode(int nr_mode, int is_save)
{
    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, mCurentPqSource.pq_source_input, mCurentPqSource.pq_sig_fmt, nr_mode);
    int ret = Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)nr_mode, mCurentSourceInputInfo);

    if ((ret ==0) && (is_save == 1)) {
        ret = SaveNoiseReductionMode((vpp_noise_reduction_mode_t)nr_mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetNoiseReductionMode(void)
{
    int mode = VPP_NOISE_REDUCTION_MODE_MID;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            mode = para.Nr;
        }

    } else {
        mSSMAction->SSMReadNoiseReduction(mSourceInputForSaveParam, &mode);
    }

    if (mode < VPP_NOISE_REDUCTION_MODE_OFF || mode > VPP_NOISE_REDUCTION_MODE_AUTO) {
        mode = VPP_NOISE_REDUCTION_MODE_MID;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);
    return mode;
}

int CPQControl::SaveNoiseReductionMode(int nr_mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Nr = nr_mode;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveNoiseReduction(mSourceInputForSaveParam, nr_mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetNoiseReductionMode(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));

    if (mbCpqCfg_nr_enable) {
        if (mPQdb->PQ_GetNR2Params((vpp_noise_reduction_mode_t)nr_mode, source_input_param, &regs) == 0) {
            di_regs.table_name = TABLE_NAME_NR;
            di_regs.table_len = regs.length;
            am_reg_t tmp_buf[regs.length];
            for (unsigned int i=0;i<regs.length;i++) {
                  tmp_buf[i].addr = regs.am_reg[i].addr;
                  tmp_buf[i].mask = regs.am_reg[i].mask;
                  tmp_buf[i].type = regs.am_reg[i].type;
                  tmp_buf[i].val  = regs.am_reg[i].val;
            }
            di_regs.table_ptr = tmp_buf;

            ret = DI_LoadRegs(di_regs);
        } else {
            LOGE("PQ_GetNR2Params failed\n");
        }
    } else {
        ret = 0;
        LOGE("NoiseReduction is disabled\n");
    }

    if (ret < 0)
        LOGE("%s failed\n",__FUNCTION__);

    return ret;
}

//Gamma
int CPQControl::SetGammaValue(vpp_gamma_curve_t gamma_curve, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, gamma_curve);
    int ret = -1;
    if (mbCpqCfg_gamma_enable) {
        ret = Cpq_LoadGamma(gamma_curve);
        if ((ret == 0) && (is_save == 1)) {
            ret = mSSMAction->SSMSaveGammaValue(mCurentSourceInputInfo.source_input, gamma_curve);
        }
    } else {
        LOGD("Gamma moudle disabled!\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetGammaValue()
{
    int gammaValue = 0;
    if (mSSMAction->SSMReadGammaValue(mCurentSourceInputInfo.source_input, &gammaValue) < 0) {
        LOGE("%s, SSMReadGammaValue ERROR\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, gammaValue);
    return gammaValue;
}

int CPQControl::Cpq_LoadGamma(vpp_gamma_curve_t gamma_curve)
{
    int ret = 0;
    tcon_gamma_table_t gamma_r, gamma_g, gamma_b;

    ret |= mPQdb->PQ_GetGammaSpecialTable(gamma_curve, "Red", &gamma_r);
    ret |= mPQdb->PQ_GetGammaSpecialTable(gamma_curve, "Green", &gamma_g);
    ret |= mPQdb->PQ_GetGammaSpecialTable(gamma_curve, "Blue", &gamma_b);

    if (ret < 0) {
        LOGE("%s, PQ_GetGammaSpecialTable failed!\n", __FUNCTION__);
    } else {
        Cpq_SetGammaTbl_R((unsigned short *) gamma_r.data);
        Cpq_SetGammaTbl_G((unsigned short *) gamma_g.data);
        Cpq_SetGammaTbl_B((unsigned short *) gamma_b.data);
    }

    return ret;
}

int CPQControl::Cpq_SetGammaTbl_R(unsigned short red[256])
{
    struct tcon_gamma_table_s Redtbl;
    int ret = -1, i = 0;

    for (i = 0; i < 256; i++) {
        Redtbl.data[i] = red[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_R, &Redtbl);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }
    return ret;
}

int CPQControl::Cpq_SetGammaTbl_G(unsigned short green[256])
{
    struct tcon_gamma_table_s Greentbl;
    int ret = -1, i = 0;

    for (i = 0; i < 256; i++) {
        Greentbl.data[i] = green[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_G, &Greentbl);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetGammaTbl_B(unsigned short blue[256])
{
    struct tcon_gamma_table_s Bluetbl;
    int ret = -1, i = 0;

    for (i = 0; i < 256; i++) {
        Bluetbl.data[i] = blue[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_B, &Bluetbl);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

//MEMC
bool CPQControl::hasMemcFunc() {
    if (mMemcFd > 0 && mbCpqCfg_memc_enable) {
        LOGD("%s, has memc\n", __FUNCTION__);
        return true;
    }
    LOGD("%s, has NO memc\n", __FUNCTION__);
    return false;
}

int CPQControl::initMemc(void) {
    int ret = -1;
    if (mbCpqCfg_memc_enable) {
        if (mMemcFd > 0) {
            Memc_enable(1);
            ret = 0;
        } else {
            ret = -1;
        }
    } else {
        LOGD("%s, memc disabled\n", __FUNCTION__);
        ret = 0;
    }

    if (ret >= 0) {
        LOGD("%s, sucess\n", __FUNCTION__);
    } else {
        LOGE("%s, fail\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::Memc_enable(int enable)
{
    int ret = -1;
    ret = MEMCDeviceIOCtl(MEMDEV_CONTRL, &enable);

    if (ret >= 0) {
        LOGD("%s, sucess\n", __FUNCTION__);
    } else {
        LOGE("%s, fail\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::SetMemcMode(int memc_mode, int is_save)
{
    LOGD("%s, mode = %d\n", __FUNCTION__, memc_mode);
    int ret = -1;

    ret =Cpq_SetMemcMode((vpp_memc_mode_t)memc_mode, mCurentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        ret = SaveMemcMode((vpp_memc_mode_t)memc_mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMemcMode()
{
    int level = VPP_MEMC_MODE_OFF;
    if (mSSMAction->SSMReadMemcMode(mSourceInputForSaveParam, &level) < 0) {
        LOGE("%s, SSMReadMemcDeblurLevel ERROR!!!\n", __FUNCTION__);
        return VPP_MEMC_MODE_OFF;
    } else {
        LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    }

    if (level < VPP_MEMC_MODE_OFF || level >= VPP_MEMC_MODE_MAX) {
        level = VPP_MEMC_MODE_OFF;
    }

    return level;
}

int CPQControl::SaveMemcMode(vpp_memc_mode_t memc_mode)
{
    LOGD("%s, source: %d, mode = %d\n", __FUNCTION__, mSourceInputForSaveParam, memc_mode);
    int ret = mSSMAction->SSMSaveMemcMode(mSourceInputForSaveParam, memc_mode);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMemcMode(vpp_memc_mode_t memc_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    int DeJudder_level = 0;
    int DeBlur_Level = 0;
    int offset = source_input_param.source_input * VPP_MEMC_MODE_MAX + memc_mode;

    if (mMemcFd > 0 && mbCpqCfg_memc_enable) {
        if (mSSMAction->SSMReadMemcDeJudderLevel(offset, &DeJudder_level) < 0) {
            LOGE("%s, SSMReadMemcDeJudderLevel ERROR!!!\n", __FUNCTION__);
            ret = -1;
        }

        if (mSSMAction->SSMReadMemcDeblurLevel(offset, &DeBlur_Level) < 0) {
            LOGE("%s, SSMReadMemcDeblurLevel ERROR!!!\n", __FUNCTION__);
            ret = -1;
        }

        ret = Cpq_SetMemcDeJudderLevel(DeJudder_level, mCurentSourceInputInfo);
        ret |= Cpq_SetMemcDeBlurLevel(DeBlur_Level, mCurentSourceInputInfo);

    } else {
        LOGE("Memc moudle disabled!!!\n");
        ret = 0;
    }

    return ret;
}

int CPQControl::SetMemcDeBlurLevel(int level, int is_save)
{
    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    int ret = -1;

    ret = Cpq_SetMemcDeBlurLevel(level, mCurentSourceInputInfo);
    if ((ret ==0) && (is_save == 1)) {
        ret = SaveMemcDeBlurLevel(level);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMemcDeBlurLevel(void)
{
    int level = 0;
    int mode = GetMemcMode();
    int offset = mSourceInputForSaveParam * VPP_MEMC_MODE_MAX + mode;
    if (mSSMAction->SSMReadMemcDeblurLevel(offset, &level) < 0) {
        LOGE("%s, SSMReadMemcDeblurLevel ERROR!!!\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s, source: %d, level = %d offset = %d\n", __FUNCTION__, mSourceInputForSaveParam, level, offset);
    return level;
}

int CPQControl::SaveMemcDeBlurLevel(int level)
{
    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    int mode = GetMemcMode();
    int offset = mSourceInputForSaveParam * VPP_MEMC_MODE_MAX + mode;
    int ret = mSSMAction->SSMSaveMemcDeblurLevel(offset, level);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetMemcDeBlurLevel(int level, source_input_param_t source_input_param)
{
    int ret = -1;
    if (mbCpqCfg_memc_enable) {
        //ret = MEMCDeviceIOCtl(FRC_IOC_SET_MEMC_LEVEL, &level);
        ret = 0;
    } else {
        LOGD("%s memc Disabled!\n",__FUNCTION__);
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetMemcDeJudderLevel(int level, int is_save)
{
    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    int ret = -1;
    ret = Cpq_SetMemcDeJudderLevel(level, mCurentSourceInputInfo);
    if ((ret ==0) && (is_save == 1)) {
        ret = SaveMemcDeJudderLevel(level);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetMemcDeJudderLevel(void)
{
    int level = 0;
    int mode = GetMemcMode();
    int offset = mSourceInputForSaveParam * VPP_MEMC_MODE_MAX + mode;
    if (mSSMAction->SSMReadMemcDeJudderLevel(offset, &level) < 0) {
        LOGE("%s, SSMReadMemcDeJudderLevel ERROR!!!\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    return level;
}

int CPQControl::SaveMemcDeJudderLevel(int level)
{
    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    int mode = GetMemcMode();
    int offset = mSourceInputForSaveParam * VPP_MEMC_MODE_MAX + mode;
    int ret = mSSMAction->SSMSaveMemcDeJudderLevel(offset, level);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMemcDeJudderLevel(int level, source_input_param_t source_input_param)
{
    int ret = -1;
    if (mbCpqCfg_memc_enable) {
        ret = MEMCDeviceIOCtl(FRC_IOC_SET_MEMC_LEVEL, &level);
    } else {
        LOGD("%s memc Disabled!\n",__FUNCTION__);
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

//Displaymode
int CPQControl::SetDisplayMode(vpp_display_mode_t display_mode, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, display_mode);
    int ret = -1;
    if (mbCpqCfg_display_overscan_enable) {
        if ((mCurentSourceInputInfo.source_input == SOURCE_DTV) || (mCurentSourceInputInfo.source_input == SOURCE_TV)) {
            ret = Cpq_SetDisplayModeAllTiming(mCurentSourceInputInfo.source_input, display_mode);
        } else {
            ret = Cpq_SetDisplayModeAllTiming(mCurentSourceInputInfo.source_input, display_mode);
            ret = Cpq_SetDisplayModeOneTiming(mCurentSourceInputInfo.source_input, display_mode);
        }

        if ((ret == 0) && (is_save == 1))
            ret = SaveDisplayMode(display_mode);
    } else {
        LOGD("%s:Display overscan disabled!\n", __FUNCTION__);
        ret= 0;
    }

    return ret;
}

int CPQControl::GetDisplayMode()
{
    int mode = VPP_DISPLAY_MODE_169;
    mSSMAction->SSMReadDisplayMode(mCurentSourceInputInfo.source_input, &mode);
    if (mode < VPP_DISPLAY_MODE_169 || mode >= VPP_DISPLAY_MODE_MAX) {
        mode = VPP_DISPLAY_MODE_169;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, mode);
    return mode;
}

int CPQControl::SaveDisplayMode(vpp_display_mode_t display_mode)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, display_mode);
    int ret = mSSMAction->SSMSaveDisplayMode(mCurentSourceInputInfo.source_input, (int)display_mode);
    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetDisplayModeOneTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int ret = -1;
    tvin_cutwin_t cutwin;

    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(mCurentSourceInputInfo, display_mode, &cutwin);
    } else {
        ret = mPQdb->PQ_GetOverscanParams(mCurentSourceInputInfo, display_mode, &cutwin);
    }

    if (ret == 0) {
        int ScreenModeValue = Cpq_GetScreenModeValue(display_mode);
        if (source_input == SOURCE_MPEG) {//MPEG
            //DtvKit AFD or Local video
            if ((mbDtvKitEnable && (ScreenModeValue == SCREEN_MODE_NORMAL)) ||
                (!mbDtvKitEnable)) {
                ScreenModeValue = SCREEN_MODE_FULL_STRETCH;
                cutwin.vs = 0;
                cutwin.hs = 0;
                cutwin.ve = 0;
                cutwin.he = 0;
            }
        } else if ((source_input >= SOURCE_HDMI1) && (source_input <= SOURCE_HDMI4) &&
                   (GetPQMode() == VPP_PICTURE_MODE_MONITOR)) {//hdmi monitor mode
                cutwin.vs = 0;
                cutwin.hs = 0;
                cutwin.ve = 0;
                cutwin.he = 0;
        }

        LOGD("%s: screenmode:%d hs:%d he:%d vs:%d ve:%d\n", __FUNCTION__, ScreenModeValue, cutwin.hs, cutwin.he, cutwin.vs, cutwin.ve);
        Cpq_SetVideoCrop(cutwin.vs, cutwin.hs, cutwin.ve, cutwin.he);
        Cpq_SetVideoScreenMode(ScreenModeValue);
    } else {
        LOGD("PQ_GetOverscanParams failed!\n");
    }

    return ret;
}

int CPQControl::Cpq_SetDisplayModeAllTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int i = 0, ScreenModeValue = 0;
    int ret = -1;
    ve_pq_load_t ve_pq_load_reg;
    memset(&ve_pq_load_reg, 0, sizeof(ve_pq_load_t));

    ve_pq_load_reg.param_id = TABLE_NAME_OVERSCAN;
    ve_pq_load_reg.length = SIG_TIMING_TYPE_MAX;

    ve_pq_table_t ve_pq_table[SIG_TIMING_TYPE_MAX];
    tvin_cutwin_t cutwin[SIG_TIMING_TYPE_MAX];
    memset(ve_pq_table, 0, sizeof(ve_pq_table));
    memset(cutwin, 0, sizeof(cutwin));

    tvin_sig_fmt_t sig_fmt[SIG_TIMING_TYPE_MAX];
    ve_pq_timing_type_t flag[SIG_TIMING_TYPE_MAX];
    sig_fmt[0] = TVIN_SIG_FMT_HDMI_720X480P_60HZ;
    sig_fmt[1] = TVIN_SIG_FMT_HDMI_1280X720P_60HZ;
    sig_fmt[2] = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
    sig_fmt[3] = TVIN_SIG_FMT_HDMI_3840_2160_00HZ;
    sig_fmt[4] = TVIN_SIG_FMT_CVBS_NTSC_M;
    sig_fmt[5] = TVIN_SIG_FMT_CVBS_NTSC_443;
    sig_fmt[6] = TVIN_SIG_FMT_CVBS_PAL_I;
    sig_fmt[7] = TVIN_SIG_FMT_CVBS_PAL_M;
    sig_fmt[8] = TVIN_SIG_FMT_CVBS_PAL_60;
    sig_fmt[9] = TVIN_SIG_FMT_CVBS_PAL_CN;
    sig_fmt[10] = TVIN_SIG_FMT_CVBS_SECAM;
    sig_fmt[11] = TVIN_SIG_FMT_CVBS_NTSC_50;
    flag[0] = SIG_TIMING_TYPE_SD;
    flag[1] = SIG_TIMING_TYPE_HD;
    flag[2] = SIG_TIMING_TYPE_FHD;
    flag[3] = SIG_TIMING_TYPE_UHD;
    flag[4] = SIG_TIMING_TYPE_NTSC_M;
    flag[5] = SIG_TIMING_TYPE_NTSC_443;
    flag[6] = SIG_TIMING_TYPE_PAL_I;
    flag[7] = SIG_TIMING_TYPE_PAL_M;
    flag[8] = SIG_TIMING_TYPE_PAL_60;
    flag[9] = SIG_TIMING_TYPE_PAL_CN;
    flag[10] = SIG_TIMING_TYPE_SECAM;
    flag[11] = SIG_TIMING_TYPE_NTSC_50;

    source_input_param_t source_input_param;
    source_input_param.source_input = source_input;
    source_input_param.trans_fmt = mCurentSourceInputInfo.trans_fmt;
    ScreenModeValue = Cpq_GetScreenModeValue(display_mode);
    if (source_input == SOURCE_DTV) {//DTV
        for (i=0;i<SIG_TIMING_TYPE_NTSC_M;i++) {
            ve_pq_table[i].src_timing = (0x1<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];
            if (mbCpqCfg_seperate_db_enable) {
                ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            } else {
                ret = mPQdb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            }

            if (ret == 0) {
                LOGD("signal_fmt:0x%x, screen mode:%d hs:%d he:%d vs:%d ve:%d!\n", sig_fmt[i], ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                LOGD("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;
    } else if (source_input == SOURCE_TV) {//ATV
        for (i=SIG_TIMING_TYPE_NTSC_M;i<SIG_TIMING_TYPE_MAX;i++) {
            ve_pq_table[i].src_timing = (0x1<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];
            if (mbCpqCfg_seperate_db_enable) {
                ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            } else {
                ret = mPQdb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            }

            if (ret == 0) {
                LOGD("signal_fmt:0x%x, screen mode:%d hs:%d he:%d vs:%d ve:%d!\n", sig_fmt[i], ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                LOGD("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;
    } else {//HDMI && MPEG
        ve_pq_table[0].src_timing = (0x0<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (0x0);
        ve_pq_table[0].value1 = 0;
        ve_pq_table[0].value2 = 0;
        ve_pq_load_reg.param_ptr = (long long)ve_pq_table;

        ret = 0;
    }

    if (ret == 0) {
        ret = Cpq_LoadDisplayModeRegs(ve_pq_load_reg);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    } else {
        LOGD("%s success!\n",__FUNCTION__);
        return 0;
    }

}

int CPQControl::Cpq_GetScreenModeValue(vpp_display_mode_t display_mode)
{
    int value = SCREEN_MODE_16_9;

    switch ( display_mode ) {
    case VPP_DISPLAY_MODE_169:
        value = SCREEN_MODE_16_9;
        break;
    case VPP_DISPLAY_MODE_MODE43:
        value = SCREEN_MODE_4_3;
        break;
    case VPP_DISPLAY_MODE_NORMAL:
        value = SCREEN_MODE_NORMAL;
        break;
    case VPP_DISPLAY_MODE_FULL:
        value = SCREEN_MODE_NONLINEAR;
        Cpq_SetNonLinearFactor(20);
        break;
    case VPP_DISPLAY_MODE_NOSCALEUP:
        value = SCREEN_MODE_NORMAL_NOSCALEUP;
        break;
    case VPP_DISPLAY_MODE_MOVIE:
    case VPP_DISPLAY_MODE_PERSON:
    case VPP_DISPLAY_MODE_CAPTION:
    case VPP_DISPLAY_MODE_CROP:
    case VPP_DISPLAY_MODE_CROP_FULL:
    case VPP_DISPLAY_MODE_ZOOM:
    default:
        value = SCREEN_MODE_FULL_STRETCH;
        break;
    }

    return value;
}

int CPQControl::Cpq_SetVideoScreenMode(int value)
{
    LOGD("Cpq_SetVideoScreenMode, value = %d\n" , value);
    char val[64] = {0};
    sprintf(val, "%d", value);
    pqWriteSys(SCREEN_MODE_PATH, val);
    return 0;
}

int CPQControl::Cpq_SetVideoCrop(int Voffset0, int Hoffset0, int Voffset1, int Hoffset1)
{
    char set_str[32];
    int wr_size;

    LOGD("Cpq_SetVideoCrop value: %d %d %d %d\n", Voffset0, Hoffset0, Voffset1, Hoffset1);
    int fd = open(CROP_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("Open %s error(%s)!\n", CROP_PATH, strerror(errno));
        return -1;
    }

    memset(set_str, 0, 32);
    sprintf(set_str, "%d %d %d %d", Voffset0, Hoffset0, Voffset1, Hoffset1);
    wr_size = write(fd, set_str, strlen(set_str));
    if (wr_size < 0) {
        LOGE("write error = %s\n", strerror(errno));
    }

    close(fd);

    return 0;
}

int CPQControl::Cpq_SetNonLinearFactor(int value)
{
    LOGD("Cpq_SetNonLinearFactor : %d\n", value);
    FILE *fp = fopen(NOLINER_FACTORY, "w");
    if (fp == NULL) {
        LOGE("Open %s error(%s)!\n", NOLINER_FACTORY, strerror(errno));
        return -1;
    }

    fprintf(fp, "%d", value);
    fclose(fp);
    fp = NULL;
    return 0;
}

//lcd hdr info
int CPQControl::Cpq_SetHdrInfo(const lcd_optical_info_t *plcd_hdrinfo)
{
    int ret = 0;

    ret = LCDDeviceIOCtl(LCD_IOC_CMD_SET_HDR_INFO, plcd_hdrinfo);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetLCDhdrinfo(void)
{
    int ret = -1;
    lcd_optical_info_t lcd_hdrinfo;

    if (mbCpqCfg_lcd_hdrinfo_enable) {
        if (mPQdb->PQ_GetLCDHDRInfoParams(mCurentSourceInputInfo, &lcd_hdrinfo) == 0) {
            ret = Cpq_SetHdrInfo(&lcd_hdrinfo);
        } else {
            LOGE("mPQdb->PQ_GetLCDHDRInfoParams failed\n");
        }
    } else {
        LOGD("LCD hdr info disabled\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

//Backlight
int CPQControl::SetBacklight(int value, int is_save)
{
    int ret = -1;
    LOGD("%s: value = %d\n", __FUNCTION__, value);
    if (value < 0 || value > 100) {
        value = DEFAULT_BACKLIGHT_BRIGHTNESS;
    }

    if (isFileExist(LDIM_PATH)) {//local diming
        int temp = (value * 255 / 100);
        Cpq_SetBackLight(temp);
    }

    if (is_save == 1) {
        ret = SaveBacklight(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    } else {
        LOGD("%s success!\n",__FUNCTION__);
        return 0;
    }

}

int CPQControl::GetBacklight(void)
{
    int data = 0;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.Backlight;
        }
    } else {
        mSSMAction->SSMReadBackLightVal(&data);
    }

    if (data < 0 || data > 100) {
        data = DEFAULT_BACKLIGHT_BRIGHTNESS;
    }

    return data;
}

int CPQControl::SaveBacklight(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.Backlight = value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveBackLightVal(value);
    }

    if (ret < 0) {
    }
    return ret;
}

int CPQControl::Cpq_SetBackLight(int value)
{
    LOGD("%s, value = %d\n",__FUNCTION__, value);
    char val[64] = {0};
    sprintf(val, "%d", value);
    return pqWriteSys(BACKLIGHT_PATH, val);
}

void CPQControl::Cpq_GetBacklight(int *value)
{
    int ret = 0;
    char buf[64] = {0};

    ret = pqReadSys(BACKLIGHT_PATH, buf, sizeof(buf));
    if (ret > 0) {
        ret = strtol(buf, NULL, 10);
    } else {
        ret = 0;
    }

    *value = ret;
}

void CPQControl::Set_Backlight(int value)
{
    Cpq_SetBackLight(value);
}

//dynamic backlight
int CPQControl::SetDynamicBacklight(Dynamic_backlight_status_t mode, int is_save)
{
    LOGD("%s, mode = %d\n",__FUNCTION__, mode);
    int ret = -1;
    if (isFileExist(LDIM_PATH)) {//local diming
        char val[64] = {0};
        sprintf(val, "%d", mode);
        pqWriteSys(LDIM_CONTROL_PATH, val);
    }

    if (is_save == 1) {
        ret = mSSMAction->SSMSaveDynamicBacklightMode(mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDynamicBacklight()
{
    int ret = -1;
    int mode = -1;
    ret = mSSMAction->SSMReadDynamicBacklightMode(&mode);
    if (0 == ret) {
        return mode;
    } else {
        return ret;
    }
    LOGD("%s: value is %d\n", __FUNCTION__, mode);
}

int CPQControl::DynamicBackLightInit(void)
{
    int ret = 0;
    Dynamic_backlight_status_t mode = (Dynamic_backlight_status_t)GetDynamicBacklight();
    ret = SetDynamicBacklight(mode, 1);

    if (!isFileExist(LDIM_PATH)) {
        if (isFileExist(BACKLIGHT_PATH)) {
            mDynamicBackLight.setObserver(this);
            mDynamicBackLight.startDected();
        } else {
            LOGD("No auto backlight moudle!\n");
        }
    }

    return ret;
}

int CPQControl::GetHistParam(ve_hist_t *hist)
{
    memset(hist, 0, sizeof(ve_hist_s));
    int ret = VPPDeviceIOCtl(AMVECM_IOC_G_HIST_AVG, hist);
    if (ret < 0) {
        //LOGE("GetAVGHistParam, error(%s)!\n", strerror(errno));
        hist->ave = -1;
    }
    return ret;
}

bool CPQControl::isFileExist(const char *file_name)
{
    struct stat tmp_st;
    int ret = -1;

    ret = stat(file_name, &tmp_st);
    if (ret != 0 ) {
       LOGE("%s, %s don't exist!\n", __FUNCTION__, file_name);
       return false;
    } else {
       return true;
    }
}

void CPQControl::GetDynamicBacklighConfig(int *thtf, int *lut_mode, int *heigh_param, int *low_param)
{
    *thtf = mPQConfigFile->GetInt(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_THTF, 0);
    *lut_mode = mPQConfigFile->GetInt(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTMODE, 1);

    const char *buf = NULL;
    buf = mPQConfigFile->GetString(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTHIGH, NULL);
    pqTransformStringToInt(buf, heigh_param);

    buf = mPQConfigFile->GetString(CFG_SECTION_BACKLIGHT, CFG_AUTOBACKLIGHT_LUTLOW, NULL);
    pqTransformStringToInt(buf, low_param);
}

void CPQControl::GetDynamicBacklighParam(dynamic_backlight_Param_t *DynamicBacklightParam)
{
    int value = 0;
    ve_hist_t hist;
    memset(&hist, 0, sizeof(ve_hist_t));
    GetHistParam(&hist);
    DynamicBacklightParam->hist.ave = hist.ave;
    DynamicBacklightParam->hist.sum = hist.sum;
    DynamicBacklightParam->hist.width = hist.width;
    DynamicBacklightParam->hist.height = hist.height;

    Cpq_GetBacklight(&value);
    DynamicBacklightParam->CurBacklightValue = value;
    DynamicBacklightParam->UiBackLightValue  = GetBacklight();
    DynamicBacklightParam->CurDynamicBacklightMode = (Dynamic_backlight_status_t)GetDynamicBacklight();
    DynamicBacklightParam->VideoStatus = GetVideoPlayStatus();
}

int CPQControl::GetVideoPlayStatus(void)
{
    int curVideoState = 0;

    if (mbVideoIsPlaying) {
        curVideoState = 1;//video playing
    } else {
        curVideoState = 0;//video stoping
    }

    return curVideoState;
}

int CPQControl::SetLocalContrastMode(local_contrast_mode_t mode, int is_save)
{
    LOGD("%s: mode is %d!\n",__FUNCTION__, mode);
    int ret = -1;
    if (mbCpqCfg_local_contrast_enable) {
        ret = Cpq_SetLocalContrastMode(mode);
        if ((ret == 0) && (is_save == 1)) {
            ret = SaveLocalContrastMode(mode);
        }
    } else {
        LOGD("%s: local contrast moudle disabled!\n",__FUNCTION__);
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::GetLocalContrastMode(void)
{
    int mode = LOCAL_CONTRAST_MODE_MID;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            mode = para.LocalContrast;
        }
    } else {
        mSSMAction->SSMReadLocalContrastMode(mCurentSourceInputInfo.source_input, &mode);
    }

    if (mode < LOCAL_CONTRAST_MODE_OFF || mode > LOCAL_CONTRAST_MODE_MAX) {
        mode = LOCAL_CONTRAST_MODE_MID;
    }

    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, mode);
    return mode;
}

int CPQControl::SaveLocalContrastMode(local_contrast_mode_t mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.LocalContrast = (int)mode;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveLocalContrastMode(mCurentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetLocalContrastMode(local_contrast_mode_t mode)
{
    int ret = -1;

    if (mbCpqCfg_local_contrast_enable) {
        if (!mbDatabaseMatchChipStatus) {
            LOGD("%s: DB don't match chip\n", __FUNCTION__);
            ret = 0;
        } else {
            ve_lc_curve_parm_t lc_param;
            am_regs_t regs;
            memset(&lc_param, 0x0, sizeof(ve_lc_curve_parm_t));
            memset(&regs, 0x0, sizeof(am_regs_t));

            ret = mPQdb->PQ_GetLocalContrastNodeParams(mCurentSourceInputInfo, mode, &lc_param);
            if (ret == 0 ) {
                ret = VPPDeviceIOCtl(AMVECM_IOC_S_LC_CURVE, &lc_param);
                if (ret == 0) {
                    ret = mPQdb->PQ_GetLocalContrastRegParams(mCurentSourceInputInfo, mode, &regs);
                    if (ret == 0) {
                        ret = Cpq_LoadRegs(regs);
                    } else {
                        LOGE("%s: PQ_GetLocalContrastRegParams failed\n", __FUNCTION__ );
                    }
                } else {
                    LOGE("%s VPPDeviceIOCtl failed\n",__FUNCTION__);
                }
            } else {
                LOGE("%s: PQ_GetLocalContrastNodeParams failed\n", __FUNCTION__ );
            }
        }
    }else {
        LOGE("%s LocalContrast Disabled\n",__FUNCTION__);
        ret = 0;
    }

    return ret;
}

//load aad pq
int CPQControl::Cpq_SetAAD(const db_aad_param_t *pAAD)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_AAD_PARAM, pAAD);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetAad(void)
{
    int ret = -1;

    if (mbCpqCfg_aad_enable) {
        aad_param_t newaad;
        if (mPQdb->PQ_GetAADParams(mCurentSourceInputInfo, &newaad) == 0) {
            db_aad_param_t db_newaad;
            db_newaad.aad_param_cabc_aad_en   = newaad.aad_param_cabc_aad_en;
            db_newaad.aad_param_aad_en        = newaad.aad_param_aad_en;
            db_newaad.aad_param_tf_en         = newaad.aad_param_tf_en;
            db_newaad.aad_param_force_gain_en = newaad.aad_param_force_gain_en;
            db_newaad.aad_param_sensor_mode   = newaad.aad_param_sensor_mode;
            db_newaad.aad_param_mode          = newaad.aad_param_mode;
            db_newaad.aad_param_dist_mode     = newaad.aad_param_dist_mode;
            db_newaad.aad_param_tf_alpha      = newaad.aad_param_tf_alpha;
            db_newaad.aad_param_sensor_input[0] = newaad.aad_param_sensor_input[0];
            db_newaad.aad_param_sensor_input[1] = newaad.aad_param_sensor_input[1];
            db_newaad.aad_param_sensor_input[2] = newaad.aad_param_sensor_input[2];
            db_newaad.db_LUT_Y_gain.length                   = newaad.aad_param_LUT_Y_gain_len;
            db_newaad.db_LUT_Y_gain.cabc_aad_param_ptr_len   = (long long)&(newaad.aad_param_LUT_Y_gain);
            db_newaad.db_LUT_RG_gain.length                  = newaad.aad_param_LUT_RG_gain_len;
            db_newaad.db_LUT_RG_gain.cabc_aad_param_ptr_len  = (long long)&(newaad.aad_param_LUT_RG_gain);
            db_newaad.db_LUT_BG_gain.length                  = newaad.aad_param_LUT_BG_gain_len;
            db_newaad.db_LUT_BG_gain.cabc_aad_param_ptr_len  = (long long)&(newaad.aad_param_LUT_BG_gain);
            db_newaad.db_gain_lut.length                     = newaad.aad_param_gain_lut_len;
            db_newaad.db_gain_lut.cabc_aad_param_ptr_len     = (long long)&(newaad.aad_param_gain_lut);
            db_newaad.db_xy_lut.length                       = newaad.aad_param_xy_lut_len;
            db_newaad.db_xy_lut.cabc_aad_param_ptr_len       = (long long)&(newaad.aad_param_xy_lut);

            ret = Cpq_SetAAD(&db_newaad);
        } else {
            LOGE("mPQdb->PQ_GetAADParams failed\n");
        }
    } else {
        LOGD("AAD moudle disabled\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s succes\n",__FUNCTION__);
    }

    return ret;
}

//load aad pq
int CPQControl::Cpq_SetCABC(const db_cabc_param_t *pCABC)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_CABC_PARAM, pCABC);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetCabc(void)
{
    int ret = -1;

    if (mbCpqCfg_cabc_enable) {
        cabc_param_t newcabc;
        if (mPQdb->PQ_GetCABCParams(mCurentSourceInputInfo, &newcabc) == 0) {
            db_cabc_param_t db_newcabc;
            db_newcabc.cabc_param_cabc_en          = newcabc.cabc_param_cabc_en;
            db_newcabc.cabc_param_hist_mode        = newcabc.cabc_param_hist_mode;
            db_newcabc.cabc_param_tf_en            = newcabc.cabc_param_tf_en;
            db_newcabc.cabc_param_sc_flag          = newcabc.cabc_param_sc_flag;
            db_newcabc.cabc_param_bl_map_mode      = newcabc.cabc_param_bl_map_mode;
            db_newcabc.cabc_param_bl_map_en        = newcabc.cabc_param_bl_map_en;
            db_newcabc.cabc_param_temp_proc        = newcabc.cabc_param_temp_proc;
            db_newcabc.cabc_param_max95_ratio      = newcabc.cabc_param_max95_ratio;
            db_newcabc.cabc_param_hist_blend_alpha = newcabc.cabc_param_hist_blend_alpha;
            db_newcabc.cabc_param_init_bl_min      = newcabc.cabc_param_init_bl_min;
            db_newcabc.cabc_param_init_bl_max      = newcabc.cabc_param_init_bl_max;
            db_newcabc.cabc_param_tf_alpha         = newcabc.cabc_param_tf_alpha;
            db_newcabc.cabc_param_sc_hist_diff_thd = newcabc.cabc_param_sc_hist_diff_thd;
            db_newcabc.cabc_param_sc_apl_diff_thd  = newcabc.cabc_param_sc_apl_diff_thd;
            db_newcabc.cabc_param_patch_bl_th      = newcabc.cabc_param_patch_bl_th;
            db_newcabc.cabc_param_patch_on_alpha   = newcabc.cabc_param_patch_on_alpha;
            db_newcabc.cabc_param_patch_bl_off_th  = newcabc.cabc_param_patch_bl_off_th;
            db_newcabc.cabc_param_patch_off_alpha  = newcabc.cabc_param_patch_off_alpha;
            db_newcabc.db_o_bl_cv.length                      = newcabc.cabc_param_o_bl_cv_len;
            db_newcabc.db_o_bl_cv.cabc_aad_param_ptr_len      = (long long)&(newcabc.cabc_param_o_bl_cv);
            db_newcabc.db_maxbin_bl_cv.length                 = newcabc.cabc_param_maxbin_bl_cv_len;
            db_newcabc.db_maxbin_bl_cv.cabc_aad_param_ptr_len = (long long)&(newcabc.cabc_param_maxbin_bl_cv);

            ret = Cpq_SetCABC(&db_newcabc);
        } else {
            LOGE("mPQdb->PQ_GetCABCParams failed\n");
        }
    } else {
        LOGD("CABC moudle disabled\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s succes\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetDnlpMode(Dynamic_contrast_mode_t mode, int is_save)
{
    int ret = -1;
    ret = Cpq_SetDnlpMode(mode, mCurentSourceInputInfo);

    if (ret == 0) {
        ret = SaveDnlpMode((Dynamic_contrast_mode_t)mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDnlpMode()
{
    int level = DYNAMIC_CONTRAST_OFF;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            level = para.DynamicContrast;
        }
    } else {
        mSSMAction->SSMReadDnlpMode(mCurentSourceInputInfo.source_input, &level);
    }

    if (level < LOCAL_CONTRAST_MODE_OFF || level > LOCAL_CONTRAST_MODE_MAX) {
        level = LOCAL_CONTRAST_MODE_MID;
    }

    LOGD("%s, source: %d, timming: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, level);
    return level;
}

int CPQControl::SaveDnlpMode(Dynamic_contrast_mode_t mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.DynamicContrast = (int)mode;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveDnlpMode(mCurentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetVENewDNLP(const ve_dnlp_curve_param_t *pDNLP)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_VE_NEW_DNLP, pDNLP);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetDnlpMode(Dynamic_contrast_mode_t level, source_input_param_t source_input_param)
{
    int ret = -1;
    ve_dnlp_curve_param_t newdnlp;

    if (mbCpqCfg_dnlp_enable) {
        if (mPQdb->PQ_GetDNLPParams(mCurentSourceInputInfo, level, &newdnlp) == 0) {
            ret = Cpq_SetVENewDNLP(&newdnlp);
        } else {
            LOGE("mPQdb->PQ_GetDNLPParams failed!\n");
        }
    } else {
        ret = 0;
        LOGE("%s DnlpMode Disabled\n", __FUNCTION__);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDNLPStatus(ve_dnlp_state_t status)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_DNLP_STATE, &status);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetColorDemoMode(vpp_color_demomode_t demomode)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, demomode);
    int ret = -1;
    cm_regmap_t regmap;
    unsigned long *temp_regmap;
    int i = 0;
    vpp_display_mode_t displaymode = VPP_DISPLAY_MODE_MODE43;

    switch (demomode) {
    case VPP_COLOR_DEMO_MODE_YOFF:
        temp_regmap = DemoColorYOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_COFF:
        temp_regmap = DemoColorCOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_GOFF:
        temp_regmap = DemoColorGOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_MOFF:
        temp_regmap = DemoColorMOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ROFF:
        temp_regmap = DemoColorROffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_BOFF:
        temp_regmap = DemoColorBOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_RGBOFF:
        temp_regmap = DemoColorRGBOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_YMCOFF:
        temp_regmap = DemoColorYMCOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ALLOFF:
        temp_regmap = DemoColorALLOffRegMap;
        break;

    case VPP_COLOR_DEMO_MODE_ALLON:
    default:
        if (displaymode == VPP_DISPLAY_MODE_MODE43) {
            temp_regmap = DemoColorSplit4_3RegMap;
        } else {
            temp_regmap = DemoColorSplitRegMap;
        }

        break;
    }

    for (i = 0; i < CM_REG_NUM; i++) {
        regmap.reg[i] = temp_regmap[i];
    }

    ret = VPPDeviceIOCtl(AMSTREAM_IOC_CM_REGMAP, regmap);
    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetColorBaseMode(vpp_color_basemode_t basemode, int isSave)
{
    LOGD("%s, source: %d, value:%d\n", __FUNCTION__, mCurentSourceInputInfo.source_input, basemode);

    int ret = Cpq_SetColorBaseMode(basemode, mCurentSourceInputInfo);

    if ((ret ==0) && (isSave == 1)) {
        ret = SaveColorBaseMode(basemode);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return ret;
}

vpp_color_basemode_t CPQControl::GetColorBaseMode(void)
{
    vpp_color_basemode_t data = VPP_COLOR_BASE_MODE_OFF;
    unsigned char tmp_base_mode = 0;

    mSSMAction->SSMReadColorBaseMode(&tmp_base_mode);
    data = (vpp_color_basemode_t) tmp_base_mode;

    if (data < VPP_COLOR_BASE_MODE_OFF || data >= VPP_COLOR_BASE_MODE_MAX) {
        data = VPP_COLOR_BASE_MODE_OPTIMIZE;
    }

    LOGD("%s: mode is %d\n", __FUNCTION__, data);
    return data;
}



int CPQControl::SaveColorBaseMode(vpp_color_basemode_t basemode)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, basemode);
    int ret = -1;

    if (basemode == VPP_COLOR_BASE_MODE_DEMO) {
        ret = 0;
    } else {
        ret = mSSMAction->SSMSaveColorBaseMode(basemode);
    }

    return ret;
}



int CPQControl::Cpq_SetColorBaseMode(vpp_color_basemode_t basemode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    if (mbCpqCfg_cm2_enable) {
        if (mPQdb->PQ_GetCM2Params((vpp_color_management2_t)basemode, source_input_param, &regs) == 0) {
            ret = Cpq_LoadRegs(regs);
        } else {
            LOGE("PQ_GetCM2Params failed\n");
        }
    } else {
        LOGD("CM moudle disabled\n");
        ret = 0;
    }

    return ret;
}

int CPQControl::SetColorGamutMode(vpp_colorgamut_mode_t value, int is_save)
{
    int ret =0;
    LOGD("%s, source:%d, value:%d\n",__FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetColorGamutMode(value, mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveColorGamutMode(value);
    }

    if (ret < 0) {
        LOGD("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return 0;
}

int CPQControl::GetColorGamutMode(void)
{
    int data = VPP_COLORGAMUT_MODE_AUTO;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            data = para.ColorGamut;
        }
    } else {
        mSSMAction->SSMReadColorGamutMode(mSourceInputForSaveParam, &data);
    }

    if (data < VPP_COLORGAMUT_MODE_SRC || data > VPP_COLORGAMUT_MODE_NATIVE) {
        data = VPP_COLORGAMUT_MODE_AUTO;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n",__FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, data);
    return data;
}

int CPQControl::SaveColorGamutMode(vpp_colorgamut_mode_t value)
{
    int ret    = 1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.ColorGamut = (int)value;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveColorGamutMode(mSourceInputForSaveParam, value);
    }

    if (ret < 0)
        LOGD("%s failed\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetColorGamutMode(vpp_colorgamut_mode_t value, source_input_param_t source_input_param)
{
    char val[64] = {0};
    sprintf(val, "%d", value);
    return 0;//pqWriteSys(VPP_MODULE_COLORGAMUT_PATH, val);
}

//SmoothPlus mode
int CPQControl::SetSmoothPlusMode(int smoothplus_mode, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, smoothplus_mode);
    int ret = Cpq_SetSmoothPlusMode((vpp_smooth_plus_mode_t)smoothplus_mode, mCurentSourceInputInfo);
    if ((ret ==0) && (is_save == 1)) {
        ret = SaveSmoothPlusMode((vpp_smooth_plus_mode_t)smoothplus_mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetSmoothPlusMode(void)
{
    int mode = VPP_SMOOTH_PLUS_MODE_MID;
    mSSMAction->SSMReadSmoothPlus(mSourceInputForSaveParam, &mode);
    if (mode < VPP_SMOOTH_PLUS_MODE_OFF || mode > VPP_SMOOTH_PLUS_MODE_AUTO) {
        mode = VPP_SMOOTH_PLUS_MODE_MID;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);
    return mode;
}

int CPQControl::SaveSmoothPlusMode(int smoothplus_mode)
{
    int ret = mSSMAction->SSMSaveSmoothPlus(mSourceInputForSaveParam, smoothplus_mode);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetSmoothPlusMode(vpp_smooth_plus_mode_t smoothplus_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));

    if (mbCpqCfg_smoothplus_enable) {
        if (mPQdb->PQ_GetSmoothPlusParams(smoothplus_mode, source_input_param, &regs) == 0) {
            di_regs.table_name = TABLE_NAME_SMOOTHPLUS;
            di_regs.table_len = regs.length;
            am_reg_t tmp_buf[regs.length];
            for (unsigned int i=0;i<regs.length;i++) {
                  tmp_buf[i].addr = regs.am_reg[i].addr;
                  tmp_buf[i].mask = regs.am_reg[i].mask;
                  tmp_buf[i].type = regs.am_reg[i].type;
                  tmp_buf[i].val  = regs.am_reg[i].val;
            }
            di_regs.table_ptr = tmp_buf;

            ret = DI_LoadRegs(di_regs);
        } else {
            LOGE("PQ_GetSmoothPlusParams failed!\n");
        }
    } else {
        LOGD("Smooth Plus disabled\n");
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetHDRTMData(int *reGain)
{
    int ret = -1;
    if (reGain == NULL) {
        LOGD("%s: reGain is NULL.\n", __FUNCTION__);
    } else {
        vpp_hdr_tone_mapping_t hdrToneMapping;
        memset(&hdrToneMapping, 0, sizeof(vpp_hdr_tone_mapping_t));
        hdrToneMapping.lut_type = LUT_TYPE_HLG;
        hdrToneMapping.lutlength = 149;
        hdrToneMapping.tm_lut = reGain;

        LOGD("hdrToneMapping.lut_type = %d\n", hdrToneMapping.lut_type);
        LOGD("hdrToneMapping.lutlength = %d\n", hdrToneMapping.lutlength);
        //SYS_LOGD("hdrToneMapping.tm_lut = %s\n", hdrToneMapping.tm_lut);

        ret = VPPDeviceIOCtl(AMVECM_IOC_S_HDR_TM, &hdrToneMapping);
    }

    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

//HDR TMO
int CPQControl::Cpq_SetHDRTMOParams(const hdr_tmo_sw_s *phdrtmo)
{
    int ret = 0;

    ret = VPPDeviceIOCtl(AMVECM_IOC_S_HDR_TMO, phdrtmo);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetHDRTMOMode(hdr_tmo_t mode, int is_save)
{
    int ret = -1;
    hdr_tmo_sw_s hdrtmo_param;

    LOGD("%s, source: %d, mode = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);

    if (mbCpqCfg_hdrtmo_enable) {
        if (mPQdb->PQ_GetHDRTMOParams(mCurentSourceInputInfo, mode, &hdrtmo_param) == 0) {
            ret = Cpq_SetHDRTMOParams(&hdrtmo_param);
            if ((ret ==0) && (is_save == 1)) {
                ret = SaveHDRTMOMode(mode);
            }
        } else {
            LOGE("mPQdb->PQ_GetHDRTMOParams failed!\n");
        }
    } else {
        LOGD("hdr tmo disabled\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHDRTMOMode()
{
    int data = HDR_TMO_DYNAMIC;
    mSSMAction->SSMReadHdrTmoVal(mSourceInputForSaveParam, &data);

    if (data < HDR_TMO_OFF || data > HDR_TMO_STATIC) {
        data = HDR_TMO_DYNAMIC;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, data);

    return data;
}

int CPQControl::SaveHDRTMOMode(hdr_tmo_t mode)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mSourceInputForSaveParam, mode);

    int ret = mSSMAction->SSMSaveHdrTmoVal(mSourceInputForSaveParam, mode);
    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetBlackExtensionParam(source_input_param_t source_input_param)
{
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    int ret = -1;

    ret = mPQdb->PQ_GetBlackExtensionParams(source_input_param, &regs);
    if (ret < 0) {
        LOGE("%s: PQ_GetBlackExtensionParams failed!\n", __FUNCTION__);
    } else {
        ret = Cpq_LoadRegs(regs);
    }

    if (ret < 0) {
        LOGE("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::SetMpegNr(vpp_pq_level_t mode, int is_save)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;

    ret = Cpq_SetMpegNr(mode, mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveMpegNr(mode);
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetMpegNr(void)
{
    int mode = VPP_DEBLOCK_MODE_OFF;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            mode = para.MpegNr;
        }
    } else {
        mSSMAction->SSMReadMpegNoiseReduction(mCurentSourceInputInfo.source_input, &mode);
    }

    LOGD("%s: MpegNr = %d \n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveMpegNr(vpp_pq_level_t mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.MpegNr = (int)mode;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        ret = mSSMAction->SSMSaveMpegNoiseReduction(mCurentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMpegNr(vpp_pq_level_t mode, source_input_param_t source_input_param)
{
    int ret = -1;

    ret = Cpq_SetDeblockMode((vpp_deblock_mode_t) mode, source_input_param);
    ret |= Cpq_SetDemoSquitoMode((vpp_DemoSquito_mode_t) mode, source_input_param);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetDeblockMode(vpp_deblock_mode_t mode, int is_save)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    if (mbCpqCfg_deblock_enable) {
        ret = Cpq_SetDeblockMode(mode, mCurentSourceInputInfo);
        if ((ret == 0) && (is_save == 1)) {
            ret = SaveDeblockMode(mode);
        }
    } else {
        LOGD("%s: deblock disabled\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetDeblockMode(void)
{
    int mode = VPP_DEBLOCK_MODE_OFF;

    mSSMAction->SSMReadDeblockMode(mCurentSourceInputInfo.source_input, &mode);

    if (mode < VPP_DEBLOCK_MODE_OFF || mode > VPP_DEBLOCK_MODE_AUTO) {
        mode = VPP_DEBLOCK_MODE_OFF;
    }

    LOGD("%s: DeblockMode = %d \n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveDeblockMode(vpp_deblock_mode_t mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveDeblockMode(mCurentSourceInputInfo.source_input, (int)mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDeblockMode(vpp_deblock_mode_t deblock_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));

    if (mbCpqCfg_deblock_enable) {
        if (mPQdb->PQ_GetDeblockParams((vpp_deblock_mode_t)deblock_mode, source_input_param, &regs) == 0) {
            di_regs.table_name = TABLE_NAME_DEBLOCK;
            di_regs.table_len = regs.length;
            am_reg_t tmp_buf[regs.length];
            for (unsigned int i=0;i<regs.length;i++) {
                  tmp_buf[i].addr = regs.am_reg[i].addr;
                  tmp_buf[i].mask = regs.am_reg[i].mask;
                  tmp_buf[i].type = regs.am_reg[i].type;
                  tmp_buf[i].val  = regs.am_reg[i].val;
            }
            di_regs.table_ptr = tmp_buf;

            ret = DI_LoadRegs(di_regs);
        } else {
            LOGE("PQ_GetDeblockParams failed!\n");
        }
    } else {
        ret = 0;
        LOGE("PQ_GetDeblockParams Disabled!\n");
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetDemoSquitoMode(vpp_DemoSquito_mode_t mode, int is_save)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetDemoSquitoMode(mode, mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDemoSquitoMode(mode);
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetDemoSquitoMode(void)
{
    int mode = VPP_DEMOSQUITO_MODE_OFF;

    mSSMAction->SSMReadDemoSquitoMode(mCurentSourceInputInfo.source_input, &mode);

    if (mode < VPP_DEMOSQUITO_MODE_OFF || mode > VPP_DEMOSQUITO_MODE_AUTO) {
        mode = VPP_DEMOSQUITO_MODE_OFF;
    }

    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveDemoSquitoMode(vpp_DemoSquito_mode_t mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveDemoSquitoMode(mCurentSourceInputInfo.source_input, mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDemoSquitoMode(vpp_DemoSquito_mode_t DeMosquito_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));

    if (mbCpqCfg_demoSquito_enable) {
        if (mPQdb->PQ_GetDemoSquitoParams(DeMosquito_mode, source_input_param, &regs) == 0) {
            di_regs.table_name = TABLE_NAME_DEMOSQUITO;
            di_regs.table_len = regs.length;
            am_reg_t tmp_buf[regs.length];
            for (unsigned int i=0;i<regs.length;i++) {
                  tmp_buf[i].addr = regs.am_reg[i].addr;
                  tmp_buf[i].mask = regs.am_reg[i].mask;
                  tmp_buf[i].type = regs.am_reg[i].type;
                  tmp_buf[i].val  = regs.am_reg[i].val;
            }
            di_regs.table_ptr = tmp_buf;

            ret = DI_LoadRegs(di_regs);
        } else {
            LOGE("DemoSquitoMode failed!\n");
        }
    } else {
        ret = 0;
        LOGD("DemoSquitoMode Disabled!\n");
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetMcDiMode(vpp_mcdi_mode_e mode, int is_save)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetMcDiMode(mode , mCurentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveMcDiMode(mode);
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetMcDiMode(void)
{
    int mode = VPP_MCDI_MODE_OFF;

    mSSMAction->SSMReadMcDiMode(mCurentSourceInputInfo.source_input, &mode);

    if (mode < VPP_MCDI_MODE_OFF || mode > VPP_MCDI_MODE_MAX) {
        mode = VPP_MCDI_MODE_OFF;
    }

    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveMcDiMode(vpp_mcdi_mode_e mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveMcDiMode(mCurentSourceInputInfo.source_input, mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMcDiMode(vpp_mcdi_mode_e McDi_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    am_pq_param_t di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(am_pq_param_t));

    if (mbCpqCfg_mcdi_enable) {
        if (mPQdb->PQ_GetMCDIParams(McDi_mode, source_input_param, &regs) == 0) {
            di_regs.table_name = TABLE_NAME_MCDI;
            di_regs.table_len = regs.length;
            am_reg_t tmp_buf[regs.length];
            for (unsigned int i=0;i<regs.length;i++) {
                  tmp_buf[i].addr = regs.am_reg[i].addr;
                  tmp_buf[i].mask = regs.am_reg[i].mask;
                  tmp_buf[i].type = regs.am_reg[i].type;
                  tmp_buf[i].val  = regs.am_reg[i].val;
            }
            di_regs.table_ptr = tmp_buf;

            ret = DI_LoadRegs(di_regs);
        } else {
            LOGE("MCDI failed!\n");
        }
    } else {
        ret = 0;
        LOGD("MCDI Disabled!\n");
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::SetBlackStretch(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_BlackStretch(level, mCurentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveBlackStretch(level);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetBlackStretch(void)
{
    int level = VPP_PQ_LV_OFF;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            level = para.BlackStretch;
        }
    } else {
        mSSMAction->SSMReadBlackStretch(mCurentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, level);

    return level;
}

int CPQControl::SaveBlackStretch(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.BlackStretch = level;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        mSSMAction->SSMSaveBlackStretch(mCurentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_BlackStretch(int level,source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    if (mbCpqCfg_blackextension_enable) {
        ret = mPQdb->PQ_GetBlackStretchParams(level,source_input_param, &regs);

        if (ret < 0) {
            LOGE("%s: PQ_GetBlackStretchParams failed!\n", __FUNCTION__);
        } else {
            ret = Cpq_LoadRegs(regs);
        }
    } else {
        LOGE("%s: BlackStretch disabled!\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);


    return ret;
}

int CPQControl::SetBlueStretch(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_BlueStretch(level, mCurentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveBlueStretch(level);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetBlueStretch(void)
{
    int level = VPP_PQ_LV_OFF;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            level = para.BlueStretch;
        }
    } else {
        mSSMAction->SSMReadBlueStretch(mCurentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, level);

    return level;
}


int CPQControl::SaveBlueStretch(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.BlueStretch = level;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        mSSMAction->SSMSaveBlueStretch(mCurentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_BlueStretch(int level,source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    if (mbCpqCfg_bluestretch_enable) {
        ret = mPQdb->PQ_GetBlueStretchParams(level,source_input_param, &regs);

        if (ret < 0) {
            LOGE("%s: PQ_GetBlueStretchParams failed!\n", __FUNCTION__);
        } else {
            ret = Cpq_LoadRegs(regs);
        }
    } else {
        LOGE("%s: BlueStretch disabled!\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);


    return ret;
}

int CPQControl::SetChromaCoring(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_ChromaCoring(level, mCurentSourceInputInfo);

    if (ret == 0 && is_save == 1) {
        SaveChromaCoring(level);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetChromaCoring(void)
{
    int level = VPP_PQ_LV_OFF;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            level = para.ChromaCoring;
        }
    } else {
        mSSMAction->SSMReadChromaCoring(mCurentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, mSourceInputForSaveParam, mCurentPqSource.pq_sig_fmt, level);

    return level;
}


int CPQControl::SaveChromaCoring(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();

        if (GetPictureModeData(mCurentPqSource, pq_mode, &para) == 0) {
            para.ChromaCoring = level;
            ret = SetPictureModeData(mCurentPqSource, pq_mode, &para);
        }
    } else {
        mSSMAction->SSMSaveChromaCoring(mCurentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_ChromaCoring(int level,source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    if (mbCpqCfg_chromacoring_enable) {
        ret = mPQdb->PQ_GetChromaCoringParams(level,source_input_param, &regs);

        if (ret < 0) {
            LOGE("%s: PQ_GetChromaCoringParams failed!\n", __FUNCTION__);
        } else {
            ret = Cpq_LoadRegs(regs);
        }
    } else {
        LOGE("%s: ChromaCoring disabled!\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

//av/atv decode
int CPQControl::SetCVD2Values(void)
{
    int ret = 0;

    if (mbCpqCfg_cvd2_enable) {
        am_regs_t regs;
        ret = mPQdb->PQ_GetCVD2Params(mCurentSourceInputInfo, &regs);
        if (ret < 0) {
            LOGE ( "%s, PQ_GetCVD2Params failed\n", __FUNCTION__);
        } else {
            ret = AFEDeviceIOCtl(TVIN_IOC_LOAD_REG, &regs);
            if ( ret < 0 ) {
                LOGE ( "%s: ioctl failed\n", __FUNCTION__);
            }
        }

        if (ret < 0) {
            LOGE("%s failed\n", __FUNCTION__);
        } else {
            LOGD("%s success\n", __FUNCTION__);
        }
    } else {
        LOGD("%s cvd2 disable\n", __FUNCTION__);
    }

    return ret;
}

//PQ Factory
int CPQControl::FactoryResetPQMode(void)
{
    if (mbCpqCfg_new_picture_mode_enable) {
        ResetPQModeSetting(SOURCE_TV, VPP_PICTURE_MODE_STANDARD);
    } else {
        if (mbCpqCfg_seperate_db_enable) {
            mpOverScandb->PQ_ResetAllPQModeParams();
        } else {
            mPQdb->PQ_ResetAllPQModeParams();
        }
    }
    return 0;
}

int CPQControl::FactoryResetColorTemp(void)
{
    if (mbCpqCfg_new_picture_mode_enable) {
        Cpq_RestoreColorTemperatureParamsFromDB(mCurentSourceInputInfo);
    } else {
        mPQdb->PQ_ResetAllColorTemperatureParams();
    }
    return 0;
}

int CPQControl::FactorySetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode, int brightness)
{
    int ret = -1;
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.brightness = brightness;
                if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.brightness = brightness;
                if (mpOverScandb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
            pq_para.brightness = brightness;
            if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
        } else {
            ret = -1;
        }
    }

    return ret;
}

int CPQControl::FactoryGetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode)
{
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.brightness = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.brightness = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
            pq_para.brightness = -1;
        }
    }
    return pq_para.brightness;
}

int CPQControl::FactorySetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode, int contrast)
{
    int ret = -1;
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.contrast = contrast;
                if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.contrast = contrast;
                if (mpOverScandb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
            pq_para.contrast = contrast;
            if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
        } else {
            ret = -1;
        }
    }

    return ret;
}

int CPQControl::FactoryGetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode)
{
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.contrast = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.contrast = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
            pq_para.contrast = -1;
        }
    }

    return pq_para.contrast;
}

int CPQControl::FactorySetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode, int saturation)
{
    int ret = -1;
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.saturation = saturation;
                if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.saturation = saturation;
                if (mpOverScandb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
            pq_para.saturation = saturation;
            if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
        } else {
            ret = -1;
        }
    }

    return ret;
}

int CPQControl::FactoryGetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode)
{
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.saturation = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.saturation = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
            pq_para.saturation = -1;
        }
    }

    return pq_para.saturation;
}

int CPQControl::FactorySetPQMode_Hue(source_input_param_t source_input_param, int pq_mode, int hue)
{
    int ret = -1;
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.hue = hue;
                if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.hue = hue;
                if (mpOverScandb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
            pq_para.hue = hue;
            if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
        } else {
            ret = -1;
        }
    }

    return ret;
}

int CPQControl::FactoryGetPQMode_Hue(source_input_param_t source_input_param, int pq_mode)
{
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.hue = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.hue = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
            pq_para.hue = -1;
        }
    }

    return pq_para.hue;
}

int CPQControl::FactorySetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode, int sharpness)
{
    int ret = -1;
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.sharpness = sharpness;
                if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                pq_para.sharpness = sharpness;
                if (mpOverScandb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                    ret = 0;
                } else {
                    ret = 1;
                }
            } else {
                ret = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
            pq_para.sharpness = sharpness;
            if (mPQdb->PQ_SetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
        } else {
            ret = -1;
        }
    }
    return ret;
}

int CPQControl::FactoryGetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode)
{
    vpp_pq_para_t pq_para;
    if (mbCpqCfg_seperate_db_enable) {
        if (CheckPQModeTableInDb()) {
            if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                 pq_para.sharpness = -1;
            }
        } else {
            if (mpOverScandb->PQ_GetPQModeParams(source_input_param.source_input,
                                             (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
                pq_para.sharpness = -1;
            }
        }
    } else {
        if (mPQdb->PQ_GetPQModeParams(source_input_param.source_input, (vpp_picture_mode_t) pq_mode, &pq_para) != 0) {
             pq_para.sharpness = -1;
        }
    }
    return pq_para.sharpness;
}

int CPQControl::FactorySetColorTemp_Rgain(int source_input,int colortemp_mode, int rgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.r_gain = rgain;
    LOGD("%s, source[%d], colortemp_mode[%d], rgain[%d].", __FUNCTION__, source_input, colortemp_mode, rgain);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Rgain(int source_input __unused, int colortemp_mode, int rgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.r_gain = rgain;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Rgain error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Rgain(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.r_gain;
    }

    LOGE("FactoryGetColorTemp_Rgain error!\n");
    return -1;
}

int CPQControl::FactorySetColorTemp_Ggain(int source_input, int colortemp_mode, int ggain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.g_gain = ggain;
    LOGD("%s, source[%d], colortemp_mode[%d], ggain[%d].", __FUNCTION__, source_input, colortemp_mode, ggain);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Ggain(int source_input __unused, int colortemp_mode, int ggain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.g_gain = ggain;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Ggain error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Ggain(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.g_gain;
    }

    LOGE("FactoryGetColorTemp_Ggain error!\n");
    return -1;
}

int CPQControl::FactorySetColorTemp_Bgain(int source_input, int colortemp_mode, int bgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.b_gain = bgain;
    LOGD("%s, source[%d], colortemp_mode[%d], bgain[%d].", __FUNCTION__, source_input, colortemp_mode, bgain);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Bgain(int source_input __unused, int colortemp_mode, int bgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.b_gain = bgain;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Bgain error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Bgain(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.b_gain;
    }

    LOGE("FactoryGetColorTemp_Bgain error!\n");
    return -1;
}

int CPQControl::FactorySetColorTemp_Roffset(int source_input, int colortemp_mode, int roffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.r_post_offset = roffset;
    LOGD("%s, source[%d], colortemp_mode[%d], r_post_offset[%d].", __FUNCTION__, source_input, colortemp_mode, roffset);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Roffset(int source_input __unused, int colortemp_mode, int roffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.r_post_offset = roffset;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Roffset error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Roffset(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.r_post_offset;
    }

    LOGE("FactoryGetColorTemp_Roffset error!\n");
    return -1;
}

int CPQControl::FactorySetColorTemp_Goffset(int source_input, int colortemp_mode, int goffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.g_post_offset = goffset;
    LOGD("%s, source[%d], colortemp_mode[%d], g_post_offset[%d].", __FUNCTION__, source_input, colortemp_mode, goffset);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Goffset(int source_input __unused, int colortemp_mode, int goffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.g_post_offset = goffset;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Goffset error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Goffset(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.g_post_offset;
    }

    LOGE("FactoryGetColorTemp_Goffset error!\n");
    return -1;
}

int CPQControl::FactorySetColorTemp_Boffset(int source_input, int colortemp_mode, int boffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo);
    rgbogo.b_post_offset = boffset;
    LOGD("%s, source_input[%d], colortemp_mode[%d], b_post_offset[%d].", __FUNCTION__, source_input,
         colortemp_mode, boffset);
    rgbogo.en = 1;

    if (Cpq_SetRGBOGO(&rgbogo) == 0) {
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }
}

int CPQControl::FactorySaveColorTemp_Boffset(int source_input __unused, int colortemp_mode, int boffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        rgbogo.b_post_offset = boffset;
        return SetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, rgbogo);
    } else {
        LOGE("FactorySaveColorTemp_Boffset error!\n");
        return -1;
    }
}

int CPQControl::FactoryGetColorTemp_Boffset(int source_input __unused, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    if (0 == GetColorTemperatureParams((vpp_color_temperature_mode_t) colortemp_mode, &rgbogo)) {
        return rgbogo.b_post_offset;
    }

    LOGE("FactoryGetColorTemp_Boffset error!\n");
    return -1;
}

int CPQControl::FactoryResetNonlinear(void)
{
    return mPQdb->PQ_ResetAllNoLineParams();
}

int CPQControl::FactorySetParamsDefault(void)
{
    FactoryResetPQMode();
    FactoryResetNonlinear();
    FactoryResetColorTemp();
    if (mbCpqCfg_seperate_db_enable) {
        mpOverScandb->PQ_ResetAllOverscanParams();
    } else {
        mPQdb->PQ_ResetAllOverscanParams();
    }
    return 0;
}

int CPQControl::FactorySetNolineParams(source_input_param_t source_input_param, int type, noline_params_t noline_params)
{
    int ret = -1;

    switch (type) {
    case NOLINE_PARAMS_TYPE_BRIGHTNESS:
        ret = mPQdb->PQ_SetNoLineAllBrightnessParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_CONTRAST:
        ret = mPQdb->PQ_SetNoLineAllContrastParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_SATURATION:
        ret = mPQdb->PQ_SetNoLineAllSaturationParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);

    case NOLINE_PARAMS_TYPE_HUE:
        ret = mPQdb->PQ_SetNoLineAllHueParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_SHARPNESS:
        ret = mPQdb->PQ_SetNoLineAllSharpnessParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_VOLUME:
        ret = mPQdb->PQ_SetNoLineAllVolumeParams(source_input_param.source_input,
                noline_params.osd0, noline_params.osd25, noline_params.osd50, noline_params.osd75,
                noline_params.osd100);
        break;

    default:
        break;
    }

    return ret;
}

noline_params_t CPQControl::FactoryGetNolineParams(source_input_param_t source_input_param, int type)
{
    int ret = -1;
    noline_params_t noline_params;
    memset(&noline_params, 0, sizeof(noline_params_t));

    switch (type) {
    case NOLINE_PARAMS_TYPE_BRIGHTNESS:
        ret = mPQdb->PQ_GetNoLineAllBrightnessParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_CONTRAST:
        ret = mPQdb->PQ_GetNoLineAllContrastParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_SATURATION:
        ret = mPQdb->PQ_GetNoLineAllSaturationParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_HUE:
        ret = mPQdb->PQ_GetNoLineAllHueParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_SHARPNESS:
        ret = mPQdb->PQ_GetNoLineAllSharpnessParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    case NOLINE_PARAMS_TYPE_VOLUME:
        ret = mPQdb->PQ_GetNoLineAllVolumeParams(source_input_param.source_input,
                &noline_params.osd0, &noline_params.osd25, &noline_params.osd50,
                &noline_params.osd75, &noline_params.osd100);
        break;

    default:
        break;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return noline_params;
}

int CPQControl::FactorySetHdrMode(int mode)
{
    return SetHDRMode(mode);
}

int CPQControl::FactoryGetHdrMode(void)
{
    return GetHDRMode();
}

int CPQControl::FactorySetOverscanParam(source_input_param_t source_input_param, tvin_cutwin_t cutwin_t)
{
    int ret = -1;
    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_SetOverscanParams(source_input_param, cutwin_t);
    } else {
        ret = mPQdb->PQ_SetOverscanParams(source_input_param, cutwin_t);
    }
    if (ret != 0) {
        LOGE("%s failed.\n", __FUNCTION__);
    } else {
        LOGD("%s success.\n", __FUNCTION__);
    }

    return ret;
}

tvin_cutwin_t CPQControl::FactoryGetOverscanParam(source_input_param_t source_input_param)
{
    int ret = -1;
    tvin_cutwin_t cutwin_t;
    memset(&cutwin_t, 0, sizeof(cutwin_t));

    if (source_input_param.trans_fmt < TVIN_TFMT_2D || source_input_param.trans_fmt > TVIN_TFMT_3D_LDGD) {
        return cutwin_t;
    }
    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, VPP_DISPLAY_MODE_169, &cutwin_t);
    } else {
        ret = mPQdb->PQ_GetOverscanParams(source_input_param, VPP_DISPLAY_MODE_169, &cutwin_t);
    }

    if (ret != 0) {
        LOGE("%s failed.\n", __FUNCTION__);
    } else {
        LOGD("%s success.\n", __FUNCTION__);
    }

    return cutwin_t;
}

int CPQControl::FactorySetGamma(int gamma_r_value, int gamma_g_value, int gamma_b_value)
{
    int ret = 0;
    tcon_gamma_table_t gamma_r, gamma_g, gamma_b;

    memset(gamma_r.data, (unsigned short)gamma_r_value, sizeof(tcon_gamma_table_t));
    memset(gamma_g.data, (unsigned short)gamma_g_value, sizeof(tcon_gamma_table_t));
    memset(gamma_b.data, (unsigned short)gamma_b_value, sizeof(tcon_gamma_table_t));

    ret |= Cpq_SetGammaTbl_R((unsigned short *) gamma_r.data);
    ret |= Cpq_SetGammaTbl_G((unsigned short *) gamma_g.data);
    ret |= Cpq_SetGammaTbl_B((unsigned short *) gamma_b.data);

    return ret;
}

int CPQControl::FcatorySSMRestore(void)
{
    resetAllUserSettingParam();
    return 0;
}

int CPQControl::Cpq_SetXVYCCMode(vpp_xvycc_mode_t xvycc_mode, source_input_param_t source_input_param)
{
    int ret = -1;
    am_regs_t regs, regs_1;
    memset(&regs, 0, sizeof(am_regs_t));
    memset(&regs_1, 0, sizeof(am_regs_t));

    if (mbCpqCfg_xvycc_enable) {
        if (mPQdb->PQ_GetXVYCCParams((vpp_xvycc_mode_t) xvycc_mode, source_input_param, &regs, &regs_1) == 0) {
            ret = Cpq_LoadRegs(regs);
            ret |= Cpq_LoadRegs(regs_1);
        } else {
            LOGE("PQ_GetXVYCCParams failed!\n");
        }
    } else {
        LOGD("XVYCC Moudle disabled!\n");
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_RGB_OGO, rgbogo);
    if (ret < 0) {
        LOGE("%s failed(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_GetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_G_RGB_OGO, rgbogo);
    if (ret < 0) {
        LOGE("%s failed(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetGammaOnOff(int onoff)
{
    int ret = -1;

    if (onoff == 1) {
        LOGD("%s: enable gamma!\n", __FUNCTION__);
        ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_EN);
    } else {
        LOGD("%s: disable gamma!\n", __FUNCTION__);
        ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_DIS);
    }

    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}


int CPQControl::FactorySetDNLPCurveParams(source_input_param_t source_input_param, int level, int final_gain)
{
    int ret = -1;
    int cur_final_gain = -1;
    char tmp_buf[128];

    cur_final_gain = mPQdb->PQ_GetDNLPGains(source_input_param, (Dynamic_contrast_mode_t)level);
    if (cur_final_gain == final_gain) {
        LOGD("FactorySetDNLPCurveParams, same value, no need to update!");
        return ret;
    } else {
        LOGD("%s final_gain = %d \n", __FUNCTION__, final_gain);
        sprintf(tmp_buf, "%s %s %d", "w", "final_gain", final_gain);
        pqWriteSys("/sys/class/amvecm/dnlp_debug", tmp_buf);
        ret |= mPQdb->PQ_SetDNLPGains(source_input_param, (Dynamic_contrast_mode_t)level, final_gain);

    }
    return ret;
}

int CPQControl::FactoryGetDNLPCurveParams(source_input_param_t source_input_param, int level)
{
    return mPQdb->PQ_GetDNLPGains(source_input_param, (Dynamic_contrast_mode_t)level);
}

int CPQControl::FactorySetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr, int val)
{
    return mPQdb->PQ_SetNoiseReductionParams(nr_mode, source_input_param, addr, val);
}

int CPQControl::FactoryGetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr)
{
    return mPQdb->PQ_GetNoiseReductionParams(nr_mode, source_input_param, addr);
}

int CPQControl::SetEyeProtectionMode(tv_source_input_t source_input __unused, int enable, int is_save __unused)
{
    LOGD("%s: mode:%d!\n", __FUNCTION__, enable);
    int ret = -1;
    vpp_color_temperature_mode_t TempMode = (vpp_color_temperature_mode_t)GetColorTemperature();
    tcon_rgb_ogo_t param;
    memset(&param, 0, sizeof(tcon_rgb_ogo_t));
    if (TempMode == VPP_COLOR_TEMPERATURE_MODE_USER) {
        ret = Cpq_GetColorTemperatureUser(mCurentSourceInputInfo.source_input, &param);
    } else {
        ret = GetColorTemperatureParams(TempMode, &param);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        if (enable) {
            param.b_gain /= 2;
        }
        ret = Cpq_SetRGBOGO(&param);
        mSSMAction->SSMSaveEyeProtectionMode(enable);
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetEyeProtectionMode(tv_source_input_t source_input __unused)
{
    int mode = -1;

    if (mSSMAction->SSMReadEyeProtectionMode(&mode) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    } else {
        LOGD("%s: mode is %d!\n",__FUNCTION__, mode);
        return mode;
    }
}

int CPQControl::SetFlagByCfg(void)
{
    pq_ctrl_t pqControlVal;
    memset(&pqControlVal, 0x0, sizeof(pq_ctrl_t));
    const char *config_value;

    //load hdmi auto flag
    config_value = mPQConfigFile->GetString(CFG_SECTION_HDMI, CGF_HDMI_AUTOSWITCH_ALLM_EN, "disable" );
    if (strcmp(config_value, "enable") == 0 ) {
        mbAllmModeCfg_enable = true;
    } else {
        mbAllmModeCfg_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_HDMI, CGF_HDMI_AUTOSWITCH_ITCONTENT_EN, "disable" );
    if (strcmp(config_value, "enable") == 0 ) {
        mbItcontentModeCfg_enable = true;
    } else {
        mbItcontentModeCfg_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_HDMI, CGF_HDMI_AUTOSWITCH_DVI_EN, "disable" );
    if (strcmp(config_value, "enable") == 0 ) {
        mbDviModeCfg_enable = true;
    } else {
        mbDviModeCfg_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BIG_SMALL_DB_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_seperate_db_enable = true;
    } else {
        mbCpqCfg_seperate_db_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_ALL_PQ_MOUDLE_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_pq_enable = true;
    } else {
        mbCpqCfg_pq_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DI_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_di_enable = true;
    } else {
        mbCpqCfg_di_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MCDI_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_mcdi_enable = true;
        pqWriteSys(MCDI_MODULE_CONTROL_PATH, "1");
    } else {
        mbCpqCfg_mcdi_enable = false;
        pqWriteSys(MCDI_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEBLOCK_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_deblock_enable = true;
        pqWriteSys(DEBLOCK_MODULE_CONTROL_PATH, "13");//bit2~bit3
    } else {
        mbCpqCfg_deblock_enable = false;
        pqWriteSys(DEBLOCK_MODULE_CONTROL_PATH, "1");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEMOSQUITO_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_demoSquito_enable = true;
        pqWriteSys(DEMOSQUITO_MODULE_CONTROL_PATH, "1");//bit0
    } else {
        mbCpqCfg_demoSquito_enable = false;
        pqWriteSys(DEMOSQUITO_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_NOISEREDUCTION_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_nr_enable = true;
        pqWriteSys(NR2_MODULE_CONTROL_PATH, "1");
    } else {
        mbCpqCfg_nr_enable = false;
        pqWriteSys(NR2_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS0_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness0_enable = true;
        pqControlVal.sharpness0_en = 1;
    } else {
        mbCpqCfg_sharpness0_enable = false;
        pqControlVal.sharpness0_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS1_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness1_enable = true;
        pqControlVal.sharpness1_en = 1;
    } else {
        mbCpqCfg_sharpness1_enable = false;
        pqControlVal.sharpness1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DNLP_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_dnlp_enable = true;
        pqControlVal.dnlp_en = 1;
    } else {
        mbCpqCfg_dnlp_enable = false;
        pqControlVal.dnlp_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CM2_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cm2_enable = true;
        pqControlVal.cm_en = 1;
    } else {
        mbCpqCfg_cm2_enable = false;
        pqControlVal.cm_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_enable = true;
        pqControlVal.vadj1_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_enable = false;
        pqControlVal.vadj1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_WITHOSD_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_withOSD_enable = true;
        pqControlVal.vadj2_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_withOSD_enable = false;
        pqControlVal.vadj2_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_enable = true;
        pqControlVal.vd1_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_enable = false;
        pqControlVal.vd1_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_WITHOSD_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_withOSD_enable = true;
        pqControlVal.post_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_withOSD_enable = false;
        pqControlVal.post_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_WHITEBALANCE_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_whitebalance_enable = true;
        pqControlVal.wb_en = 1;
    } else {
        mbCpqCfg_whitebalance_enable = false;
        pqControlVal.wb_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_GAMMA_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_gamma_enable = true;
        pqControlVal.gamma_en = 1;
    } else {
        mbCpqCfg_gamma_enable = false;
        pqControlVal.gamma_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LOCAL_CONTRAST_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_local_contrast_enable = true;
        pqControlVal.lc_en = 1;
    } else {
        mbCpqCfg_local_contrast_enable = false;
        pqControlVal.lc_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLACKEXTENSION_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_blackextension_enable = true;
        pqControlVal.black_ext_en = 1;
    } else {
        mbCpqCfg_blackextension_enable = false;
        pqControlVal.black_ext_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_XVYCC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_xvycc_enable = true;
    } else {
        mbCpqCfg_xvycc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DISPLAY_OVERSCAN_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_display_overscan_enable = true;
    } else {
        mbCpqCfg_display_overscan_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_HDMI_OUT_WITH_FBC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_hdmi_out_with_fbc_enable = true;
    } else {
        mbCpqCfg_hdmi_out_with_fbc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_BACKLIGHT, CFG_LCD_HDR_INFO_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_lcd_hdrinfo_enable = true;
    } else {
        mbCpqCfg_lcd_hdrinfo_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CVD2_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cvd2_enable = true;
    } else {
        mbCpqCfg_cvd2_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_PQ_MODE_CHECK_SOURCE_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_pq_param_check_source_enable = true;
    } else {
        mbCpqCfg_pq_param_check_source_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_ai_enable = true;
    } else {
        mbCpqCfg_ai_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SMOOTHPLUS_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_smoothplus_enable = true;
    } else {
        mbCpqCfg_smoothplus_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_HDRTMO_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_hdrtmo_enable = true;
    } else {
        mbCpqCfg_hdrtmo_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMC_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_memc_enable = true;
    } else {
        mbCpqCfg_memc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AAD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_aad_enable = true;
    } else {
        mbCpqCfg_aad_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CABC_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cabc_enable = true;
    } else {
        mbCpqCfg_cabc_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLACK_BLUE_CHROMA_DB_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_seperate_black_blue_chorma_db_enable = true;
    } else {
        mbCpqCfg_seperate_black_blue_chorma_db_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLUESTRETCH_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_bluestretch_enable = true;
    } else {
        mbCpqCfg_bluestretch_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CHROMACORING_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_chromacoring_enable = true;
    } else {
        mbCpqCfg_chromacoring_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LOCALDIMMING_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_LocalDimming_enable = true;
    } else {
        mbCpqCfg_LocalDimming_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AISR_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_aisr_enable = true;
    } else {
        mbCpqCfg_aisr_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_NEW_PICTURE_MODE_ENABLE, "enable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_new_picture_mode_enable = true;
    } else {
        mbCpqCfg_new_picture_mode_enable = false;
    }

    vpp_pq_ctrl_t amvecmConfigVal;
    amvecmConfigVal.length = 14;//this is the count of pq_ctrl_s option
    amvecmConfigVal.ptr = &pqControlVal;
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_PQ_CTRL, &amvecmConfigVal);
    if (ret < 0) {
        LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
    }

    Cpq_SetVadjEnableStatus(pqControlVal.vadj1_en, pqControlVal.vadj2_en);

    return 0;
}

int CPQControl::SetPLLValues(source_input_param_t source_input_param)
{
    am_regs_t regs;
    int ret = 0;
    if (mPQdb->PQ_GetPLLParams (source_input_param, &regs ) == 0 ) {
        ret = AFEDeviceIOCtl(TVIN_IOC_LOAD_REG, &regs);
        if ( ret < 0 ) {
            LOGE ( "%s error(%s)!\n", __FUNCTION__, strerror(errno));
            return -1;
        }
    } else {
        LOGE ( "%s, PQ_GetPLLParams failed!\n", __FUNCTION__ );
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SSMReadNTypes(int id, int data_len, int offset)
{
    int value = 0;
    int ret = 0;

    ret = mSSMAction->SSMReadNTypes(id, data_len, &value, offset);

    if (ret < 0) {
        LOGE("Cpq_SSMReadNTypes, error(%s).\n", strerror ( errno ) );
        return -1;
    } else {
        return value;
    }
}

int CPQControl::Cpq_SSMWriteNTypes(int id, int data_len, int data_buf, int offset)
{
    int ret = 0;
    ret = mSSMAction->SSMWriteNTypes(id, data_len, &data_buf, offset);

    if (ret < 0) {
        LOGE("Cpq_SSMWriteNTypes, error(%s).\n", strerror ( errno ) );
    }

    return ret;
}

int CPQControl::Cpq_GetSSMActualAddr(int id)
{
    return mSSMAction->GetSSMActualAddr(id);
}

int CPQControl::Cpq_GetSSMActualSize(int id)
{
    return mSSMAction->GetSSMActualSize(id);
}

int CPQControl::SSMRecovery(void)
{
    return mSSMAction->SSMRecovery();
}

int CPQControl::Cpq_GetSSMStatus()
{
    return mSSMAction->GetSSMStatus();
}

int CPQControl::SetCurrentSourceInputInfo(source_input_param_t source_input_param)
{
    pthread_mutex_lock(&PqControlMutex);

    //get hdr type
    hdr_type_t newHdrType = HDR_TYPE_SDR;
    newHdrType = Cpq_GetSourceHDRType(source_input_param.source_input);

    LOGD("%s:new source info: source=%d,sigFmt=%d(0x%x),hdrType=%d\n", __FUNCTION__,
                                                                         source_input_param.source_input,
                                                                         source_input_param.sig_fmt,
                                                                         source_input_param.sig_fmt,
                                                                         newHdrType);
    //get output type
    output_type_t NewOutputType = OUTPUT_TYPE_LVDS;
    NewOutputType = CheckOutPutMode(source_input_param.source_input);

    if ((mCurentSourceInputInfo.source_input != source_input_param.source_input) ||
         (mCurentSourceInputInfo.sig_fmt     != source_input_param.sig_fmt) ||
         (mCurentSourceInputInfo.trans_fmt   != source_input_param.trans_fmt) ||
         (mCurrentHdrType                    != newHdrType) ||
         (mPQdb->mOutPutType                 != NewOutputType)) {
        LOGD("%s: update source input info\n", __FUNCTION__);
        mCurentSourceInputInfo.source_input = source_input_param.source_input;
        mCurentSourceInputInfo.trans_fmt    = source_input_param.trans_fmt;
        mCurentSourceInputInfo.sig_fmt      = source_input_param.sig_fmt;
        mPQdb->mOutPutType                  = NewOutputType;

        mSourceInputForSaveParam = mCurentSourceInputInfo.source_input;

        mCurentPqSource.pq_source_input = mSourceInputForSaveParam;
        mCurentPqSource.pq_sig_fmt = CheckPQTimming(newHdrType);
        LOGD("%s:mCurentPqSource is %d  mCurentPqTimming is %d\n", __FUNCTION__, mCurentPqSource.pq_source_input, mCurentPqSource.pq_sig_fmt);

        //update hdr type
        mCurrentHdrType                = newHdrType;
        mPQdb->mHdrType                = newHdrType;
        if (mbCpqCfg_seperate_db_enable) {
            mpOverScandb->mHdrType = newHdrType;
        }

        //load display mode setting
        vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
        SetDisplayMode(display_mode, 1);

        //load cvd2 parameter for cvbs decode
        if (((SOURCE_TV == mCurentSourceInputInfo.source_input)
            || (SOURCE_AV1 == mCurentSourceInputInfo.source_input)
            || (SOURCE_AV2 == mCurentSourceInputInfo.source_input))) {
            SetCVD2Values();
        }

        //load pq setting
        if (source_input_param.sig_fmt == TVIN_SIG_FMT_NULL) {//exit source
            mCurrentHdrType = HDR_TYPE_NONE;
            mPQdb->mHdrType = HDR_TYPE_NONE;
            if (mbCpqCfg_seperate_db_enable) {
                mpOverScandb->mHdrType = HDR_TYPE_NONE;
            }
            mCurrentTvinInfo.hdr_info    = 0;
            if ((mCurentSourceInputInfo.source_input == SOURCE_MPEG)
                || (mCurentSourceInputInfo.source_input == SOURCE_DTV)) {
                mCurentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
                //LoadPQSettings();
            } else if ((mCurentSourceInputInfo.source_input == SOURCE_TV)
                    || (mCurentSourceInputInfo.source_input == SOURCE_AV1)
                    || (mCurentSourceInputInfo.source_input == SOURCE_AV2)) {
                mCurentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_CVBS_NTSC_M;
            } else {
                mCurentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
            }
        } else if (source_input_param.sig_fmt == TVIN_SIG_FMT_MAX) {//enter nosignal
            mCurrentHdrType = HDR_TYPE_NONE;
            mPQdb->mHdrType = HDR_TYPE_NONE;
            if (mbCpqCfg_seperate_db_enable) {
                mpOverScandb->mHdrType = HDR_TYPE_NONE;
            }
            mCurrentTvinInfo.hdr_info    = 0;
            if ((mCurentSourceInputInfo.source_input    == SOURCE_TV)
                || (mCurentSourceInputInfo.source_input == SOURCE_AV1)
                || (mCurentSourceInputInfo.source_input == SOURCE_AV2)) {
               mCurentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_CVBS_NTSC_M;
            } else {
               mCurentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
            }
            LoadPQSettings();
        } else {//play video
            LoadPQSettings();
        }
    } else {
        LOGD("%s: same signal, no need set\n", __FUNCTION__);
    }

    pthread_mutex_unlock(&PqControlMutex);

    return 0;
}

int CPQControl::SetCurrentSource(tv_source_input_t source_input)
{
    LOGD("%s: source_input = %d\n", __FUNCTION__, source_input);

    source_input_param_t new_source_input_param;
    new_source_input_param.source_input = source_input;
    new_source_input_param.trans_fmt    = mCurentSourceInputInfo.trans_fmt;
    new_source_input_param.sig_fmt      = getVideoResolutionToFmt();
    SetCurrentSourceInputInfo(new_source_input_param);

    return 0;
}

source_input_param_t CPQControl::GetCurrentSourceInputInfo()
{
    pthread_mutex_lock(&PqControlMutex);
    source_input_param_t info = mCurentSourceInputInfo;
    pthread_mutex_unlock(&PqControlMutex);
    return info;
}

int CPQControl::GetRGBPattern() {
    char value[32] = {0};
    pqReadSys(VIDEO_RGB_SCREEN, value, sizeof(value));
    return strtol(value, NULL, 10);
}

int CPQControl::SetRGBPattern(int r, int g, int b) {
    int value = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    char str[32] = {0};
    sprintf(str, "%d", value);
    int ret = pqWriteSys(VIDEO_RGB_SCREEN, str);
    return ret;
}

int CPQControl::FactorySetDDRSSC(int step) {
    if (step < 0 || step > 5) {
        LOGE ("%s, step = %d is too long", __FUNCTION__, step);
        return -1;
    }

    return mSSMAction->SSMSaveDDRSSC(step);
}

int CPQControl::FactoryGetDDRSSC() {
    unsigned char data = 0;
    mSSMAction->SSMReadDDRSSC(&data);
    return data;
}

int CPQControl::SetLVDSSSC(int step) {
    if (step > 4)
        step = 4;

    FILE *fp = fopen(SSC_PATH, "w");
    if (fp != NULL) {
        fprintf(fp, "%d", step);
        fclose(fp);
    } else {
        LOGE("open /sys/class/lcd/ss ERROR(%s)!!\n", strerror(errno));
        return -1;
    }
    return 0;
}
int CPQControl::FactorySetLVDSSSC(int step) {
    int data[2] = {0, 0};
    int value = 0, panel_idx = 0, tmp = 0;
    const char *PanelIdx;
    if (step > 4)
        step = 4;
    PanelIdx = "0";//config_get_str ( CFG_SECTION_TV, "get.panel.index", "0" ); can't parse ini file in systemcontrol
    panel_idx = strtoul(PanelIdx, NULL, 10);
    LOGD ("%s, panel_idx = %x", __FUNCTION__, panel_idx);
    mSSMAction->SSMReadLVDSSSC(data);

    //every 2 bits represent one panel, use 2 byte store 8 panels
    value = (data[1] << 8) | data[0];
    step = step & 0x03;
    panel_idx = panel_idx * 2;
    tmp = 3 << panel_idx;
    value = (value & (~tmp)) | (step << panel_idx);
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    LOGD ("%s, tmp = %x, save value = %x", __FUNCTION__, tmp, value);

    SetLVDSSSC(step);
    return mSSMAction->SSMSaveLVDSSSC(data);
}

int CPQControl::FactoryGetLVDSSSC() {
    int data[2] = {0, 0};
    int value = 0, panel_idx = 0;
    const char *PanelIdx = "0";//config_get_str ( CFG_SECTION_TV, "get.panel.index", "0" );can't parse ini file in systemcontrol

    panel_idx = strtoul(PanelIdx, NULL, 10);
    mSSMAction->SSMReadLVDSSSC(data);
    value = (data[1] << 8) | data[0];
    value = (value >> (2 * panel_idx)) & 0x03;
    LOGD ("%s, panel_idx = %x, value= %d", __FUNCTION__, panel_idx, value);
    return value;
}

int CPQControl::SetGrayPattern(int value) {
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    value = value << 16 | 0x8080;

    LOGD("SetGrayPattern /sys/class/video/test_screen : %x", value);
    FILE *fp = fopen(TEST_SCREEN, "w");
    if (fp == NULL) {
        LOGE("Open /sys/classs/video/test_screen error(%s)!\n", strerror(errno));
        return -1;
    }

    fprintf(fp, "0x%x", value);
    fclose(fp);
    fp = NULL;

    return 0;
}

int CPQControl::GetGrayPattern() {
    int value = 0;
    int arg_num;

    FILE *fp = fopen(TEST_SCREEN, "r+");
    if (fp == NULL) {
        LOGE("Open /sys/class/video/test_screen error(%s)!\n", strerror(errno));
        return -1;
    }

    arg_num = fscanf(fp, "%x", &value);
    if (arg_num < 0) {
        LOGE("arg_num: %d\n", arg_num);
    }

    LOGD("GetGrayPattern /sys/class/video/test_screen %x", value);
    fclose(fp);
    fp = NULL;
    if (value < 0) {
        return 0;
    } else {
        value = value >> 16;
        if (value > 255) {
            value = 255;
        }
        return value;
    }
}

int CPQControl::SetHDRMode(int mode)
{
    int ret = -1;
    if ((mCurentSourceInputInfo.source_input == SOURCE_MPEG) ||
       ((mCurentSourceInputInfo.source_input >= SOURCE_HDMI1) && mCurentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_CSCTYPE, &mode);
        if (ret < 0) {
            LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        }
    } else {
        LOGE("%s: Curent source no hdr status!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHDRMode()
{
    ve_csc_type_t mode = VPP_MATRIX_NULL;
    if ((mCurentSourceInputInfo.source_input == SOURCE_MPEG) ||
       ((mCurentSourceInputInfo.source_input >= SOURCE_HDMI1) && mCurentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
        int ret = VPPDeviceIOCtl(AMVECM_IOC_G_CSCTYPE, &mode);
        if (ret < 0) {
            LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
            mode = VPP_MATRIX_NULL;
        } else {
            LOGD("%s: mode is %d\n", __FUNCTION__, mode);
        }
    } else {
        LOGD("%s: Curent source no hdr status!\n", __FUNCTION__);
    }

    return mode;
}

hdr_type_t CPQControl::Cpq_GetSourceHDRType(tv_source_input_t source_input)
{
    hdr_type_t newHdrType = HDR_TYPE_SDR;
    if ((source_input == SOURCE_MPEG)
        ||(source_input == SOURCE_DTV)) {
        if (!mbVideoIsPlaying) {
            newHdrType = HDR_TYPE_SDR;
        } else {
            char buf[32] = {0};
            int ret = pqReadSys(SYS_VIDEO_HDR_FMT, buf, sizeof(buf));
            if (ret < 0) {
                newHdrType = HDR_TYPE_SDR;
                LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
            } else {
                if (0 == strcmp(buf, "src_fmt = SDR")) {
                    newHdrType = HDR_TYPE_SDR;
                } else if (0 == strcmp(buf, "src_fmt = HDR10")) {
                    newHdrType = HDR_TYPE_HDR10;
                } else if (0 == strcmp(buf, "src_fmt = HDR10+")) {
                    newHdrType = HDR_TYPE_HDR10PLUS;
                } else if (0 == strcmp(buf, "src_fmt = HDR10 prime")) {
                    newHdrType = HDR_TYPE_PRIMESL;
                } else if (0 == strcmp(buf, "src_fmt = HLG")) {
                    newHdrType = HDR_TYPE_HLG;
                } else if (0 == strcmp(buf, "src_fmt = Dolby Vison")) {
                    newHdrType = HDR_TYPE_DOVI;
                } else if (0 == strcmp(buf, "src_fmt = MVC")) {
                    newHdrType = HDR_TYPE_MVC;
                } else {
                    LOGD("%s: invalid hdr type!\n", __FUNCTION__);
                    newHdrType = HDR_TYPE_SDR;
                }
            }
        }
    } else if ((source_input == SOURCE_HDMI1)
             || (source_input == SOURCE_HDMI2)
             || (source_input == SOURCE_HDMI3)
             || (source_input == SOURCE_HDMI4)) {
            int signalRange                  = (mCurrentTvinInfo.hdr_info >> 29) & 0x1;
            int signalColorPrimaries         = (mCurrentTvinInfo.hdr_info >> 16) & 0xff;
            int signalTransferCharacteristic = (mCurrentTvinInfo.hdr_info >> 8)  & 0xff;
            int dvFlag                       = (mCurrentTvinInfo.hdr_info >> 30) & 0x1;
            LOGD("%s: signalRange=0x%x, signalColorPrimaries=0x%x, signalTransferCharacteristic=0x%x, dvFlag=0x%x\n",
                    __FUNCTION__, signalRange, signalColorPrimaries, signalTransferCharacteristic, dvFlag);
            if (((signalTransferCharacteristic == 0xe) || (signalTransferCharacteristic == 0x12))
                && (signalColorPrimaries == 0x9)) {
                newHdrType = HDR_TYPE_HLG;
            } else if ((signalTransferCharacteristic == 0x30) && (signalColorPrimaries == 0x9)) {
                newHdrType = HDR_TYPE_HDR10PLUS;
            } else if ((signalTransferCharacteristic == 0x10) || (signalColorPrimaries == 0x9)) {
                newHdrType = HDR_TYPE_HDR10;
            } else if (dvFlag == 0x1) {
                newHdrType = HDR_TYPE_DOVI;
            } else {
                newHdrType = HDR_TYPE_SDR;
            }
    } else {
        LOGD("%s: This source no hdr status\n", __FUNCTION__);
        newHdrType = HDR_TYPE_SDR;
    }

    return newHdrType;
}

tvpq_databaseinfo_t CPQControl::GetDBVersionInfo(db_name_t name) {
    bool val = false;
    tvpq_databaseinfo_t pqdatabaseinfo_t;
    memset(&pqdatabaseinfo_t, 0, sizeof(pqdatabaseinfo_t));
    database_attribute_t dbAttribute;

    switch (name) {
        case DB_NAME_PQ:
            val = mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);
            break;
        case DB_NAME_OVERSCAN:
            val = mpOverScandb->GetOverScanDbVersion(dbAttribute.ToolVersion, dbAttribute.ProjectVersion, dbAttribute.dbversion, dbAttribute.GenerateTime);
            break;
        default:
            val = mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);;
            break;
    }

    if (val) {
        strcpy(pqdatabaseinfo_t.ToolVersion, dbAttribute.ToolVersion.c_str());
        strcpy(pqdatabaseinfo_t.ProjectVersion, dbAttribute.ProjectVersion.c_str());
        strcpy(pqdatabaseinfo_t.GenerateTime, dbAttribute.GenerateTime.c_str());
    }

    return pqdatabaseinfo_t;
}

int CPQControl::GetSourceHDRType()
{
    LOGD("%s: hdr type is %d\n", __FUNCTION__, mCurrentHdrType);
    return mCurrentHdrType;
}

int CPQControl::SetCurrentHdrInfo(int hdrInfo)
{
    int ret = 0;
    if (mCurrentTvinInfo.hdr_info != (unsigned int)hdrInfo) {
        mCurrentTvinInfo.hdr_info = (unsigned int)hdrInfo;
        LOGD("%s: HDR info=0x%x\n", __FUNCTION__, mCurrentTvinInfo.hdr_info);
        SetCurrentSourceInputInfo(mCurentSourceInputInfo);
    } else {
        LOGD("%s: same HDR info\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::SetDtvKitSourceEnable(bool isEnable)
{
    mbDtvKitEnable = isEnable;
    return 0;
}

//AI
void CPQControl::AipqInit()
{
    if (GetAipqEnable() == 1) {
        enableAipq(true);
    }
}

int CPQControl::SetAipqEnable(bool isEnable)
{
    enableAipq(isEnable);
    mSSMAction->SSMSaveAipqEnableVal(isEnable ? 1 : 0);
    return 0;
}

int CPQControl::GetAipqEnable()
{
    int data = 0;
    mSSMAction->SSMReadAipqEnableVal(&data);

    if (data < 0 || data > 1) {
        data = 0;
    }
    return data;
}

void CPQControl::enableAipq(bool isEnable)
{
    if (mbCpqCfg_ai_enable) {
        pqWriteSys(DECODER_COMMON_PARAMETERS_DEBUG_VDETECT,  isEnable ? "1" : "0");
        pqWriteSys(VDETECT_AIPQ_ENABLE,  isEnable ? "1" : "0");
    } else {
        LOGE("ai pq is disable\n", __FUNCTION__);
    }
}

int CPQControl::AiParamLoad(void)
{
    int ret = -1;

    if (mbCpqCfg_ai_enable) {
        ai_pic_table_t aiRegs;
        memset(&aiRegs, 0, sizeof(ai_pic_table_t));
        ret = mPQdb->PQ_GetAIParams(mCurentSourceInputInfo, &aiRegs);
        if (ret >= 0) {
            LOGD("%s: width: %d, height: %d, array: %s.\n", __FUNCTION__, aiRegs.width, aiRegs.height, aiRegs.table_ptr);
            ret = VPPDeviceIOCtl(AMVECM_IOC_S_AIPQ_TABLE, &aiRegs);
            if (ret < 0) {
                LOGE("%s: iocontrol failed\n", __FUNCTION__);
            }
        } else {
            LOGE("%s: get AI pq params failed\n", __FUNCTION__);
        }
    } else {
        LOGE("ai is disable\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }

    return ret;
}

bool CPQControl::hasAisrFunc()
{
    return mbCpqCfg_aisr_enable;
}

int CPQControl::SetAiSrEnable(bool isEnable)
{
    LOGD("%s isEnable = %d\n", __FUNCTION__, isEnable);
    int ret = -1;
    ret = Cpq_SetAiSrEnable(isEnable);

    if (ret < 0) {
        LOGD("%s Cpq_SetAiSrEnable fail\n", __FUNCTION__);
        return ret;
    } else {
        ret = SaveAiSrEnable(isEnable);
    }

    if (ret < 0) {
        LOGD("%s failed\n", __FUNCTION__);
    } else {
        LOGD("%s success\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetAiSrEnable()
{
    int data = 0;
    mSSMAction->SSMReadAiSrEnable(&data);
    LOGD(" %s, data = %d\n", __FUNCTION__, data);

    if (data < 0 || data > 1) {
        data = 0;
    }
    return data;
}

int CPQControl::SaveAiSrEnable(bool enable)
{
    LOGD(" %s, enable = %d\n", __FUNCTION__, enable);
    int ret = mSSMAction->SSMSaveAiSrEnable(enable ? 1 : 0);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetAiSrEnable(bool enable)
{
    int ret = 0;
    if (mbCpqCfg_aisr_enable) {
        ret =pqWriteSys(VIDEO_AISR_ENABLE, enable ? "1" : "0");
    } else {
        LOGE("%s disabled\n",__FUNCTION__);
    }
    return ret;
}

int CPQControl::ResetPQModeSetting(tv_source_input_t source_input, vpp_picture_mode_t pq_mode)
{
    int i = 0, j = 0, k = 0, config_val = 0;
    vpp_pictur_mode_para_t picture;
    pq_src_param_t src;

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        for (j = PQ_FMT_DEFAUT; j < PQ_FMT_MAX; j++) {
            src.pq_source_input = (tv_source_input_t)i;
            src.pq_sig_fmt = (pq_sig_fmt_t)j;

            //picture
            if (j == PQ_FMT_DOBLY) {
                config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DV_PICTUREMODE_DEF, VPP_PICTURE_MODE_DV_BRIGHT);
            } else {
                config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_PICTUREMODE_DEF, VPP_PICTURE_MODE_STANDARD);
            }
            mSSMAction->SSMSavePictureMode(i * PQ_FMT_MAX + j, config_val);
            mSSMAction->SSMSaveLastPictureMode(i * PQ_FMT_MAX + j, config_val);

            //picture mode params
            for (k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
                if (mPQdb->PQ_GetPictureModeParams(src, vpp_picture_mode_t(k), &picture) == 0) {
                    SetPictureModeData(src, vpp_picture_mode_t(k), &picture);
                } else {
                    RsetPictureModeData(src, vpp_picture_mode_t(k));
                }
            }
        }
    }

    return 0;
}

int CPQControl::SetLocalDimming(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;

    ret = Cpq_LocalDimming((vpp_pq_level_t)level);

    if (ret == 0 && is_save == 1) {
        ret = mSSMAction->SSMSaveLocalDimming(mSourceInputForSaveParam, level);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetLocalDimming(void)
{
    int level = VPP_PQ_LV_OFF;
    if (mSSMAction->SSMReadLocalDimming(mSourceInputForSaveParam, &level) < 0) {
        LOGE("%s, SSMReadLocalDimming ERROR!!!\n", __FUNCTION__);
        return VPP_PQ_LV_OFF;
    } else {
        LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    return level;
}

int CPQControl::Cpq_LocalDimming(vpp_pq_level_t level)
{
    int ret = -1;
    aml_ldim_pq_s ld;
    memset(&ld, 0, sizeof(aml_ldim_pq_s));

    if (mbCpqCfg_LocalDimming_enable && (mLdFd > 0)) {
        ret = mPQdb->PQ_GetLocalDimmingParams(level, mCurentSourceInputInfo, &ld);

        if (ret < 0) {
            LOGE("%s: PQ_GetLocalDimmingParams failed!\n", __FUNCTION__);
        } else {
            ret = LCDLdimDeviceIOCtl(AML_LDIM_IOC_CMD_SET_INFO_NEW, &ld);
        }
    } else {
        LOGE("%s: LocalDimming disabled!\n", __FUNCTION__);
        ret = 0;
    }

    return ret;
}

void CPQControl::resetAllUserSettingParam()
{
    int i = 0, config_val = 0;
    vpp_pq_para_t pq_para;
    const char *buf = NULL;

    if (mbCpqCfg_new_picture_mode_enable) {
        resetPQUiSetting();
        resetPQTableSetting();
        return;
    }

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        if (mbCpqCfg_seperate_db_enable) {
            mpOverScandb->PQ_GetPQModeParams((tv_source_input_t)i, VPP_PICTURE_MODE_USER, &pq_para);
        } else {
            mPQdb->PQ_GetPQModeParams((tv_source_input_t)i, VPP_PICTURE_MODE_USER, &pq_para);
        }
        /*LOGD("%s: brightness=%d, contrast=%d, saturation=%d, hue=%d, sharpness=%d, backlight=%d, nr=%d\n",
                 __FUNCTION__, pq_para.brightness, pq_para.contrast, pq_para.saturation, pq_para.hue,
                 pq_para.sharpness, pq_para.backlight, pq_para.nr);*/
        mSSMAction->SSMSaveBrightness((tv_source_input_t)i, pq_para.brightness);
        mSSMAction->SSMSaveContrast((tv_source_input_t)i, pq_para.contrast);
        mSSMAction->SSMSaveSaturation((tv_source_input_t)i, pq_para.saturation);
        mSSMAction->SSMSaveHue((tv_source_input_t)i, pq_para.hue);
        mSSMAction->SSMSaveSharpness((tv_source_input_t)i, pq_para.sharpness);
        mSSMAction->SSMSaveBackLightVal(pq_para.backlight);
        mSSMAction->SSMSaveNoiseReduction((tv_source_input_t)i, pq_para.nr);
        mSSMAction->SSMSaveColorGamutMode((tv_source_input_t)i, 0);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_PICTUREMODE_DEF, VPP_PICTURE_MODE_STANDARD);
        mSSMAction->SSMSavePictureMode(i, config_val);
        mSSMAction->SSMSaveLastPictureMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORTEMPTUREMODE_DEF, VPP_COLOR_TEMPERATURE_MODE_STANDARD);
        mSSMAction->SSMSaveColorTemperature(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DISPLAYMODE_DEF, VPP_DISPLAY_MODE_NORMAL);
        mSSMAction->SSMSaveDisplayMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_GAMMALEVEL_DEF, VPP_GAMMA_CURVE_DEFAULT);
        mSSMAction->SSMSaveGammaValue(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_AUTOASPECT_DEF, 1);
        mSSMAction->SSMSaveAutoAspect(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_43STRETCH_DEF, 0);
        mSSMAction->SSMSave43Stretch(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DNLPLEVEL_DEF, DYNAMIC_CONTRAST_MID);
        mSSMAction->SSMSaveDnlpMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DNLPGAIN_DEF, 0);
        mSSMAction->SSMSaveDnlpGainValue(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_LOCALCONTRASTMODE_DEF, 2);
        mSSMAction->SSMSaveLocalContrastMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DEBLOCKMODE_DEF, VPP_DEBLOCK_MODE_OFF);
        mSSMAction->SSMSaveDeblockMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DEMOSQUITOMODE_DEF, VPP_DEMOSQUITO_MODE_OFF);
        mSSMAction->SSMSaveDemoSquitoMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_MCDI_DEF, VPP_MCDI_MODE_STANDARD);
        mSSMAction->SSMSaveMcDiMode(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_MEMCMODE_DEF, VPP_MEMC_MODE_HIGH);
        mSSMAction->SSMSaveMemcMode(i, config_val);

        buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMCDEBLURLEVEL_DEF, NULL);
        int Deblur_para[VPP_MEMC_MODE_MAX] = { 0, 3, 6, 10 };
        pqTransformStringToInt(buf, Deblur_para);
        for (int j = VPP_MEMC_MODE_OFF; j < VPP_MEMC_MODE_MAX; j++) {
            mSSMAction->SSMSaveMemcDeblurLevel(i * VPP_MEMC_MODE_MAX + j, Deblur_para[j]);
        }

        buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMCDEJUDDERLEVEL_DEF, NULL);
        int DeJudder_para[VPP_MEMC_MODE_MAX] = { 0, 3, 6, 10 };
        pqTransformStringToInt(buf, DeJudder_para);
        for (int j = VPP_MEMC_MODE_OFF; j < VPP_MEMC_MODE_MAX; j++) {
            mSSMAction->SSMSaveMemcDeJudderLevel(i * VPP_MEMC_MODE_MAX + j, DeJudder_para[j]);
        }

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_BLACKSTRETCH_DEF, VPP_PQ_LV_OFF);
        mSSMAction->SSMSaveBlackStretch(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_BLUESTRETCH_DEF, VPP_PQ_LV_OFF);
        mSSMAction->SSMSaveBlueStretch(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_CHMROMACORING_DEF, VPP_PQ_LV_OFF);
        mSSMAction->SSMSaveChromaCoring(i, config_val);

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_LOCALDIMMING_DEF, VPP_PQ_LV_OFF);
        mSSMAction->SSMSaveLocalDimming(i, config_val);
    }

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORDEMOMODE_DEF, VPP_COLOR_DEMO_MODE_ALLON);
    mSSMAction->SSMSaveColorDemoMode(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORBASEMODE_DEF, VPP_COLOR_BASE_MODE_OPTIMIZE);
    mSSMAction->SSMSaveColorBaseMode (config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_R_DEF, 0);
    mSSMAction->SSMSaveRGBGainRStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_G_DEF, 0);
    mSSMAction->SSMSaveRGBGainGStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBGAIN_B_DEF, 0);
    mSSMAction->SSMSaveRGBGainBStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_R_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetRStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_G_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetGStart(0, config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_RGBPOSTOFFSET_B_DEF_DEF, 1024);
    mSSMAction->SSMSaveRGBPostOffsetBStart(0, config_val);

    int8_t std_buf[6] = { 0, 0, 0, 0, 0, 0 };
    int8_t warm_buf[6] = { 0, 0, -8, 0, 0, 0 };
    int8_t cold_buf[6] = { -8, 0, 0, 0, 0, 0 };
    for (i = 0; i < 6; i++) {
        mSSMAction->SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_STANDARD * 6, std_buf[i]); //0~5
        mSSMAction->SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_WARM * 6, warm_buf[i]); //6~11
        mSSMAction->SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_COLD * 6, cold_buf[i]); //12~17
    }

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORSPACE_DEF, VPP_COLOR_SPACE_AUTO);
    mSSMAction->SSMSaveColorSpaceStart(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DDRSSC_DEF, 0);
    mSSMAction->SSMSaveDDRSSC(config_val);

    buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LVDSSSC_DEF, NULL);
    int tmp[2] = {0, 0};
    pqTransformStringToInt(buf, tmp);
    mSSMAction->SSMSaveLVDSSSC(tmp);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_EYEPROJECTMODE_DEF, 0);
    mSSMAction->SSMSaveEyeProtectionMode(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_EDID_VERSION_DEF, 0);
    mSSMAction->SSMEdidRestoreDefault(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_HDCP_SWITCHER_DEF, 0);
    mSSMAction->SSMHdcpSwitcherRestoreDefault(0);

    buf = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_COLOR_RANGE_MODE_DEF, NULL);
    if (strcmp(buf, "full") == 0) {
        mSSMAction->SSMSColorRangeModeRestoreDefault(1);
    } else if (strcmp(buf, "limit") == 0) {
        mSSMAction->SSMSColorRangeModeRestoreDefault(2);
    } else {
        mSSMAction->SSMSColorRangeModeRestoreDefault(0);
    }
    //static frame
    Cpq_SSMWriteNTypes(SSM_RW_BLACKOUT_ENABLE_START, 1, 1, 0);
    //screen color for signal
    Cpq_SSMWriteNTypes(CUSTOMER_DATA_POS_SCREEN_COLOR_START, 1, 0, 0);

    mSSMAction->SSMSaveAipqEnableVal(0);
    mSSMAction->SSMSaveAiSrEnable(1);

    return;
}


int CPQControl::pqWriteSys(const char *path, const char *val)
{
    int fd;
    if ((fd = open(path, O_RDWR)) < 0) {
        LOGE("writeSys, open %s error(%s)\n", path, strerror (errno));
        return -1;
    }

    int len = write(fd, val, strlen(val));
    close(fd);
    return len;
}

int CPQControl::pqReadSys(const char *path, char *buf, int count)
{
    int fd, len;

    if ( NULL == buf ) {
        LOGE("buf is NULL.\n");
        return -1;
    }

    if ((fd = open(path, O_RDONLY)) < 0) {
        LOGE("pqReadSys, open %s error(%s).\n", path, strerror (errno));
        return -1;
    }

    len = read(fd, buf, count);
    if (len < 0) {
        LOGE("pqReadSys %s error, %s\n", path, strerror(errno));
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

void CPQControl::pqTransformStringToInt(const char *buf, int *val)
{
    if (buf != NULL) {
        //LOGD("%s: %s\n", __FUNCTION__, buf);
        char temp_buf[256];
        char *p = NULL;
        int i = 0;
        strcpy(temp_buf, buf);
        p = strtok(temp_buf, ",");
        while (NULL != p) {
           val[i++] = atoi(p);
           p = strtok(NULL,  ",");
        }
    } else {
        LOGE("%s:Invalid param!\n", __FUNCTION__);
    }
    return;
}

int CPQControl::Cpq_GetInputVideoFrameHeight(tv_source_input_t source_input)
{
    int inputFrameHeight = 0;

    char inputModeBuf[32] = {0};
    if (pqReadSys(SYS_VIDEO_FRAME_HEIGHT, inputModeBuf, sizeof(inputModeBuf)) > 0) {
        inputFrameHeight = atoi(inputModeBuf);
    } else {
        LOGD("Read %s failed!\n", SYS_VIDEO_FRAME_HEIGHT);
    }

    if (inputFrameHeight <= 0) {
        LOGD("%s: inputFrameHeight is invalid, return default value!\n", __FUNCTION__);
        inputFrameHeight = 1080;
    }

    LOGD("%s: inputFrameHeight is %d!\n", __FUNCTION__, inputFrameHeight);
    return inputFrameHeight;
}

output_type_t CPQControl::CheckOutPutMode(tv_source_input_t source_input)
{
    output_type_t OutPutType = OUTPUT_TYPE_LVDS;
    if (!isFileExist(HDMI_OUTPUT_CHECK_PATH)) {//LVDS output
        OutPutType = OUTPUT_TYPE_LVDS;
    } else {
        int outputFrameHeight = 1080;
        char outputModeBuf[32] = {0};
        if ((pqReadSys(SYS_DISPLAY_MODE_PATH, outputModeBuf, sizeof(outputModeBuf)) < 0) || (strlen(outputModeBuf) == 0)) {
            LOGD("Read %s failed!\n", SYS_DISPLAY_MODE_PATH);
        } else {
            LOGD("%s: current output mode is %s!\n", __FUNCTION__, outputModeBuf);
            if (strstr(outputModeBuf, "null")) {
                return OUTPUT_TYPE_MAX;
            } else if (strstr(outputModeBuf, "480cvbs")) {//NTSC output
                OutPutType = OUTPUT_TYPE_NTSC;
            } else if(strstr(outputModeBuf, "576cvbs")) {//PAL output
                OutPutType = OUTPUT_TYPE_PAL;
            } else {//HDMI output
                //get output height
                if (strstr(outputModeBuf, "smpte")) {
                    outputFrameHeight = 4096;
                } else {
                    const char delim[2] = "p";
                    char *token;

                    token = strtok(outputModeBuf, delim);
                    if (token != NULL) {
                        outputFrameHeight = atoi(token);
                    }
                }
                LOGD("%s: outputFrameHeight: %d\n", __FUNCTION__, outputFrameHeight);
                //check outputmode
                if ((source_input == SOURCE_MPEG)
                    || (source_input == SOURCE_DTV)
                    || (source_input == SOURCE_HDMI1)
                    || (source_input == SOURCE_HDMI2)
                    || (source_input == SOURCE_HDMI3)
                    || (source_input == SOURCE_HDMI4)) {//hdmi/dtv/mpeg input
                    int inputFrameHeight = Cpq_GetInputVideoFrameHeight(source_input);
                    if (inputFrameHeight > 1088) {//inputsource is 4k
                        OutPutType = OUTPUT_TYPE_HDMI_4K;
                    } else {
                        if (inputFrameHeight >= outputFrameHeight) {//input height >= output height
                            OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                        } else {//input height < output height
                            if (inputFrameHeight > 720 && inputFrameHeight <= 1088) {//inputsource is 1080
                                OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                            } else if (inputFrameHeight > 576 && inputFrameHeight <= 720) {//inputsource is 720
                                if (outputFrameHeight == 4096) {
                                    OutPutType = OUTPUT_TYPE_HDMI_HD_4096;
                                } else if (outputFrameHeight >= inputFrameHeight * 2) {
                                    OutPutType = OUTPUT_TYPE_HDMI_HD_UPSCALE;
                                } else {
                                    OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                                }
                            } else {//inputsource is 480
                                if (outputFrameHeight == 4096) {
                                    OutPutType = OUTPUT_TYPE_HDMI_SD_4096;
                                } else if ((outputFrameHeight * 8) >= (inputFrameHeight * 15)) {
                                    OutPutType = OUTPUT_TYPE_HDMI_SD_UPSCALE;
                                } else {
                                    OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                                }
                            }
                        }
                    }
                } else {//atv/av input
                    if (outputFrameHeight >= 720) {
                        OutPutType = OUTPUT_TYPE_HDMI_SD_UPSCALE;
                    } else {
                        OutPutType = OUTPUT_TYPE_HDMI_NOSCALE;
                    }
                }
            }
        }
    }

    LOGD("%s: output mode is %d\n", __FUNCTION__, OutPutType);
    return OutPutType;
}

bool CPQControl::CheckPQModeTableInDb(void)
{
    bool ret = mPQdb->CheckPQModeTableInDb();
    if (ret) {
        LOGD("pq mode table exist\n");
    } else {
        LOGD("pq mode table don't exist\n");
    }
    return ret;
}

bool CPQControl::isPqDatabaseMachChip()
{
    bool matchStatus = false;
    meson_cpu_ver_e chipVersion = MESON_CPU_VERSION_NULL;

    database_attribute_t dbAttribute;
    mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);
    if (dbAttribute.ChipVersion.length() == 0) {
        LOGD("%s: ChipVersion is null!\n", __FUNCTION__);
        chipVersion = MESON_CPU_VERSION_NULL;
    } else {
        int flagPosition = dbAttribute.ChipVersion.find("_");
        std::string versionStr = dbAttribute.ChipVersion.substr(flagPosition+1, 1);
        LOGD("%s: versionStr is %s!\n", __FUNCTION__, versionStr.c_str());
        if (versionStr == "A") {
            chipVersion = MESON_CPU_VERSION_A;
        } else if (versionStr ==  "B") {
            chipVersion = MESON_CPU_VERSION_B;
        } else if (versionStr == "C") {
            chipVersion = MESON_CPU_VERSION_C;
        } else {
            chipVersion = MESON_CPU_VERSION_NULL;
        }
    }

    if (chipVersion == MESON_CPU_VERSION_NULL) {
        LOGD("%s: database don't have chipversion!\n", __FUNCTION__);
        matchStatus = true;
    } else {
        int ret = VPPDeviceIOCtl(AMVECM_IOC_S_MESON_CPU_VER, &chipVersion);
        if (ret < 0) {
            LOGD("%s: database don't match chip!\n", __FUNCTION__);
            matchStatus = false;
        } else {
            LOGD("%s: database is match chip!\n", __FUNCTION__);
            matchStatus = true;
        }
    }

    return matchStatus;
}

int CPQControl::Cpq_SetVadjEnableStatus(int isvadj1Enable, int isvadj2Enable)
{
    LOGD("%s: isvadj1Enable = %d, isvadj2Enable = %d.\n", __FUNCTION__, isvadj1Enable, isvadj2Enable);
    int ret = -1;
    if ((!mbCpqCfg_amvecm_basic_enable) && (!mbCpqCfg_amvecm_basic_withOSD_enable)) {
        ret = 0;
        LOGD("%s: all vadj module disabled.\n", __FUNCTION__);
    } else {
        am_pic_mode_t params;
        memset(&params, 0, sizeof(params));
        params.flag |= (0x1<<6);
        params.vadj1_en = isvadj1Enable;
        params.vadj2_en = isvadj2Enable;
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_PIC_MODE, &params);
        if (ret < 0) {
            LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        }
    }

    return ret;
}
