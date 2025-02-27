/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define MAX_WB_GAMMA_POINT                (21)

#ifndef __TV_SOURCE_INPUT__
#define __TV_SOURCE_INPUT__
typedef enum tv_source_input_e {
    SOURCE_INVALID = -1,
    SOURCE_TV = 0,
    SOURCE_AV1,
    SOURCE_AV2,
    SOURCE_YPBPR1,
    SOURCE_YPBPR2,
    SOURCE_HDMI1,
    SOURCE_HDMI2,
    SOURCE_HDMI3,
    SOURCE_HDMI4,
    SOURCE_VGA,
    SOURCE_MPEG,
    SOURCE_DTV,
    SOURCE_SVIDEO,
    SOURCE_IPTV,
    SOURCE_DUMMY,
    SOURCE_SPDIF,
    SOURCE_MAX,
} tv_source_input_t;
#endif

#ifndef __TV_SOURCE_INPUT_TYPE__
#define __TV_SOURCE_INPUT_TYPE__
typedef enum tv_source_input_type_e {
    SOURCE_TYPE_TV,
    SOURCE_TYPE_AV,
    SOURCE_TYPE_COMPONENT,
    SOURCE_TYPE_HDMI,
    SOURCE_TYPE_VGA,
    SOURCE_TYPE_MPEG,
    SOURCE_TYPE_DTV,
    SOURCE_TYPE_SVIDEO,
    SOURCE_TYPE_IPTV,
    SOURCE_TYPE_SPDIF,
    SOURCE_TYPE_MAX,
} tv_source_input_type_t;
#endif

#ifndef __TVIN_PORT__
#define __TVIN_PORT__
/* tvin input port select */
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
    TVIN_PORT_CVBS3,    //as atv demod to tvafe
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
#endif

#ifndef __TVIN_TRANS_FMT__
#define __TVIN_TRANS_FMT__
typedef enum tvin_trans_fmt {
    TVIN_TFMT_2D = 0,
    TVIN_TFMT_3D_LRH_OLOR,  // 1 Primary: Side-by-Side(Half) Odd/Left picture, Odd/Right p
    TVIN_TFMT_3D_LRH_OLER,  // 2 Primary: Side-by-Side(Half) Odd/Left picture, Even/Right picture
    TVIN_TFMT_3D_LRH_ELOR,  // 3 Primary: Side-by-Side(Half) Even/Left picture, Odd/Right picture
    TVIN_TFMT_3D_LRH_ELER,  // 4 Primary: Side-by-Side(Half) Even/Left picture, Even/Right picture
    TVIN_TFMT_3D_TB,        // 5 Primary: Top-and-Bottom
    TVIN_TFMT_3D_FP,        // 6 Primary: Frame Packing
    TVIN_TFMT_3D_FA,        // 7 Secondary: Field Alternative
    TVIN_TFMT_3D_LA,        // 8 Secondary: Line Alternative
    TVIN_TFMT_3D_LRF,       // 9 Secondary: Side-by-Side(Full)
    TVIN_TFMT_3D_LD,        // 10 Secondary: L+depth
    TVIN_TFMT_3D_LDGD,      // 11 Secondary: L+depth+Graphics+Graphics-depth
    /* normal 3D format */
    TVIN_TFMT_3D_DET_TB,    // 12
    TVIN_TFMT_3D_DET_LR,    // 13
    TVIN_TFMT_3D_DET_INTERLACE,// 14
    TVIN_TFMT_3D_DET_CHESSBOARD,// 15
    TVIN_TFMT_3D_MAX,
} tvin_trans_fmt_t;
#endif

