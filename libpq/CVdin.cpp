/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "CVdin"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "CVdin.h"
#include "CPQLog.h"
#include "CConfigFile.h"

CVdin *CVdin::mInstance;

CVdin *CVdin::getInstance()
{
    if (NULL == mInstance) {
        mInstance = new CVdin();
    }

    return mInstance;
}

CVdin::CVdin()
{
    //open vdin module
    mVdin0DevFd = VDIN_OpenModule();

    //load tvcfg file
    const char* tvConfigFilePath = getenv(CFG_TV_CONFIG_FILE_PATH_STR);
    if (!tvConfigFilePath) {
        LOGD("%s: read tvconfig file path failed use default path:%s\n", __FUNCTION__, CONFIG_FILE_PATH_DEF);
        tvConfigFilePath = CONFIG_FILE_PATH_DEF;
    } else {
        LOGD("%s: tvconfig file path is %s\n", __FUNCTION__, tvConfigFilePath);
    }
    LoadConfigFile(tvConfigFilePath);

    Tvin_LoadPortToSourceInputMap();
}

CVdin::~CVdin()
{
    VDIN_CloseModule();
    UnloadConfigFile();
}

int CVdin::VDIN_OpenModule()
{
    int fd = open(VDIN_DEV_PATH, O_RDWR );

    if ( fd < 0 ) {
        LOGE("Open %s error(%s)\n", VDIN_DEV_PATH, strerror(errno));
        return -1;
    }

    return fd;
}

int CVdin::VDIN_CloseModule()
{
    if (mVdin0DevFd != -1) {
        close ( mVdin0DevFd );
        mVdin0DevFd = -1;
    }

    return 0;
}

int CVdin::VDIN_DeviceIOCtl( int request, ... )
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;

    if (mVdin0DevFd < 0) {
        mVdin0DevFd = VDIN_OpenModule();
    }

    if ( mVdin0DevFd >= 0 ) {
        va_start (ap, request);
        arg = va_arg (ap, void *);
        va_end (ap);

        tmp_ret = ioctl(mVdin0DevFd, request, arg);
        return tmp_ret;
    }

    return -1;
}

int CVdin::VDIN_OpenPort(tvin_port_t port)
{
    tvin_parm_t vdinParam;

    vdinParam.port = port;
    vdinParam.index = 0;
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_OPEN, &vdinParam );
    if (ret < 0) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_ClosePort()
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_CLOSE);
    if (ret < 0) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_StartDec(const tvin_parm_t *vdinParam)
{
    if ( vdinParam == NULL ) {
        return -1;
    }

    LOGD("VDIN_StartDec: index = [%d] port = [0x%x] format = [0x%x]\n",
        vdinParam->index, (unsigned int)vdinParam->port, (unsigned int)(vdinParam->info.fmt));

    int ret = VDIN_DeviceIOCtl(TVIN_IOC_START_DEC, vdinParam);
    if (ret < 0) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_StopDec()
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_STOP_DEC);
    if (ret < 0) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_GetSignalEventInfo(struct vdin_event_info_s *SignalEventInfo)
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_G_EVENT_INFO, SignalEventInfo);
    if (ret < 0) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_GetSignalInfo(struct tvin_info_s *SignalInfo )
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_G_SIG_INFO, SignalInfo);
    if ( ret < 0 ) {
        LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }
    return ret;
}

