/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MODULE_TAG "PQ"
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
    mAmvideoFd  =-1;
    mDiFd = -1;
    mLcdFd = -1;
    mLdFd = -1;
    mMemcFd = -1;
    mpVdin = NULL;
    mPQdb = NULL;
    mpOverScandb = NULL;
    mSSMAction =  NULL;
    mPQConfigFile = NULL;
    mDolbyVision = NULL;
    mpObserver = NULL;
    mInitialized   = false;
    mbDtvKitEnable = false;
    mbDatabaseMatchChipStatus = false;
    mbCpqCfg_whitebalance_enable = false;
    mbCpqCfg_dnlp_enable = false;
    mbCpqCfg_xvycc_enable = false;
    mbCpqCfg_display_overscan_enable=  false;
    mbCpqCfg_local_contrast_enable = false;
    mbCpqCfg_hdmi_out_with_fbc_enable = false;
    mbCpqCfg_pq_param_check_source_enable = false;
    mbCpqCfg_seperate_db_enable = false;
    mbCpqCfg_amvecm_basic_enable = false;
    mbCpqCfg_amvecm_basic_withOSD_enable = false;
    mbCpqCfg_contrast_rgb_enable = false;
    mbCpqCfg_contrast_rgb_withOSD_enable = false;
    mbCpqCfg_blackextension_enable = false;
    mbCpqCfg_sharpness0_enable = false;
    mbCpqCfg_sharpness1_enable = false;
    mbCpqCfg_di_enable = false;
    mbCpqCfg_mcdi_enable = false;
    mbCpqCfg_deblock_enable = false;
    mbCpqCfg_nr_enable = false;
    mbCpqCfg_demoSquito_enable = false;
    mbCpqCfg_gamma_enable = false;
    mbCpqCfg_cm2_enable = false;
    mbCpqCfg_pq_mode_check_source_enable = false;
    mbCpqCfg_pq_mode_check_hdr_enable = false;
    mbCpqCfg_pq_param_check_hdr_enable = false;
    mbCpqCfg_ldim_enable = false;
    mbCpqCfg_lcd_hdrinfo_enable = false;
    mbAllmModeCfg_enable = false;
    mbItcontentModeCfg_enable = false;
    mbDviModeCfg_enable = false;
    mbCpqCfg_pq_enable = false;
    mbCpqCfg_cvd2_enable = false;
    mbCpqCfg_ai_enable = false;
    mbCpqCfg_aad_enable = false;
    mbCpqCfg_cabc_enable = false;
    mbCpqCfg_smoothplus_enable = false;
    mbCpqCfg_hdrtmo_enable = false;
    mbCpqCfg_memc_enable = false;
    mbCpqCfg_seperate_black_blue_chorma_db_enable = false;
    mbCpqCfg_bluestretch_enable = false;
    mbCpqCfg_chromacoring_enable = false;
    mbCpqCfg_LocalDimming_enable = false;
    mbCpqCfg_aisr_enable = false;
    mbCpqCfg_new_picture_mode_enable = false;
    mSourceInputForSaveParam = SOURCE_MPEG;
    memset(rgbfrompq, 0, sizeof(tcon_rgb_ogo_t));
    memset(&mCurrentSourceInputInfo, 0, sizeof(source_input_param_t));
    memset(&mCurrentSignalInfo, 0, sizeof(struct tvin_parm_s));
    memset(&mCurrentTvinInfo, 0, sizeof(tvin_inputparam_t));
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
    mCurrentSourceInputInfo.source_input = SOURCE_MPEG;
    mCurrentSourceInputInfo.trans_fmt    = TVIN_TFMT_2D;
    mCurrentHdrType                     = HDRTYPE_SDR;
    mSourceInputForSaveParam            = SOURCE_MPEG;

    pthread_mutex_lock(&PqControlMutex); //for coverity
    mCurrentSourceInputInfo.sig_fmt      = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
    pthread_mutex_unlock(&PqControlMutex);

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
        Cpq_SetDNLPStatus(DNLP_ON);
    } else {
        Cpq_SetDNLPStatus(DNLP_OFF);
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
    InitAutoNr();

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

        if (((mCurrentSourceInputInfo.source_input == SOURCE_DTV)
            || (mCurrentSourceInputInfo.source_input == SOURCE_MPEG))
            && (frameszieEventFlag == 1)) {
            source_input_param_t new_source_input_param;
            new_source_input_param.source_input = mCurrentSourceInputInfo.source_input;
            new_source_input_param.trans_fmt    = mCurrentSourceInputInfo.trans_fmt;
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

enum tvin_sig_fmt_e CPQControl::getVideoResolutionToFmt()
{
    int ret = -1;
    int fd = -1;
    char buf[32] = {0};
    enum tvin_sig_fmt_e sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;

    fd = open(SYS_VIDEO_FRAME_HEIGHT, O_RDONLY);
    if (fd < 0) {
        LOGE("[%s] open: %s error!\n", __FUNCTION__, SYS_VIDEO_FRAME_HEIGHT);
        return sig_fmt;
    }

    ret = read(fd, buf, sizeof(buf));
    if (ret >0) {
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

int CPQControl::Cpq_GetAllmAndVrrInfo(void)
{
    int ret = -1;
    struct tvin_latency_s allmInfo;
    struct vdin_vrr_freesync_param_s vrrparm;

    memset(&allmInfo, 0x0, sizeof(struct tvin_latency_s));
    memset(&vrrparm, 0x0, sizeof(struct vdin_vrr_freesync_param_s));

    ret = mpVdin->Tvin_GetAllmInfo(&allmInfo);
    LOGD("%s allm_mode: %d, it_content: %d, cn_type: %d\n", __FUNCTION__,
        allmInfo.allm_mode, allmInfo.it_content, allmInfo.cn_type);

    mCurrentTvinInfo.allmInfo.allm_mode  = allmInfo.allm_mode;
    mCurrentTvinInfo.allmInfo.it_content = allmInfo.it_content;
    mCurrentTvinInfo.allmInfo.cn_type    = allmInfo.cn_type;

    ret |= mpVdin->Tvin_GetVrrFreesyncParm(&vrrparm);
    LOGD("%s cur_vrr_status: %d\n", __FUNCTION__, vrrparm.cur_vrr_status);

    mCurrentTvinInfo.vrrparm.cur_vrr_status = vrrparm.cur_vrr_status;

    LOGD("%s get info %s\n", __FUNCTION__, (ret < 0) ? "fail" : "success");
    return ret;
}

int CPQControl::SetCurrenSourceInfo(struct tvin_parm_s sig_info)
{
    if (mCurrentSignalInfo.info.trans_fmt  != sig_info.info.trans_fmt
        || mCurrentSignalInfo.info.fmt     != sig_info.info.fmt
        || mCurrentSignalInfo.info.status  != sig_info.info.status
        || mCurrentSignalInfo.port         != sig_info.port
        || mCurrentSignalInfo.info.signal_type != sig_info.info.signal_type) {
        mCurrentSignalInfo.info.trans_fmt    = sig_info.info.trans_fmt;
        mCurrentSignalInfo.info.fmt          = sig_info.info.fmt;
        mCurrentSignalInfo.info.status       = sig_info.info.status;
        mCurrentSignalInfo.info.cfmt         = sig_info.info.cfmt;
        mCurrentSignalInfo.info.signal_type     = sig_info.info.signal_type;
        mCurrentSignalInfo.info.fps          = sig_info.info.fps;
        mCurrentSignalInfo.info.is_dvi       = sig_info.info.is_dvi;
        mCurrentSignalInfo.info.aspect_ratio = sig_info.info.aspect_ratio;

        mCurrentSignalInfo.port              = sig_info.port;

        tv_source_input_t SourceInput;
        SourceInput = mpVdin->Tvin_PortToSourceInput(sig_info.port);

        if ((SourceInput == SOURCE_MPEG)
            || (SourceInput != SOURCE_MPEG && mCurrentSignalInfo.info.status == TVIN_SIG_STATUS_STABLE)) {
            source_input_param_t source_input_param;
            memset(&source_input_param, 0x0, sizeof(source_input_param_t));

            Cpq_GetAllmAndVrrInfo();

            mCurrentTvinInfo.is_dvi              = mCurrentSignalInfo.info.is_dvi;
            mCurrentTvinInfo.hdr_info            = mCurrentSignalInfo.info.signal_type;

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
    struct tvin_parm_s tempSignalInfo;
    int ret = mpVdin->Tvin_GetVdinParam(&tempSignalInfo);

    if (ret < 0) {
        LOGD("%s Get Signal Info error\n", __FUNCTION__);
    } else {
        SetCurrenSourceInfo(tempSignalInfo);
        LOGD("%s source_input is %d, port is %d, sig_fmt is %d, status is %d, isDVI is %d, signal_type is 0x%x\n", __FUNCTION__,
            mCurrentSourceInputInfo.source_input, mCurrentSignalInfo.port, mCurrentSignalInfo.info.fmt, mCurrentSignalInfo.info.status,
            mCurrentSignalInfo.info.is_dvi, mCurrentSignalInfo.info.signal_type);
    }

    return;
}

void CPQControl::onAllmOrVrrStatusChange(void)
{
    int ret = -1;

    ret = Cpq_GetAllmAndVrrInfo();
    if (ret < 0) {
        LOGD("%s Get vrr or allm status error\n", __FUNCTION__);
    } else {
        SetPqModeToGame();
    }
}

void CPQControl::stopVdin(void)
{
    SetCurrentSource(SOURCE_MPEG);

    struct tvin_parm_s tempSignalInfo;
    tempSignalInfo.info.trans_fmt    = TVIN_TFMT_2D;
    tempSignalInfo.info.fmt          = TVIN_SIG_FMT_NULL;
    tempSignalInfo.info.status       = TVIN_SIG_STATUS_NULL;
    tempSignalInfo.info.cfmt         = TVIN_COLOR_FMT_MAX;
    tempSignalInfo.info.signal_type     = 0;
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

    struct vdin_event_info SignalEventInfo;
    memset(&SignalEventInfo, 0, sizeof(struct vdin_event_info));
    ret = mpVdin->Tvin_GetSignalEventInfo(&SignalEventInfo);

    if (ret < 0) {
        stopVdin();
        LOGD("Get vidn event error!\n");
    } else {
        tv_source_input_type_t source_type   = mpVdin->Tvin_SourceInputToSourceInputType(mCurrentSourceInputInfo.source_input);
        enum tvin_sg_chg_flg vdinEventType = (enum tvin_sg_chg_flg)SignalEventInfo.event_sts;
        switch (vdinEventType) {
        case TVIN_SIG_CHG_SDR2HDR:
        case TVIN_SIG_CHG_HDR2SDR:
        case TVIN_SIG_CHG_DV2NO:
        case TVIN_SIG_CHG_NO2DV: {
            LOGD("%s: hdr info change!\n", __FUNCTION__);
            struct tvin_info_s vdinSignalInfo;
            memset(&vdinSignalInfo, 0, sizeof(struct tvin_info_s));
            ret = mpVdin->Tvin_GetSignalInfo(&vdinSignalInfo);
            if (ret < 0) {
                LOGD("%s: Get vidn event error!\n", __FUNCTION__);
            } else {
                if ((mCurrentSignalInfo.info.status == TVIN_SIG_STATUS_STABLE)
                    && (mCurrentSignalInfo.info.signal_type != vdinSignalInfo.signal_type)) {
                    if (source_type == SOURCE_TYPE_HDMI) {
                        SetCurrentHdrInfo(vdinSignalInfo.signal_type);
                    }
                    mCurrentSignalInfo.info.signal_type = vdinSignalInfo.signal_type;
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
        case TVIN_SIG_CHG_VRR:
            LOGD("%s: allm || vrr info change!\n", __FUNCTION__);
            onAllmOrVrrStatusChange();
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

    SetCurrentSourceInputInfo(mCurrentSourceInputInfo);
}

int CPQControl::LoadPQSettings()
{
    int ret = 0;

    if (!mbCpqCfg_pq_enable) {
        LOGD("All PQ module disabled\n");
        struct pq_ctrl_s pqControlVal;
        memset(&pqControlVal, 0, sizeof(struct pq_ctrl_s));
        struct vpp_pq_ctrl_s amvecmConfigVal;
        memset(&amvecmConfigVal, 0, sizeof(struct vpp_pq_ctrl_s));
        amvecmConfigVal.length = 14;//this is the count of pq_ctrl_s option
        amvecmConfigVal.ptr    = &pqControlVal;
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_PQ_CTRL, &amvecmConfigVal);
        if (ret < 0) {
            LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
        }
    } else {
        LOGD("%s source_input: %d, sig_fmt: 0x%x(%d), trans_fmt: 0x%x\n", __FUNCTION__,
            mCurrentSourceInputInfo.source_input, mCurrentSourceInputInfo.sig_fmt,
            mCurrentSourceInputInfo.sig_fmt, mCurrentSourceInputInfo.trans_fmt);

        SetPqModeToGame();

        if (mbCpqCfg_new_picture_mode_enable) {
            ret |= LoadPQUISettings();
            ret |= LoadPQTableSettings();
            return ret;
        }

        ret |= Cpq_SetXVYCCMode(VPP_XVYCC_MODE_STANDARD, mCurrentSourceInputInfo);

        ret |= Cpq_SetDIModuleParam(mCurrentSourceInputInfo);

        int cur_mode  = GetPQMode();
        //save cur_mode
        SaveLastPQMode(cur_mode);
        //save new mode for pq setting
        SavePQMode(cur_mode);

        //load pq setting
        ret |= Cpq_SetPQMode((vpp_picture_mode_t)cur_mode, mCurrentSourceInputInfo);
    }

    return ret;
}

int CPQControl::LoadPQUISettings()
{
    int ret = 0;

    //picture mode
    vpp_picture_mode_t pqmode = (vpp_picture_mode_t)GetPQMode();
    ret = Set_PictureMode (pqmode);

    return ret;
}

int CPQControl::LoadPQTableSettings()
{
    int ret = 0;

    ret |= Cpq_SetXVYCCMode(VPP_XVYCC_MODE_STANDARD, mCurrentSourceInputInfo);

    ret |= Cpq_SetDIModuleParam(mCurrentSourceInputInfo);

    int LDLevel = GetLocalDimming();
    ret |= SetLocalDimming(LDLevel, 1);

    vpp_mcdi_mode_t mcdimode =(vpp_mcdi_mode_t)GetMcDiMode();
    ret |= Cpq_SetMcDiMode(mcdimode, mCurrentSourceInputInfo);

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
    ret |= Cpq_SetSmoothPlusMode(smoothplus_mode, mCurrentSourceInputInfo);

    //color customize (CM)
    vpp_cms_cm_param_t param;
    memset(&param, 0, sizeof(vpp_cms_cm_param_t));
    mSSMAction->SSMReadColorCustomizeParams(0, sizeof(vpp_cms_cm_param_t), (int *)&param);
    for (int i = VPP_COLOR_9_PURPLE; i < VPP_COLOR_9_MAX; i++) {
        ret |= Cpq_SetColorCustomize((vpp_cms_color_t)i, VPP_CMS_TYPE_SAT, param.data[i].sat);
        ret |= Cpq_SetColorCustomize((vpp_cms_color_t)i, VPP_CMS_TYPE_HUE, param.data[i].hue);
        ret |= Cpq_SetColorCustomize((vpp_cms_color_t)i, VPP_CMS_TYPE_LUMA, param.data[i].luma);
    }

    return ret;
}

//reset picture ui setting of all input source, used by init or factory reset
void CPQControl::resetPQUiSetting(void)
{
    int i = 0, j = 0, k = 0;

    //Picture mode param & Color Temp
    vpp_pictur_mode_para_t picture;
    tcon_rgb_ogo_t ColorTemp;

    //PQ MODE
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_STANDARD, PQ_SRC_DEFAULT, PQ_FMT_DEFAULT);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_HDMI1, PQ_FMT_DOLVI);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_HDMI2, PQ_FMT_DOLVI);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_HDMI3, PQ_FMT_DOLVI);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_HDMI4, PQ_FMT_DOLVI);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_MPEG, PQ_FMT_DOLVI);
    mSSMAction->SetPictureMode(VPP_PICTURE_MODE_DOLVI_BRIGHT, PQ_SRC_DTV, PQ_FMT_DOLVI);

    for (i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
            //picture mode params
            for (k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
                if (mPQdb->PQ_GetPictureModeParams((pq_source_input_t)i, (pq_sig_fmt_t)j, vpp_picture_mode_t(k), &picture) == 0) {
                    if (!mSSMAction->SetPictureModeData(&picture, i, j, k)) {
                        LOGE("%s SetPictureModeData fail\n", __FUNCTION__);
                    }
                }
            }

            //ColorTemp
            for (k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
                if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, (pq_source_input_t)i, (pq_sig_fmt_t)j,  &ColorTemp) == 0) {
                    if (!mSSMAction->SetColorTemperatureData(&ColorTemp, i, j, k)) {
                        LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
                    }
                }
            }
        }
    }

    //wb gamma
    WB_GAMMA_TABLE WbGamma;
    memset(&WbGamma, 0, sizeof(WB_GAMMA_TABLE));
    for (k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
        if (!mSSMAction->SetWhitebalanceGammaData(&WbGamma, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, k)) {
            LOGE("%s SetColorTemperatureData fail\n", __FUNCTION__);
        }
    }

    return;
}

//reset current input source picture ui setting by Reset_Picture
void CPQControl::resetCurSrcPqUiSetting(void)
{
    //ResetPictureModeAll();
    ResetPictureModeBySrc();
    ResetPictureModeDataBySrc();
    ResetColorTemperatureDataBySrc();
    mbResetPicture = true;

    return;
}

void CPQControl::resetPQTableSetting(void)
{
    int i = 0, j = 0, config_val = 0;
    const char *buf = NULL;

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_GAMMALEVEL_DEF, VPP_GAMMA_CURVE_6);
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

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_COLORBASEMODE_DEF, VPP_COLOR_BASE_MODE_ENHANCE);
    mSSMAction->SSMSaveColorBaseMode (VPP_COLOR_BASE_MODE_OPTIMIZE);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_DDRSSC_DEF, 0);
    mSSMAction->SSMSaveDDRSSC(config_val);

    buf = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LVDSSSC_DEF, NULL);
    int lvds_ssc_buf[3] = {0, 0, 0};
    pqTransformStringToInt(buf, lvds_ssc_buf);
    for (i = 0; i < 8; i++) { //8 panel
        mSSMAction->SSMSaveLVDSSSC(i * MAX_LVDS_SSC_PARAM_SIZE, sizeof(int) * 3, lvds_ssc_buf);
    }

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_EDID_VERSION_DEF, 0);
    mSSMAction->SSMEdidRestoreDefault(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_HDCP_SWITCHER_DEF, 0);
    mSSMAction->SSMHdcpSwitcherRestoreDefault(0);

    buf = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_COLOR_RANGE_MODE_DEF, "default");
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

    //color customize
    int offset = 0;
    vpp_cms_cm_param_t param_cm;
    vpp_cms_3dlut_param_t param_3dlut;
    memset(&param_cm, 0, sizeof(vpp_cms_cm_param_t));
    memset(&param_3dlut, 0, sizeof(vpp_cms_3dlut_param_t));
    Cpq_GetColorCustomizeDefValue(&param_cm, &param_3dlut);
    mSSMAction->SSMSaveColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param_cm);
    mSSMAction->SSMSaveColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param_3dlut);

    return;
}

int CPQControl::Set_PictureMode(vpp_picture_mode_t pq_mode)
{
    int ret = 0;
    SetPcGameMode(pq_mode);

    ret = SetPQPictureMode(pq_mode);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::SetPQPictureMode(vpp_picture_mode_t pq_mode)
{
    int ret = 0;
    LOGD("%s: src: %d, timming: %d, pq_mode: %d!\n", __FUNCTION__, CurSource, CurTimming, pq_mode);
    SetFacColorParams(mCurrentSourceInputInfo, pq_mode);

    vpp_pictur_mode_para_t pq_para;
    if (!GetPictureModeData(&pq_para, pq_mode)) {
        LOGE("%s: GetPictureModeData fail!\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s:\n"
        "PictureMode:      %3d.\n"
        "Brightness:       %3d, Contrast:         %3d, Saturation:       %3d, Hue:             %3d\n"
        "Sharpness:        %3d, Backlight:        %3d, Nr:               %3d, DynamicContrast: %3d\n"
        "ColorGamut:       %3d, ColorTemperature: %3d, LocalContrast:    %3d, BlackStretch:    %3d\n"
        "BlueStretch:      %3d, MpegNr:           %3d, ChromaCoring:     %3d, DvMode:          %3d\n"
        "DvDarkDetail:     %3d, SuperResolution:  %3d, GammaMidLuminance %3d.\n",
        __FUNCTION__,
        pq_mode,
        pq_para.Brightness, pq_para.Contrast, pq_para.Saturation, pq_para.Hue,
        pq_para.Sharpness, pq_para.Backlight, pq_para.Nr, pq_para.DynamicContrast,
        pq_para.ColorGamut, pq_para.ColorTemperature, pq_para.LocalContrast, pq_para.BlueStretch,
        pq_para.BlueStretch, pq_para.MpegNr, pq_para.ChromaCoring, pq_para.DolbyMode,
        pq_para.DolbyDarkDetail, pq_para.SuperResolution, pq_para.GammaMidLuminance);

    ret |= Cpq_SetBrightness(pq_para.Brightness, mCurrentSourceInputInfo);
    ret |= Cpq_SetContrast(pq_para.Contrast, mCurrentSourceInputInfo);
    ret |= Cpq_SetSaturation(pq_para.Saturation, mCurrentSourceInputInfo);
    ret |= Cpq_SetHue(pq_para.Hue, mCurrentSourceInputInfo);
    ret |= Cpq_SetSharpness(pq_para.Sharpness, mCurrentSourceInputInfo);
    ret |= Cpq_SetSuperResolution(pq_para.SuperResolution);
    ret |= Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)pq_para.Nr, mCurrentSourceInputInfo);
    ret |= Cpq_SetDnlpMode((Dynamic_contrast_mode_t)pq_para.DynamicContrast, mCurrentSourceInputInfo);
    ret |= Cpq_SetLocalContrastMode((local_contrast_mode_t)pq_para.LocalContrast);
    ret |= Cpq_SetColorGamutMode((vpp_colorgamut_mode_t)pq_para.ColorGamut, mCurrentSourceInputInfo);
    ret |= Cpq_BlackStretch(pq_para.BlackStretch, mCurrentSourceInputInfo);
    ret |= Cpq_BlueStretch(pq_para.BlueStretch, mCurrentSourceInputInfo);
    ret |= Cpq_ChromaCoring(pq_para.ChromaCoring, mCurrentSourceInputInfo);
    ret |= Cpq_SetMpegNr((vpp_pq_level_t)pq_para.MpegNr, mCurrentSourceInputInfo);
    ret |= Cpq_SetWhitebalance(pq_para.ColorTemperature, pq_para.GammaMidLuminance);
    //dv
    ret |= Cpq_SetAmdolbyPQMode(pq_para.DolbyMode);
    ret |= Cpq_SetAmdolbyDarkDetail(pq_para.DolbyDarkDetail);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success! pq_mode: %d\n",__FUNCTION__, pq_mode);
    }

    return ret;
}