#ifndef __TVIN_SIG_FMT__
#define __TVIN_SIG_FMT__
/* tvin signal format table */
typedef enum tvin_sig_fmt_e {
    TVIN_SIG_FMT_NULL = 0,
	/* HDMI Formats */
	TVIN_SIG_FMT_HDMI_640X480P_60HZ = 0x401,
	TVIN_SIG_FMT_HDMI_720X480P_60HZ = 0x402,
	TVIN_SIG_FMT_HDMI_1280X720P_60HZ = 0x403,
	TVIN_SIG_FMT_HDMI_1920X1080I_60HZ = 0x404,
	TVIN_SIG_FMT_HDMI_1440X480I_60HZ = 0x405,
	TVIN_SIG_FMT_HDMI_1440X240P_60HZ = 0x406,
	TVIN_SIG_FMT_HDMI_2880X480I_60HZ = 0x407,
	TVIN_SIG_FMT_HDMI_2880X240P_60HZ = 0x408,
	TVIN_SIG_FMT_HDMI_1440X480P_60HZ = 0x409,
	TVIN_SIG_FMT_HDMI_1920X1080P_60HZ = 0x40a,
	TVIN_SIG_FMT_HDMI_720X576P_50HZ = 0x40b,
	TVIN_SIG_FMT_HDMI_1280X720P_50HZ = 0x40c,
	TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_A = 0x40d,
	TVIN_SIG_FMT_HDMI_1440X576I_50HZ = 0x40e,
	TVIN_SIG_FMT_HDMI_1440X288P_50HZ = 0x40f,
	TVIN_SIG_FMT_HDMI_2880X576I_50HZ = 0x410,
	TVIN_SIG_FMT_HDMI_2880X288P_50HZ = 0x411,
	TVIN_SIG_FMT_HDMI_1440X576P_50HZ = 0x412,
	TVIN_SIG_FMT_HDMI_1920X1080P_50HZ = 0x413,
	TVIN_SIG_FMT_HDMI_1920X1080P_24HZ = 0x414,
	TVIN_SIG_FMT_HDMI_1920X1080P_25HZ = 0x415,
	TVIN_SIG_FMT_HDMI_1920X1080P_30HZ = 0x416,
	TVIN_SIG_FMT_HDMI_2880X480P_60HZ = 0x417,
	TVIN_SIG_FMT_HDMI_2880X576P_50HZ = 0x418,
	TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_B = 0x419,
	TVIN_SIG_FMT_HDMI_1920X1080I_100HZ = 0x41a,
	TVIN_SIG_FMT_HDMI_1280X720P_100HZ = 0x41b,
	TVIN_SIG_FMT_HDMI_720X576P_100HZ = 0x41c,
	TVIN_SIG_FMT_HDMI_1440X576I_100HZ = 0x41d,
	TVIN_SIG_FMT_HDMI_1920X1080I_120HZ = 0x41e,
	TVIN_SIG_FMT_HDMI_1280X720P_120HZ = 0x41f,
	TVIN_SIG_FMT_HDMI_720X480P_120HZ = 0x420,
	TVIN_SIG_FMT_HDMI_1440X480I_120HZ = 0x421,
	TVIN_SIG_FMT_HDMI_720X576P_200HZ = 0x422,
	TVIN_SIG_FMT_HDMI_1440X576I_200HZ = 0x423,
	TVIN_SIG_FMT_HDMI_720X480P_240HZ = 0x424,
	TVIN_SIG_FMT_HDMI_1440X480I_240HZ = 0x425,
	TVIN_SIG_FMT_HDMI_1280X720P_24HZ = 0x426,
	TVIN_SIG_FMT_HDMI_1280X720P_25HZ = 0x427,
	TVIN_SIG_FMT_HDMI_1280X720P_30HZ = 0x428,
	TVIN_SIG_FMT_HDMI_1920X1080P_120HZ = 0x429,
	TVIN_SIG_FMT_HDMI_1920X1080P_100HZ = 0x42a,
	TVIN_SIG_FMT_HDMI_1280X720P_60HZ_FRAME_PACKING = 0x42b,
	TVIN_SIG_FMT_HDMI_1280X720P_50HZ_FRAME_PACKING = 0x42c,
	TVIN_SIG_FMT_HDMI_1280X720P_24HZ_FRAME_PACKING = 0x42d,
	TVIN_SIG_FMT_HDMI_1280X720P_30HZ_FRAME_PACKING = 0x42e,
	TVIN_SIG_FMT_HDMI_1920X1080I_60HZ_FRAME_PACKING = 0x42f,
	TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_FRAME_PACKING = 0x430,
	TVIN_SIG_FMT_HDMI_1920X1080P_24HZ_FRAME_PACKING = 0x431,
	TVIN_SIG_FMT_HDMI_1920X1080P_30HZ_FRAME_PACKING = 0x432,
	TVIN_SIG_FMT_HDMI_800X600_00HZ = 0x433,
	TVIN_SIG_FMT_HDMI_1024X768_00HZ = 0x434,
	TVIN_SIG_FMT_HDMI_720X400_00HZ = 0x435,
	TVIN_SIG_FMT_HDMI_1280X768_00HZ = 0x436,
	TVIN_SIG_FMT_HDMI_1280X800_00HZ = 0x437,
	TVIN_SIG_FMT_HDMI_1280X960_00HZ = 0x438,
	TVIN_SIG_FMT_HDMI_1280X1024_00HZ = 0x439,
	TVIN_SIG_FMT_HDMI_1360X768_00HZ = 0x43a,
	TVIN_SIG_FMT_HDMI_1366X768_00HZ = 0x43b,
	TVIN_SIG_FMT_HDMI_1600X1200_00HZ = 0x43c,
	TVIN_SIG_FMT_HDMI_1920X1200_00HZ = 0x43d,
	TVIN_SIG_FMT_HDMI_1440X900_00HZ = 0x43e,
	TVIN_SIG_FMT_HDMI_1400X1050_00HZ = 0x43f,
	TVIN_SIG_FMT_HDMI_1680X1050_00HZ = 0x440,
	/* for alternative and 4k2k */
	TVIN_SIG_FMT_HDMI_1920X1080I_60HZ_ALTERNATIVE = 0x441,
	TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_ALTERNATIVE = 0x442,
	TVIN_SIG_FMT_HDMI_1920X1080P_24HZ_ALTERNATIVE = 0x443,
	TVIN_SIG_FMT_HDMI_1920X1080P_30HZ_ALTERNATIVE = 0x444,
	TVIN_SIG_FMT_HDMI_3840_2160_00HZ = 0x445,
	TVIN_SIG_FMT_HDMI_4096_2160_00HZ = 0x446,
	TVIN_SIG_FMT_HDMI_1600X900_60HZ = 0x447,/* upper layer set TVIN_SIG_FMT_HDMI_HDR */
	TVIN_SIG_FMT_HDMI_RESERVE8 = 0x448,
	TVIN_SIG_FMT_HDMI_RESERVE9 = 0x449,
	TVIN_SIG_FMT_HDMI_RESERVE10 = 0x44a,
	TVIN_SIG_FMT_HDMI_RESERVE11 = 0x44b,
	TVIN_SIG_FMT_HDMI_720X480P_60HZ_FRAME_PACKING = 0x44c,
	TVIN_SIG_FMT_HDMI_720X576P_50HZ_FRAME_PACKING = 0x44d,
	TVIN_SIG_FMT_HDMI_640X480P_72HZ = 0x44e,
	TVIN_SIG_FMT_HDMI_640X480P_75HZ = 0x44f,
	TVIN_SIG_FMT_HDMI_1152X864_00HZ = 0x450,
	TVIN_SIG_FMT_HDMI_3840X600_00HZ = 0x451,
	TVIN_SIG_FMT_HDMI_720X350_00HZ = 0x452,
	TVIN_SIG_FMT_HDMI_2688X1520_00HZ = 0x453,
	TVIN_SIG_FMT_HDMI_1920X2160_60HZ = 0x454,
	TVIN_SIG_FMT_HDMI_960X540_60HZ = 0x455,
	TVIN_SIG_FMT_HDMI_2560X1440_00HZ = 0x456,
	TVIN_SIG_FMT_HDMI_640X350_85HZ = 0x457,
	TVIN_SIG_FMT_HDMI_640X400_85HZ = 0x458,
	TVIN_SIG_FMT_HDMI_848X480_60HZ = 0x459,
	TVIN_SIG_FMT_HDMI_1792X1344_85HZ = 0x45a,
	TVIN_SIG_FMT_HDMI_1856X1392_00HZ = 0x45b,
	TVIN_SIG_FMT_HDMI_1920X1440_00HZ = 0x45c,
	TVIN_SIG_FMT_HDMI_2048X1152_60HZ = 0x45d,
	TVIN_SIG_FMT_HDMI_2560X1600_00HZ = 0x45e,
	TVIN_SIG_FMT_HDMI_720X480I_60HZ = 0x45f,
	TVIN_SIG_FMT_HDMI_720X576I_50HZ = 0x460,
	TVIN_SIG_FMT_HDMI_MAX,
	TVIN_SIG_FMT_HDMI_THRESHOLD = 0x600,
	/* Video Formats */
	TVIN_SIG_FMT_CVBS_NTSC_M = 0x601,
	TVIN_SIG_FMT_CVBS_NTSC_443 = 0x602,
	TVIN_SIG_FMT_CVBS_PAL_I = 0x603,
	TVIN_SIG_FMT_CVBS_PAL_M = 0x604,
	TVIN_SIG_FMT_CVBS_PAL_60 = 0x605,
	TVIN_SIG_FMT_CVBS_PAL_CN = 0x606,
	TVIN_SIG_FMT_CVBS_SECAM = 0x607,
	TVIN_SIG_FMT_CVBS_NTSC_50 = 0x608,
	TVIN_SIG_FMT_CVBS_MAX = 0x609,
	TVIN_SIG_FMT_CVBS_THRESHOLD = 0x800,
	/* 656 Formats */
	TVIN_SIG_FMT_BT656IN_576I_50HZ = 0x801,
	TVIN_SIG_FMT_BT656IN_480I_60HZ = 0x802,
	/* 601 Formats */
	TVIN_SIG_FMT_BT601IN_576I_50HZ = 0x803,
	TVIN_SIG_FMT_BT601IN_480I_60HZ = 0x804,
	/* Camera Formats */
	TVIN_SIG_FMT_CAMERA_640X480P_30HZ = 0x805,
	TVIN_SIG_FMT_CAMERA_800X600P_30HZ = 0x806,
	TVIN_SIG_FMT_CAMERA_1024X768P_30HZ = 0x807,
	TVIN_SIG_FMT_CAMERA_1920X1080P_30HZ = 0x808,
	TVIN_SIG_FMT_CAMERA_1280X720P_30HZ = 0x809,
	TVIN_SIG_FMT_BT601_MAX = 0x80a,
	TVIN_SIG_FMT_BT601_THRESHOLD = 0xa00,
	//upper use HDR Formats
	TVIN_SIG_FMT_RESERVED_THRESHOLD = 0xb00,
	TVIN_SIG_FMT_HDMI_HDR10 = 0xb01,
	TVIN_SIG_FMT_HDMI_HDR10PLUS = 0xb02,
	TVIN_SIG_FMT_HDMI_HLG = 0xb03,
	TVIN_SIG_FMT_HDMI_DOLBY = 0xb04,
	TVIN_SIG_FMT_HDMI_UPPER_FORMAT_MAX = 0xb20,
	TVIN_SIG_FMT_HDMI_UPPER_THRESHOLD = 0xc00,
	TVIN_SIG_FMT_MAX,
} tvin_sig_fmt_t;
#endif

