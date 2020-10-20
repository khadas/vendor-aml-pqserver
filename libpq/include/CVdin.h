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

// ***************************************************************************
// *** VDIN general definition/enum/struct ***********************************
// ***************************************************************************
// ***************************************************************************
// *** struct definitions *********************************************
// ***************************************************************************
typedef enum tvin_aspect_ratio_e {
    TVIN_ASPECT_NULL = 0,
    TVIN_ASPECT_1x1,
    TVIN_ASPECT_4x3_FULL,
    TVIN_ASPECT_14x9_FULL,
    TVIN_ASPECT_14x9_LB_CENTER,
    TVIN_ASPECT_14x9_LB_TOP,
    TVIN_ASPECT_16x9_FULL,
    TVIN_ASPECT_16x9_LB_CENTER,
    TVIN_ASPECT_16x9_LB_TOP,
    TVIN_ASPECT_MAX,
} tvin_aspect_ratio_t;

//tvin signal status
typedef enum tvin_sig_status_e {
    TVIN_SIG_STATUS_NULL = 0, // processing status from init to the finding of the 1st confirmed status
    TVIN_SIG_STATUS_NOSIG,    // no signal - physically no signal
    TVIN_SIG_STATUS_UNSTABLE, // unstable - physically bad signal
    TVIN_SIG_STATUS_NOTSUP,   // not supported - physically good signal & not supported
    TVIN_SIG_STATUS_STABLE,   // stable - physically good signal & supported
} tvin_sig_status_t;

typedef enum tvin_color_fmt_e {
    TVIN_RGB444 = 0,
    TVIN_YUV422, // 1
    TVIN_YUV444, // 2
    TVIN_YUYV422,// 3
    TVIN_YVYU422,// 4
    TVIN_UYVY422,// 5
    TVIN_VYUY422,// 6
    TVIN_NV12,   // 7
    TVIN_NV21,   // 8
    TVIN_BGGR,   // 9  raw data
    TVIN_RGGB,   // 10 raw data
    TVIN_GBRG,   // 11 raw data
    TVIN_GRBG,   // 12 raw data
    TVIN_COLOR_FMT_MAX,
} tvin_color_fmt_t;


typedef enum tvin_sig_change_flag_e {
     TVIN_SIG_CHG_NONE      = 0,
     TVIN_SIG_CHG_SDR2HDR   = 0x01,
     TVIN_SIG_CHG_HDR2SDR   = 0x02,
     TVIN_SIG_CHG_DV2NO     = 0x04,
     TVIN_SIG_CHG_NO2DV     = 0x08,
     TVIN_SIG_CHG_COLOR_FMT = 0x10,
     TVIN_SIG_CHG_RANGE     = 0x20, /*color range:full or limit*/
     TVIN_SIG_CHG_BIT       = 0x40, /*color bit deepth: 8,10,12 ...*/
     TVIN_SIG_CHG_VS_FRQ    = 0x80,
     TVIN_SIG_CHG_DV_ALLM   = 0x100,/*allm info*/
     TVIN_SIG_CHG_AFD       = 0x200,/*aspect ratio*/
     TVIN_SIG_CHG_CLOSE_FE  = 0x40000000, /*closed frontend*/
     TVIN_SIG_CHG_STS       = 0x80000000, /*sm state change*/
} tvin_sig_change_flag_t;