void CPQControl::SetPcGameMode(vpp_picture_mode_t pq_mode)
{
    //enable or disable game/pc mode
    if ((mCurrentSourceInputInfo.source_input == SOURCE_HDMI1) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI2) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI3) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI4)) {//HDMI source;

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
        ret |= Cpq_SetSharpness0VariableParam(source_input_param);
    } else {
        LOGD("%s: Sharpness0 module disabled!\n", __FUNCTION__);
    }

    if (mbCpqCfg_sharpness1_enable) {
        ret |= Cpq_SetSharpness1VariableParam(source_input_param);
    } else {
        LOGD("%s: Sharpness1 module disabled!\n", __FUNCTION__);
    }

    if (mbCpqCfg_amvecm_basic_enable || mbCpqCfg_amvecm_basic_withOSD_enable) {
        ret |= Cpq_SetBrightnessBasicParam(source_input_param);
        ret |= Cpq_SetContrastBasicParam(source_input_param);
        ret |= Cpq_SetSaturationBasicParam(source_input_param);
        ret |= Cpq_SetHueBasicParam(source_input_param);
    } else {
        LOGD("%s: brightness contrast saturation hue module disabled!\n", __FUNCTION__);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success! pqMode: %d\n",__FUNCTION__, pqMode);
    }

    return ret;
}

pq_source_input_t CPQControl::CheckPQSource(tv_source_input_t source)
{
    pq_source_input_t src = PQ_SRC_DEFAULT;
    switch (source) {
    case SOURCE_TV:
        src = PQ_SRC_TV;
        break;
    case SOURCE_AV1:
        src = PQ_SRC_AV1;
        break;
    case SOURCE_AV2:
        src = PQ_SRC_AV2;
        break;
    case SOURCE_YPBPR1:
        src = PQ_SRC_YPBPR1;
        break;
    case SOURCE_YPBPR2:
        src = PQ_SRC_YPBPR2;
        break;
    case SOURCE_HDMI1:
        src = PQ_SRC_HDMI1;
        break;
    case SOURCE_HDMI2:
        src = PQ_SRC_HDMI2;
        break;
    case SOURCE_HDMI3:
        src = PQ_SRC_HDMI3;
        break;
    case SOURCE_HDMI4:
        src = PQ_SRC_HDMI4;
        break;
    case SOURCE_VGA:
        src = PQ_SRC_VGA;
        break;
    case SOURCE_MPEG:
        src = PQ_SRC_MPEG;
        break;
    case SOURCE_DTV:
        src = PQ_SRC_DTV;
        break;
    case SOURCE_SVIDEO:
        src = PQ_SRC_SVIDEO;
        break;
    case SOURCE_IPTV:
        src = PQ_SRC_IPTV;
        break;
    case SOURCE_DUMMY:
        src = PQ_SRC_DUMMY;
        break;
    case SOURCE_SPDIF:
        src = PQ_SRC_SPDIF;
        break;
    case SOURCE_ADTV:
        src = PQ_SRC_ADTV;
        break;
    default:
        src = PQ_SRC_DEFAULT;
        break;
    }

    return src;
}

pq_sig_fmt_t CPQControl::CheckPQTimming(enum hdr_type_e hdr_type)
{
    pq_sig_fmt_t timming = PQ_FMT_DEFAULT;
    switch (hdr_type) {
    case HDRTYPE_HDR10:
        timming = PQ_FMT_HDR;
        break;
    case HDRTYPE_HDR10PLUS:
        timming = PQ_FMT_HDRP;
        break;
    case HDRTYPE_DOVI:
        timming = PQ_FMT_DOLVI;
        break;
    case HDRTYPE_HLG:
        timming = PQ_FMT_HLG;
        break;
    case HDRTYPE_SDR:
        timming = PQ_FMT_SDR;
        break;
    default:
        timming = PQ_FMT_DEFAULT;
        break;
    }

    return timming;
}

int CPQControl::SetAmdolbyDarkDetail(int mode, int is_save)
{
    int ret =0;
    LOGD("%s, mode = %d\n", __FUNCTION__, mode);
    ret = Cpq_SetAmdolbyDarkDetail(mode);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveAmdolbyDarkDetail(mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }
    return 0;
}

int CPQControl::GetAmdolbyDarkDetail(void)
{
    int mode = -1;

    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(&para, pq_mode)) {
            mode = para.DolbyDarkDetail;
        }
    }

    LOGD("%s, source: %d, timming: %d, mode = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveAmdolbyDarkDetail(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.DolbyDarkDetail = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret=  0;
            }
        }
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetAmdolbyDarkDetail(int mode)
{
    if (mode < 0) {
        LOGD("%s Skip dv dark detail!\n",__FUNCTION__);
        return 0;
    }

    int ret = -1;

    ret = mDolbyVision->SetAmdolbyPQDarkDetail(mode);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! mode: %d\n",__FUNCTION__, mode);

    return ret;
}