#ifndef __SOURCE_INPUT_PARAM__
#define __SOURCE_INPUT_PARAM__
typedef struct source_input_param_s {
    int source_input;
    int sig_fmt;
    int trans_fmt;
} source_input_param_t;
#endif

#ifndef __TVIN_CUTWIN__
#define __TVIN_CUTWIN__
typedef struct tvin_cutwin_s {
    unsigned short hs;
    unsigned short he;
    unsigned short vs;
    unsigned short ve;
} tvin_cutwin_t;
#endif

#ifndef __NOLINE_PARAMS__
#define __NOLINE_PARAMS__
typedef struct noline_params_s {
    int osd0;
    int osd25;
    int osd50;
    int osd75;
    int osd100;
} noline_params_t;
#endif

#ifndef __NOLINE_PARAMS_TYPE__
#define __NOLINE_PARAMS_TYPE__
typedef enum noline_params_type_e {
    NOLINE_PARAMS_TYPE_BRIGHTNESS,
    NOLINE_PARAMS_TYPE_CONTRAST,
    NOLINE_PARAMS_TYPE_SATURATION,
    NOLINE_PARAMS_TYPE_HUE,
    NOLINE_PARAMS_TYPE_SHARPNESS,
    NOLINE_PARAMS_TYPE_VOLUME,
    NOLINE_PARAMS_TYPE_BACKLIGHT,
    NOLINE_PARAMS_TYPE_MAX,
} noline_params_type_t;
#endif