//tvin port table
typedef enum tvin_port_e {
    TVIN_PORT_NULL    = 0x00000000,
    TVIN_PORT_MPEG0   = 0x00000100,
    TVIN_PORT_BT656   = 0x00000200,
    TVIN_PORT_BT601,
    TVIN_PORT_CAMERA,
    TVIN_PORT_VGA0    = 0x00000400,
    TVIN_PORT_VGA1,
    TVIN_PORT_VGA2,
    TVIN_PORT_VGA3,
    TVIN_PORT_VGA4,
    TVIN_PORT_VGA5,
    TVIN_PORT_VGA6,
    TVIN_PORT_VGA7,
    TVIN_PORT_COMP0   = 0x00000800,
    TVIN_PORT_COMP1,
    TVIN_PORT_COMP2,
    TVIN_PORT_COMP3,
    TVIN_PORT_COMP4,
    TVIN_PORT_COMP5,
    TVIN_PORT_COMP6,
    TVIN_PORT_COMP7,
    TVIN_PORT_CVBS0   = 0x00001000,
    TVIN_PORT_CVBS1,
    TVIN_PORT_CVBS2,
    TVIN_PORT_CVBS3,
    TVIN_PORT_CVBS4,
    TVIN_PORT_CVBS5,
    TVIN_PORT_CVBS6,
    TVIN_PORT_CVBS7,
    TVIN_PORT_SVIDEO0 = 0x00002000,
    TVIN_PORT_SVIDEO1,
    TVIN_PORT_SVIDEO2,
    TVIN_PORT_SVIDEO3,
    TVIN_PORT_SVIDEO4,
    TVIN_PORT_SVIDEO5,
    TVIN_PORT_SVIDEO6,
    TVIN_PORT_SVIDEO7,
    TVIN_PORT_HDMI0   = 0x00004000,
    TVIN_PORT_HDMI1,
    TVIN_PORT_HDMI2,
    TVIN_PORT_HDMI3,
    TVIN_PORT_HDMI4,
    TVIN_PORT_HDMI5,
    TVIN_PORT_HDMI6,
    TVIN_PORT_HDMI7,
    TVIN_PORT_DVIN0   = 0x00008000,
    TVIN_PORT_VIU     = 0x0000C000,
    TVIN_PORT_MIPI    = 0x00010000,
    TVIN_PORT_ISP     = 0x00020000,
    TVIN_PORT_DTV     = 0x00040000,
    TVIN_PORT_MAX     = 0x80000000,
} tvin_port_t;

typedef struct vdin_info_s {
    tvin_trans_fmt    trans_fmt;
    tvin_sig_fmt_e    fmt;
    tvin_sig_status_e status;
    tvin_color_fmt_e  cfmt;
    unsigned int      fps;
    unsigned int      is_dvi;
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
    unsigned int hdr_info;
    tvin_aspect_ratio_e aspect_ratio;
} vdin_info_t;

typedef struct vdin_parm_s {
    int                         index;    // index of frontend for vdin
    enum tvin_port_e            port;     // must set port in IOCTL
    struct vdin_info_s          info;
    unsigned int                hist_pow;
    unsigned int                luma_sum;
    unsigned int                pixel_sum;
    unsigned short              histgram[64];
    unsigned int                flag;
    unsigned short              dest_width;//for vdin horizontal scale down
    unsigned short              dest_height;//for vdin vertical scale down
    bool                        h_reverse;//for vdin horizontal reverse
    bool                        v_reverse;//for vdin vertical reverse
    unsigned int                reserved;
} vdin_parm_t;

typedef struct vdin_event_info_s {
    /*enum tvin_sg_chg_flg*/
    unsigned int event_sts;
} vdin_event_info_t;

// ***************************************************************************
// *** IOCTL command definition **********************************************
// ***************************************************************************

#define TVIN_IOC_MAGIC 'T'
//VDIN
#define TVIN_IOC_G_PARM             _IOR(TVIN_IOC_MAGIC, 0x05, struct vdin_parm_s)
#define TVIN_IOC_G_SIG_INFO         _IOR(TVIN_IOC_MAGIC, 0x07, struct vdin_info_s)
#define TVIN_IOC_G_EVENT_INFO       _IOW(TVIN_IOC_MAGIC, 0x0a, struct vdin_event_info_s)


#define VDIN_DEV_PATH               "/dev/vdin0"

class CVdin {
public:
    CVdin();
    ~CVdin();
    static CVdin *getInstance();
    int Tvin_GetSignalEventInfo(vdin_event_info_s *SignalEventInfo);
    int Tvin_GetSignalInfo(vdin_info_s *SignalInfo);
    int Tvin_GetVdinParam(vdin_parm_s *vdinParam);
    tv_source_input_t Tvin_PortToSourceInput (tvin_port_t port);
    tv_source_input_type_t Tvin_SourceInputToSourceInputType ( tv_source_input_t source_input );

private:
    //VDIN
    int VDIN_OpenModule();
    int VDIN_CloseModule();
    int VDIN_DeviceIOCtl(int request, ...);
    int VDIN_GetSignalEventInfo(vdin_event_info_s *SignalEventInfo);
    int VDIN_GetSignalInfo(vdin_info_s *SignalInfo);
    int VDIN_GetVdinParam(vdin_parm_s *vdinParam);
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