int CPQControl::Cpq_SetAmdolbyPQMode(int mode)
{
    if (mode < 0) {
        LOGD("%s Skip DV Mode\n",__FUNCTION__);
        return 0;
    }

    int ret = -1;

    ret = mDolbyVision->SetAmdolbyPQMode((dolby_pq_mode_t)mode);

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

int CPQControl::Cpq_LoadDisplayModeRegs(struct ve_pq_load_s regs)
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

int CPQControl::DI_LoadRegs(struct am_pq_parm_s di_regs)
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
                LOGD("%s: BlackExtension module disabled!\n", __FUNCTION__);
            }
        }

        if (mbCpqCfg_sharpness0_enable) {
            ret |= Cpq_SetSharpness0VariableParam(source_input_param);
        } else {
            LOGD("%s: Sharpness0 module disabled!\n", __FUNCTION__);
        }

        if (mbCpqCfg_sharpness1_enable) {
            ret |= Cpq_SetSharpness1VariableParam(source_input_param);
        } else {
            LOGD("%s: Sharpness1 module disabled!\n", __FUNCTION__);
        }
    }

    if (mbCpqCfg_amvecm_basic_enable) {
        ret |= Cpq_SetBrightnessBasicParam(source_input_param);
        ret |= Cpq_SetContrastBasicParam(source_input_param);
        ret |= Cpq_SetSaturationBasicParam(source_input_param);
        ret |= Cpq_SetHueBasicParam(source_input_param);
    } else {
        LOGD("%s: brightness contrast saturation hue module disabled\n", __FUNCTION__);
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
    if (!mbCpqCfg_di_enable) {
        LOGD("DI module disabled!\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetDIParams(source_input_param, &regs) < 0) {
        LOGE("%s GetDIParams failed!\n",__FUNCTION__);
        return -1;
    }

    if (regs.length == 0) {
        LOGE("%s: get DI Module Param failed!\n",__FUNCTION__);
        return-1;
    }

    struct am_pq_parm_s di_regs;
    memset(&di_regs, 0x0, sizeof(struct am_pq_parm_s));
    di_regs.table_name |= TABLE_NAME_DI;
    di_regs.table_len = regs.length;
    struct am_reg_s tmp_buf[regs.length];
    for (unsigned int i=0;i<regs.length;i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success!\n",__FUNCTION__);
    return 0;
}

int CPQControl::SetPQMode(int pq_mode, int is_save)
{
    LOGD("%s, source: %d, timming: %d, pq_mode: %d\n", __FUNCTION__, CurSource, CurTimming, pq_mode);
    int ret = -1;

    int cur_mode = GetPQMode();
    if (cur_mode == pq_mode && mbResetPicture != true) {
        LOGD("%s Same PQ mode,no need set again!\n", __FUNCTION__);
        ret = 0;
        return ret;
    }

    if (is_save == 1) {
        SavePQMode(pq_mode);
    }

    if (mbCpqCfg_new_picture_mode_enable) {
        ret = Set_PictureMode((vpp_picture_mode_t)pq_mode);
    } else {
        ret = Cpq_SetPQMode((vpp_picture_mode_t)pq_mode, mCurrentSourceInputInfo);
    }

    if ((ret == 0) && (is_save == 1)) {
        if ((mCurrentSourceInputInfo.source_input >= SOURCE_HDMI1) &&
            (mCurrentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
            vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
            ret = SetDisplayMode(display_mode, 1);
        }
    }

    if (mbResetPicture == true) {
        mbResetPicture = false;
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
    if ( mbCpqCfg_new_picture_mode_enable) {
        if (GetPictureMode((vpp_picture_mode_t *)&mode) != true) {
            LOGE("%s GetPictureMode failed!\n", __FUNCTION__);
            return mode;
        }
    } else {
        mSSMAction->SSMReadPictureMode(mSourceInputForSaveParam, &mode);
    }

    if (mode < VPP_PICTURE_MODE_STANDARD || mode >= VPP_PICTURE_MODE_MAX) {
        mode = VPP_PICTURE_MODE_STANDARD;
    }

    return mode;
}

int CPQControl::SavePQMode(int pq_mode)
{
    LOGD("%s, source:%d, timming: %d, mode:%d\n", __FUNCTION__, CurSource, CurTimming, pq_mode);

    int ret = -1;
    if ( mbCpqCfg_new_picture_mode_enable) {
        if (SetPictureMode((vpp_picture_mode_t)pq_mode) != true) {
            LOGE("%s: SetPictureMode fail\n", __FUNCTION__);
            return -1;
        }

        return 0;
    } else {
        ret = mSSMAction->SSMSavePictureMode(mSourceInputForSaveParam, pq_mode);
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::GetLastPQMode(void)
{
    int mode = VPP_PICTURE_MODE_STANDARD;

    mSSMAction->SSMReadLastPictureMode(mSourceInputForSaveParam, &mode);

    if (mode < VPP_PICTURE_MODE_STANDARD || mode >= VPP_PICTURE_MODE_MAX) {
        mode = VPP_PICTURE_MODE_STANDARD;
    }

    LOGD("%s, source: %d, mode: %d\n", __FUNCTION__, CurSource, mode);
    return mode;

}


int CPQControl::SaveLastPQMode(int pq_mode)
{
    int ret = -1;
    LOGD("%s, source: %d, mode: %d\n", __FUNCTION__, CurSource, pq_mode);

    ret = mSSMAction->SSMSaveLastPictureMode(mSourceInputForSaveParam, pq_mode);

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
    if ((mCurrentSourceInputInfo.source_input == SOURCE_HDMI1) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI2) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI3) ||
        (mCurrentSourceInputInfo.source_input == SOURCE_HDMI4)) {//HDMI source;
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

        if ((pq_mode == VPP_PICTURE_MODE_MONITOR) || (pq_mode == VPP_PICTURE_MODE_GAME)) {
            Cpq_SetNoiseReductionMode(VPP_NOISE_REDUCTION_MODE_OFF, source_input_param);
        } else {
            Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)GetNoiseReductionMode(), source_input_param);
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

//color temperature
int CPQControl::SetColorTemperature(int temp_mode, int is_save)
{
    int ret = -1;
    LOGD("%s: source:%d, mode: %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, temp_mode);

    ret = Cpq_SetWhitebalance(temp_mode, GetGammaValue());

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveColorTemperature((vpp_color_temperature_mode_t)temp_mode);
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
        if (GetPictureModeData(&para, pq_mode)) {
            mode = para.ColorTemperature;
        }
    } else {
        mSSMAction->SSMReadColorTemperature(mSourceInputForSaveParam, &mode);
    }

    if (mode < VPP_COLOR_TEMPERATURE_MODE_STANDARD || mode >= VPP_COLOR_TEMPERATURE_MODE_MAX) {
        mode = VPP_COLOR_TEMPERATURE_MODE_STANDARD;
    }

    LOGD("%s: source: %d, timming: %d mode: %d!\n",__FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveColorTemperature(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.ColorTemperature = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret  = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveColorTemperature(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetColorTemperature(int level)
{
    if (!mbCpqCfg_whitebalance_enable) {
        LOGD("%s ColorTemperature disabled!\n",__FUNCTION__);
        return 0;
    }

    tcon_rgb_ogo_t rgbogo;
    memset(&rgbogo, 0, sizeof(tcon_rgb_ogo_t));
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, level)) {
        LOGD("%s Get ColorTemp from CRI_DATA\n",__FUNCTION__);
    } else if (GetColorTemperatureData(&rgbogo, level)) {
        LOGD("%s Get ColorTemp from SSMDATA\n",__FUNCTION__);
    } else {
        LOGE("%s have no data\n",__FUNCTION__);
        return -1;
    }

    int ret = 0;
    if (GetEyeProtectionMode(mCurrentSourceInputInfo.source_input))//if eye protection mode is enable, b_gain / 2.
        rgbogo.b_gain /= 2;

    ret |= Cpq_SetRGBOGO(&rgbogo);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success! level: %d\n",__FUNCTION__, level);
        LOGD("Enable      = %5d\n"
             "rgain       = %5d, ggain       = %5d, bgain       = %5d\n"
             "roffset     = %5d, goffset     = %5d, boffset     = %5d\n"
             "rpre_offset = %5d, gpre_offset = %5d, bpre_offset = %5d\n",
             rgbogo.en,
             rgbogo.r_gain, rgbogo.g_gain, rgbogo.b_gain,
             rgbogo.r_post_offset, rgbogo.g_post_offset, rgbogo.b_post_offset,
             rgbogo.r_pre_offset, rgbogo.g_pre_offset, rgbogo.b_pre_offset);
    }

    return ret;
}

int CPQControl::Cpq_SetWhitebalance(int level, int gamma)
{
    int ret = 0;
    ret |= Cpq_SetColorTemperature(level);

    ret |= Cpq_LoadGamma((vpp_gamma_curve_t)gamma, (vpp_color_temperature_mode_t)level);

    if (ret < 0) {
        LOGE("%s  fail\n",__FUNCTION__);
    } else {
        LOGD("%s success! level: %d\n",__FUNCTION__, level);
    }

    return ret;
}

int CPQControl::SetColorTempParams(int ColorTemp, tcon_rgb_ogo_t *param)
{
    if (!mbCpqCfg_whitebalance_enable) {
        LOGD("whitebalance module disabled!\n");
        return 0;
    }

    if (GetEyeProtectionMode(mSourceInputForSaveParam) == 1) {
        LOGD("eye protection mode is enable\n");
        param->b_gain /= 2;
    }

    if (Cpq_SetRGBOGO(param) != 0) {
        LOGE("%s Cpq_SetRGBOGO fail\n", __FUNCTION__);
        return -1;
    }

    if (!SetColorTemperatureData(param, ColorTemp)) {
        LOGE("%s SetColorTemperatureData failed\n", __FUNCTION__);
        return -1;
    }

    LOGD("Enable      = %5d\n"
         "rgain       = %5d, ggain       = %5d, bgain       = %5d\n"
         "roffset     = %5d, goffset     = %5d, boffset     = %5d\n"
         "rpre_offset = %5d, gpre_offset = %5d, bpre_offset = %5d\n",
         param->en,
         param->r_gain, param->g_gain, param->b_gain,
         param->r_post_offset, param->g_post_offset, param->b_post_offset,
         param->r_pre_offset, param->g_pre_offset, param->b_pre_offset);

    LOGD("%s success\n", __FUNCTION__);
    return 0;
}

tvpq_rgb_ogo_t CPQControl::GetColorTempParams(int ColorTemp)
{
    tcon_rgb_ogo_t param;
    tvpq_rgb_ogo_t output;
    memset(&param, 0, sizeof(tcon_rgb_ogo_t));
    memset(&output, 0, sizeof(tvpq_rgb_ogo_t));

    if (!GetColorTemperatureData(&param, ColorTemp)) {
        LOGE("%s GetColorTemperatureData failed\n", __FUNCTION__);
        return output;
    }

    memcpy(&output, &param, sizeof(tcon_rgb_ogo_t));

    return output;
}

//Brightness
int CPQControl::SetBrightness(int value, int is_save)
{
    int ret =0;
    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, value);
    ret = Cpq_SetBrightness(value, mCurrentSourceInputInfo);

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveBrightness(value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret ;
}

int CPQControl::GetBrightness(void)
{
    int data = 50;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pq_para_t pq_para;

    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.Brightness;
        }
    } else {
        if (GetPQParams(mCurrentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.brightness;
        }
    }
    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming,  data);
    return data;
}

int CPQControl::SaveBrightness(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.Brightness = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveBrightness(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
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
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        LOGD("%s: brightness disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 && value > 100) {
        LOGE("%s:Brightness out of range\n", __FUNCTION__);
        return -1;
    }

    int params;
    if (mPQdb->PQ_GetBrightnessParams(source_input_param, value, &params) < 0) {
        LOGE("%s:PQ_GetBrightnessParams failed\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoBrightness(params) < 0) {
        LOGE("%s:Cpq_SetVideoBrightness failed\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s success! value: %d\n",__FUNCTION__, value);
    return 0;
}

int CPQControl::Cpq_SetVideoBrightness(int value)
{
    LOGD("%s brightness:%d\n", __FUNCTION__, value);
    struct am_vdj_mode_s params;
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
    int ret = Cpq_SetContrast(value, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.Contrast;
        }
    } else {
        if (GetPQParams(mCurrentSourceInputInfo, pq_mode, &pq_para) == 0) {
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.Contrast = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveContrast(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
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
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        LOGD("%s: contrast disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        LOGE("%s: Contrast out of range\n", __FUNCTION__);
        return -1;
    }

    int params;
    if (mPQdb->PQ_GetContrastParams(source_input_param, value, &params) < 0) {
        LOGE("%s: Cpq_SetVideoContrast failed\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoContrast(params) < 0) {
        LOGE("%s: Cpq_SetVideoContrast failed\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s success! value: %d, params: %d\n",__FUNCTION__, value, params);
    return 0;
}

int CPQControl::Cpq_SetVideoContrast(int value)
{
    LOGD("%s Contrast: %d\n", __FUNCTION__, value);

    struct am_vdj_mode_s params;
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
    int ret = Cpq_SetSaturation(value, mCurrentSourceInputInfo);
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
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.Saturation;
        }
    } else {
        if (GetPQParams(mCurrentSourceInputInfo, pq_mode, &pq_para) == 0) {
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.Saturation = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveSaturation(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
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
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        LOGD("%s: saturation disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        LOGE("%s: Saturation out of range\n", __FUNCTION__);
        return -1;
    }

    int hue_level = 0;
    if (((source_input_param.source_input == SOURCE_TV) ||
        (source_input_param.source_input == SOURCE_AV1) ||
        (source_input_param.source_input == SOURCE_AV2)) &&
        ((source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_M) ||
        (source_input_param.sig_fmt == TVIN_SIG_FMT_CVBS_NTSC_443))) {
        hue_level = 100 - GetHue();
    } else {
        hue_level = 50;
    }

    int hue = 0;
    if (mPQdb->PQ_GetHueParams(source_input_param, hue_level, &hue) < 0) {
        LOGE("%s: PQ_GetHueParams failed\n", __FUNCTION__);
        return -1;
    }

    int saturation = 0;
    if (mPQdb->PQ_GetSaturationParams(source_input_param, value, &saturation) < 0) {
        LOGE("%s: PQ_GetSaturationParams failed\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoSaturationHue(saturation, hue) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! value: %d\n",__FUNCTION__, value);
    return 0;
}

//Hue
int CPQControl::SetHue(int value, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, value);
    int ret = Cpq_SetHue(value, mCurrentSourceInputInfo);
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
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.Hue;
        }
    } else {
        if (GetPQParams(mCurrentSourceInputInfo, pq_mode, &pq_para) == 0) {
            data = pq_para.hue;
        }
    }

    if (data < 0 || data > 100) {
        data = 50;
    }

    LOGD("%s, source: %d, timming: %d value = %d\n", __FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveHue(int value)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.Hue = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveHue(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
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
    if (!mbCpqCfg_amvecm_basic_enable && !mbCpqCfg_amvecm_basic_withOSD_enable) {
        LOGD("%s: hue disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        LOGE("%s: Hue out of range\n", __FUNCTION__);
        return -1;
    }

    int hue_params = 0;
    if (mPQdb->PQ_GetHueParams(source_input_param, (100 - value), &hue_params) < 0) {
        LOGE("%s:PQ_GetHueParams failed\n", __FUNCTION__);
        return -1;
    }

    int saturation_params = 0;
    if (mPQdb->PQ_GetSaturationParams(source_input_param, GetSaturation(), &saturation_params) < 0) {
        LOGE("%s:PQ_GetSaturationParams failed\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetVideoSaturationHue(saturation_params, hue_params) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! value: %d\n",__FUNCTION__, value);
    return 0;
}

int CPQControl::Cpq_SetVideoSaturationHue(int satVal, int hueVal)
{
    signed long temp;
    LOGD("%s: satVal:%d hueVal:%d\n", __FUNCTION__, satVal, hueVal);
    struct am_vdj_mode_s params;
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
    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, value);
    int ret = Cpq_SetSharpness(value, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.Sharpness;
        }
    } else {
        if (GetPQParams(mCurrentSourceInputInfo, pq_mode, &pq_para) == 0) {
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.Sharpness = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveSharpness(mSourceInputForSaveParam, value);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness(int value, source_input_param_t source_input_param)
{
    int ret = 0;
    //sharpness 0
    ret |= Cpq_SetSharpness0Variable(value, source_input_param);
    //sharpness 1
    ret |= Cpq_SetSharpness1Variable(value, source_input_param);

    if (ret < 0) {
        LOGE("%s: Fail !\n", __FUNCTION__);
    } else {
        LOGD("%s: Success ! value: %d\n", __FUNCTION__, value);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0Variable(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        LOGD("%s: sharpness0 disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        LOGE("%s: invalid value\n", __FUNCTION__);
        return -1;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness0Params(source_input_param, value, &regs) < 0) {
        LOGE("%s: PQ_GetSharpness0Params failed\n", __FUNCTION__);
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s: Cpq_LoadRegs failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetSharpness1Variable(int value, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        LOGD("%s: sharpness1 disabled\n", __FUNCTION__);
        return 0;
    }

    if (value < 0 || value > 100) {
        LOGE("%s: invalid value\n", __FUNCTION__);
        return -1;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness1Params(source_input_param, value, &regs) < 0) {
        LOGE("%s: PQ_GetSharpness1Params failed\n", __FUNCTION__);
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s: Cpq_LoadRegs failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

//SuperResolution
int CPQControl::SetSuperResolution(int value, int is_save)
{
    if (is_save) {
        SaveSharpness(value);
    }

    if (Cpq_SetSuperResolution(value) < 0) {
        LOGE("%s: fail\n", __FUNCTION__, value);
        return -1;
    }

    LOGD("%s: success! value: %d, is_save: %d\n", __FUNCTION__, value, is_save);
    return 0;
}

int CPQControl::GetSuperResolution(void)
{
    int value = VPP_SUPER_RESOLUTION_OFF;
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pictur_mode_para_t params;
    if (!GetPictureModeData(&params, pq_mode)) {
        LOGE("%s: fail\n", __FUNCTION__, value);
        return -1;
    }

    value = params.SuperResolution;

    if (value < VPP_SUPER_RESOLUTION_OFF || value >= VPP_SUPER_RESOLUTION_MAX) {
        value = VPP_SUPER_RESOLUTION_OFF;
    }

    LOGD("%s: success value: %d\n", __FUNCTION__, value);
    return value;
}

int CPQControl::SaveSuperResolution(int value)
{
    vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
    vpp_pictur_mode_para_t params;
    if (!GetPictureModeData(&params, pq_mode)) {
        LOGE("%s: GetPictureModeData fail\n",__FUNCTION__);
        return -1;
    }

    params.SuperResolution = value;

    if (!SetPictureModeData(&params, pq_mode)) {
        LOGE("%s: SetPictureModeData fail\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s: success SuperResolution = %d\n", __FUNCTION__, value);
    return 0;
}

int CPQControl::Cpq_SetSuperResolution(int value)
{
    int ret = 0;
    //sharpness 0
    ret |= Cpq_SetSharpness0FixedParam(value, mCurrentSourceInputInfo);
    //sharpness 1
    ret |= Cpq_SetSharpness1FixedParam(value, mCurrentSourceInputInfo);

    if (ret < 0) {
        LOGE("%s: Fail !\n", __FUNCTION__);
    } else {
        LOGD("%s: Success ! value: %d\n", __FUNCTION__, value);
    }

    return ret;
}

int CPQControl::Cpq_SetSharpness0FixedParam(int mode, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        LOGD("%s: sharpness0 disable!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness0FixedParams(mode, source_input_param, &regs) < 0) {
        LOGE("%s: PQ_GetSharpness0FixedParams failed\n", __FUNCTION__);
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetSharpness0VariableParam(source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness0_enable) {
        LOGD("%s: sharpness0 disable!\n", __FUNCTION__);
        return 0;
    }

    if (mPQdb->PQ_SetSharpness0VariableParams(source_input_param) < 0) {
        LOGE("%s failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetSharpness1FixedParam(int mode, source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        LOGD("%s: sharpness1 disable!\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSharpness1FixedParams(mode, source_input_param, &regs) < 0) {
        LOGE("%s: PQ_GetSharpness1FixedParams failed\n", __FUNCTION__);
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetSharpness1VariableParam(source_input_param_t source_input_param)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_sharpness1_enable) {
        LOGD("%s: sharpness1 disable!\n", __FUNCTION__);
        return 0;
    }

    if (mPQdb->PQ_SetSharpness1VariableParams(source_input_param) < 0) {
        LOGE("%s failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

void CPQControl::InitAutoNr(void)
{
    int ret = -1;
    const char *buff = NULL;
    int buf[128] = {0};

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_TH, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str1[128] = {0};
        sprintf(str1, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_th",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14]);
        LOGE("%s str1 = %s\n", __FUNCTION__, str1);
        ret = pqWriteSys(AML_AUTO_NR_PARAMS, str1);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_LP_YGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str2[128] = {0};
        sprintf(str2, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_lp_ygain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        LOGE("%s str2 = %s\n", __FUNCTION__, str2);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str2);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_HP_YGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str3[128] = {0};
        sprintf(str3, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_hp_ygain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        LOGE("%s str3 = %s\n", __FUNCTION__, str3);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str3);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_LP_CGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str4[128] = {0};
        sprintf(str4, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_lp_cgain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        LOGE("%s str4 = %s\n", __FUNCTION__, str4);

        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str4);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_MOTION_HP_CGAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str5[128] = {0};
        sprintf(str5, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", "motion_hp_cgain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
            buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
        LOGE("%s str5 = %s\n", __FUNCTION__, str5);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str5);
    }

    buff = mPQConfigFile->GetString(CFG_SECTION_AUTO_NR, CFG_AUTO_NR_APL_GAIN, NULL);
    pqTransformStringToInt(buff, buf);
    if (buff != NULL) {
        char str6[128] = {0};
        sprintf(str6, "%s %d %d %d %d %d %d %d %d", "apl_gain",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        LOGE("%s str6 = %s\n", __FUNCTION__, str6);
        ret |= pqWriteSys(AML_AUTO_NR_PARAMS, str6);
    }

    if (ret < 0) {
        LOGE("%s failed\n", __FUNCTION__);
    }

    return;
}

//NoiseReductionMode
int CPQControl::SetNoiseReductionMode(int nr_mode, int is_save)
{
    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, nr_mode);
    int ret = Cpq_SetNoiseReductionMode((vpp_noise_reduction_mode_t)nr_mode, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.Nr = nr_mode;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveNoiseReduction(mSourceInputForSaveParam, nr_mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetNoiseReductionMode(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_nr_enable) {
        LOGE("NoiseReduction is disabled\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetNR2Params((vpp_noise_reduction_mode_t)nr_mode, source_input_param, &regs) < 0) {
        LOGE("PQ_GetNR2Params failed\n");
        return -1;
    }

    struct am_pq_parm_s di_regs;
    memset(&di_regs, 0x0, sizeof(struct am_pq_parm_s));
    di_regs.table_name = TABLE_NAME_NR;
    di_regs.table_len = regs.length;
    struct am_reg_s tmp_buf[regs.length];
    for (unsigned int i=0;i<regs.length;i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    }

    LOGD("%s success! nr_mode: %d\n",__FUNCTION__, nr_mode);
    return 0;
}

//Gamma
int CPQControl::SetGammaValue(vpp_gamma_curve_t gamma_curve, int is_save)
{
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, gamma_curve);
    int ret = 0;
    ret |= Cpq_LoadGamma(gamma_curve, (vpp_color_temperature_mode_t)GetColorTemperature());

    if ((ret == 0) && (is_save == 1)) {
        ret = SaveGammaValue((int)gamma_curve);
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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (!GetPictureModeData(&para, pq_mode)) {
            LOGE("%s, GetPictureModeData ERROR\n", __FUNCTION__);
        }
        gammaValue = para.GammaMidLuminance;
    } else {
        if (mSSMAction->SSMReadGammaValue(mCurrentSourceInputInfo.source_input, &gammaValue) < 0) {
            LOGE("%s, SSMReadGammaValue ERROR\n", __FUNCTION__);
        }
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, gammaValue);
    return gammaValue;
}

int CPQControl::SaveGammaValue(int gamma)
{
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        vpp_pictur_mode_para_t para;
        if (!GetPictureModeData(&para, pq_mode)) {
            LOGE("%s, GetPictureModeData fail\n", __FUNCTION__);
            return -1;
        }

        para.GammaMidLuminance = gamma;

        if (!SetPictureModeData(&para, pq_mode)) {
            LOGE("%s, SetPictureModeData fail\n", __FUNCTION__);
            return -1;
        }
    } else {
        if (mSSMAction->SSMSaveGammaValue(mCurrentSourceInputInfo.source_input, gamma) < 0) {
            LOGE("%s, SSMSaveGammaValue fail\n", __FUNCTION__);
            return -1;
        }
    }

    return 0;
}

int CPQControl::Cpq_LoadGamma(vpp_gamma_curve_t gamma_curve, vpp_color_temperature_mode_t level)
{
    if (!mInitialized) {
        return 0;
    }

    if (!mbCpqCfg_gamma_enable) {
        LOGD("Gamma module disabled!\n");
        return 0;
    }

    int ret = 0;
    GAMMA_TABLE Gamma;
    memset(&Gamma, 0, sizeof(GAMMA_TABLE));
    ret |= GetBaseGammaData((int)level, &Gamma);
    ret |= GetWBGammaData((int)level, &Gamma);
    ret |= GetGammaPowerData((int)gamma_curve, &Gamma);

    if (ret < 0) {
        LOGE("%s, fail!\n", __FUNCTION__);
    } else {
        ret |= Cpq_SetGammaTbl_R((unsigned short *) Gamma.R.data);
        ret |= Cpq_SetGammaTbl_G((unsigned short *) Gamma.G.data);
        ret |= Cpq_SetGammaTbl_B((unsigned short *) Gamma.B.data);
    }

    if (ret < 0) {
        LOGE("%s, fail!\n", __FUNCTION__);
    } else {
        LOGD("%s, success! gamma_curve: %d, level: %d\n", __FUNCTION__, gamma_curve, level);
    }

    return ret;
}

int CPQControl::GetBaseGammaData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    ret |= mPQdb->PQ_GetGammaWhiteBalanceSpecialTable((vpp_color_temperature_mode_t)level, "Red",   &pData->R);
    ret |= mPQdb->PQ_GetGammaWhiteBalanceSpecialTable((vpp_color_temperature_mode_t)level, "Green", &pData->G);
    ret |= mPQdb->PQ_GetGammaWhiteBalanceSpecialTable((vpp_color_temperature_mode_t)level, "Blue",  &pData->B);

    if (ret < 0) {
        for (int i = 0; i < mPQdb->Gamma_nodes; i++) {
            pData->R.data[i] = (i * 4);
            if (pData->R.data[i] > 1023)
                pData->R.data[i] = 1023;
            pData->G.data[i] = (i * 4);
            if (pData->G.data[i] > 1023)
                pData->G.data[i] = 1023;
            pData->B.data[i] = (i * 4);
            if (pData->B.data[i] > 1023)
                pData->B.data[i] = 1023;
        }

        LOGE("%s: from pq.db fail, gen a linearity Table\n", __FUNCTION__);
        ret = 0;
    }

    if (ret < 0) {
        LOGE("%s: Fail\n", __FUNCTION__);
    } else {
        LOGD("%s: Success! level: %d, Gamma node = %d\n", __FUNCTION__, level, mPQdb->Gamma_nodes);
    }

    return ret;
}

unsigned int BT1886_GAMMA_256[256] = {
    0x00000,0x000A1,0x0019F,0x002D1,0x0042C,0x005A8,0x00741,0x008F3,0x00ABD,0x00C9C,0x00E8F,0x01094,0x012AB,0x014D2,0x01709,0x0194F,
    0x01BA3,0x01E05,0x02074,0x022F0,0x02578,0x0280C,0x02AAB,0x02D56,0x0300B,0x032CB,0x03596,0x0386A,0x03B49,0x03E30,0x04122,0x0441C,
    0x04720,0x04A2C,0x04D41,0x0505E,0x05384,0x056B2,0x059E8,0x05D26,0x0606C,0x063B9,0x0670E,0x06A6A,0x06DCE,0x07138,0x074AA,0x07823,
    0x07BA3,0x07F29,0x082B7,0x0864B,0x089E5,0x08D86,0x0912E,0x094DB,0x0988F,0x09C49,0x0A009,0x0A3D0,0x0A79C,0x0AB6E,0x0AF46,0x0B324,
    0x0B707,0x0BAF1,0x0BEDF,0x0C2D4,0x0C6CE,0x0CACD,0x0CED2,0x0D2DC,0x0D6EB,0x0DB00,0x0DF1A,0x0E339,0x0E75D,0x0EB86,0x0EFB4,0x0F3E8,
    0x0F820,0x0FC5D,0x1009F,0x104E6,0x10932,0x10D82,0x111D8,0x11632,0x11A90,0x11EF3,0x122EA,0x126BD,0x12A92,0x12E68,0x13241,0x1361C,
    0x139F8,0x13DD7,0x141B7,0x14599,0x1497D,0x14D63,0x1514B,0x15534,0x1591F,0x15D0C,0x160FA,0x164EB,0x168DD,0x16CD1,0x170C6,0x174BD,
    0x178B6,0x17CB0,0x180AC,0x184AA,0x188AA,0x18CAA,0x190AD,0x194B1,0x198B7,0x19CBE,0x1A0C7,0x1A4D1,0x1A8DD,0x1ACEA,0x1B0F9,0x1B50A,
    0x1B91C,0x1BD2F,0x1C144,0x1C55A,0x1C972,0x1CD8B,0x1D1A6,0x1D5C2,0x1D9DF,0x1DDFE,0x1E21E,0x1E640,0x1EA63,0x1EE87,0x1F2AD,0x1F6D4,
    0x1FAFD,0x1FF27,0x20352,0x2077F,0x20BAC,0x20FDC,0x2140C,0x2183E,0x21C71,0x220A5,0x224DB,0x22912,0x22D4A,0x23183,0x235BE,0x239FA,
    0x23E37,0x24276,0x246B5,0x24AF6,0x24F38,0x2537B,0x257C0,0x25C06,0x2604C,0x26495,0x268DE,0x26D28,0x27174,0x275C0,0x27A0E,0x27E5D,
    0x282AE,0x286FF,0x28B51,0x28FA5,0x293FA,0x29850,0x29CA7,0x2A0FF,0x2A558,0x2A9B2,0x2AE0E,0x2B26A,0x2B6C8,0x2BB26,0x2BF86,0x2C3E7,
    0x2C849,0x2CCAC,0x2D110,0x2D575,0x2D9DB,0x2DE42,0x2E2AA,0x2E713,0x2EB7D,0x2EFE9,0x2F455,0x2F8C2,0x2FD31,0x301A0,0x30610,0x30A82,
    0x30EF4,0x31367,0x317DC,0x31C51,0x320C7,0x3253E,0x329B7,0x32E30,0x332AA,0x33725,0x33BA1,0x3401F,0x3449D,0x3491C,0x34D9C,0x3521C,
    0x3569E,0x35B21,0x35FA5,0x36429,0x368AF,0x36D35,0x371BD,0x37645,0x37ACE,0x37F58,0x383E3,0x3886F,0x38CFC,0x3918A,0x39619,0x39AA8,
    0x39F39,0x3A3CA,0x3A85C,0x3ACEF,0x3B183,0x3B618,0x3BAAE,0x3BF44,0x3C3DC,0x3C874,0x3CD0D,0x3D1A7,0x3D642,0x3DADE,0x3DF7A,0x3E418,
};
unsigned int BT1886_GAMMA_257[257] = {
    0x00000,0x000A0,0x0019D,0x002CD,0x00426,0x005A0,0x00737,0x008E7,0x00AAF,0x00C8B,0x00E7B,0x0107E,0x01292,0x014B6,0x016EA,0x0192D,
    0x01B7E,0x01DDC,0x02048,0x022C0,0x02545,0x027D5,0x02A71,0x02D18,0x02FCA,0x03286,0x0354D,0x0381D,0x03AF8,0x03DDC,0x040C9,0x043BF,
    0x046BF,0x049C7,0x04CD8,0x04FF1,0x05312,0x0563C,0x0596E,0x05CA7,0x05FE8,0x06331,0x06681,0x069D9,0x06D38,0x0709E,0x0740B,0x0777F,
    0x07AFA,0x07E7C,0x08205,0x08594,0x08929,0x08CC5,0x09068,0x09410,0x097BF,0x09B74,0x09F2F,0x0A2F1,0x0A6B8,0x0AA84,0x0AE57,0x0B230,
    0x0B60E,0x0B9F2,0x0BDDB,0x0C1CA,0x0C5BF,0x0C9B9,0x0CDB8,0x0D1BC,0x0D5C6,0x0D9D5,0x0DDEA,0x0E203,0x0E622,0x0EA45,0x0EE6E,0x0F29B,
    0x0F6CE,0x0FB05,0x0FF42,0x10383,0x107C9,0x10C13,0x11062,0x114B6,0x1190F,0x11D6C,0x12192,0x12561,0x12931,0x12D03,0x130D7,0x134AE,
    0x13886,0x13C5F,0x1403B,0x14419,0x147F8,0x14BD9,0x14FBC,0x153A1,0x15787,0x15B70,0x15F59,0x16345,0x16733,0x16B22,0x16F12,0x17305,
    0x176F9,0x17AEF,0x17EE6,0x182DF,0x186DA,0x18AD6,0x18ED4,0x192D3,0x196D4,0x19AD6,0x19EDA,0x1A2E0,0x1A6E7,0x1AAF0,0x1AEFA,0x1B305,
    0x1B712,0x1BB21,0x1BF31,0x1C342,0x1C755,0x1CB6A,0x1CF7F,0x1D397,0x1D7AF,0x1DBC9,0x1DFE5,0x1E401,0x1E820,0x1EC3F,0x1F060,0x1F482,
    0x1F8A6,0x1FCCB,0x200F1,0x20519,0x20942,0x20D6C,0x21197,0x215C4,0x219F2,0x21E22,0x22252,0x22684,0x22AB8,0x22EEC,0x23322,0x23759,
    0x23B91,0x23FCA,0x24405,0x24841,0x24C7E,0x250BC,0x254FB,0x2593C,0x25D7E,0x261C1,0x26605,0x26A4A,0x26E91,0x272D9,0x27721,0x27B6B,
    0x27FB6,0x28403,0x28850,0x28C9F,0x290EE,0x2953F,0x29991,0x29DE4,0x2A238,0x2A68D,0x2AAE3,0x2AF3A,0x2B393,0x2B7EC,0x2BC47,0x2C0A2,
    0x2C4FF,0x2C95D,0x2CDBC,0x2D21B,0x2D67C,0x2DADE,0x2DF41,0x2E3A5,0x2E80A,0x2EC70,0x2F0D7,0x2F53F,0x2F9A9,0x2FE13,0x3027E,0x306EA,
    0x30B57,0x30FC5,0x31434,0x318A4,0x31D15,0x32187,0x325FA,0x32A6E,0x32EE3,0x33359,0x337D0,0x33C48,0x340C0,0x3453A,0x349B5,0x34E30,
    0x352AD,0x3572A,0x35BA8,0x36028,0x364A8,0x36929,0x36DAB,0x3722E,0x376B2,0x37B37,0x37FBC,0x38443,0x388CA,0x38D53,0x391DC,0x39666,
    0x39AF1,0x39F7D,0x3A40A,0x3A898,0x3AD26,0x3B1B5,0x3B646,0x3BAD7,0x3BF69,0x3C3FC,0x3C88F,0x3CD24,0x3D1B9,0x3D650,0x3DAE7,0x3DF7F,
    0x3E418,
};
int CPQControl::GetGammaPowerData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    int ret = 0;
    if (level == VPP_GAMMA_CURVE_BT1886) {
        if (mPQdb->Gamma_nodes == 256) {
            ret |= DBGammaBlend(&pData->R, BT1886_GAMMA_256);
            ret |= DBGammaBlend(&pData->G, BT1886_GAMMA_256);
            ret |= DBGammaBlend(&pData->B, BT1886_GAMMA_256);
        } else {
            ret |= DBGammaBlend(&pData->R, BT1886_GAMMA_257);
            ret |= DBGammaBlend(&pData->G, BT1886_GAMMA_257);
            ret |= DBGammaBlend(&pData->B, BT1886_GAMMA_257);
        }
    } else {
        double GammaPower = GetGammaPower((vpp_gamma_curve_t)level);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->R.data, 2.2, GammaPower);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->G.data, 2.2, GammaPower);
        ret |= GammaOperation::GetInstance()->GammaOperation_BaseGammaConvert(pData->B.data, 2.2, GammaPower);
    }

    if (ret < 0) {
        LOGE("%s, fail\n", __FUNCTION__);
    } else {
        LOGD("%s: Success! GammaPower: %d\n", __FUNCTION__, level);
    }

    return ret;
}

int CPQControl::GetWBGammaData(int level, GAMMA_TABLE *pData)
{
    if (pData == NULL) {
        LOGE("%s, pData is NULL\n", __FUNCTION__);
        return -1;
    }

    #if 0
    float x[MAX_WB_GAMMA_POINT] = {0, 13, 26, 39, 51, 64, 77, 90, 103, 116, 129, 141, 154, 167, 180, 193, 206, 218, 231, 244, 256};
    #else
    float x[MAX_WB_GAMMA_POINT] = {0, 13, 26, 38, 51, 64, 76, 90, 102, 115, 128, 141, 154, 166, 179, 192, 205, 218, 230, 243, 255};
    #endif

    float y_r[MAX_WB_GAMMA_POINT] = {0};
    float y_g[MAX_WB_GAMMA_POINT] = {0};
    float y_b[MAX_WB_GAMMA_POINT] = {0};

    WB_GAMMA_TABLE Param;
    if (FactoryGetWhitebalanceGammaData(&Param, level)) {
        LOGD("%s, get Data fro CRI DATA, gamma Points: %d\n", __FUNCTION__, MAX_WB_GAMMA_POINT);
    } else if (GetWhitebalanceGammaData(&Param, level)) {
        LOGD("%s, get Data fro SSM DATA, gamma Points: %d\n", __FUNCTION__, MAX_WB_GAMMA_POINT);
    } else {
        LOGE("%s, Have no %d point Whitebalance Gamma\n", __FUNCTION__, MAX_WB_GAMMA_POINT);
        return 0;
    }

    for (int i = 0; i < MAX_WB_GAMMA_POINT; i++) {
        int index = x[i];
        y_r[i] = (float)(pData->R.data[index] + Param.R_OFFSET[i]);
        if (y_r[i] < 0.0) y_r[i] = 0.0;
        if (y_r[i] > 1023.0) y_r[i] = 1023.0;

        y_g[i] = (float)(pData->G.data[index] + Param.G_OFFSET[i]);
        if (y_g[i] < 0.0) y_g[i] = 0.0;
        if (y_g[i] > 1023.0) y_g[i] = 1023.0;

        y_b[i] = (float)(pData->B.data[index] + Param.B_OFFSET[i]);
        if (y_b[i] < 0.0) y_b[i] = 0.0;
        if (y_b[i] > 1023.0) y_b[i] = 1023.0;
    }

    interpolation_info_t output;
    output.x = x;

    output.y = y_r;
    if (CubeInterpolationProcess(output, pData->R.data, MAX_WB_GAMMA_POINT) != 0) {
        LOGE("%s CubeInterpolationProcess R fail\n",__FUNCTION__);
        return -1;
    }

    output.y = y_g;
    if (CubeInterpolationProcess(output, pData->G.data, MAX_WB_GAMMA_POINT) != 0) {
        LOGE("%s CubeInterpolationProcess G fail\n",__FUNCTION__);
        return -1;
    }

    output.y = y_b;
    if (CubeInterpolationProcess(output, pData->B.data, MAX_WB_GAMMA_POINT) != 0) {
        LOGE("%s CubeInterpolationProcess B fail\n",__FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::CubeInterpolationProcess(interpolation_info_t output, unsigned short *gamma, int num_points)
{
    interpolation_info_t *output_ptr = NULL;
    int i;                 /* loop index, as usual */
    int ret = -1;

    /* result of the interpolation */
    float result;
    output_ptr = GammaOperation::GetInstance()->nat_cubic_spline(num_points, &output);

    /* Now use our spline on each val we made up */
    for (i = 0; i < mPQdb->Gamma_nodes; i++) {
        /* Make sure to test the return value before we use the result */
        if ((ret = GammaOperation::GetInstance()->evaluate(output_ptr, (float)i, &result)) < 0) {
            /* Should fail on none of the inputs */
           //LOGE("%s evaluate failed: %d\n", __FUNCTION__, ret);
           return ret;
        }
        /* print the input x value and the interpolated y */
        //LOGE("%s %.2f,%.2f\n", __FUNCTION__,  (float)i, result);
        gamma[i] = (unsigned short)result;
        if (i > 0) {
            if (gamma[i - 1] > gamma[i]) {
                gamma[i] = gamma[i - 1];
            }
        }
        if (gamma[i] >= 1020) {
            gamma[i] = 1020;
        }
        //LOGE("%s gamma.data[%d] = %hd\n", __FUNCTION__, i, gamma[i]);
    }

    return ret;
}

int CPQControl::DBGammaBlend(tcon_gamma_table_t *wb_gamma, unsigned int *index_gamma)
{
    unsigned int i, final_value;
    unsigned int blend_alp, blend_bet;
    tcon_gamma_table_t target_gamma;
    unsigned int Node = (unsigned int)mPQdb->Gamma_nodes;
    for (i = 1; i < Node - 1; i++) {
        blend_alp = index_gamma[i] / 1000;
        blend_bet = index_gamma[i] % 1000;
        if (blend_alp > (Node - 1)) {
            LOGD("%s, blend_gamma->data[i] = %d\n", __FUNCTION__, i, index_gamma[i]);
            LOGD("%s, blend_alp = %d\n", __FUNCTION__, blend_alp);
            LOGD("%s, blend_bet = %d\n", __FUNCTION__, blend_bet);
            continue;
        }
        final_value = wb_gamma->data[blend_alp] + (wb_gamma->data[blend_alp + 1] - wb_gamma->data[blend_alp]) * blend_bet / 1000;
        target_gamma.data[i] = (unsigned short)final_value;
    }

    target_gamma.data[0]   = wb_gamma->data[0];
    target_gamma.data[Node - 1] = wb_gamma->data[Node - 1];

    memcpy(wb_gamma, &target_gamma, sizeof(tcon_gamma_table_t));

    return 0;
}

double CPQControl::GetGammaPower(vpp_gamma_curve_t mode)
{
    double gamma_power = 2.2;
    switch (mode) {
        case VPP_GAMMA_CURVE_DEFAULT:
            gamma_power = 2.2;
        break;
        case VPP_GAMMA_CURVE_1:
            gamma_power = 1.7;
        break;
        case VPP_GAMMA_CURVE_2:
            gamma_power = 1.8;
        break;
        case VPP_GAMMA_CURVE_3:
            gamma_power = 1.9;
        break;
        case VPP_GAMMA_CURVE_4:
            gamma_power = 2.0;
        break;
        case VPP_GAMMA_CURVE_5:
            gamma_power = 2.1;
        break;
        case VPP_GAMMA_CURVE_6:
            gamma_power = 2.2;
        break;
        case VPP_GAMMA_CURVE_7:
            gamma_power = 2.3;
        break;
        case VPP_GAMMA_CURVE_8:
            gamma_power = 2.4;
        break;
        case VPP_GAMMA_CURVE_9:
            gamma_power = 2.5;
        break;
        case VPP_GAMMA_CURVE_10:
            gamma_power = 2.6;
        break;
        case VPP_GAMMA_CURVE_11:
            gamma_power = 2.7;
        break;
        default:
            gamma_power = 2.2;
        break;
    }

    return gamma_power;
}

int CPQControl::SetWhitebalanceGamma(int channel, int point, int offset)
{
    if (channel >= MAX_CH) {
        LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return -1;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return -1;
    }

    int colortemp = GetColorTemperature();
    WB_GAMMA_TABLE pData;
    if (FactoryGetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, CRI DATA have data, skip this API\n", __FUNCTION__);
        return 0;
    }

    if (!GetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, GetWhitebalanceGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        pData.R_OFFSET[point] = offset;
    else if (channel == GREEN_CH)
        pData.G_OFFSET[point] = offset;
    else if (channel == BLUE_CH)
        pData.B_OFFSET[point] = offset;
    else
        return -1;

    if (!SetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, SetWhitebalanceGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadGamma((vpp_gamma_curve_t)GetGammaValue(), (vpp_color_temperature_mode_t)colortemp) < 0) {
        LOGE("%s, Cpq_LoadGamma fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::GetWhitebalanceGamma(int channel, int point)
{
    int offset = 0;
    if (channel >= MAX_CH) {
        LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return offset;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return offset;
    }

    int colortemp = GetColorTemperature();
    WB_GAMMA_TABLE pData;
    if (FactoryGetWhitebalanceGammaData(&pData, colortemp)) {
        LOGD("%s, GET data FROM CRI DATA\n", __FUNCTION__);
    } else if (GetWhitebalanceGammaData(&pData, colortemp)) {
        LOGD("%s, GET data FROM SSM DATA\n", __FUNCTION__);
    } else {
        LOGE("%s, Have no data\n", __FUNCTION__);
        return 0;
    }

    if (channel == RED_CH)
        offset = pData.R_OFFSET[point];
    else if (channel == GREEN_CH)
        offset =  pData.G_OFFSET[point];
    else if (channel == BLUE_CH)
        offset =  pData.B_OFFSET[point];
    else
        offset =  0;

    return offset;
}

int CPQControl::FactorySetWhitebalanceGamma(int colortemp, int channel, int point, int offset)
{
    if (channel >= MAX_CH) {
        LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return -1;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return -1;
    }

    CheckCriDataWhitebalanceGammaData();

    WB_GAMMA_TABLE pData;
    if (!FactoryGetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, GetWhitebalanceGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (channel == RED_CH)
        pData.R_OFFSET[point] = offset;
    else if (channel == GREEN_CH)
        pData.G_OFFSET[point] = offset;
    else if (channel == BLUE_CH)
        pData.B_OFFSET[point] = offset;
    else
        return -1;

    if (!FactorySetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, SetWhitebalanceGammaData fail\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_LoadGamma((vpp_gamma_curve_t)GetGammaValue(), (vpp_color_temperature_mode_t)colortemp) < 0) {
        LOGE("%s, Cpq_LoadGamma fail\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetWhitebalanceGamma(int colortemp, int channel, int point)
{
    int offset = 0;
    if (channel >= MAX_CH) {
        LOGE("%s, channel = %d, out of range\n", __FUNCTION__, channel);
        return offset;
    }

    if (point >= MAX_WB_GAMMA_POINT) {
        LOGE("%s, point = %d, out of range\n", __FUNCTION__, point);
        return offset;
    }

    CheckCriDataWhitebalanceGammaData();

    WB_GAMMA_TABLE pData;
    if (!FactoryGetWhitebalanceGammaData(&pData, colortemp)) {
        LOGE("%s, GetWhitebalanceGammaData fail\n", __FUNCTION__);
        return offset;
    }

    if (channel == RED_CH)
        offset = pData.R_OFFSET[point];
    else if (channel == GREEN_CH)
        offset =  pData.G_OFFSET[point];
    else if (channel == BLUE_CH)
        offset =  pData.B_OFFSET[point];
    else
        offset =  0;

    return offset;
}

int CPQControl::Cpq_SetGammaTbl_R(unsigned short red[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Redtbl;
    int ret = -1;
    unsigned int i = 0;

    for (i = 0; i < sizeof(Redtbl)/sizeof(__u16); i++) {
        Redtbl.data[i] = red[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_R, &Redtbl);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }
    return ret;
}

int CPQControl::Cpq_SetGammaTbl_G(unsigned short green[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Greentbl;
    int ret = -1;
    unsigned int i = 0;

    for (i = 0; i < sizeof(Greentbl)/sizeof(__u16); i++) {
        Greentbl.data[i] = green[i];
    }

    ret = VPPDeviceIOCtl(AMVECM_IOC_GAMMA_TABLE_G, &Greentbl);
    if (ret < 0) {
        LOGE("%s error(%s)!\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::Cpq_SetGammaTbl_B(unsigned short blue[GAMMA_NUMBER])
{
    struct tcon_gamma_table_s Bluetbl;
    int ret = -1;
    unsigned int i = 0;

    for (i = 0; i < sizeof(Bluetbl)/sizeof(__u16); i++) {
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

int CPQControl::Memc_enable(int enable)
{
    int ret = -1;
    ret = MEMCDeviceIOCtl(FRC_IOC_SET_MEMC_ON_OFF, &enable);

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

    ret = Cpq_SetMemcMode(memc_mode, mCurrentSourceInputInfo);

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

int CPQControl::Cpq_SetMemcMode(int memc_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_memc_enable) {
        LOGD("Memc module disabled!!!\n");
        return 0;
    }

    int ret = 0;
    int offset = source_input_param.source_input * 4 + memc_mode;
    int DeJudder_level = 0;
    if (mSSMAction->SSMReadMemcDeJudderLevel(offset, &DeJudder_level) < 0) {
        LOGE("%s, SSMReadMemcDeJudderLevel ERROR!!!\n", __FUNCTION__);
    }
    ret |= Cpq_SetMemcDeJudderLevel(DeJudder_level, mCurrentSourceInputInfo);

    int DeBlur_Level = 0;
    if (mSSMAction->SSMReadMemcDeblurLevel(offset, &DeBlur_Level) < 0) {
        LOGE("%s, SSMReadMemcDeblurLevel ERROR!!!\n", __FUNCTION__);
    }
    ret |= Cpq_SetMemcDeBlurLevel(DeBlur_Level, mCurrentSourceInputInfo);

    if (memc_mode == VPP_MEMC_MODE_OFF)
        Memc_enable(0);
    else
        Memc_enable(1);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! memc_mode: %d\n",__FUNCTION__, memc_mode);

    return ret;
}

int CPQControl::SetMemcDeBlurLevel(int level, int is_save)
{
    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    int ret = -1;

    ret = Cpq_SetMemcDeBlurLevel(level, mCurrentSourceInputInfo);
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
    int offset = mSourceInputForSaveParam * 4 + mode;
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
    int offset = mSourceInputForSaveParam * 4 + mode;
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
    ret = Cpq_SetMemcDeJudderLevel(level, mCurrentSourceInputInfo);
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
    int offset = mSourceInputForSaveParam * 4 + mode;
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
    int offset = mSourceInputForSaveParam * 4 + mode;
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
    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, display_mode);
    int ret = -1;
    if (mbCpqCfg_display_overscan_enable) {
        if ((mCurrentSourceInputInfo.source_input == SOURCE_DTV) || (mCurrentSourceInputInfo.source_input == SOURCE_TV)) {
            ret = Cpq_SetDisplayModeAllTiming(mCurrentSourceInputInfo.source_input, display_mode);
        } else {
            ret = Cpq_SetDisplayModeAllTiming(mCurrentSourceInputInfo.source_input, display_mode);
            ret = Cpq_SetDisplayModeOneTiming(mCurrentSourceInputInfo.source_input, display_mode);
        }

        if ((ret == 0) && (is_save == 1))
            ret = SaveDisplayMode(display_mode);
    } else {
        LOGD("%s:Display overscan disabled!\n", __FUNCTION__);
        ret= 0;
    }

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::GetDisplayMode()
{
    int mode = VPP_DISPLAY_MODE_169;
    mSSMAction->SSMReadDisplayMode(mCurrentSourceInputInfo.source_input, &mode);
    if (mode < VPP_DISPLAY_MODE_169 || mode >= VPP_DISPLAY_MODE_MAX) {
        mode = VPP_DISPLAY_MODE_169;
    }

    LOGD("%s, source: %d, value = %d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, mode);
    return mode;
}

int CPQControl::SaveDisplayMode(vpp_display_mode_t display_mode)
{
    int ret = mSSMAction->SSMSaveDisplayMode(mCurrentSourceInputInfo.source_input, (int)display_mode);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetDisplayModeOneTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode)
{
    int ret = -1;
    tvin_cutwin_t cutwin;

    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(mCurrentSourceInputInfo, display_mode, &cutwin);
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
    struct ve_pq_load_s ve_pq_load_reg;
    memset(&ve_pq_load_reg, 0, sizeof(struct ve_pq_load_s));

    ve_pq_load_reg.param_id = TABLE_NAME_OVERSCAN;
    ve_pq_load_reg.length = SIG_TIMING_TYPE_MAX;

    ve_pq_table_t ve_pq_table[SIG_TIMING_TYPE_MAX];
    tvin_cutwin_t cutwin[SIG_TIMING_TYPE_MAX];
    memset(ve_pq_table, 0, sizeof(ve_pq_table));
    memset(cutwin, 0, sizeof(cutwin));

    enum tvin_sig_fmt_e sig_fmt[SIG_TIMING_TYPE_MAX];
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
    source_input_param.trans_fmt = mCurrentSourceInputInfo.trans_fmt;
    ScreenModeValue = Cpq_GetScreenModeValue(display_mode);
    if (source_input == SOURCE_DTV) {//DTV
        for (i=0;i<SIG_TIMING_TYPE_NTSC_M;i++) {
            ve_pq_table[i].src_timing = (0x1<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];
            if (mbCpqCfg_seperate_db_enable) {
                ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            }

            if (ret == 0) {
                LOGD("signal_fmt:0x%x, screen mode:%d hs:%d he:%d vs:%d ve:%d!\n", sig_fmt[i], ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                LOGD("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = &ve_pq_table;
    } else if (source_input == SOURCE_TV) {//ATV
        for (i=SIG_TIMING_TYPE_NTSC_M;i<SIG_TIMING_TYPE_MAX;i++) {
            ve_pq_table[i].src_timing = (0x1<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (flag[i]);
            source_input_param.sig_fmt = sig_fmt[i];
            if (mbCpqCfg_seperate_db_enable) {
                ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, display_mode, cutwin+i);
            }

            if (ret == 0) {
                LOGD("signal_fmt:0x%x, screen mode:%d hs:%d he:%d vs:%d ve:%d!\n", sig_fmt[i], ScreenModeValue, cutwin[i].he, cutwin[i].hs, cutwin[i].ve, cutwin[i].vs);
                ve_pq_table[i].value1 = ((cutwin[i].he & 0xffff)<<16) | (cutwin[i].hs & 0xffff);
                ve_pq_table[i].value2 = ((cutwin[i].ve & 0xffff)<<16) | (cutwin[i].vs & 0xffff);
            } else {
                LOGD("PQ_GetOverscanParams failed!\n");
            }
        }
        ve_pq_load_reg.param_ptr = &ve_pq_table;
    } else {//HDMI && MPEG
        ve_pq_table[0].src_timing = (0x0<<31) | ((ScreenModeValue & 0x7f) << 24) | ((source_input & 0x7f) << 16 ) | (0x0);
        ve_pq_table[0].value1 = 0;
        ve_pq_table[0].value2 = 0;
        ve_pq_load_reg.param_ptr = &ve_pq_table;

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
int CPQControl::Cpq_SetHdrInfo(const struct lcd_optical_info_s *plcd_hdrinfo)
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
    struct lcd_optical_info_s lcd_hdrinfo;

    if (mbCpqCfg_lcd_hdrinfo_enable) {
        if (mPQdb->PQ_GetLCDHDRInfoParams(mCurrentSourceInputInfo, &lcd_hdrinfo) == 0) {
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

        if (GetPictureModeData(&para, pq_mode)) {
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.Backlight = value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
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
            LOGD("No auto backlight module!\n");
        }
    }

    return ret;
}

int CPQControl::GetHistParam(struct ve_hist_s *hist)
{
    memset(hist, 0, sizeof(struct ve_hist_s));
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
    struct ve_hist_s hist;
    memset(&hist, 0, sizeof(struct ve_hist_s));
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
        LOGD("%s: local contrast module disabled!\n",__FUNCTION__);
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
        if (GetPictureModeData(&para, pq_mode)) {
            mode = para.LocalContrast;
        }
    } else {
        mSSMAction->SSMReadLocalContrastMode(mCurrentSourceInputInfo.source_input, &mode);
    }

    if (mode < LOCAL_CONTRAST_MODE_OFF || mode > LOCAL_CONTRAST_MODE_MAX) {
        mode = LOCAL_CONTRAST_MODE_MID;
    }

    LOGD("%s, source: %d, timming: %d, value = %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    return mode;
}

int CPQControl::SaveLocalContrastMode(local_contrast_mode_t mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.LocalContrast = (int)mode;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveLocalContrastMode(mCurrentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetLocalContrastMode(local_contrast_mode_t mode)
{
    if (!mbDatabaseMatchChipStatus) {
        LOGD("%s: DB don't match chip\n", __FUNCTION__);
        return 0;
    }

    if (!mbCpqCfg_local_contrast_enable) {
        LOGD("%s LocalContrast Disabled\n",__FUNCTION__);
        return 0;
    }

    struct ve_lc_curve_parm_s lc_param;
    memset(&lc_param, 0x0, sizeof(struct ve_lc_curve_parm_s));
    if (mPQdb->PQ_GetLocalContrastNodeParams(mCurrentSourceInputInfo, mode, &lc_param) < 0) {
        LOGE("%s: PQ_GetLocalContrastNodeParams failed\n", __FUNCTION__ );
        return -1;
    }
    if (VPPDeviceIOCtl(AMVECM_IOC_S_LC_CURVE, &lc_param) < 0) {
        LOGE("%s VPPDeviceIOCtl failed\n",__FUNCTION__);
        return -1;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetLocalContrastRegParams(mCurrentSourceInputInfo, mode, &regs) < 0) {
        LOGE("%s: PQ_GetLocalContrastRegParams failed\n", __FUNCTION__ );
        return -1;
    }
    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s Cpq_LoadRegs failed\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! mode: %d\n",__FUNCTION__, mode);
    return 0;
}

//load aad pq
int CPQControl::Cpq_SetAAD(const struct db_aad_param_s *pAAD)
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
        if (mPQdb->PQ_GetAADParams(mCurrentSourceInputInfo, &newaad) == 0) {
            struct db_aad_param_s db_newaad;
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
        LOGD("AAD module disabled\n");
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
int CPQControl::Cpq_SetCABC(const struct db_cabc_param_s *pCABC)
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
          if (mPQdb->PQ_GetCABCParams(mCurrentSourceInputInfo, &newcabc) == 0) {
              struct db_cabc_param_s db_newcabc;
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
        LOGD("CABC module disabled\n");
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
    LOGD("%s src = %d, timming = %d, mode = %d!\n",__FUNCTION__, CurSource, CurTimming, mode);

    int ret = -1;
    ret = Cpq_SetDnlpMode(mode, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            level = para.DynamicContrast;
        }
    } else {
        mSSMAction->SSMReadDnlpMode(mCurrentSourceInputInfo.source_input, &level);
    }

    if (level < LOCAL_CONTRAST_MODE_OFF || level > LOCAL_CONTRAST_MODE_MAX) {
        level = LOCAL_CONTRAST_MODE_MID;
    }

    LOGD("%s, source: %d, timming: %d, level = %d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}

int CPQControl::SaveDnlpMode(Dynamic_contrast_mode_t mode)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.DynamicContrast = (int)mode;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveDnlpMode(mCurrentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDnlpMode(Dynamic_contrast_mode_t level, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_dnlp_enable) {
        LOGD("%s DnlpMode Disabled\n", __FUNCTION__);
        return 0;
    }

    struct ve_dnlp_curve_param_s newdnlp;
    memset(&newdnlp, 0, sizeof(ve_dnlp_curve_param_s));
    if (mPQdb->PQ_GetDNLPParams(mCurrentSourceInputInfo, level, &newdnlp) < 0) {
        LOGE("mPQdb->PQ_GetDNLPParams failed!\n");
        return -1;
    }

    if (VPPDeviceIOCtl(AMVECM_IOC_VE_NEW_DNLP, &newdnlp) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! level: %d\n",__FUNCTION__, level);
    return 0;
}

int CPQControl::Cpq_SetDNLPStatus(enum dnlp_state_e status)
{
    int ret = VPPDeviceIOCtl(AMVECM_IOC_S_DNLP_STATE, &status);
    if (ret < 0) {
        LOGE("%s error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CPQControl::SetColorBaseMode(vpp_color_basemode_t basemode, int isSave)
{
    LOGD("%s, source: %d, value:%d\n", __FUNCTION__, mCurrentSourceInputInfo.source_input, basemode);
    int ret = Cpq_SetColorBaseMode(basemode, mCurrentSourceInputInfo);

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
    int ret = -1;

    if (basemode == VPP_COLOR_BASE_MODE_DEMO) {
        ret = 0;
    } else {
        ret = mSSMAction->SSMSaveColorBaseMode(basemode);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetColorBaseMode(vpp_color_basemode_t basemode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_cm2_enable) {
        LOGD("CM module disabled\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetCM2Params((vpp_color_management2_t)basemode, source_input_param, &regs) < 0) {
        LOGE("PQ_GetCM2Params failed\n");
        return -1;
    }

    if (Cpq_LoadRegs(regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! basemode: %d\n",__FUNCTION__, basemode);
    return 0;
}

int CPQControl::SetColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value, int isSave)
{
    int ret = -1;

    LOGD("%s color:%d, type:%d, value:%d, isSave:%d\n", __FUNCTION__, color, type, value, isSave);

    if (type > VPP_CMS_TYPE_LUMA) {
        LOGE("%s type is error, not CM module request\n",__FUNCTION__);
        return ret;
    }

    ret = Cpq_SetColorCustomize(color, type, value);

    if ((ret ==0) && (isSave == 1)) {
        ret = SaveColorCustomize(color, type, value);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return ret;
}

vpp_single_color_param_cm_t CPQControl::GetColorCustomize(vpp_cms_color_t color)
{
    int offset = 0;
    vpp_cms_cm_param_t param;
    vpp_single_color_param_cm_t single_param;
    memset(&param, 0, sizeof(vpp_cms_cm_param_t));
    memset(&single_param, 0, sizeof(vpp_single_color_param_cm_t));

    LOGD("%s color:%d\n", __FUNCTION__, color);

    if (mSSMAction->SSMReadColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param) < 0) {
        LOGE("%s SSMReadColorCustomizeParams failed!\n", __FUNCTION__);
    }

    for (int i = VPP_COLOR_9_PURPLE; i < VPP_COLOR_9_MAX; i++) {
        LOGD("%s param.data[%d].sat/hue/luma: %d %d %d\n", __FUNCTION__,
            i, param.data[i].sat, param.data[i].hue, param.data[i].luma);
    }

    single_param.sat = param.data[color].sat;
    single_param.hue = param.data[color].hue;
    single_param.luma = param.data[color].luma;

    return single_param;
}

int CPQControl::SaveColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value)
{
    int offset = 0;
    vpp_cms_cm_param_t param;
    memset(&param, 0, sizeof(vpp_cms_cm_param_t));

    if (mSSMAction->SSMReadColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param) < 0) {
        LOGE("%s SSMReadColorCustomizeParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (type == VPP_CMS_TYPE_SAT) {
        param.data[color].sat = value;
    } else if (type == VPP_CMS_TYPE_HUE) {
        param.data[color].hue = value;
    } else if (type == VPP_CMS_TYPE_LUMA) {
        param.data[color].luma = value;
    }

    if (mSSMAction->SSMSaveColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param) < 0) {
        LOGE("%s SSMSaveCustomizeColorParams failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value)
{
    struct cm_color_md pData;
    memset(&pData, 0, sizeof(struct cm_color_md));

    if (type == VPP_CMS_TYPE_SAT) {
        if (value < 0) {
            value = (value * (0 - CMS_SAT_MIN)) / 50;
        } else {
            value = (value * CMS_SAT_MAX) / 50;
        }
        if (value < CMS_SAT_MIN) value = CMS_SAT_MIN;
        if (value > CMS_SAT_MAX) value = CMS_SAT_MAX;
    } else if (type == VPP_CMS_TYPE_HUE) {
        value = (((value + 50) * (CMS_HUE_MAX - CMS_HUE_MIN)) / 100) - CMS_HUE_MAX;
        if (value < CMS_HUE_MIN) value = CMS_HUE_MIN;
        if (value > CMS_HUE_MAX) value = CMS_HUE_MAX;
    } else if (type == VPP_CMS_TYPE_LUMA) {
        value = (((value + 50) * (CMS_LUMA_MAX - CMS_LUMA_MIN)) / 100) - CMS_LUMA_MAX;
        if (value < CMS_LUMA_MIN) value = CMS_LUMA_MIN;
        if (value > CMS_LUMA_MAX) value = CMS_LUMA_MAX;
    }

    pData.color_type = cm_9_color;
    pData.cm_9_color_md = (enum ecm2colormode)color;
    pData.cm_14_color_md = cm_14_ecm2colormode_max;
    pData.color_value = value;

    LOGD("%s cm_9_color_md = %d, cm_14_color_md = %d, color_value = %d.\n", __FUNCTION__,
        pData.cm_9_color_md, pData.cm_14_color_md, pData.color_value);

    if (type == VPP_CMS_TYPE_SAT) {
        if (VPPDeviceIOCtl(AMVECM_IOC_S_CMS_SAT, &pData) < 0) {
            LOGE("%s VPP_IOC_SET_CMS_SAT error(%s)\n", __FUNCTION__, strerror(errno));
            return -1;
        }
    } else if (type == VPP_CMS_TYPE_HUE) {
        if (VPPDeviceIOCtl(AMVECM_IOC_S_CMS_HUE_HS, &pData) < 0) {
            LOGE("%s AMVECM_IOC_S_CMS_HUE_HS error(%s)\n", __FUNCTION__, strerror(errno));
            return -1;
        }
    } else if (type == VPP_CMS_TYPE_LUMA) {
        if (VPPDeviceIOCtl(AMVECM_IOC_S_CMS_LUMA, &pData) < 0) {
            LOGE("%s AMVECM_IOC_S_CMS_LUMA error(%s)\n", __FUNCTION__, strerror(errno));
            return -1;
        }
    }

    return 0;
}

int CPQControl::SetColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value, int isSave)
{
    int ret = -1;

    LOGD("%s color:%d, type:%d, value:%d, isSave:%d\n", __FUNCTION__, color, type, value, isSave);

    if (type < VPP_CMS_TYPE_RED) {
        LOGE("%s type is error, not 3DLUT module request\n",__FUNCTION__);
        return ret;
    }

    ret = Cpq_SetColorCustomizeBy3DLut(color, type, value);

    if ((ret ==0) && (isSave == 1)) {
        ret = SaveColorCustomizeBy3DLut(color, type, value);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success\n",__FUNCTION__);
    }

    return ret;
}

vpp_single_color_param_3dlut_t CPQControl::GetColorCustomizeBy3DLut(vpp_cms_6color_t color)
{
    int offset = 0;
    vpp_cms_3dlut_param_t param;
    vpp_single_color_param_3dlut_t single_param;
    memset(&param, 0, sizeof(vpp_cms_3dlut_param_t));
    memset(&single_param, 0, sizeof(vpp_single_color_param_3dlut_t));

    if (mSSMAction->SSMReadColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param) < 0) {
        LOGE("%s SSMReadColorCustomizeParamsBy3DLut failed!\n",__FUNCTION__);
    }

    for (int i = VPP_COLOR_6_RED; i < VPP_COLOR_6_MAX; i++) {
        LOGD("%s param.data[%d].red/green/blue: %d %d %d\n", __FUNCTION__,
            i, param.data[i].red, param.data[i].green, param.data[i].blue);
    }

    single_param.red = param.data[color].red;
    single_param.green = param.data[color].green;
    single_param.blue = param.data[color].blue;

    return single_param;
}

int CPQControl::SaveColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value)
{
    int offset = 0;
    vpp_cms_3dlut_param_t param;
    memset(&param, 0, sizeof(vpp_cms_3dlut_param_t));

    if (mSSMAction->SSMReadColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param) < 0) {
        LOGE("%s SSMReadColorCustomizeParamsBy3DLut failed!\n", __FUNCTION__);
        return -1;
    }

    if (type == VPP_CMS_TYPE_RED) {
        param.data[color].red = value;
    } else if (type == VPP_CMS_TYPE_GREEN) {
        param.data[color].green = value;
    } else if (type == VPP_CMS_TYPE_BLUE) {
        param.data[color].blue = value;
    }

    if (mSSMAction->SSMSaveColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param) < 0) {
        LOGE("%s SSMSaveColorCustomizeParamsBy3DLut failed!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_SetColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value)
{
    int gain = 0;
    int offset = 0;
    vpp_cms_3dlut_param_t param;
    struct color_tune_parm_s pData;
    memset(&param, 0, sizeof(vpp_cms_3dlut_param_t));
    memset(&pData, 0, sizeof(struct color_tune_parm_s));

    if (mSSMAction->SSMReadColorCustomizeParamsBy3DLut(offset, sizeof(struct color_tune_parm_s), (int *)&param) < 0) {
        LOGE("%s SSMReadColorCustomizeParamsBy3DLut failed!\n", __FUNCTION__);
        return -1;
    }

    pData.rgain_r = param.data[VPP_COLOR_6_RED].red;
    pData.rgain_g = param.data[VPP_COLOR_6_RED].green;
    pData.rgain_b = param.data[VPP_COLOR_6_RED].blue;
    pData.ggain_r = param.data[VPP_COLOR_6_GREEN].red;
    pData.ggain_g = param.data[VPP_COLOR_6_GREEN].green;
    pData.ggain_b = param.data[VPP_COLOR_6_GREEN].blue;
    pData.bgain_r = param.data[VPP_COLOR_6_BLUE].red;
    pData.bgain_g = param.data[VPP_COLOR_6_BLUE].green;
    pData.bgain_b = param.data[VPP_COLOR_6_BLUE].blue;
    pData.cgain_r = param.data[VPP_COLOR_6_CYAN].red;
    pData.cgain_g = param.data[VPP_COLOR_6_CYAN].green;
    pData.cgain_b = param.data[VPP_COLOR_6_CYAN].blue;
    pData.mgain_r = param.data[VPP_COLOR_6_MAGENTA].red;
    pData.mgain_g = param.data[VPP_COLOR_6_MAGENTA].green;
    pData.mgain_b = param.data[VPP_COLOR_6_MAGENTA].blue;
    pData.ygain_r = param.data[VPP_COLOR_6_YELLOW].red;
    pData.ygain_g = param.data[VPP_COLOR_6_YELLOW].green;
    pData.ygain_b = param.data[VPP_COLOR_6_YELLOW].blue;

    if (value < -100 || value > 100) {
        value = 0;
    }
    gain = value * 2048 / 200;

    switch (color) {
        case VPP_COLOR_6_RED:
            if (type == VPP_CMS_TYPE_RED) {
                pData.rgain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.rgain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.rgain_b = gain;
            }
            break;
        case VPP_COLOR_6_GREEN:
            if (type == VPP_CMS_TYPE_RED) {
                pData.ggain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.ggain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.ggain_b = gain;
            }
            break;
        case VPP_COLOR_6_BLUE:
            if (type == VPP_CMS_TYPE_RED) {
                pData.bgain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.bgain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.bgain_b = gain;
            }
            break;
        case VPP_COLOR_6_CYAN:
            if (type == VPP_CMS_TYPE_RED) {
                pData.cgain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.cgain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.cgain_b = gain;
            }
            break;
        case VPP_COLOR_6_MAGENTA:
            if (type == VPP_CMS_TYPE_RED) {
                pData.mgain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.mgain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.mgain_b = gain;
            }
            break;
        case VPP_COLOR_6_YELLOW:
            if (type == VPP_CMS_TYPE_RED) {
                pData.ygain_r = gain;
            } else if (type == VPP_CMS_TYPE_GREEN) {
                pData.ygain_g = gain;
            } else if (type == VPP_CMS_TYPE_BLUE) {
                pData.ygain_b = gain;
            }
            break;
        default:
            break;
    }

    pData.en = mPQConfigFile->GetInt(CFG_SECTION_CMS, CFG_CMS_3DLUT_EN, 0);

    if (VPPDeviceIOCtl(AMVECM_IOC_S_COLOR_TUNE, &pData) < 0) {
        LOGE("%s AMVECM_IOC_S_COLOR_TUNE error(%s)\n", __FUNCTION__, strerror(errno));
        return -1;
    }

    return 0;
}

int CPQControl::Cpq_GetColorCustomizeDefValue(vpp_cms_cm_param_t *pCmsCm, vpp_cms_3dlut_param_t *pCms3DLut)
{
    int i = 0;
    const char *buff = NULL;

    //color customize(CM) default value
    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_READ, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_RED]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_GREEN, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_GREEN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_BLUE, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_BLUE]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_CYAN, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_CYAN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_PURPLE, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_PURPLE]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_YELLOW, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_YELLOW]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_SKIN, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_SKIN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_YELLOW_GREEN, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_YELLOW_GREEN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_CM_BLUE_GREEN, NULL);
    pqTransformStringToInt(buff, (int *)&pCmsCm->data[VPP_COLOR_9_BLUE_GREEN]);
    if (!buff)   return -1;

    for (i = VPP_COLOR_9_PURPLE; i < VPP_COLOR_9_MAX; i++) {
        LOGD("%s pCmsCm color:%d, %d %d %d\n", __FUNCTION__,
            i, pCmsCm->data[i].sat, pCmsCm->data[i].hue, pCmsCm->data[i].luma);
    }

    //color customize(3DLUT) default value
    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_RED, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_RED]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_GREEN, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_GREEN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_BLUE, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_BLUE]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_CYAN, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_CYAN]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_MAGENTA, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_MAGENTA]);
    if (!buff)   return -1;

    buff = mPQConfigFile->GetString(CFG_SECTION_CMS, CFG_CMS_3DLUT_YELLOW, NULL);
    pqTransformStringToInt(buff, (int *)&pCms3DLut->data[VPP_COLOR_6_YELLOW]);
    if (!buff)   return -1;

    for (i = VPP_COLOR_6_RED; i < VPP_COLOR_6_MAX; i++) {
        LOGD("%s pCms3DLut color:%d, %d %d %d\n", __FUNCTION__,
            i, pCms3DLut->data[i].red, pCms3DLut->data[i].green, pCms3DLut->data[i].blue);
    }

    return 0;
}

int CPQControl::ResetColorCustomize(vpp_cms_method_t mode)
{
    int offset = 0;
    vpp_cms_cm_param_t param_cm;
    vpp_cms_3dlut_param_t param_3dlut;

    memset(&param_cm, 0, sizeof(vpp_cms_cm_param_t));
    memset(&param_3dlut, 0, sizeof(vpp_cms_3dlut_param_t));

    Cpq_GetColorCustomizeDefValue(&param_cm, &param_3dlut);

    if (mode == VPP_CMS_METHOD_CM) {
        mSSMAction->SSMSaveColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param_cm);
    } else if (mode == VPP_CMS_METHOD_3DLUT) {
        mSSMAction->SSMSaveColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param_3dlut);
    } else {
        LOGE("%s mode is out of range\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::SetColorGamutMode(vpp_colorgamut_mode_t value, int is_save)
{
    int ret =0;
    LOGD("%s, source:%d, value:%d\n",__FUNCTION__, mSourceInputForSaveParam, value);
    ret = Cpq_SetColorGamutMode(value, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            data = para.ColorGamut;
        }
    } else {
        mSSMAction->SSMReadColorGamutMode(mSourceInputForSaveParam, &data);
    }

    if (data < VPP_COLORGAMUT_MODE_SRC || data > VPP_COLORGAMUT_MODE_NATIVE) {
        data = VPP_COLORGAMUT_MODE_AUTO;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n",__FUNCTION__, CurSource, CurTimming, data);
    return data;
}

int CPQControl::SaveColorGamutMode(vpp_colorgamut_mode_t value)
{
    int ret    = 1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.ColorGamut = (int)value;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
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
    int ret = Cpq_SetSmoothPlusMode((vpp_smooth_plus_mode_t)smoothplus_mode, mCurrentSourceInputInfo);
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
    if (!mbCpqCfg_smoothplus_enable) {
        LOGD("Smooth Plus disabled\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetSmoothPlusParams(smoothplus_mode, source_input_param, &regs) < 0) {
        LOGE("PQ_GetSmoothPlusParams failed!\n");
        return -1;
    }

    struct am_pq_parm_s di_regs;
    memset(&di_regs, 0x0,sizeof(struct am_pq_parm_s));
    di_regs.table_name = TABLE_NAME_SMOOTHPLUS;
    di_regs.table_len = regs.length;
    struct am_reg_s tmp_buf[regs.length];
    for (unsigned int i=0;i<regs.length;i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! smoothplus_mode: %d\n",__FUNCTION__, smoothplus_mode);
    return 0;
}

int CPQControl::SetHDRTMData(int *reGain)
{
    int ret = -1;
    if (reGain == NULL) {
        LOGD("%s: reGain is NULL.\n", __FUNCTION__);
    } else {
        struct hdr_tone_mapping_s hdrToneMapping;
        memset(&hdrToneMapping, 0, sizeof(struct hdr_tone_mapping_s));
        hdrToneMapping.lut_type = HLG_LUT;
        hdrToneMapping.lutlength = 149;
        hdrToneMapping.tm_lut = reGain;

        LOGD("hdrToneMapping.lut_type = %d\n", hdrToneMapping.lut_type);
        LOGD("hdrToneMapping.lutlength = %d\n", hdrToneMapping.lutlength);
        //LOGD("hdrToneMapping.tm_lut = %s\n", hdrToneMapping.tm_lut);

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
int CPQControl::SetHDRTMOMode(hdr_tmo_t mode, int is_save)
{
    LOGD("%s, source: %d, timming, mode = %d\n", __FUNCTION__, CurSource, CurTimming, mode);

    int ret = Cpq_SetHDRTMOMode((int)mode);

    if ((ret ==0) && (is_save == 1)) {
        ret = SaveHDRTMOMode(mode);
    }

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    } else {
        LOGD("%s success!\n",__FUNCTION__);
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
    }

    return ret;
}

int CPQControl::Cpq_SetHDRTMOMode(int mode)
{
    if (!mbCpqCfg_hdrtmo_enable) {
        LOGD("hdr tmo disabled\n");
        return 0;
    }

    struct hdr_tmo_sw hdrtmo_param;
    if (mPQdb->PQ_GetHDRTMOParams(mCurrentSourceInputInfo, (hdr_tmo_t)mode, &hdrtmo_param) < 0) {
        LOGE("mPQdb->PQ_GetHDRTMOParams failed!\n");
        return -1;
    }

    if (VPPDeviceIOCtl(AMVECM_IOC_S_HDR_TMO, &hdrtmo_param) < 0) {
        LOGE("VPPDeviceIOCtl AMVECM_IOC_S_HDR_TMO failed!\n");
        return -1;
    }

    LOGD("%s success! mode: %d\n",__FUNCTION__, mode);
    return 0;
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
    LOGD("%s: src: %d, timming: %d, mode is %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    int ret = -1;

    ret = Cpq_SetMpegNr(mode, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            mode = para.MpegNr;
        }
    } else {
        mSSMAction->SSMReadMpegNoiseReduction(mCurrentSourceInputInfo.source_input, &mode);
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
        if (GetPictureModeData(&para, pq_mode)) {
            para.MpegNr = (int)mode;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        ret = mSSMAction->SSMSaveMpegNoiseReduction(mCurrentSourceInputInfo.source_input, mode);
    }

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMpegNr(vpp_pq_level_t mode, source_input_param_t source_input_param)
{
    int ret = -1;

    ret = Cpq_SetDeblockMode((vpp_deblock_mode_t) mode, source_input_param);
    ret |= Cpq_SetDemoSquitoMode((vpp_demosquito_mode_t) mode, source_input_param);

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! mode: %d\n",__FUNCTION__, mode);

    return ret;
}

int CPQControl::SetDeblockMode(vpp_deblock_mode_t mode, int is_save)
{
    LOGD("%s: src = %d, timming = %d, mode is %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    int ret = -1;
    ret = Cpq_SetDeblockMode(mode, mCurrentSourceInputInfo);
    if ((ret == 0) && (is_save == 1)) {
        ret = SaveDeblockMode(mode);
    }

    if (ret < 0) {
        LOGE("%s failed!\n", __FUNCTION__);
    } else {
        LOGD("%s success!\n", __FUNCTION__);
    }
    return ret;
}

int CPQControl::GetDeblockMode(void)
{
    int mode = VPP_DEBLOCK_MODE_OFF;

    mSSMAction->SSMReadDeblockMode(mCurrentSourceInputInfo.source_input, &mode);

    if (mode < VPP_DEBLOCK_MODE_OFF || mode > VPP_DEBLOCK_MODE_AUTO) {
        mode = VPP_DEBLOCK_MODE_OFF;
    }

    LOGD("%s: DeblockMode = %d \n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveDeblockMode(vpp_deblock_mode_t mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveDeblockMode(mCurrentSourceInputInfo.source_input, (int)mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDeblockMode(vpp_deblock_mode_t deblock_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_deblock_enable) {
        LOGD("PQ_GetDeblockParams Disabled!\n");
        return 0;
    }

    int ret = -1;
    am_regs_t regs;
    struct am_pq_parm_s di_regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    memset(&di_regs, 0x0,sizeof(struct am_pq_parm_s));

    if (mPQdb->PQ_GetDeblockParams((vpp_deblock_mode_t)deblock_mode, source_input_param, &regs) == 0) {
        di_regs.table_name = TABLE_NAME_DEBLOCK;
        di_regs.table_len = regs.length;
        struct am_reg_s tmp_buf[regs.length];
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

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! deblock_mode: %d\n",__FUNCTION__, deblock_mode);

    return ret;
}

int CPQControl::SetDemoSquitoMode(vpp_demosquito_mode_t mode, int is_save)
{
    LOGD("%s: src = %d, timming = %d, mode is %d\n", __FUNCTION__, CurSource, CurTimming, mode);
    int ret = -1;
    ret = Cpq_SetDemoSquitoMode(mode, mCurrentSourceInputInfo);

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

    mSSMAction->SSMReadDemoSquitoMode(mCurrentSourceInputInfo.source_input, &mode);

    if (mode < VPP_DEMOSQUITO_MODE_OFF || mode > VPP_DEMOSQUITO_MODE_AUTO) {
        mode = VPP_DEMOSQUITO_MODE_OFF;
    }

    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveDemoSquitoMode(vpp_demosquito_mode_t mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveDemoSquitoMode(mCurrentSourceInputInfo.source_input, mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetDemoSquitoMode(vpp_demosquito_mode_t DeMosquito_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_demoSquito_enable) {
        LOGD("DemoSquitoMode Disabled!\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetDemoSquitoParams(DeMosquito_mode, source_input_param, &regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    struct am_pq_parm_s di_regs;
    memset(&di_regs, 0x0,sizeof(struct am_pq_parm_s));
    di_regs.table_name = TABLE_NAME_DEMOSQUITO;
    di_regs.table_len = regs.length;
    struct am_reg_s tmp_buf[regs.length];
    for (unsigned int i=0;i<regs.length;i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! DeMosquito_mode: %d\n",__FUNCTION__, DeMosquito_mode);
    return 0;
}

int CPQControl::SetMcDiMode(vpp_mcdi_mode_e mode, int is_save)
{
    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    int ret = -1;
    ret = Cpq_SetMcDiMode(mode , mCurrentSourceInputInfo);

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

    mSSMAction->SSMReadMcDiMode(mCurrentSourceInputInfo.source_input, &mode);

    if (mode < VPP_MCDI_MODE_OFF || mode > VPP_MCDI_MODE_MAX) {
        mode = VPP_MCDI_MODE_OFF;
    }

    LOGD("%s: mode is %d\n", __FUNCTION__, mode);
    return mode;
}

int CPQControl::SaveMcDiMode(vpp_mcdi_mode_e mode)
{
    int ret = -1;

    ret = mSSMAction->SSMSaveMcDiMode(mCurrentSourceInputInfo.source_input, mode);

    if (ret < 0)
        LOGE("%s failed\n", __FUNCTION__);

    return ret;
}

int CPQControl::Cpq_SetMcDiMode(vpp_mcdi_mode_e McDi_mode, source_input_param_t source_input_param)
{
    if (!mbCpqCfg_mcdi_enable) {
        LOGD("MCDI Disabled!\n");
        return 0;
    }

    am_regs_t regs;
    memset(&regs, 0x0, sizeof(am_regs_t));
    if (mPQdb->PQ_GetMCDIParams(McDi_mode, source_input_param, &regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    struct am_pq_parm_s di_regs;
    memset(&di_regs, 0x0,sizeof(struct am_pq_parm_s));
    di_regs.table_name = TABLE_NAME_MCDI;
    di_regs.table_len = regs.length;
    struct am_reg_s tmp_buf[regs.length];
    for (unsigned int i=0;i<regs.length;i++) {
          tmp_buf[i].addr = regs.am_reg[i].addr;
          tmp_buf[i].mask = regs.am_reg[i].mask;
          tmp_buf[i].type = regs.am_reg[i].type;
          tmp_buf[i].val  = regs.am_reg[i].val;
    }

    di_regs.table_ptr = tmp_buf;

    if (DI_LoadRegs(di_regs) < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
        return -1;
    }

    LOGD("%s success! McDi_mode: %d\n",__FUNCTION__, McDi_mode);
    return 0;
}

int CPQControl::SetBlackStretch(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_BlackStretch(level, mCurrentSourceInputInfo);

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

        if (GetPictureModeData(&para, pq_mode)) {
            level = para.BlackStretch;
        }
    } else {
        mSSMAction->SSMReadBlackStretch(mCurrentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, CurSource, CurTimming, level);

    return level;
}

int CPQControl::SaveBlackStretch(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.BlackStretch = level;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        mSSMAction->SSMSaveBlackStretch(mCurrentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_BlackStretch(int level,source_input_param_t source_input_param)
{
    if (!mbCpqCfg_blackextension_enable) {
        LOGD("%s: BlackStretch disabled!\n", __FUNCTION__);
        return 0;
    }

    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    ret = mPQdb->PQ_GetBlackStretchParams(level,source_input_param, &regs);

    if (ret < 0) {
        LOGE("%s: PQ_GetBlackStretchParams failed!\n", __FUNCTION__);
    } else {
        ret = Cpq_LoadRegs(regs);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! level: %d\n",__FUNCTION__, level);

    return ret;
}

int CPQControl::SetBlueStretch(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_BlueStretch(level, mCurrentSourceInputInfo);

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

        if (GetPictureModeData(&para, pq_mode)) {
            level = para.BlueStretch;
        }
    } else {
        mSSMAction->SSMReadBlueStretch(mCurrentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, CurSource, CurTimming, level);

    return level;
}


int CPQControl::SaveBlueStretch(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.BlueStretch = level;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        mSSMAction->SSMSaveBlueStretch(mCurrentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_BlueStretch(int level,source_input_param_t source_input_param)
{
    if (!mbCpqCfg_bluestretch_enable) {
        LOGD("%s: BlueStretch disabled!\n", __FUNCTION__);
        return 0;
    }

    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    ret = mPQdb->PQ_GetBlueStretchParams(level,source_input_param, &regs);

    if (ret < 0) {
        LOGE("%s: PQ_GetBlueStretchParams failed!\n", __FUNCTION__);
    } else {
        ret = Cpq_LoadRegs(regs);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGE("%s success! level: %d\n",__FUNCTION__, level);

    return ret;
}

int CPQControl::SetChromaCoring(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;
    ret = Cpq_ChromaCoring(level, mCurrentSourceInputInfo);

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
        if (GetPictureModeData(&para, pq_mode)) {
            level = para.ChromaCoring;
        }
    } else {
        mSSMAction->SSMReadChromaCoring(mCurrentSourceInputInfo.source_input, &level);
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s:source:%d, timming:%d, value:%d\n", __FUNCTION__, CurSource, CurTimming, level);
    return level;
}


int CPQControl::SaveChromaCoring(int level)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t para;
        vpp_picture_mode_t pq_mode = (vpp_picture_mode_t)GetPQMode();
        if (GetPictureModeData(&para, pq_mode)) {
            para.ChromaCoring = level;
            if (SetPictureModeData(&para, pq_mode)) {
                ret = 0;
            }
        }
    } else {
        mSSMAction->SSMSaveChromaCoring(mCurrentSourceInputInfo.source_input, level);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);

    return ret;
}


int CPQControl::Cpq_ChromaCoring(int level,source_input_param_t source_input_param)
{
    if (!mbCpqCfg_chromacoring_enable) {
        LOGD("%s: ChromaCoring disabled!\n", __FUNCTION__);
        return 0;
    }

    int ret = -1;
    am_regs_t regs;
    memset(&regs, 0, sizeof(am_regs_t));

    ret = mPQdb->PQ_GetChromaCoringParams(level,source_input_param, &regs);

    if (ret < 0) {
        LOGE("%s: PQ_GetChromaCoringParams failed!\n", __FUNCTION__);
    } else {
        ret = Cpq_LoadRegs(regs);
    }

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success! level: %d\n",__FUNCTION__, level);

    return ret;
}

//av/atv decode
int CPQControl::SetCVD2Values(void)
{
    if (!mbCpqCfg_cvd2_enable) {
        LOGD("%s cvd2 disable\n", __FUNCTION__);
        return 0;
    }

    am_regs_t regs;
    if (mPQdb->PQ_GetCVD2Params(mCurrentSourceInputInfo, &regs) < 0) {
        LOGE ( "%s, PQ_GetCVD2Params failed\n", __FUNCTION__);
        return -1;
    }

    if (AFEDeviceIOCtl(TVIN_IOC_LOAD_REG, &regs) < 0) {
        LOGE ( "%s: ioctl failed\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s success\n", __FUNCTION__);
    return 0;
}

//PQ Factory
int CPQControl::FactoryResetPQMode(void)
{
    if (mbCpqCfg_new_picture_mode_enable) {
        ResetPictureModeDataAll();
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
        ResetColorTemperatureDataAll();
        ResetWhitebalanceGammaDataAll();
    } else {
        mPQdb->PQ_ResetAllColorTemperatureParams();
    }
    return 0;
}

int CPQControl::FactorySetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode, int brightness)
{
    int ret = -1;
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeDataBySrcTimming src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        params.Brightness = brightness;

        if (!SetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
             LOGE("[%s] mDataBase->SetPictureModeDataBySrcTimming src:%d, timing:%d  mode %d failed", __FUNCTION__, src, CurTimming, pq_mode);
             return -1;
        }

        return 0;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] GetPictureModeDataBySrcTimming src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        return params.Brightness;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] GetPictureModeDataBySrcTimming src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        params.Contrast = contrast;

        if (!SetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
             LOGE("[%s] SetPictureModeDataBySrcTimming src:%d, timing:%d  mode %d failed", __FUNCTION__, src, CurTimming, pq_mode);
             return -1;
        }

        return 0;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        return params.Contrast;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        params.Saturation = saturation;

        if (!SetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
             LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, src, CurTimming, pq_mode);
             return -1;
        }

        return 0;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        return params.Saturation;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        params.Hue = hue;

        if (!SetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
             LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, src, CurTimming, pq_mode);
             return -1;
        }

        return 0;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        return params.Hue;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        params.Sharpness = sharpness;

        if (!SetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
             LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, src, CurTimming, pq_mode);
             return -1;
        }

        return 0;
    }

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
    if (mbCpqCfg_new_picture_mode_enable) {
        vpp_pictur_mode_para_t params;
        pq_source_input_t src = CheckPQSource(source_input_param.source_input);
        if (!GetPictureModeDataBySrcTimming(&params, src, CurTimming, (vpp_picture_mode_t)pq_mode)) {
            LOGE("[%s] mSSMAction->GetPictureModeData src:%d, timing:%d  mode %d have no data", __FUNCTION__, src, CurTimming, pq_mode);
            return -1;
        }

        return params.Sharpness;
    }

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
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.r_gain = rgain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Rgain(int source_input, int colortemp_mode, int rgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.r_gain = rgain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Rgain(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.r_gain;
}

int CPQControl::FactorySetColorTemp_Ggain(int source_input, int colortemp_mode, int ggain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.g_gain = ggain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Ggain(int source_input, int colortemp_mode, int ggain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.g_gain = ggain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}


int CPQControl::FactoryGetColorTemp_Ggain(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.g_gain;
}

int CPQControl::FactorySetColorTemp_Bgain(int source_input, int colortemp_mode, int bgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.b_gain = bgain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Bgain(int source_input, int colortemp_mode, int bgain)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.b_gain = bgain;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Bgain(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.b_gain;
}

int CPQControl::FactorySetColorTemp_Roffset(int source_input, int colortemp_mode, int roffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.r_post_offset = roffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Roffset(int source_input, int colortemp_mode, int roffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.r_post_offset = roffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Roffset(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.r_post_offset;
}

int CPQControl::FactorySetColorTemp_Goffset(int source_input, int colortemp_mode, int goffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.g_post_offset = goffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Goffset(int source_input, int colortemp_mode, int goffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.g_post_offset = goffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetColorTemp_Goffset(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.g_post_offset;
}

int CPQControl::FactorySetColorTemp_Boffset(int source_input, int colortemp_mode, int boffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.b_post_offset = boffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    if (Cpq_SetRGBOGO(&rgbogo) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactorySaveColorTemp_Boffset(int source_input, int colortemp_mode, int boffset)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    rgbogo.b_post_offset = boffset;

    if (!SetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGE("%s SetColorTemperatureDataBySrcTimming fail!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}


int CPQControl::FactoryGetColorTemp_Boffset(int source_input, int colortemp_mode)
{
    tcon_rgb_ogo_t rgbogo;
    memset (&rgbogo, 0, sizeof (rgbogo));
    pq_source_input_t src = CheckPQSource((tv_source_input_t)source_input);
    if (FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from cri_data \n", __FUNCTION__);
    } else if (GetColorTemperatureDataBySrcTimming(&rgbogo, src, CurTimming, colortemp_mode)) {
        LOGD("%s Get RGBGainOffset from ssm_data \n", __FUNCTION__);
    } else {
        LOGE("%s fail!\n", __FUNCTION__);
        return -1;
    }

    return rgbogo.b_post_offset;
}

int CPQControl::FactorySetRGBGainOffset(int colortemp_mode, tcon_rgb_ogo_t params)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    if (Cpq_SetRGBOGO(&params) != 0) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    if (!FactorySetWhitebalanceRGBGainOffsetData(&params, colortemp_mode)) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int CPQControl::FactoryGetRGBGainOffset(int colortemp_mode, tcon_rgb_ogo_t *params)
{
    CheckCriDataWhitebalanceRGBGainOffsetData();

    if (!FactoryGetWhitebalanceRGBGainOffsetData(params, colortemp_mode)) {
        LOGE("%s error!\n", __FUNCTION__);
        return -1;
    }

    return 0;
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
        break;

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

int CPQControl::FactorySetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t cutwin_t)
{
    int ret = -1;

    LOGD("%s %d %d %d %d %d %d %d %d\n", __FUNCTION__,
          source_input_param.source_input, source_input_param.sig_fmt, source_input_param.trans_fmt,
          dmode, cutwin_t.hs, cutwin_t.he, cutwin_t.vs, cutwin_t.ve);

    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_SetOverscanParams(source_input_param, dmode, cutwin_t);
    }

    if (ret != 0) {
        LOGE("%s failed.\n", __FUNCTION__);
    } else {
        LOGD("%s success.\n", __FUNCTION__);
    }

    return ret;
}

tvin_cutwin_t CPQControl::FactoryGetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode)
{
    int ret = -1;
    tvin_cutwin_t cutwin_t;
    memset(&cutwin_t, 0, sizeof(cutwin_t));

    LOGD("%s %d %d %d %d\n", __FUNCTION__,
          source_input_param.source_input, source_input_param.sig_fmt, source_input_param.trans_fmt, dmode);

    if (source_input_param.trans_fmt < TVIN_TFMT_2D || source_input_param.trans_fmt > TVIN_TFMT_3D_LDGD) {
        return cutwin_t;
    }
    if (mbCpqCfg_seperate_db_enable) {
        ret = mpOverScandb->PQ_GetOverscanParams(source_input_param, dmode, &cutwin_t);
    }

    LOGD("%s %d %d %d %d\n", __FUNCTION__,
          cutwin_t.hs, cutwin_t.he, cutwin_t.vs, cutwin_t.ve);

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
    mSSMAction->SSMSaveEyeProtectionMode(enable);

    ret = Cpq_SetColorTemperature(GetColorTemperature());

    if (ret < 0)
        LOGE("%s failed!\n",__FUNCTION__);
    else
        LOGD("%s success!\n",__FUNCTION__);

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
    struct pq_ctrl_s pqControlVal;
    memset(&pqControlVal, 0x0, sizeof(struct pq_ctrl_s));
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_ALL_PQ_MODULE_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_pq_enable = true;
    } else {
        mbCpqCfg_pq_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_di_enable = true;
    } else {
        mbCpqCfg_di_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MCDI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_mcdi_enable = true;
        pqWriteSys(MCDI_MODULE_CONTROL_PATH, "1");
    } else {
        mbCpqCfg_mcdi_enable = false;
        pqWriteSys(MCDI_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEBLOCK_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_deblock_enable = true;
        pqWriteSys(DEBLOCK_MODULE_CONTROL_PATH, "13");//bit2~bit3
    } else {
        mbCpqCfg_deblock_enable = false;
        pqWriteSys(DEBLOCK_MODULE_CONTROL_PATH, "1");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DEMOSQUITO_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_demoSquito_enable = true;
        pqWriteSys(DEMOSQUITO_MODULE_CONTROL_PATH, "1");//bit0
    } else {
        mbCpqCfg_demoSquito_enable = false;
        pqWriteSys(DEMOSQUITO_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_NOISEREDUCTION_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_nr_enable = true;
        pqWriteSys(NR2_MODULE_CONTROL_PATH, "1");
    } else {
        mbCpqCfg_nr_enable = false;
        pqWriteSys(NR2_MODULE_CONTROL_PATH, "0");
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS0_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness0_enable = true;
        pqControlVal.sharpness0_en = 1;
    } else {
        mbCpqCfg_sharpness0_enable = false;
        pqControlVal.sharpness0_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_SHARPNESS1_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_sharpness1_enable = true;
        pqControlVal.sharpness1_en = 1;
    } else {
        mbCpqCfg_sharpness1_enable = false;
        pqControlVal.sharpness1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DNLP_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_dnlp_enable = true;
        pqControlVal.dnlp_en = 1;
    } else {
        mbCpqCfg_dnlp_enable = false;
        pqControlVal.dnlp_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CM2_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cm2_enable = true;
        pqControlVal.cm_en = 1;
    } else {
        mbCpqCfg_cm2_enable = false;
        pqControlVal.cm_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_enable = true;
        pqControlVal.vadj1_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_enable = false;
        pqControlVal.vadj1_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_AMVECM_BASCI_WITHOSD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_amvecm_basic_withOSD_enable = true;
        pqControlVal.vadj2_en = 1;
    } else {
        mbCpqCfg_amvecm_basic_withOSD_enable = false;
        pqControlVal.vadj2_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_enable = true;
        pqControlVal.vd1_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_enable = false;
        pqControlVal.vd1_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CONTRAST_RGB_WITHOSD_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_contrast_rgb_withOSD_enable = true;
        pqControlVal.post_ctrst_en = 1;
    } else {
        mbCpqCfg_contrast_rgb_withOSD_enable = false;
        pqControlVal.post_ctrst_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_WHITEBALANCE_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_whitebalance_enable = true;
        pqControlVal.wb_en = 1;
    } else {
        mbCpqCfg_whitebalance_enable = false;
        pqControlVal.wb_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_GAMMA_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_gamma_enable = true;
        pqControlVal.gamma_en = 1;
    } else {
        mbCpqCfg_gamma_enable = false;
        pqControlVal.gamma_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_LOCAL_CONTRAST_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_local_contrast_enable = true;
        pqControlVal.lc_en = 1;
    } else {
        mbCpqCfg_local_contrast_enable = false;
        pqControlVal.lc_en = 0;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLACKEXTENSION_ENABLE, "disable");
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_DISPLAY_OVERSCAN_ENABLE, "disable");
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_CVD2_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_cvd2_enable = true;
    } else {
        mbCpqCfg_cvd2_enable = false;
    }

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_PQ_MODE_CHECK_SOURCE_ENABLE, "disable");
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_MEMC_ENABLE, "disable");
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_BLACK_BLUE_CHROMA_DB_ENABLE, "disable");
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

    config_value = mPQConfigFile->GetString(CFG_SECTION_PQ, CFG_NEW_PICTURE_MODE_ENABLE, "disable");
    if (strcmp(config_value, "enable") == 0) {
        mbCpqCfg_new_picture_mode_enable = true;
    } else {
        mbCpqCfg_new_picture_mode_enable = false;
    }

    struct vpp_pq_ctrl_s amvecmConfigVal;
    //struct vpp_pq_ctrl_s amvecmConfigVal = {0, {(void *)"0"}};
    memset(&amvecmConfigVal, 0, sizeof(struct vpp_pq_ctrl_s));
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
    enum hdr_type_e newHdrType = HDRTYPE_SDR;
    newHdrType = Cpq_GetSourceHDRType(source_input_param.source_input);

    LOGD("%s:new source info: source=%d,sigFmt=%d(0x%x),hdrType=%d\n", __FUNCTION__,
                                                                         source_input_param.source_input,
                                                                         source_input_param.sig_fmt,
                                                                         source_input_param.sig_fmt,
                                                                         newHdrType);
    //check pq src timming
    pq_src_param_t PqSrcTim;
    PqSrcTim.pq_source_input = CheckPQSource(source_input_param.source_input);
    PqSrcTim.pq_sig_fmt = CheckPQTimming(newHdrType);
    LOGD("%s: PqSrcTim.pq_source_input:%d  PqSrcTim.pq_sig_fmt:%d\n", __FUNCTION__, PqSrcTim.pq_source_input, PqSrcTim.pq_sig_fmt);

    //get output type
    output_type_t NewOutputType = OUTPUT_TYPE_LVDS;
    NewOutputType = CheckOutPutMode(source_input_param.source_input);

    if (mCurrentHdrType != newHdrType) {
        LOGD("%s: callback hdr type to client %d %d\n", __FUNCTION__, newHdrType, mCurrentHdrType);
        Cpq_SetHdrType(newHdrType);
    }

    if ((mCurrentSourceInputInfo.source_input != source_input_param.source_input) ||
         (mCurrentSourceInputInfo.sig_fmt     != source_input_param.sig_fmt) ||
         (mCurrentSourceInputInfo.trans_fmt   != source_input_param.trans_fmt) ||
         (mCurrentHdrType                    != newHdrType) ||
         (mPQdb->mOutPutType                 != NewOutputType) ||
         (CurSource != PqSrcTim.pq_source_input) ||
         (CurTimming != PqSrcTim.pq_sig_fmt)) {
        LOGD("%s: update source input info\n", __FUNCTION__);
        CurSource = PqSrcTim.pq_source_input;
        CurTimming = PqSrcTim.pq_sig_fmt;
        mCurrentSourceInputInfo.source_input = source_input_param.source_input;
        mCurrentSourceInputInfo.trans_fmt    = source_input_param.trans_fmt;
        mCurrentSourceInputInfo.sig_fmt      = source_input_param.sig_fmt;
        mPQdb->mOutPutType                  = NewOutputType;

        mSourceInputForSaveParam = mCurrentSourceInputInfo.source_input;

        //update hdr type
        mCurrentHdrType                = newHdrType;
        mPQdb->mHdrType                = newHdrType;
        if (mbCpqCfg_seperate_db_enable) {
            mpOverScandb->mHdrType = newHdrType;
        }

        //load display mode setting
        vpp_display_mode_t display_mode = (vpp_display_mode_t)GetDisplayMode();
        SetDisplayMode(display_mode, 1);

        //load pq setting
        if (source_input_param.sig_fmt == TVIN_SIG_FMT_NULL) {//exit source
            mCurrentHdrType = HDRTYPE_NONE;
            mPQdb->mHdrType = HDRTYPE_NONE;
            if (mbCpqCfg_seperate_db_enable) {
                mpOverScandb->mHdrType = HDRTYPE_NONE;
            }
            mCurrentTvinInfo.hdr_info    = 0;
            if ((mCurrentSourceInputInfo.source_input == SOURCE_MPEG)
                || (mCurrentSourceInputInfo.source_input == SOURCE_DTV)) {
                mCurrentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
                //LoadPQSettings();
            } else if ((mCurrentSourceInputInfo.source_input == SOURCE_TV)
                    || (mCurrentSourceInputInfo.source_input == SOURCE_AV1)
                    || (mCurrentSourceInputInfo.source_input == SOURCE_AV2)) {
                mCurrentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_CVBS_NTSC_M;
            } else {
                mCurrentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
            }
        } else if (source_input_param.sig_fmt == TVIN_SIG_FMT_MAX) {//enter nosignal
            mCurrentHdrType = HDRTYPE_NONE;
            mPQdb->mHdrType = HDRTYPE_NONE;
            if (mbCpqCfg_seperate_db_enable) {
                mpOverScandb->mHdrType = HDRTYPE_NONE;
            }
            mCurrentTvinInfo.hdr_info    = 0;
            if ((mCurrentSourceInputInfo.source_input    == SOURCE_TV)
                || (mCurrentSourceInputInfo.source_input == SOURCE_AV1)
                || (mCurrentSourceInputInfo.source_input == SOURCE_AV2)) {
               mCurrentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_CVBS_NTSC_M;
            } else {
               mCurrentSourceInputInfo.sig_fmt = TVIN_SIG_FMT_HDMI_1920X1080P_60HZ;
            }
            LoadPQSettings();
        } else {//play video
            LoadPQSettings();
        }
    } else {
        LOGD("%s: same signal, no need set\n", __FUNCTION__);
    }

    if (mCurrentSignalInfo.info.status == TVIN_SIG_STATUS_STABLE) {
        //load cvd2 parameter for cvbs decode
        if (((SOURCE_TV == mCurrentSourceInputInfo.source_input)
            || (SOURCE_AV1 == mCurrentSourceInputInfo.source_input)
            || (SOURCE_AV2 == mCurrentSourceInputInfo.source_input))) {
            SetCVD2Values();
        }
    }

    pthread_mutex_unlock(&PqControlMutex);

    return 0;
}

int CPQControl::SetPqModeToGame(void)
{
    LOGD("%s: start\n", __FUNCTION__);

    if (mCurrentSourceInputInfo.source_input < SOURCE_HDMI1 ||
        mCurrentSourceInputInfo.source_input > SOURCE_HDMI4)
        return 0;

    if ((mCurrentTvinInfo.allmInfo.allm_mode == true) ||
        (mCurrentTvinInfo.allmInfo.it_content == true && mCurrentTvinInfo.allmInfo.cn_type == GAME) ||
        (mCurrentHdrType == HDRTYPE_DOVI && mCurrentTvinInfo.vrrparm.cur_vrr_status != VDIN_VRR_OFF)) {
        LOGD("%s: game mode\n", __FUNCTION__);
        SetPQMode((int)VPP_PICTURE_MODE_GAME, 1);
    } else if ((mCurrentTvinInfo.allmInfo.allm_mode == false) ||
               (mCurrentTvinInfo.allmInfo.it_content == false && mCurrentTvinInfo.allmInfo.cn_type != GAME) ||
               (mCurrentHdrType == HDRTYPE_DOVI && mCurrentTvinInfo.vrrparm.cur_vrr_status == VDIN_VRR_OFF)) {
        int last_pq_mode = GetLastPQMode();
        LOGD("%s: last_pq_mode:%d\n", __FUNCTION__, last_pq_mode);
        SetPQMode(last_pq_mode, 1);
    } else {
        LOGD("%s: nothing\n", __FUNCTION__);
    }

    return 0;
}

int CPQControl::SetCurrentSource(tv_source_input_t source_input)
{
    LOGD("%s: source_input = %d\n", __FUNCTION__, source_input);

    source_input_param_t new_source_input_param;
    new_source_input_param.source_input = source_input;
    new_source_input_param.trans_fmt    = mCurrentSourceInputInfo.trans_fmt;
    new_source_input_param.sig_fmt      = getVideoResolutionToFmt();
    SetCurrentSourceInputInfo(new_source_input_param);

    return 0;
}

source_input_param_t CPQControl::GetCurrentSourceInputInfo()
{
    pthread_mutex_lock(&PqControlMutex);
    source_input_param_t info = mCurrentSourceInputInfo;
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

/*1000ppm each level*/
int CPQControl::FactorySetLVDSSSCLevel(int level)
{
    if (level > 4)
        level = 4;
    LOGD("%s,SSC_level = %d", __FUNCTION__, level);

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGD ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    ssm_param.level = level;

    if (LCDDeviceIOCtl(LCD_IOC_CMD_SET_SS, &ssm_param) < 0) {
        LOGE("%s,LCD_IOC_CMD_SET_SS fail", __FUNCTION__);
        return -1;
    }

    return SaveLVDSSSCParams(&ssm_param);
}

int CPQControl::FactoryGetLVDSSSCLevel(void)
{
    int level = 0;

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGD ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    level = ssm_param.level;

    return level;
}

/*Frep step range:
step 0 : 29.5KHZ
step 1 : 31.5KHZ
step 2 : 50KHZ
step 3 : 75KHZ
step 4 : 100KHZ
step 5 : 150KHZ
step 5 : 200KHZ*/
int CPQControl::FactorySetLVDSSSCFrep(int step)
{
    if (step > 6)
        step = 6;
    LOGD("%s,SSC_Frep =  fail", __FUNCTION__, step);

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGE ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    ssm_param.freq = step;

    if (LCDDeviceIOCtl(LCD_IOC_CMD_SET_SS, &ssm_param) < 0) {
        LOGD ("%s,LCD_IOC_CMD_SET_SS fail", __FUNCTION__);
        return -1;
    }

    return SaveLVDSSSCParams(&ssm_param);
}

int CPQControl::FactoryGetLVDSSSCFrep(void)
{
    int setp = 0;

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGD ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    setp = ssm_param.freq;

    return setp;
}

/* mode:
mode 0 : center ss
mode 1 : up ss
mode 2 : down ss */
int CPQControl::FactorySetLVDSSSCMode(int mode)
{
    if (mode > 3)
        mode = 0;

    LOGD("%s,SSC_mode =  fail", __FUNCTION__, mode);

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGD ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    ssm_param.mode = mode;

    if (LCDDeviceIOCtl(LCD_IOC_CMD_SET_SS, &ssm_param) < 0) {
        LOGE("%s,LCD_IOC_CMD_SET_SS fail", __FUNCTION__);
        return -1;
    }

    return SaveLVDSSSCParams(&ssm_param);
}

int CPQControl::FactoryGetLVDSSSCMode(void)
{
    int mode = 0;

    struct aml_lcd_ss_ctl_s ssm_param;
    memset(&ssm_param, 0, sizeof(struct aml_lcd_ss_ctl_s));

    if (GetLVDSSSCParams(&ssm_param) < 0) {
        LOGD ("%s,GetLVDSSSCParams fail", __FUNCTION__);
    }

    mode = ssm_param.mode;

    return mode;
}

int CPQControl::GetLVDSSSCParams(struct aml_lcd_ss_ctl_s *param)
{
    int ret = -1;
    const char *PanelIdx =  "0";//config_get_str ( CFG_SECTION_TV, "get.panel.index", "0" ); can't parse ini file in systemcontrol
    int panel_idx = strtoul(PanelIdx, NULL, 10);
    int offset = panel_idx * MAX_LVDS_SSC_PARAM_SIZE;

    ret = mSSMAction->SSMReadLVDSSSC(offset, sizeof(struct aml_lcd_ss_ctl_s), (int *)param);

    return ret;
}

int CPQControl::SaveLVDSSSCParams(struct aml_lcd_ss_ctl_s *param)
{
    int ret = -1;
    const char *PanelIdx =  "0";//config_get_str ( CFG_SECTION_TV, "get.panel.index", "0" ); can't parse ini file in systemcontrol
    int panel_idx = strtoul(PanelIdx, NULL, 10);
    int offset = panel_idx * MAX_LVDS_SSC_PARAM_SIZE;

    ret = mSSMAction->SSMSaveLVDSSSC(offset, sizeof(struct aml_lcd_ss_ctl_s), (int *)param);

    return ret;
}

int CPQControl::SetDecodeLumaParamsCheckVal(int param_type, int val)
{
    int ret = 0;
    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            if (val < 0 || val > 511) {
                ret =  -1;
            } else {
                ret =  0;
            }
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            if (val < 0 || val > 1023) {
                ret =  -1;
            } else {
                ret =  0;
            }
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            if (val < 0 || val > 255) {
                ret =  -1;
            } else {
                ret =  0;
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

int CPQControl::FactorySetDecodeLumaParams(source_input_param_t source_input_param, int param_type, int val)
{
    unsigned int tmp_val;
    char tmp_buf[128];
    int addr, reg_mask;
    int bri_val, con_val;

    if (SetDecodeLumaParamsCheckVal(param_type, val) < 0) {
        LOGE("%s, error: val[%d] is out of range", __FUNCTION__, val);
        return -1;
    }

    if (param_type == VIDEO_DECODE_BRIGHTNESS) {
        addr = 0x157;
        reg_mask = 0x1ff;
    }else if (param_type == VIDEO_DECODE_CONTRAST) {
        addr = 0x157;
        reg_mask = 0x3ff0000;
    }else if (param_type == VIDEO_DECODE_SATURATION) {
        addr = DECODE_SAT_ADDR;
        reg_mask = 0xff;
    } else {
        LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }

    tmp_val = mPQdb->PQ_GetCVD2Param(source_input_param, addr, param_type, reg_mask);
    LOGD("%s, get value is %d, try to set value: %d\n", __FUNCTION__, tmp_val, val);

    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            tmp_val &=  ~0x1ff;
            tmp_val |= val;
            tmp_val |= (0x8 << 12);
            tmp_val |= (0x8 << 28);

            con_val = (mPQdb->PQ_GetCVD2Param(source_input_param, addr, VIDEO_DECODE_CONTRAST, 0x3ff0000) >> 16) & 0x3ff;
            tmp_val |= (con_val << 16);
            LOGD("%s: setting val is %d", __FUNCTION__, tmp_val);
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, 0x157);
            pqWriteSys("/sys/class/tvafe/tvafe0/reg", tmp_buf);
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            tmp_val &= ~0x3ff0000;
            tmp_val |= (val << 16);
            tmp_val |= (0x8 << 12);
            tmp_val |= (0x8 << 28);

            bri_val = mPQdb->PQ_GetCVD2Param(source_input_param, addr, VIDEO_DECODE_BRIGHTNESS, 0x1ff) & 0x1ff;
            tmp_val |= bri_val;
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, 0x157);
            pqWriteSys("/sys/class/tvafe/tvafe0/reg", tmp_buf);
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            tmp_val &= ~0xff;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "wv", tmp_val, DECODE_SAT_ADDR);
            pqWriteSys("/sys/class/tvafe/tvafe0/reg", tmp_buf);
            break;
        }
    }

    mPQdb->PQ_SetCVD2Param(source_input_param, addr, tmp_val, param_type, reg_mask);

    return 0;
}

int CPQControl::FactoryGetDecodeLumaParams(source_input_param_t source_input_param, int param_type)
{
    unsigned int rval = 0;
    unsigned int reg_val = 0;
    int addr, reg_mask;

    if (param_type == VIDEO_DECODE_BRIGHTNESS) {
        addr = 0x157;
        reg_mask = 0x1ff;
    }else if (param_type == VIDEO_DECODE_CONTRAST) {
        addr = 0x157;
        reg_mask = 0x3ff0000;
    }else if (param_type == VIDEO_DECODE_SATURATION) {
        addr = DECODE_SAT_ADDR;
        reg_mask = 0xff;
    } else {
        LOGE("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }

    reg_val = mPQdb->PQ_GetCVD2Param(source_input_param, addr, param_type, reg_mask);

    switch (param_type) {
        case VIDEO_DECODE_BRIGHTNESS: {
            rval = reg_val & 0x1ff;
            break;
        }
        case VIDEO_DECODE_CONTRAST: {
            rval = (reg_val >> 16) & 0x3ff;
            break;
        }
        case VIDEO_DECODE_SATURATION: {
            rval = reg_val & 0xff;
            break;
        }
    }

    LOGD("%s, type [%d], get reg_val: %u, ret val: %u\n", __FUNCTION__, param_type, reg_val, rval);
    return rval;
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
    if ((mCurrentSourceInputInfo.source_input == SOURCE_MPEG) ||
       ((mCurrentSourceInputInfo.source_input >= SOURCE_HDMI1) && mCurrentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_CSCTYPE, &mode);
        if (ret < 0) {
            LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
        }
    } else {
        LOGE("%s: Current source no hdr status!\n", __FUNCTION__);
    }

    return ret;
}

int CPQControl::GetHDRMode()
{
    enum vpp_matrix_csc_e mode = VPP_MATRIX_NULL;
    if ((mCurrentSourceInputInfo.source_input == SOURCE_MPEG) ||
       ((mCurrentSourceInputInfo.source_input >= SOURCE_HDMI1) && mCurrentSourceInputInfo.source_input <= SOURCE_HDMI4)) {
        int ret = VPPDeviceIOCtl(AMVECM_IOC_G_CSCTYPE, &mode);
        if (ret < 0) {
            LOGE("%s error: %s!\n", __FUNCTION__, strerror(errno));
            mode = VPP_MATRIX_NULL;
        } else {
            LOGD("%s: mode is %d\n", __FUNCTION__, mode);
        }
    } else {
        LOGD("%s: Current source no hdr status!\n", __FUNCTION__);
    }

    return mode;
}

enum hdr_type_e CPQControl::Cpq_GetSourceHDRType(tv_source_input_t source_input)
{
    enum hdr_type_e newHdrType = HDRTYPE_SDR;
    if ((source_input == SOURCE_MPEG)
        ||(source_input == SOURCE_DTV)) {
        if (!mbVideoIsPlaying) {
            newHdrType = HDRTYPE_SDR;
        } else {
            char buf[32] = {0};
            int ret = pqReadSys(SYS_VIDEO_HDR_FMT, buf, sizeof(buf));
            if (ret < 0) {
                newHdrType = HDRTYPE_SDR;
                LOGE("%s error: %s\n", __FUNCTION__, strerror(errno));
            } else {
                if (0 == strcmp(buf, "src_fmt = SDR")) {
                    newHdrType = HDRTYPE_SDR;
                } else if (0 == strcmp(buf, "src_fmt = HDR10")) {
                    newHdrType = HDRTYPE_HDR10;
                } else if (0 == strcmp(buf, "src_fmt = HDR10+")) {
                    newHdrType = HDRTYPE_HDR10PLUS;
                } else if (0 == strcmp(buf, "src_fmt = HDR10 prime")) {
                    newHdrType = HDRTYPE_PRIMESL;
                } else if (0 == strcmp(buf, "src_fmt = HLG")) {
                    newHdrType = HDRTYPE_HLG;
                } else if (0 == strcmp(buf, "src_fmt = Dolby Vison")) {
                    newHdrType = HDRTYPE_DOVI;
                } else if (0 == strcmp(buf, "src_fmt = MVC")) {
                    newHdrType = HDRTYPE_MVC;
                } else {
                    LOGD("%s: invalid hdr type!\n", __FUNCTION__);
                    newHdrType = HDRTYPE_SDR;
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
                newHdrType = HDRTYPE_HLG;
            } else if ((signalTransferCharacteristic == 0x30) && (signalColorPrimaries == 0x9)) {
                newHdrType = HDRTYPE_HDR10PLUS;
            } else if ((signalTransferCharacteristic == 0x10) || (signalColorPrimaries == 0x9)) {
                newHdrType = HDRTYPE_HDR10;
            } else if (dvFlag == 0x1) {
                newHdrType = HDRTYPE_DOVI;
            } else {
                newHdrType = HDRTYPE_SDR;
            }
    } else {
        LOGD("%s: This source no hdr status\n", __FUNCTION__);
        newHdrType = HDRTYPE_SDR;
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
        if (strlen(dbAttribute.ToolVersion.c_str()) <= sizeof(pqdatabaseinfo_t.ToolVersion)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.ToolVersion, dbAttribute.ToolVersion.c_str());
        }
        if (strlen(dbAttribute.ProjectVersion.c_str()) <= sizeof(pqdatabaseinfo_t.ProjectVersion)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.ProjectVersion, dbAttribute.ProjectVersion.c_str());
        }
        if (strlen(dbAttribute.GenerateTime.c_str()) <= sizeof(pqdatabaseinfo_t.GenerateTime)/sizeof(char)) {
            strcpy(pqdatabaseinfo_t.GenerateTime, dbAttribute.GenerateTime.c_str());
        }
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
        SetCurrentSourceInputInfo(mCurrentSourceInputInfo);
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
    if (!mbCpqCfg_ai_enable) {
        LOGE("%s ai is disable\n", __FUNCTION__);
        return;
    }

    pqWriteSys(DECODER_COMMON_PARAMETERS_DEBUG_VDETECT,  isEnable ? "1" : "0");
    pqWriteSys(VDETECT_AIPQ_ENABLE,  isEnable ? "1" : "0");
}

int CPQControl::AiParamLoad(void)
{
    if (!mbCpqCfg_ai_enable) {
        LOGE("%s ai is disable\n", __FUNCTION__);
        return 0;
    }

    int ret = -1;
    struct aipq_load_s aiRegs;
    memset(&aiRegs, 0, sizeof(struct aipq_load_s));
    ret = mPQdb->PQ_GetAIParams(mCurrentSourceInputInfo, &aiRegs);
    if (ret >= 0) {
        LOGD("%s: width: %d, height: %d, array: %s.\n", __FUNCTION__, aiRegs.width, aiRegs.height, (char *)aiRegs.table_ptr);
        ret = VPPDeviceIOCtl(AMVECM_IOC_S_AIPQ_TABLE, &aiRegs);
        if (ret < 0) {
            LOGE("%s: iocontrol failed\n", __FUNCTION__);
        }
    } else {
        LOGE("%s: get AI pq params failed\n", __FUNCTION__);
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

    if (data < 0 || data > 1) {
        data = 0;
    }

    LOGD(" %s, data = %d\n", __FUNCTION__, data);
    return data;
}

int CPQControl::SaveAiSrEnable(bool enable)
{
    LOGD(" %s, enable = %d\n", __FUNCTION__, enable);
    int ret = mSSMAction->SSMSaveAiSrEnable(enable ? 1 : 0);

    if (ret < 0) {
        LOGE("%s failed!\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetAiSrEnable(bool enable)
{
    if (!mbCpqCfg_aisr_enable) {
        LOGD("%s disabled\n",__FUNCTION__);
        return 0;
    }

    if (pqWriteSys(VIDEO_AISR_ENABLE, enable ? "1" : "0") < 0) {
        LOGE("%s pqWriteSys %s fail!\n",__FUNCTION__, VIDEO_AISR_ENABLE);
        return -1;
    }

    LOGD("%s success! enable: %d\n",__FUNCTION__, enable);
    return 0;
}

int CPQControl::ResetPQModeSetting(tv_source_input_t source_input, vpp_picture_mode_t pq_mode)
{
    int ret = 0;
    vpp_pictur_mode_para_t picture;
    pq_src_param_t src;
    src.pq_source_input = CheckPQSource(source_input);
    src.pq_sig_fmt = CurTimming;

    if (mPQdb->PQ_GetPictureModeParams(src.pq_source_input, src.pq_sig_fmt, pq_mode, &picture) == 0) {
        if (!mSSMAction->SetPictureModeData(&picture, src.pq_source_input, src.pq_sig_fmt, pq_mode)) {
            LOGE("%s pq_source_input = %d, pq_sig_fmt = %d, pqmode = %d\n", __FUNCTION__, src.pq_source_input, src.pq_sig_fmt, pq_mode);
            ret = -1;
        }
    }

    return ret;
}

int CPQControl::SetLocalDimming(int level, int is_save)
{
    LOGD("%s, level = %d\n", __FUNCTION__, level);
    int ret = -1;

    ret = Cpq_SetLocalDimming((vpp_pq_level_t)level);

    if (ret == 0 && is_save == 1) {
        ret = SaveLocalDimming(level);
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
    }

    if (level < VPP_PQ_LV_OFF || level >= VPP_PQ_LV_MAX) {
        level = VPP_PQ_LV_OFF;
    }

    LOGD("%s, source: %d, level = %d\n", __FUNCTION__, mSourceInputForSaveParam, level);
    return level;
}

int CPQControl::SaveLocalDimming(int level)
{
    int ret = mSSMAction->SSMSaveLocalDimming(mSourceInputForSaveParam, level);

    if (ret < 0) {
        LOGE("%s failed\n",__FUNCTION__);
    }

    return ret;
}

int CPQControl::Cpq_SetLocalDimming(vpp_pq_level_t level)
{
    if (!mbCpqCfg_LocalDimming_enable || (mLdFd > 0)) {
        LOGE("%s: LocalDimming disabled!\n", __FUNCTION__);
        return 0;
    }

    aml_ldim_pq_s ld;
    memset(&ld, 0, sizeof(aml_ldim_pq_s));
    if (mPQdb->PQ_GetLocalDimmingParams(level, mCurrentSourceInputInfo, &ld) < 0) {
        LOGE("%s: PQ_GetLocalDimmingParams failed!\n", __FUNCTION__);
        return -1;
    }

    if (LCDLdimDeviceIOCtl(AML_LDIM_IOC_CMD_SET_INFO_NEW, &ld) < 0) {
        LOGE("%s: LCDLdimDeviceIOCtl AML_LDIM_IOC_CMD_SET_INFO_NEW failed!\n", __FUNCTION__);
        return -1;
    }

    LOGD("%s success! level: %d\n",__FUNCTION__, level);
    return 0;
}

void CPQControl::resetAllUserSettingParam()
{
    int i = 0, config_val = 0;
    int ret = 0;
    vpp_pq_para_t pq_para;
    const char *buf = NULL;

    if (mbCpqCfg_new_picture_mode_enable) {
        resetPQUiSetting();
        resetPQTableSetting();
        return;
    }

    for (i = SOURCE_TV; i < SOURCE_MAX; i++) {
        if (mbCpqCfg_seperate_db_enable) {
            ret = mpOverScandb->PQ_GetPQModeParams((tv_source_input_t)i, VPP_PICTURE_MODE_USER, &pq_para);
        } else {
            ret = mPQdb->PQ_GetPQModeParams((tv_source_input_t)i, VPP_PICTURE_MODE_USER, &pq_para);
        }
        if (ret == -1) {
            LOGE("%s get pq_para faile\n", __FUNCTION__);
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

        config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_GAMMALEVEL_DEF, VPP_GAMMA_CURVE_6);
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
    int lvds_ssc_buf[3] = {0, 0, 0};
    pqTransformStringToInt(buf, lvds_ssc_buf);
    for (i = 0; i < 8; i++) { //8 panle
        mSSMAction->SSMSaveLVDSSSC(i * MAX_LVDS_SSC_PARAM_SIZE, sizeof(int) * 3, lvds_ssc_buf);
    }
    config_val = mPQConfigFile->GetInt(CFG_SECTION_PQ, CFG_EYEPROJECTMODE_DEF, 0);
    mSSMAction->SSMSaveEyeProtectionMode(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_EDID_VERSION_DEF, 0);
    mSSMAction->SSMEdidRestoreDefault(config_val);

    config_val = mPQConfigFile->GetInt(CFG_SECTION_HDMI, CFG_HDCP_SWITCHER_DEF, 0);
    mSSMAction->SSMHdcpSwitcherRestoreDefault(0);

    buf = mPQConfigFile->GetString(CFG_SECTION_HDMI, CFG_COLOR_RANGE_MODE_DEF, "default");
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

    //color customize
    int offset = 0;
    vpp_cms_cm_param_t param_cm;
    vpp_cms_3dlut_param_t param_3dlut;
    memset(&param_cm, 0, sizeof(vpp_cms_cm_param_t));
    memset(&param_3dlut, 0, sizeof(vpp_cms_3dlut_param_t));
    Cpq_GetColorCustomizeDefValue(&param_cm, &param_3dlut);
    mSSMAction->SSMSaveColorCustomizeParams(offset, sizeof(vpp_cms_cm_param_t), (int *)&param_cm);
    mSSMAction->SSMSaveColorCustomizeParamsBy3DLut(offset, sizeof(vpp_cms_3dlut_param_t), (int *)&param_3dlut);

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
        char temp_buf[256] = {'\0'};
        char *p = NULL;
        int i = 0;

        if (strlen(buf) <= sizeof(temp_buf)/sizeof(char)) {
            strcpy(temp_buf, buf);
        }
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
    enum meson_cpu_ver_e chipVersion = VER_NULL;

    database_attribute_t dbAttribute;
    mPQdb->PQ_GetDataBaseAttribute(&dbAttribute);
    if (dbAttribute.ChipVersion.length() == 0) {
        LOGD("%s: ChipVersion is null!\n", __FUNCTION__);
        chipVersion = VER_NULL;
    } else {
        int flagPosition = dbAttribute.ChipVersion.find("_");
        std::string versionStr = dbAttribute.ChipVersion.substr(flagPosition+1, 1);
        LOGD("%s: versionStr is %s!\n", __FUNCTION__, versionStr.c_str());
        if (versionStr == "A") {
            chipVersion = VER_A;
        } else if (versionStr ==  "B") {
            chipVersion = VER_B;
        } else if (versionStr == "C") {
            chipVersion = VER_C;
        } else {
            chipVersion = VER_NULL;
        }
    }

    if (chipVersion == VER_NULL) {
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
        struct am_vdj_mode_s params;
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


//for callback to upper client
int CPQControl::Cpq_SetHdrType(int data)
{
    if (mpObserver != NULL) {
        LOGD("%s data %d\n", __FUNCTION__, data);
        PQControlCb::HdrTypeCb cb_data;
        cb_data.mHdrType = data;
        mpObserver->GetCbDataFromLibpq(cb_data);
    } else {
        LOGD("%s mpObserver is NULL\n", __FUNCTION__);
    }

    return 0;
}

int CPQControl::Cpq_SetAllmStatus(void)
{
    int mAllmGameMode = 0;

    if (mCurrentSourceInputInfo.source_input >= SOURCE_HDMI1 &&
        mCurrentSourceInputInfo.source_input <= SOURCE_HDMI4) {
        if ((mCurrentTvinInfo.allmInfo.allm_mode == true) ||
            (mCurrentTvinInfo.allmInfo.it_content == true &&
             mCurrentTvinInfo.allmInfo.cn_type == GAME) ||
            (mCurrentTvinInfo.vrrparm.cur_vrr_status != VDIN_VRR_OFF)) {
            mAllmGameMode = 1;
        }
    } else {
        LOGD("%s: reset mPreAllmGameMode\n", __FUNCTION__);
        mPreAllmGameMode = 0;
    }

    LOGD("%s: mAllmGameMode:%d mPreAllmGameMode:%d\n", __FUNCTION__, mAllmGameMode, mPreAllmGameMode);

    if (mPreAllmGameMode != mAllmGameMode) {
        if (mpObserver != NULL) {
            PQControlCb::AllmGameModeCb cb_data;
            cb_data.mAllmGameMode = mAllmGameMode;
            mpObserver->GetCbDataFromLibpq(cb_data);
        } else {
            LOGD("%s mpObserver is NULL\n", __FUNCTION__);
            return -1;
        }

        mPreAllmGameMode = mAllmGameMode;
    }

    return 0;
}

int CPQControl::Cpq_SetFilmMakerMode(int value)
{
    int mFilmMakerMode = 0;

    if (mCurrentSourceInputInfo.source_input >= SOURCE_HDMI1 &&
        mCurrentSourceInputInfo.source_input <= SOURCE_HDMI4) {
        if (mCurrentTvinInfo.allmInfo.it_content == true &&
            mCurrentTvinInfo.allmInfo.cn_type == CINEMA) {
            mFilmMakerMode = 1;
        }
    } else if (mCurrentSourceInputInfo.source_input == SOURCE_MPEG) {
        if (value == 1) {
            mFilmMakerMode = 1;
        }
    } else {
        LOGD("%s: reset mPreFilmMakerMode\n", __FUNCTION__);
        mPreFilmMakerMode = 0;
    }

    LOGD("%s: mFilmMakerMode:%d mPreFilmMakerMode:%d\n", __FUNCTION__, mFilmMakerMode, mPreFilmMakerMode);

    if (mPreFilmMakerMode != mFilmMakerMode) {
        if (mpObserver != NULL) {
            PQControlCb::FilmMakerModeCb cb_data;
            cb_data.mFilmMakerMode = mFilmMakerMode;
            mpObserver->GetCbDataFromLibpq(cb_data);
        } else {
            LOGD("%s mpObserver is NULL\n", __FUNCTION__);
            return -1;
        }

        mPreFilmMakerMode = mFilmMakerMode;
    }

    return 0;
}

int CPQControl::Cpq_SetRefreshRate(void)
{
    char frame_rate[10] = {0};
    int mRefreshRate = 0;

    if (pqReadSys(SYS_VIDEO_FRAME_RATE, frame_rate, sizeof(frame_rate)) > 0) {
        LOGD("%s frame_rate %s\n", __FUNCTION__, frame_rate);
        mRefreshRate = atoi(frame_rate);
    } else {
        LOGD("%s read %s failed\n", __FUNCTION__, SYS_VIDEO_FRAME_RATE);
        return -1;
    }

    LOGD("%s: mRefreshRate:%d mPreRefreshRate:%d\n", __FUNCTION__, mRefreshRate, mPreRefreshRate);

    if (mPreRefreshRate != mRefreshRate) {
        if (mpObserver != NULL) {
            PQControlCb::RefreshRateCb cb_data;
            cb_data.mRefreshRate = mRefreshRate;
            mpObserver->GetCbDataFromLibpq(cb_data);
        } else {
            LOGD("%s mpObserver is NULL\n", __FUNCTION__);
            return -1;
        }

        mPreRefreshRate = mRefreshRate;
    }

    return 0;
}


int CPQControl::SetSharpnessParamsCheckVal(int param_type, int val)
{
    int ret = 0;
    switch (param_type) {
        case H_GAIN_HIGH:
        case H_GAIN_LOW:
        case V_GAIN_HIGH:
        case V_GAIN_LOW:
        case D_GAIN_HIGH:
        case D_GAIN_LOW:
        case PKGAIN_VSLUMALUT7:
        case PKGAIN_VSLUMALUT6:
        case PKGAIN_VSLUMALUT5:
        case PKGAIN_VSLUMALUT4:
        case PKGAIN_VSLUMALUT3:
        case PKGAIN_VSLUMALUT2:
        case PKGAIN_VSLUMALUT1:
        case PKGAIN_VSLUMALUT0: {
            if (val < 0 || val > 15) {
                ret =  -1;
            } else {
                ret = 0;
            }
            break;
        }
        case HP_DIAG_CORE:
        case BP_DIAG_CORE: {
            if (val < 0 || val > 63) {
                ret =  -1;
            } else {
                ret = 0;
            }
            break;
        }
    }

    return ret;
}

int CPQControl::MatchSharpnessRegAddr(int param_type, int isHd)
{
    if (isHd) {
        if (param_type >= H_GAIN_HIGH && param_type <= D_GAIN_LOW) {
            return SHARPNESS_HD_GAIN;
        } else if (param_type == HP_DIAG_CORE) {
            return SHARPNESS_HD_HP_DIAG_CORE;
        } else if (param_type == BP_DIAG_CORE) {
            return SHARPNESS_HD_BP_DIAG_CORE;
        } else if (param_type >= PKGAIN_VSLUMALUT7 && param_type <=PKGAIN_VSLUMALUT0) {
            return SHARPNESS_HD_PKGAIN_VSLUMA;
        } else {
            LOGD("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
            return -1;
        }
    }

    if (param_type >= H_GAIN_HIGH && param_type <= D_GAIN_LOW) {
        return SHARPNESS_SD_GAIN;
    } else if (param_type == HP_DIAG_CORE) {
        return SHARPNESS_SD_HP_DIAG_CORE;
    } else if (param_type == BP_DIAG_CORE) {
        return SHARPNESS_SD_BP_DIAG_CORE;
    } else if (param_type >= PKGAIN_VSLUMALUT7 && param_type <=PKGAIN_VSLUMALUT0) {
        return SHARPNESS_SD_PKGAIN_VSLUMA;
    } else {
        LOGD("%s, error: param_type[%d] is not supported", __FUNCTION__, param_type);
        return -1;
    }
    return -1;
}

unsigned int CPQControl::GetSharpnessRegVal(int addr)
{
    char tmp_buf[128] = {0};
    char rval[32] = {0};

    sprintf(tmp_buf, "%s %x", "r", addr);
    pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
    pqReadSys(PQ_SET_RW_INTERFACE, rval, sizeof(rval));
    return atoi(rval);
}

int CPQControl::FactorySetSharpnessParams(source_input_param_t source_input_param, sharpness_timing_t source_timing, int param_type, int val)
{
    int tmp_val;
    char tmp_buf[128];
    int addr;

    if (SetSharpnessParamsCheckVal(param_type, val) < 0) {
        LOGD("%s, error: val[%d] is out of range", __FUNCTION__, val);
        return -1;
    }

    addr = MatchSharpnessRegAddr(param_type, source_timing);
    tmp_val = mPQdb->PQ_GetSharpnessAdvancedParams(source_input_param, addr, source_timing);
    if (tmp_val == 0) {
        tmp_val = GetSharpnessRegVal(addr);
    }

    LOGD("%s, get value is %d, try to set value: %d\n", __FUNCTION__, tmp_val, val);

    switch (param_type) {
        case H_GAIN_HIGH: {
            LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, tmp_val);
            tmp_val = tmp_val & (~(0xf << 28));
            LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, tmp_val);
            tmp_val |= (val << 28);
            LOGD("%s, setting value : %u", __FUNCTION__, tmp_val);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys( PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case H_GAIN_LOW: {
            LOGD("%s, type [%d], get val: %d\n", __FUNCTION__, param_type, tmp_val);
            tmp_val &= (~(0xf << 12));
            tmp_val |= (val << 12);
            LOGD("%s, setting value : %d", __FUNCTION__, tmp_val);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case V_GAIN_HIGH: {
            tmp_val &= (~(0xf << 24));
            tmp_val |= (val << 24);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case V_GAIN_LOW: {
            tmp_val &= ~(0xf << 8);
            tmp_val |= (val << 8);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case D_GAIN_HIGH: {
            tmp_val &= ~(0xf << 20);
            tmp_val |= (val << 20);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case D_GAIN_LOW: {
            tmp_val &= ~(0xf << 4);
            tmp_val |= (val << 4);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_GAIN, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case HP_DIAG_CORE: {
            tmp_val &= ~0x3f;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_HP_DIAG_CORE, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case BP_DIAG_CORE: {
            tmp_val &= ~0x3f;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_BP_DIAG_CORE, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT7: {
            tmp_val &= ~(0xf << 28);
            tmp_val |= (val << 28);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT6: {
            tmp_val &= ~(0xf << 24);
            tmp_val |= (val << 24);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT5: {
            tmp_val &= ~(0xf << 20);
            tmp_val |= (val << 20);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT4: {
            tmp_val &= ~(0xf << 16);
            tmp_val |= (val << 16);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT3: {
            tmp_val &= ~(0xf << 12);
            tmp_val |= (val << 12);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT2: {
            tmp_val &= ~(0xf << 8);
            tmp_val |= (val << 8);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT1: {
            tmp_val &= ~(0xf << 4);
            tmp_val |= (val << 4);
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        case PKGAIN_VSLUMALUT0: {
            tmp_val &= ~0xf;
            tmp_val |= val;
            sprintf(tmp_buf, "%s %x %x", "w", SHARPNESS_SD_PKGAIN_VSLUMA, tmp_val);
            pqWriteSys(PQ_SET_RW_INTERFACE, tmp_buf);
            break;
        }
        default:
            break;
    }

    mPQdb->PQ_SetSharpnessAdvancedParams(source_input_param, addr, tmp_val, source_timing);

    return 0;
}

int CPQControl::FactoryGetSharpnessParams(source_input_param_t source_input_param, sharpness_timing_t source_timing, int param_type)
{
    unsigned int rval = 0;
    unsigned int reg_val = 0;
    int addr;

    addr = MatchSharpnessRegAddr(param_type, source_timing);
    reg_val = mPQdb->PQ_GetSharpnessAdvancedParams(source_input_param, addr, source_timing);
    if (reg_val == 0)
    {
        reg_val = GetSharpnessRegVal(addr);
    }

    switch (param_type) {
        case H_GAIN_HIGH: {
            LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, reg_val);
            rval = (reg_val >> 28) & 0xf;
            LOGD("%s, type [%d], get val: %u\n", __FUNCTION__, param_type, rval);
            break;
        }
        case H_GAIN_LOW: {
            rval = (reg_val >> 12) & 0xf;
            break;
        }
        case V_GAIN_HIGH: {
            rval = (reg_val >> 24) & 0xf;
            break;
        }
        case V_GAIN_LOW: {
            rval = (reg_val >> 8) & 0xf;
            break;
        }
        case D_GAIN_HIGH: {
            rval = (reg_val >> 20) & 0xf;
            break;
        }
        case D_GAIN_LOW: {
            rval = (reg_val >> 4) & 0xf;
            break;
        }
        case HP_DIAG_CORE: {
            rval = reg_val & 0x3f;
            break;
        }
        case BP_DIAG_CORE: {
            rval = reg_val & 0x3f;
            break;
        }
        case PKGAIN_VSLUMALUT7: {
            rval = (reg_val >> 28) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT6: {
            rval = (reg_val >> 24) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT5: {
            rval = (reg_val >> 20) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT4: {
            rval = (reg_val >> 16) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT3: {
            rval = (reg_val >> 12) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT2: {
            rval = (reg_val >> 8) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT1: {
            rval = (reg_val >> 4) & 0xf;
            break;
        }
        case PKGAIN_VSLUMALUT0: {
            rval = reg_val & 0xf;
            break;
        }
        default:
            break;
    }

    LOGD("%s, type [%d], get reg_val: %u, ret val: %u\n", __FUNCTION__, param_type, reg_val, rval);
    return rval;
}

int CPQControl::SetHdmiColorRangeMode(int pq_type, int isEnable)
{
    char pq_enable_buf[128];

    LOGD("%s pq_type =%d , isEnable =%d \n",__FUNCTION__,pq_type, isEnable);
    switch (pq_type) {
        case ALL: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "vpp_pq disable");
            } else {
                sprintf(pq_enable_buf, "%s", "vpp_pq enable");
            }
            pqWriteSys("/sys/class/amvecm/debug", pq_enable_buf);
            break;
        }
        case COLOR: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "0");
            } else {
                sprintf(pq_enable_buf, "%s", "1");
            }
            pqWriteSys("/sys/module/aml_media/parameters/cm_en", pq_enable_buf);
            break;
        }
        case SHARPNESS_SD: {
            break;
        }
        case SHARPNESS_HD: {
            break;
        }
        case _3D_NR_GAIN: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x2dff, 0, 18, 1);
            } else {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x2dff, 1, 18, 1);
            }
            pqWriteSys("/sys/class/amvecm/pq_reg_rw", pq_enable_buf);
            break;
        }
        case _2D_NR_GAIN: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x2dff, 0, 4, 1);
            } else {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x2dff, 1, 4, 1);
            }
            pqWriteSys("/sys/class/amvecm/pq_reg_rw", pq_enable_buf);
            break;
        }
        case DCI: {
            if (!isEnable) {
                Cpq_SetDNLPStatus(DNLP_OFF);
            } else {
                Cpq_SetDNLPStatus(DNLP_ON);
            }
            break;
        }
        case GAMMA: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "gamma disable");
            } else {
                sprintf(pq_enable_buf, "%s", "gamma enable");
            }
            pqWriteSys("/sys/class/amvecm/debug", pq_enable_buf);
            break;
        }
        case FMD: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "37 0");
            } else {
                sprintf(pq_enable_buf, "%s", "37 1");
            }
            pqWriteSys("/sys/kernel/debug/di_top/mw_di", pq_enable_buf);
            break;
        }
        case CTI_SR0: {
            if (!isEnable) {
               sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x322e, 0, 28, 1);
            } else {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x322e, 1, 28, 1);
            }
            pqWriteSys("/sys/class/amvecm/pq_reg_rw", pq_enable_buf);
            break;
        }
        case CTI_SR1: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x32ae, 0, 28, 1);
            } else {
                sprintf(pq_enable_buf, "%s %x %x %x %x", "bw", 0x32ae, 1, 28, 1);
            }
            pqWriteSys("/sys/class/amvecm/pq_reg_rw", pq_enable_buf);
            break;
        }
        case DEJAGGY_SR0: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "sr0_dejaggy_dis");
            } else {
                sprintf(pq_enable_buf, "%s", "sr0_dejaggy_en");
            }
            pqWriteSys("/sys/class/amvecm/pq_user_set", pq_enable_buf);
            break;
        }
        case DEJAGGY_SR1: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "sr1_dejaggy_dis");
            } else {
                sprintf(pq_enable_buf, "%s", "sr1_dejaggy_en");
            }
            pqWriteSys("/sys/class/amvecm/pq_user_set", pq_enable_buf);
            break;
        }
        case DRT_SR0: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "sr0_derec_dis");
            } else {
                sprintf(pq_enable_buf, "%s", "sr0_derec_en");
            }
            pqWriteSys("/sys/class/amvecm/pq_user_set", pq_enable_buf);
            break;
        }
        case DRT_SR1: {
            if (!isEnable) {
                sprintf(pq_enable_buf, "%s", "sr1_derec_dis");
            } else {
                sprintf(pq_enable_buf, "%s", "sr1_derec_en");
            }
            pqWriteSys("/sys/class/amvecm/pq_user_set", pq_enable_buf);
            break;
        }
        case BLACK_STRETCH: {
            break;
        }
        default:
            break;
    }

    return 0;
}

int CPQControl::GetHdmiColorRangeMode(int pq_type)
{
    int ret = -1;

    return ret;
}

//database
bool CPQControl::SetPictureMode(vpp_picture_mode_t PictureMode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    return mSSMAction->SetPictureMode((int)PictureMode, (int)CurSource, (int)CurTimming);
}

bool CPQControl::GetPictureMode(vpp_picture_mode_t *PictureMode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (mSSMAction->GetPictureMode((int *)PictureMode, (int)CurSource, (int)CurTimming)) {
        return true;
    }

    if (mSSMAction->GetPictureMode((int *)PictureMode, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetPictureMode(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    vpp_picture_mode_t pqmode = VPP_PICTURE_MODE_STANDARD;
    if (mSSMAction->GetPictureMode((int *)&pqmode, (int)CurSource, (int)CurTimming)) {
        if (CurTimming == PQ_FMT_DOLVI) {
            pqmode = VPP_PICTURE_MODE_DOLVI_BRIGHT;
        } else {
            pqmode = VPP_PICTURE_MODE_STANDARD;
        }
        if (!mSSMAction->SetPictureMode((int)pqmode, (int)CurSource, (int)CurTimming)) {
            LOGE("%s SetPictureMode fail\n", __FUNCTION__);
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetPictureModeBySrc(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    vpp_picture_mode_t pqmode = VPP_PICTURE_MODE_STANDARD;
    for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
        if (mSSMAction->GetPictureMode((int *)&pqmode, (int)CurSource, j)) {
            if (j == PQ_FMT_DOLVI) {
                pqmode = VPP_PICTURE_MODE_DOLVI_BRIGHT;
            } else {
                pqmode = VPP_PICTURE_MODE_STANDARD;
            }
            if (!mSSMAction->SetPictureMode((int)pqmode, (int)CurSource, j)) {
                LOGE("%s SetPictureMode fail\n", __FUNCTION__);
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetPictureModeAll(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    vpp_picture_mode_t pqmode = VPP_PICTURE_MODE_STANDARD;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
            if (mSSMAction->GetPictureMode((int *)&pqmode, i, j)) {
                if (j == PQ_FMT_DOLVI) {
                    pqmode = VPP_PICTURE_MODE_DOLVI_BRIGHT;
                } else {
                    pqmode = VPP_PICTURE_MODE_STANDARD;
                }
                if (!mSSMAction->SetPictureMode((int)pqmode, i, j)) {
                    LOGE("%s SetPictureMode fail\n", __FUNCTION__);
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::SetPictureModeData(vpp_pictur_mode_para_t *params, vpp_picture_mode_t picmode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (!mSSMAction->SetPictureModeData(params, (int)CurSource, (int)CurTimming, (int)picmode)) {
        return false;
    }

    return true;
}

bool CPQControl::GetPictureModeData(vpp_pictur_mode_para_t *params, vpp_picture_mode_t picmode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (mSSMAction->GetPictureModeData(params, (int)CurSource, (int)CurTimming, (int)picmode)) {
        return true;
    }

    if (mSSMAction->GetPictureModeData(params, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, (int)picmode)) {
        return true;
    }

    return false;
}

bool CPQControl::SetPictureModeDataBySrcTimming(vpp_pictur_mode_para_t *params, pq_source_input_t src, pq_sig_fmt_t timming, vpp_picture_mode_t picmode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (!mSSMAction->SetPictureModeData(params, (int)src, (int)timming, (int)picmode)) {
        return false;
    }

    return true;
}

bool CPQControl::GetPictureModeDataBySrcTimming(vpp_pictur_mode_para_t *params, pq_source_input_t src, pq_sig_fmt_t timming, vpp_picture_mode_t picmode)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (mSSMAction->GetPictureModeData(params, (int)src, (int)timming, (int)picmode)) {
        return true;
    }

    if (mSSMAction->GetPictureModeData(params, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, (int)picmode)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetPictureModeData(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    int ret = -1;
    vpp_pictur_mode_para_t params;
    for (int k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
        ret = 0;
        if (mSSMAction->GetPictureModeData(&params, (int)CurSource, (int)CurTimming, k)) {
            if (mPQdb->PQ_GetPictureModeParams(CurSource, CurTimming, vpp_picture_mode_t(k), &params) != 0) {
               if (mPQdb->PQ_GetPictureModeParams(PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, vpp_picture_mode_t(k), &params) != 0) {
                   LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, CurTimming, k);
                   ret = -1;
               }
            }

            if (ret == 0) {
                if (!mSSMAction->SetPictureModeData(&params, (int)CurSource, (int)CurTimming, k)) {
                     LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, CurTimming, k);
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetPictureModeDataBySrc(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    int ret = -1;
    vpp_pictur_mode_para_t params;
    for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
        for (int k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
            ret = 0;
            if (mSSMAction->GetPictureModeData(&params, (int)CurSource, j, k)) {
                if (mPQdb->PQ_GetPictureModeParams(CurSource, (pq_sig_fmt_t)j, vpp_picture_mode_t(k), &params) != 0) {
                   if (mPQdb->PQ_GetPictureModeParams(PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, vpp_picture_mode_t(k), &params) != 0) {
                       LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, j, k);
                       ret = -1;
                   }
                }

                if (ret == 0) {
                    if (!mSSMAction->SetPictureModeData(&params, (int)CurSource, j, k)) {
                         LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, j, k);
                    }
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetPictureModeDataAll(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);
    int ret = -1;
    vpp_pictur_mode_para_t params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
            for (int k = VPP_PICTURE_MODE_STANDARD; k < VPP_PICTURE_MODE_MAX; k++) {
                ret = 0;
                if (mSSMAction->GetPictureModeData(&params, i, j, k)) {
                    if (mPQdb->PQ_GetPictureModeParams((pq_source_input_t)i, (pq_sig_fmt_t)j, vpp_picture_mode_t(k), &params) != 0) {
                       if (mPQdb->PQ_GetPictureModeParams(PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, vpp_picture_mode_t(k), &params) != 0) {
                           LOGE("[%s] GetDefaultPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                           ret = -1;
                       }
                    }

                    if (ret == 0) {
                        if (!mSSMAction->SetPictureModeData(&params, i, j, k)) {
                             LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, i, j, k);
                        }
                    }
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::SetColorTemperatureData(tcon_rgb_ogo_t *params, int level)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (!mSSMAction->SetColorTemperatureData(params, (int)CurSource, (int)CurTimming, level)) {
        return false;
    }

    return true;
}

bool CPQControl::GetColorTemperatureData(tcon_rgb_ogo_t *params, int level)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (mSSMAction->GetColorTemperatureData(params, (int)CurSource, (int)CurTimming, level)) {
        return true;
    }

    if (mSSMAction->GetColorTemperatureData(params, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, level)) {
        return true;
    }

    return false;
}

bool CPQControl::SetColorTemperatureDataBySrcTimming(tcon_rgb_ogo_t *params, pq_source_input_t src, pq_sig_fmt_t timming, int level)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (!mSSMAction->SetColorTemperatureData(params, (int)src, (int)timming, level)) {
        return false;
    }

    return true;
}

bool CPQControl::GetColorTemperatureDataBySrcTimming(tcon_rgb_ogo_t *params, pq_source_input_t src, pq_sig_fmt_t timming, int level)
{
    if (mSSMAction == NULL) {
        return false;
    }

    if (mSSMAction->GetColorTemperatureData(params, (int)src, (int)timming, level)) {
        return true;
    }

    if (mSSMAction->GetColorTemperatureData(params, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, level)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetColorTemperatureData(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    int ret = -1;
    tcon_rgb_ogo_t params;
    for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
        ret = 0;
        if (mSSMAction->GetColorTemperatureData(&params, (int)CurSource, (int)CurTimming, k)) {
            if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, CurSource, CurTimming, &params) != 0) {
               if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, &params) != 0) {
                   LOGE("[%s] PQ_GetColorTemperatureData src:%d, timing:%d  mode %d failed, check XML", __FUNCTION__, CurSource, CurTimming, k);
                   ret = -1;
               }
            }

            if (ret == 0) {
                if (!mSSMAction->SetColorTemperatureData(&params, (int)CurSource, (int)CurTimming, k)) {
                     LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, CurTimming, k);
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetColorTemperatureDataBySrc(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    int ret = -1;
    tcon_rgb_ogo_t params;
    for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
        for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
            ret = 0;
            if (mSSMAction->GetColorTemperatureData(&params, (int)CurSource, j, k)) {
                if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, CurSource, (pq_sig_fmt_t)j, &params) != 0) {
                   if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, &params) != 0) {
                       LOGE("[%s] PQ_GetColorTemperatureData src:%d, timing:%d  mode %d failed, check XML", __FUNCTION__, CurSource, j, k);
                       ret = -1;
                   }
                }

                if (ret == 0) {
                    if (!mSSMAction->SetColorTemperatureData(&params, (int)CurSource, j, k)) {
                         LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, j, k);
                    }
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetColorTemperatureDataAll(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    int ret = -1;
    tcon_rgb_ogo_t params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
            for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
                ret = 0;
                if (mSSMAction->GetColorTemperatureData(&params, i, j, k)) {
                    if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, (pq_source_input_t)i, (pq_sig_fmt_t)j, &params) != 0) {
                       if (mPQdb->PQ_GetColorTemperatureData((vpp_color_temperature_mode_t)k, PQ_SRC_DEFAULT, PQ_FMT_DEFAULT, &params) != 0) {
                           LOGE("[%s] PQ_GetColorTemperatureData src:%d, timing:%d  mode %d failed, check XML\n", __FUNCTION__, i, j, k);
                           ret = -1;
                       }
                    }

                    if (ret == 0) {
                        if (!mSSMAction->SetColorTemperatureData(&params, i, j, k)) {
                             LOGE("[%s] mDataBase->SetPictureModeData src:%d, timing:%d  mode %d failed\n", __FUNCTION__, i, j, k);
                        }
                    }
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::FactoryGetWhitebalanceRGBGainOffsetData(tcon_rgb_ogo_t *pData, int level)
{
    if (mSSMAction == NULL ) {
        return false;
    }

    return mSSMAction->CriDataGetWhitebalanceRGBGainOffsetData(pData, level);
}

bool CPQControl::FactorySetWhitebalanceRGBGainOffsetData(tcon_rgb_ogo_t *pData, int level)
{
    if (mSSMAction == NULL ) {
        return false;
    }

    return mSSMAction->CriDataSetWhitebalanceRGBGainOffsetData(pData, level);
}

bool CPQControl::CheckCriDataWhitebalanceRGBGainOffsetData(void)
{
    tcon_rgb_ogo_t rgbogo;
    int colortemp = GetColorTemperature();
    if (!FactoryGetWhitebalanceRGBGainOffsetData(&rgbogo, colortemp)) {
        for (int i = VPP_COLOR_TEMPERATURE_MODE_STANDARD; i < VPP_COLOR_TEMPERATURE_MODE_MAX; i++) {
            if (i != VPP_COLOR_TEMPERATURE_MODE_USER) {
                if (GetColorTemperatureData(&rgbogo, i)) {
                    if (!FactorySetWhitebalanceRGBGainOffsetData(&rgbogo, i)) {
                        LOGE("%s FactorySetWhitebalanceRGBGainOffsetData colortemp: %d fail\n", __FUNCTION__, i);
                    }
                }
            }
        }
    }

    return true;
}

bool CPQControl::SetWhitebalanceGammaData(WB_GAMMA_TABLE *params, int level)
{
    if (mSSMAction == NULL ) {
        return false;
    }

    return mSSMAction->SetWhitebalanceGammaData(params, (int)CurSource, (int)CurTimming, level);
}

bool CPQControl::GetWhitebalanceGammaData(WB_GAMMA_TABLE *params, int level)
{
    if (mSSMAction == NULL ) {
        return false;
    }

    if (mSSMAction->GetWhitebalanceGammaData(params, (int)CurSource, (int)CurTimming, level)) {
        return true;
    }

    if (mSSMAction->GetWhitebalanceGammaData(params, (int)PQ_SRC_DEFAULT, (int)PQ_FMT_DEFAULT, level)) {
        return true;
    }

    return false;
}

bool CPQControl::ResetWhitebalanceGammaData(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    WB_GAMMA_TABLE params;
    for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
        if (mSSMAction->GetWhitebalanceGammaData(&params, (int)CurSource, (int)CurTimming, k)) {
            memset(&params, 0, sizeof(WB_GAMMA_TABLE));
            if (!mSSMAction->SetWhitebalanceGammaData(&params, (int)CurSource, (int)CurTimming, k)) {
                 LOGE("[%s] mDataBase->SetWhitebalanceGammaData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, CurTimming, k);
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetWhitebalanceGammaDataBySrc(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    WB_GAMMA_TABLE params;
    for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
        for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
            if (mSSMAction->GetWhitebalanceGammaData(&params, (int)CurSource, j, k)) {
                memset(&params, 0, sizeof(WB_GAMMA_TABLE));
                if (!mSSMAction->SetWhitebalanceGammaData(&params, (int)CurSource, j, k)) {
                     LOGE("[%s] mDataBase->SetWhitebalanceGammaData src:%d, timing:%d  mode %d failed", __FUNCTION__, CurSource, j, k);
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::ResetWhitebalanceGammaDataAll(void)
{
    if (mSSMAction == NULL) {
        return false;
    }

    LOGD("%s: start\n", __FUNCTION__);

    WB_GAMMA_TABLE params;
    for (int i = PQ_SRC_DEFAULT; i < PQ_SRC_MAX; i++) {
        for (int j = PQ_FMT_DEFAULT; j < PQ_FMT_MAX; j++) {
            for (int k = VPP_COLOR_TEMPERATURE_MODE_STANDARD; k < VPP_COLOR_TEMPERATURE_MODE_MAX; k++) {
                if (mSSMAction->GetWhitebalanceGammaData(&params, i, j, k)) {
                    memset(&params, 0, sizeof(WB_GAMMA_TABLE));
                    if (!mSSMAction->SetWhitebalanceGammaData(&params, i, j, k)) {
                         LOGE("[%s] mDataBase->SetWhitebalanceGammaData src:%d, timing:%d  mode %d failed\n", __FUNCTION__, i, j, k);
                    }
                }
            }
        }
    }

    LOGD("%s: done\n", __FUNCTION__);
    return true;
}

bool CPQControl::FactoryGetWhitebalanceGammaData(WB_GAMMA_TABLE *pData, int level)
{
    return mSSMAction->CriDataGetWhitebalanceGammaData(pData, level);
}

bool CPQControl::FactorySetWhitebalanceGammaData(WB_GAMMA_TABLE *pData, int level)
{
    return mSSMAction->CriDataSetWhitebalanceGammaData(pData, level);
}

bool CPQControl::CheckCriDataWhitebalanceGammaData(void)
{
    WB_GAMMA_TABLE pData;
    if (!mSSMAction->CriDataGetWhitebalanceGammaData(&pData, GetColorTemperature())) {
        for (int i = VPP_COLOR_TEMPERATURE_MODE_STANDARD; i < VPP_COLOR_TEMPERATURE_MODE_MAX; i++) {
            if (GetWhitebalanceGammaData(&pData, i)) {
                if (!mSSMAction->CriDataSetWhitebalanceGammaData(&pData, i)) {
                    LOGD("%s: CriDataSetWhitebalanceGammaData fail level = %d\n", __FUNCTION__, i);
                }
            }
        }
    }

    return true;
}