int CVdin::VDIN_GetVdinParam(tvin_parm_s *vdinParam)
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_G_PARM, vdinParam);
    if ( ret < 0 ) {
        LOGE( "%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_GetAllmInfo(tvin_latency_s *AllmInfo)
{
    int ret = -1;
    if (AllmInfo == NULL) {
        LOGE("%s: param is NULL\n", __FUNCTION__);
    } else {
        ret = VDIN_DeviceIOCtl(TVIN_IOC_GET_LATENCY_MODE, AllmInfo);
        if (ret < 0) {
            LOGE("%s failed, error(%s)\n", __FUNCTION__, strerror(errno));
        }
    }

    return ret;
}

int CVdin::VDIN_SetPCMode(game_pc_mode_t mode)
{
    LOGD("%s: pc mode: %d\n", __FUNCTION__, mode);

    int ret = -1;
    unsigned int status = 0;
    if (mode == MODE_ON) {
        status = 1;
    } else {
        status = 0;
    }
    ret = VDIN_DeviceIOCtl(TVIN_IOC_S_PC_MODE, &status);
    if (ret < 0) {
        LOGE("%s failed, error(%s)", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::VDIN_SetGameMode(game_pc_mode_t mode)
{
    LOGD("%s: game mode: %d\n", __FUNCTION__, mode);

    int ret = -1;
    unsigned int status = 0;
    if (mode == MODE_ON) {
        status = 1;
    } else {
        status = 0;
    }
    ret = VDIN_DeviceIOCtl(TVIN_IOC_GAME_MODE, &status);
    if (ret < 0) {
        LOGE("%s failed, error(%s)", __FUNCTION__, strerror(errno));
    }

    return ret;
}

int CVdin::Tvin_SetPCMode(game_pc_mode_t mode)
{
    int ret = -1;

    ret = VDIN_SetPCMode(mode);

    return ret;
}

int CVdin::Tvin_SetGameMode(game_pc_mode_t mode)
{
    int ret = -1;

    ret = VDIN_SetGameMode(mode);

    return ret;
}

int CVdin::Tvin_GetAllmInfo(tvin_latency_s *AllmInfo)
{
    int ret = -1;

    if (AllmInfo == NULL) {
        LOGE("%s AllmInfo is NULL\n", __FUNCTION__);
    } else {
        ret = VDIN_GetAllmInfo(AllmInfo);
    }

    return ret;
}

int CVdin::Tvin_GetVdinParam(tvin_parm_s *vdinParam)
{
    int ret = -1;
    if (vdinParam == NULL) {
        LOGE("%s vdinParam is NULL\n", __FUNCTION__);
    } else {
        ret = VDIN_GetVdinParam(vdinParam);
    }

    return ret;
}

int CVdin::Tvin_GetSignalEventInfo(vdin_event_info_s *SignalEventInfo)
{
    int ret = -1;
    if (SignalEventInfo == NULL) {
        LOGE("%s SignalEventInfo is NULL\n", __FUNCTION__);
    } else {
        ret = VDIN_GetSignalEventInfo(SignalEventInfo);
    }

    return ret;
}

int CVdin::Tvin_GetSignalInfo(tvin_info_s *SignalInfo)
{
    int ret = -1;
    if (SignalInfo == NULL) {
        LOGE("%s SignalInfo is NULL\n", __FUNCTION__);
    } else {
        ret = VDIN_GetSignalInfo(SignalInfo);
    }

    return ret;
}

tv_source_input_type_t CVdin::Tvin_SourceInputToSourceInputType ( tv_source_input_t source_input )
{
    tv_source_input_type_t ret = SOURCE_TYPE_MPEG;
    switch (source_input) {
        case SOURCE_TV:
            ret = SOURCE_TYPE_TV;
            break;
        case SOURCE_AV1:
        case SOURCE_AV2:
            ret = SOURCE_TYPE_AV;
            break;
        case SOURCE_YPBPR1:
        case SOURCE_YPBPR2:
            ret = SOURCE_TYPE_COMPONENT;
            break;
        case SOURCE_VGA:
            ret = SOURCE_TYPE_VGA;
            break;
        case SOURCE_HDMI1:
        case SOURCE_HDMI2:
        case SOURCE_HDMI3:
        case SOURCE_HDMI4:
            ret = SOURCE_TYPE_HDMI;
            break;
        case SOURCE_DTV:
            ret = SOURCE_TYPE_DTV;
            break;
        case SOURCE_IPTV:
            ret = SOURCE_TYPE_IPTV;
            break;
        case SOURCE_SPDIF:
            ret = SOURCE_TYPE_SPDIF;
            break;
        default:
            ret = SOURCE_TYPE_MPEG;
            break;
    }

    return ret;
}

static CConfigFile *pTvcfgFile = NULL;
int CVdin::LoadConfigFile(const char *file_name)
{
    if (pTvcfgFile != NULL)
        delete pTvcfgFile;

    pTvcfgFile = new CConfigFile();
    pTvcfgFile->LoadFromFile(file_name);
    return 0;
}

int CVdin::UnloadConfigFile()
{
    if (pTvcfgFile != NULL)
        delete pTvcfgFile;
    return 0;
}

const char *CVdin::ConfigGetStr(const char *section,  const char *key, const char *def_value)
{
    return pTvcfgFile->GetString(section, key, def_value);
}

unsigned int CVdin::Tvin_TransPortStringToValue(const char *port_str)
{
    if (strcasecmp(port_str, "TVIN_PORT_CVBS0") == 0) {
        return TVIN_PORT_CVBS0;
    } else if (strcasecmp(port_str, "TVIN_PORT_CVBS1") == 0) {
        return TVIN_PORT_CVBS1;
    } else if (strcasecmp(port_str, "TVIN_PORT_CVBS2") == 0) {
        return TVIN_PORT_CVBS2;
    } else if (strcasecmp(port_str, "TVIN_PORT_CVBS3") == 0) {
        return TVIN_PORT_CVBS3;
    } else if (strcasecmp(port_str, "TVIN_PORT_COMP0") == 0) {
        return TVIN_PORT_COMP0;
    } else if (strcasecmp(port_str, "TVIN_PORT_COMP1") == 0) {
        return TVIN_PORT_COMP1;
    } else if (strcasecmp(port_str, "TVIN_PORT_VGA0") == 0) {
        return TVIN_PORT_VGA0;
    } else if (strcasecmp(port_str, "TVIN_PORT_HDMI0") == 0) {
        return TVIN_PORT_HDMI0;
    } else if (strcasecmp(port_str, "TVIN_PORT_HDMI1") == 0) {
        return TVIN_PORT_HDMI1;
    } else if (strcasecmp(port_str, "TVIN_PORT_HDMI2") == 0) {
        return TVIN_PORT_HDMI2;
    } else if (strcasecmp(port_str, "TVIN_PORT_HDMI3") == 0) {
        return TVIN_PORT_HDMI3;
    }

    return TVIN_PORT_NULL;
}

void CVdin::Tvin_LoadPortToSourceInputMap()
{
    const char *config_value = NULL;

    memset(mPortToSourceInputMap, SOURCE_INVALID, sizeof(mPortToSourceInputMap));

    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_ATV, "TVIN_PORT_CVBS3");
    mPortToSourceInputMap[SOURCE_TV]  = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_AV1, "TVIN_PORT_CVBS1");
    mPortToSourceInputMap[SOURCE_AV1] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_AV2, "TVIN_PORT_CVBS2");
    mPortToSourceInputMap[SOURCE_AV2] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_YPBPR1, "TVIN_PORT_COMP0");
    mPortToSourceInputMap[SOURCE_YPBPR1] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_YPBPR2, "TVIN_PORT_COMP1");
    mPortToSourceInputMap[SOURCE_YPBPR2] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_HDMI1, "TVIN_PORT_HDMI0");
    mPortToSourceInputMap[SOURCE_HDMI1] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_HDMI2, "TVIN_PORT_HDMI2");
    mPortToSourceInputMap[SOURCE_HDMI2] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_HDMI3, "TVIN_PORT_HDMI1");
    mPortToSourceInputMap[SOURCE_HDMI3] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_HDMI4, "TVIN_PORT_HDMI3");
    mPortToSourceInputMap[SOURCE_HDMI4] = Tvin_TransPortStringToValue(config_value);
    config_value = ConfigGetStr(CFG_SECTION_SRC_INPUT, CFG_TVCHANNEL_VGA, "TVIN_PORT_VGA0");
    mPortToSourceInputMap[SOURCE_VGA]   = Tvin_TransPortStringToValue(config_value);
    mPortToSourceInputMap[SOURCE_MPEG]  = TVIN_PORT_MPEG0;
    mPortToSourceInputMap[SOURCE_DTV]   = TVIN_PORT_DTV;
    mPortToSourceInputMap[SOURCE_IPTV]  = TVIN_PORT_BT656;
    mPortToSourceInputMap[SOURCE_SPDIF] = TVIN_PORT_CVBS3;
}

tv_source_input_t CVdin::Tvin_PortToSourceInput ( tvin_port_t port )
{
    for ( int i = SOURCE_TV; i < SOURCE_MAX; i++ ) {
        if ( mPortToSourceInputMap[i] == (int)port ) {
            LOGI("%s port=0x%x tv_source_input_t=%d\n", __FUNCTION__, (int)port, i);
            return (tv_source_input_t)i;
        }
    }

    return SOURCE_MAX;
}
