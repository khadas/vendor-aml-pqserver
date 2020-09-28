/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MOUDLE_TAG "PQ"
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
        LOGD("%s: tvconfig file path is %s!\n", __FUNCTION__, tvConfigFilePath);
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
    int fd = open (VDIN_DEV_PATH, O_RDWR );

    if ( fd < 0 ) {
        LOGE("Open %s error(%s)!\n", VDIN_DEV_PATH, strerror(errno));
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

int CVdin::VDIN_DeviceIOCtl ( int request, ... )
{
    int tmp_ret = -1;
    va_list ap;
    void *arg;

    if (mVdin0DevFd < 0) {
        mVdin0DevFd = VDIN_OpenModule();
    }

    if ( mVdin0DevFd >= 0 ) {
        va_start ( ap, request );
        arg = va_arg ( ap, void * );
        va_end ( ap );

        tmp_ret = ioctl ( mVdin0DevFd, request, arg );
        return tmp_ret;
    }

    return -1;
}

int CVdin::VDIN_GetSignalEventInfo(struct vdin_event_info_s *SignalEventInfo)
{
    int ret = VDIN_DeviceIOCtl(TVIN_IOC_G_EVENT_INFO, SignalEventInfo);
    if (ret < 0) {
        LOGE("%s error(%s), ret = %d.\n", __FUNCTION__, strerror(errno), ret);
    }

    return ret;
}

int CVdin::VDIN_GetSignalInfo(struct vdin_info_s *SignalInfo )
{
    int ret = VDIN_DeviceIOCtl( TVIN_IOC_G_SIG_INFO, SignalInfo );
    if ( ret < 0 ) {
        LOGE("%s failed, error(%s).\n", __FUNCTION__, strerror ( errno ));
    }
    return ret;
}

int CVdin::VDIN_GetVdinParam(vdin_parm_s *vdinParam)
{
    int ret = VDIN_DeviceIOCtl ( TVIN_IOC_G_PARM, vdinParam );
    if ( ret < 0 ) {
        LOGE ( "Vdin get signal param, error(%s).\n", strerror ( errno ) );
    }

    return ret;
}

int CVdin::Tvin_GetVdinParam(vdin_parm_s *vdinParam)
{
    int ret = -1;
    if (vdinParam == NULL) {
        LOGE("Tvin_GetVdinParam: vdinParam is NULL.\n");
    } else {
        ret = VDIN_GetVdinParam(vdinParam);
    }

    return ret;
}

int CVdin::Tvin_GetSignalEventInfo(vdin_event_info_s *SignalEventInfo)
{
    int ret = -1;
    if (SignalEventInfo == NULL) {
        LOGE("Tvin_GetSignalEventInfo: SignalEventInfo is NULL.\n");
    } else {
        ret = VDIN_GetSignalEventInfo(SignalEventInfo);
    }

    return ret;
}

int CVdin::Tvin_GetSignalInfo(vdin_info_s *SignalInfo)
{
    int ret = -1;
    if (SignalInfo == NULL) {
        LOGE("Tvin_GetSignalInfo: SignalInfo is NULL.\n");
    } else {
        ret = VDIN_GetSignalInfo(SignalInfo);
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
            LOGI("%s port=0x%x tv_source_input_t=%d.\n", __FUNCTION__, (int)port, i);
            return (tv_source_input_t)i;
        }
    }

    return SOURCE_MAX;
}