#ifndef __TVAPP_RGB_OGO__
#define __TVAPP_RGB_OGO__
typedef struct tvpq_rgb_ogo_s {
    unsigned int en;
    int r_pre_offset;
    int g_pre_offset;
    int b_pre_offset;
    unsigned int r_gain;
    unsigned int g_gain;
    unsigned int b_gain;
    int r_post_offset;
    int g_post_offset;
    int b_post_offset;
} tvpq_rgb_ogo_t;
#endif

#ifndef __VPP_COLOR_TEMPERATURE_MODE__
#define __VPP_COLOR_TEMPERATURE_MODE__
typedef enum vpp_color_temperature_mode_e {
    VPP_COLOR_TEMPERATURE_MODE_STANDARD,
    VPP_COLOR_TEMPERATURE_MODE_WARM,
    VPP_COLOR_TEMPERATURE_MODE_COLD,
    VPP_COLOR_TEMPERATURE_MODE_USER,
    VPP_COLOR_TEMPERATURE_MODE_MAX,
} vpp_color_temperature_mode_t;
#endif

#ifndef __DISPLAY_MODE_
#define __DISPLAY_MODE_
typedef enum vpp_display_mode_e {
    VPP_DISPLAY_MODE_169,
    VPP_DISPLAY_MODE_PERSON,
    VPP_DISPLAY_MODE_MOVIE,
    VPP_DISPLAY_MODE_CAPTION,
    VPP_DISPLAY_MODE_MODE43,
    VPP_DISPLAY_MODE_FULL,
    VPP_DISPLAY_MODE_NORMAL,
    VPP_DISPLAY_MODE_NOSCALEUP,
    VPP_DISPLAY_MODE_CROP_FULL,
    VPP_DISPLAY_MODE_CROP,
    VPP_DISPLAY_MODE_ZOOM,
    VPP_DISPLAY_MODE_MAX,
} vpp_display_mode_t;
#endif

