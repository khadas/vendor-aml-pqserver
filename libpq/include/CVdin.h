/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _CVDIN_H
#define _CVDIN_H
#include "PQType.h"
#include <linux/amlogic/tvin.h>

#define CFG_TV_CONFIG_FILE_PATH_STR        "TV_CONFIG_FILE_PATH"
#define CONFIG_FILE_PATH_DEF               "/vendor/etc/tvconfig/tvconfig.conf"
#define CFG_SECTION_SRC_INPUT              "SourceInputMap"

#define CFG_TVCHANNEL_ATV                  "ro.tv.tvinchannel.atv"
#define CFG_TVCHANNEL_AV1                  "ro.tv.tvinchannel.av1"
#define CFG_TVCHANNEL_AV2                  "ro.tv.tvinchannel.av2"
#define CFG_TVCHANNEL_YPBPR1               "ro.tv.tvinchannel.ypbpr1"
#define CFG_TVCHANNEL_YPBPR2               "ro.tv.tvinchannel.ypbpr2"
#define CFG_TVCHANNEL_HDMI1                "ro.tv.tvinchannel.hdmi1"
#define CFG_TVCHANNEL_HDMI2                "ro.tv.tvinchannel.hdmi2"
#define CFG_TVCHANNEL_HDMI3                "ro.tv.tvinchannel.hdmi3"
#define CFG_TVCHANNEL_HDMI4                "ro.tv.tvinchannel.hdmi4"
#define CFG_TVCHANNEL_VGA                  "ro.tv.tvinchannel.vga"

#define VDIN_DEV_PATH                      "/dev/vdin0"


// ***************************************************************************
// *** struct definitions *********************************************
// ***************************************************************************

typedef struct tvin_inputparam_s {
    unsigned int   is_dvi;
    /*
    * bit 30: is_dv
    * bit 29: present_flag
    * bit 28-26: video_format
    *  "component", "PAL", "NTSC", "SECAM", "MAC", "unspecified"
    * bit 25: range "limited", "full_range"
    * bit 24: color_description_present_flag
    * bit 23-16: color_primaries
    *  "unknown", "bt709", "undef", "bt601", "bt470m", "bt470bg",
    *  "smpte170m", "smpte240m", "film", "bt2020"
    * bit 15-8: transfer_characteristic
    *  "unknown", "bt709", "undef", "bt601", "bt470m", "bt470bg",
    *  "smpte170m", "smpte240m", "linear", "log100", "log316",
    *  "iec61966-2-4", "bt1361e", "iec61966-2-1", "bt2020-10",
    *  "bt2020-12", "smpte-st-2084", "smpte-st-428"
    * bit 7-0: matrix_coefficient
    *  "GBR", "bt709", "undef", "bt601", "fcc", "bt470bg",
    *  "smpte170m", "smpte240m", "YCgCo", "bt2020nc", "bt2020c"
    */
    unsigned int   hdr_info;
    struct tvin_latency_s allmInfo;
    struct vdin_vrr_freesync_param_s vrrparm;
} tvin_inputparam_t;


class CVdin {
public:
    CVdin();
    ~CVdin();
    static CVdin *getInstance();
    int Tvin_GetSignalEventInfo(struct vdin_event_info *SignalEventInfo);
    int Tvin_GetSignalInfo(struct tvin_info_s *SignalInfo);
    int Tvin_GetVdinParam(struct tvin_parm_s *vdinParam);
    int Tvin_GetAllmInfo(tvin_latency_s *AllmInfo);
    int Tvin_SetPCMode(game_pc_mode_t mode);
    int Tvin_SetGameMode(game_pc_mode_t mode);
    int Tvin_GetVrrFreesyncParm(struct vdin_vrr_freesync_param_s *vrrparm);
    tv_source_input_t Tvin_PortToSourceInput (enum tvin_port_e port);
    tv_source_input_type_t Tvin_SourceInputToSourceInputType ( tv_source_input_t source_input );

private:
    //VDIN
    int VDIN_OpenModule();
    int VDIN_CloseModule();
    int VDIN_DeviceIOCtl(int request, ...);
    int VDIN_OpenPort(enum tvin_port_e port);
    int VDIN_ClosePort();
    int VDIN_StartDec(const struct tvin_parm_s *vdinParam);
    int VDIN_StopDec();
    int VDIN_GetSignalEventInfo(struct vdin_event_info *SignalEventInfo);
    int VDIN_GetSignalInfo(struct tvin_info_s *SignalInfo);
    int VDIN_GetVdinParam(struct tvin_parm_s *vdinParam);
    int VDIN_GetAllmInfo(tvin_latency_s *AllmInfo);
    int VDIN_SetPCMode(game_pc_mode_t mode);
    int VDIN_SetGameMode(game_pc_mode_t mode);
    int VDIN_GetVrrFreesyncParm(struct vdin_vrr_freesync_param_s *vrrparm);
    void Tvin_LoadPortToSourceInputMap();
    unsigned int Tvin_TransPortStringToValue(const char *port_str);
    const char *ConfigGetStr(const char *section,  const char *key, const char *def_value);
    int UnloadConfigFile();
    int LoadConfigFile(const char *file_name);


private:
    static CVdin *mInstance;
    int mVdin0DevFd;
    int mPortToSourceInputMap[SOURCE_MAX];
};
#endif