#ifndef __SHARPNESS_TIMING__
#define __SHARPNESS_TIMING__
typedef enum sharpness_timing_e {
    SHARPNESS_TIMING_SD = 0,
    SHARPNESS_TIMING_HD,
} sharpness_timing_t;
#endif

#ifndef __PQ_TYPE__
#define __PQ_TYPE__
typedef enum pq_type_e {
    ALL,
    COLOR,
    SHARPNESS_SD,
    SHARPNESS_HD,
    _3D_NR_GAIN,
    _2D_NR_GAIN,
    DCI,
    GAMMA,
    FMD,
    CTI_SR0,
    CTI_SR1,
    DEJAGGY_SR0,
    DEJAGGY_SR1,
    DRT_SR0,
    DRT_SR1,
    BLACK_STRETCH,
} pq_type_t;
#endif

#ifndef __CMS_CM_PARAM__
#define __CMS_CM_PARAM__
typedef struct vpp_single_color_param_cm_s {
    int sat;
    int hue;
    int luma;
} vpp_single_color_param_cm_t;
#endif

#ifndef __CMS_3DLUT_PARAM__
#define __CMS_3DLUT_PARAM__
typedef struct vpp_single_color_param_3dlut_s {
    int red;
    int green;
    int blue;
} vpp_single_color_param_3dlut_t;
#endif

#ifndef __PQ_DB_VERSION_INFO__
#define __PQ_DB_VERSION_INFO__
typedef struct tvpq_databaseinfo_s {
    char version[32];
}tvpq_databaseinfo_t;
#endif

typedef struct _pq_mode_parameters {
    int backlight;
    int brightness;
    int contrast;
    int saturation;
    int sharpness;
    int gamma;
    int dynamic_backlight;
    int local_contrast;
    int dynamic_contrast;
    int super_resolution;
    int color_temperature;
    int hue;
    int eye_protection;
    int hdr_tone_mapping;
    int color_gamut;
    int display_mode;
    int noise_reduction;
    int MPEG_noise_reduction;
    int smooth_plus;
}pq_mode_parameters;

typedef enum _WB_GAMMA_MODE {
    WB_GAMMA_MODE_2POINT,
    WB_GAMMA_MODE_10POINT,
    WB_GAMMA_MODE_11POINT,
    WB_GAMMA_MODE_20POINT,
    WB_GAMMA_MODE_MAX,
} WB_GAMMA_MODE;

typedef struct _WB_GAMMA_TABLE_DATA {
    int R_OFFSET[MAX_WB_GAMMA_POINT];
    int G_OFFSET[MAX_WB_GAMMA_POINT];
    int B_OFFSET[MAX_WB_GAMMA_POINT];
} WB_GAMMA_TABLE_DATA;

typedef enum vpp_cms_color_e {
    VPP_COLOR_9_PURPLE = 0,
    VPP_COLOR_9_RED,
    VPP_COLOR_9_SKIN,
    VPP_COLOR_9_YELLOW,
    VPP_COLOR_9_YELLOW_GREEN,
    VPP_COLOR_9_GREEN,
    VPP_COLOR_9_BLUE_GREEN,
    VPP_COLOR_9_CYAN,
    VPP_COLOR_9_BLUE,
    VPP_COLOR_9_MAX,
} vpp_cms_color_t;

typedef struct vpp_cms_cm_param_e {
    vpp_single_color_param_cm_t data[VPP_COLOR_9_MAX];
} vpp_cms_cm_param_t;

typedef struct _all_pq_parameters {
    pq_mode_parameters PictureModeData;
    WB_GAMMA_TABLE_DATA WBGammaData;
    vpp_cms_cm_param_t color_tune;
}all_pq_parameters;

#endif
