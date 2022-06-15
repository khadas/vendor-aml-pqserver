/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __PQTYPE_H
#define __PQTYPE_H

#include "cm.h"
#include "ve.h"
#include "ldim.h"
#include "amstream.h"
#include "amvecm.h"
// ***************************************************************************
// *** enum definitions *********************************************
// ***************************************************************************

#define MAX_PICTUREMODE_PARAM_SIZE                100

typedef enum output_type_e {
    OUTPUT_TYPE_LVDS = -1,
    OUTPUT_TYPE_PAL,
    OUTPUT_TYPE_NTSC,
    OUTPUT_TYPE_HDMI_4K = 10,
    OUTPUT_TYPE_HDMI_HD_UPSCALE,
    OUTPUT_TYPE_HDMI_SD_UPSCALE,
    OUTPUT_TYPE_HDMI_NOSCALE,
    OUTPUT_TYPE_HDMI_SD_4096,
    OUTPUT_TYPE_HDMI_HD_4096,
    OUTPUT_TYPE_MAX,
} output_type_t;

typedef enum colortemperature_type_e {
    LVDS_COOL = 0,
    LVDS_STD,
    LVDS_WARM,
    LVDS_USER,
    COLORTEMPERATURE_TYPE_MAX,
} colortemperature_type_t;

typedef union tag_suc {
    short s;
    unsigned char c[2];
} SUC;

typedef union tag_usuc {
    unsigned short s;
    unsigned char c[2];
} USUC;

typedef enum is_3d_type_e {
    INDEX_3D_INVALID = -1,
    INDEX_2D = 0,
    INDEX_3D = 1,
} is_3d_type_t;

typedef enum window_mode_e {
    WINDOW_FULL = 0,
    WINDOW_PREVIEW = 1,
    WINDOW_LUNCHER = 2,
} window_mode_t;

typedef enum vpp_deblock_mode_e {
    VPP_DEBLOCK_MODE_OFF,
    VPP_DEBLOCK_MODE_LOW,
    VPP_DEBLOCK_MODE_MIDDLE,
    VPP_DEBLOCK_MODE_HIGH,
    VPP_DEBLOCK_MODE_AUTO,
} vpp_deblock_mode_t;

typedef enum vpp_DemoSquito_mode_e {
    VPP_DEMOSQUITO_MODE_OFF,
    VPP_DEMOSQUITO_MODE_LOW,
    VPP_DEMOSQUITO_MODE_MIDDLE,
    VPP_DEMOSQUITO_MODE_HIGH,
    VPP_DEMOSQUITO_MODE_AUTO,
} vpp_DemoSquito_mode_t;

typedef enum vpp_color_space_type_e {
    VPP_COLOR_SPACE_AUTO,
    VPP_COLOR_SPACE_YUV,
    VPP_COLOR_SPACE_RGB,
} vpp_color_space_type_t;

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

typedef enum vpp_color_management2_e {
    VPP_COLOR_MANAGEMENT2_MODE_OFF,
    VPP_COLOR_MANAGEMENT2_MODE_OPTIMIZE,
    VPP_COLOR_MANAGEMENT2_MODE_ENHANCE,
    VPP_COLOR_MANAGEMENT2_MODE_DEMO,
    VPP_COLOR_MANAGEMENT2_MODE_MAX,
} vpp_color_management2_t;

typedef enum vpp_noise_reduction_mode_e {
    VPP_NOISE_REDUCTION_MODE_OFF,
    VPP_NOISE_REDUCTION_MODE_LOW,
    VPP_NOISE_REDUCTION_MODE_MID,
    VPP_NOISE_REDUCTION_MODE_HIGH,
    VPP_NOISE_REDUCTION_MODE_AUTO,
    VPP_NOISE_REDUCTION_MODE_MAX,
} vpp_noise_reduction_mode_t;

typedef enum vpp_smooth_plus_mode_e {
    VPP_SMOOTH_PLUS_MODE_OFF,
    VPP_SMOOTH_PLUS_MODE_LOW,
    VPP_SMOOTH_PLUS_MODE_MID,
    VPP_SMOOTH_PLUS_MODE_HIGH,
    VPP_SMOOTH_PLUS_MODE_AUTO,
    VPP_SMOOTH_PLUS_MODE_MAX,
} vpp_smooth_plus_mode_t;

typedef enum vpp_xvycc_mode_e {
    VPP_XVYCC_MODE_OFF,
    VPP_XVYCC_MODE_STANDARD,
    VPP_XVYCC_MODE_ENHANCE,
    VPP_XVYCC_MODE_MAX,
} vpp_xvycc_mode_t;

typedef enum vpp_mcdi_mode_e {
    VPP_MCDI_MODE_OFF,
    VPP_MCDI_MODE_STANDARD,
    VPP_MCDI_MODE_ENHANCE,
    VPP_MCDI_MODE_MAX,
} vpp_mcdi_mode_t;

typedef enum vpp_color_temperature_mode_e {
    VPP_COLOR_TEMPERATURE_MODE_STANDARD,
    VPP_COLOR_TEMPERATURE_MODE_WARM,
    VPP_COLOR_TEMPERATURE_MODE_COLD,
    VPP_COLOR_TEMPERATURE_MODE_USER,
    VPP_COLOR_TEMPERATURE_MODE_MAX,
} vpp_color_temperature_mode_t;

typedef struct vpp_pq_para_s {
    int brightness;
    int contrast;
    int saturation;
    int hue;
    int sharpness;
    int backlight;
    int nr;
    int color_temperature;
    int dv_pqmode;
    int colorgamut_mode;
    int localcontrast;
    int dynamiccontrast;
    int cm_level;
} vpp_pq_para_t;

typedef struct vpp_pictur_mode_para_s {
    int Brightness;
    int Contrast;
    int Saturation;
    int Hue;
    int Sharpness;
    int Backlight;
    int Nr;
    int DynamicContrast;
    int ColorGamut;
    int ColorTemperature;
    int LocalContrast;
    int BlackStretch;
    int BlueStretch;
    int MpegNr;
    int ChromaCoring;
    int DolbyMode;
    int DolbyDarkDetail;
} vpp_pictur_mode_para_t;

typedef enum vpp_gamma_curve_e {
    VPP_GAMMA_CURVE_DEFAULT,//choose gamma table by value has been saved.
    VPP_GAMMA_CURVE_1,
    VPP_GAMMA_CURVE_2,
    VPP_GAMMA_CURVE_3,
    VPP_GAMMA_CURVE_4,
    VPP_GAMMA_CURVE_5,
    VPP_GAMMA_CURVE_6,
    VPP_GAMMA_CURVE_7,
    VPP_GAMMA_CURVE_8,
    VPP_GAMMA_CURVE_9,
    VPP_GAMMA_CURVE_10,
    VPP_GAMMA_CURVE_11,
    VPP_GAMMA_CURVE_MAX,
} vpp_gamma_curve_t;

typedef enum vpp_memc_mode_e {
    VPP_MEMC_MODE_OFF,
    VPP_MEMC_MODE_LOW,
    VPP_MEMC_MODE_MID,
    VPP_MEMC_MODE_HIGH,
    VPP_MEMC_MODE_MAX,
} vpp_memc_mode_t;

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

/* tvin signal format table */
typedef enum tvin_sig_fmt_e {
    TVIN_SIG_FMT_NULL = 0,
    //VGA Formats
    TVIN_SIG_FMT_VGA_512X384P_60HZ_D147             = 0x001,
    TVIN_SIG_FMT_VGA_560X384P_60HZ_D147             = 0x002,
    TVIN_SIG_FMT_VGA_640X200P_59HZ_D924             = 0x003,
    TVIN_SIG_FMT_VGA_640X350P_85HZ_D080             = 0x004,
    TVIN_SIG_FMT_VGA_640X400P_59HZ_D940             = 0x005,
    TVIN_SIG_FMT_VGA_640X400P_85HZ_D080             = 0x006,
    TVIN_SIG_FMT_VGA_640X400P_59HZ_D638             = 0x007,
    TVIN_SIG_FMT_VGA_640X400P_56HZ_D416             = 0x008,
    TVIN_SIG_FMT_VGA_640X480P_66HZ_D619             = 0x009,
    TVIN_SIG_FMT_VGA_640X480P_66HZ_D667             = 0x00a,
    TVIN_SIG_FMT_VGA_640X480P_59HZ_D940             = 0x00b,
    TVIN_SIG_FMT_VGA_640X480P_60HZ_D000             = 0x00c,
    TVIN_SIG_FMT_VGA_640X480P_72HZ_D809             = 0x00d,
    TVIN_SIG_FMT_VGA_640X480P_75HZ_D000_A           = 0x00e,
    TVIN_SIG_FMT_VGA_640X480P_85HZ_D008             = 0x00f,
    TVIN_SIG_FMT_VGA_640X480P_59HZ_D638             = 0x010,
    TVIN_SIG_FMT_VGA_640X480P_75HZ_D000_B           = 0x011,
    TVIN_SIG_FMT_VGA_640X870P_75HZ_D000             = 0x012,
    TVIN_SIG_FMT_VGA_720X350P_70HZ_D086             = 0x013,
    TVIN_SIG_FMT_VGA_720X400P_85HZ_D039             = 0x014,
    TVIN_SIG_FMT_VGA_720X400P_70HZ_D086             = 0x015,
    TVIN_SIG_FMT_VGA_720X400P_87HZ_D849             = 0x016,
    TVIN_SIG_FMT_VGA_720X400P_59HZ_D940             = 0x017,
    TVIN_SIG_FMT_VGA_720X480P_59HZ_D940             = 0x018,
    TVIN_SIG_FMT_VGA_768X480P_59HZ_D896             = 0x019,
    TVIN_SIG_FMT_VGA_800X600P_56HZ_D250             = 0x01a,
    TVIN_SIG_FMT_VGA_800X600P_60HZ_D000             = 0x01b,
    TVIN_SIG_FMT_VGA_800X600P_60HZ_D000_A           = 0x01c,
    TVIN_SIG_FMT_VGA_800X600P_60HZ_D317             = 0x01d,
    TVIN_SIG_FMT_VGA_800X600P_72HZ_D188             = 0x01e,
    TVIN_SIG_FMT_VGA_800X600P_75HZ_D000             = 0x01f,
    TVIN_SIG_FMT_VGA_800X600P_85HZ_D061             = 0x020,
    TVIN_SIG_FMT_VGA_832X624P_75HZ_D087             = 0x021,
    TVIN_SIG_FMT_VGA_848X480P_84HZ_D751             = 0x022,
    TVIN_SIG_FMT_VGA_960X600P_59HZ_D635             = 0x023,
    TVIN_SIG_FMT_VGA_1024X768P_59HZ_D278            = 0x024,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D000            = 0x025,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D000_A          = 0x026,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D000_B          = 0x027,
    TVIN_SIG_FMT_VGA_1024X768P_74HZ_D927            = 0x028,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D004            = 0x029,
    TVIN_SIG_FMT_VGA_1024X768P_70HZ_D069            = 0x02a,
    TVIN_SIG_FMT_VGA_1024X768P_75HZ_D029            = 0x02b,
    TVIN_SIG_FMT_VGA_1024X768P_84HZ_D997            = 0x02c,
    TVIN_SIG_FMT_VGA_1024X768P_74HZ_D925            = 0x02d,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D020            = 0x02e,
    TVIN_SIG_FMT_VGA_1024X768P_70HZ_D008            = 0x02f,
    TVIN_SIG_FMT_VGA_1024X768P_75HZ_D782            = 0x030,
    TVIN_SIG_FMT_VGA_1024X768P_77HZ_D069            = 0x031,
    TVIN_SIG_FMT_VGA_1024X768P_71HZ_D799            = 0x032,
    TVIN_SIG_FMT_VGA_1024X1024P_60HZ_D000           = 0x033,
    TVIN_SIG_FMT_VGA_1152X864P_60HZ_D000            = 0x034,
    TVIN_SIG_FMT_VGA_1152X864P_70HZ_D012            = 0x035,
    TVIN_SIG_FMT_VGA_1152X864P_75HZ_D000            = 0x036,
    TVIN_SIG_FMT_VGA_1152X864P_84HZ_D999            = 0x037,
    TVIN_SIG_FMT_VGA_1152X870P_75HZ_D062            = 0x038,
    TVIN_SIG_FMT_VGA_1152X900P_65HZ_D950            = 0x039,
    TVIN_SIG_FMT_VGA_1152X900P_66HZ_D004            = 0x03a,
    TVIN_SIG_FMT_VGA_1152X900P_76HZ_D047            = 0x03b,
    TVIN_SIG_FMT_VGA_1152X900P_76HZ_D149            = 0x03c,
    TVIN_SIG_FMT_VGA_1280X720P_59HZ_D855            = 0x03d,
    TVIN_SIG_FMT_VGA_1280X720P_60HZ_D000_A          = 0x03e,
    TVIN_SIG_FMT_VGA_1280X720P_60HZ_D000_B          = 0x03f,
    TVIN_SIG_FMT_VGA_1280X720P_60HZ_D000_C          = 0x040,
    TVIN_SIG_FMT_VGA_1280X720P_60HZ_D000_D          = 0x041,
    TVIN_SIG_FMT_VGA_1280X768P_59HZ_D870            = 0x042,
    TVIN_SIG_FMT_VGA_1280X768P_59HZ_D995            = 0x043,
    TVIN_SIG_FMT_VGA_1280X768P_60HZ_D100            = 0x044,
    TVIN_SIG_FMT_VGA_1280X768P_85HZ_D000            = 0x045,
    TVIN_SIG_FMT_VGA_1280X768P_74HZ_D893            = 0x046,
    TVIN_SIG_FMT_VGA_1280X768P_84HZ_D837            = 0x047,
    TVIN_SIG_FMT_VGA_1280X800P_59HZ_D810            = 0x048,
    TVIN_SIG_FMT_VGA_1280X800P_59HZ_D810_A          = 0x049,
    TVIN_SIG_FMT_VGA_1280X800P_60HZ_D000            = 0x04a,
    TVIN_SIG_FMT_VGA_1280X800P_85HZ_D000            = 0x04b,
    TVIN_SIG_FMT_VGA_1280X960P_60HZ_D000            = 0x04c,
    TVIN_SIG_FMT_VGA_1280X960P_60HZ_D000_A          = 0x04d,
    TVIN_SIG_FMT_VGA_1280X960P_75HZ_D000            = 0x04e,
    TVIN_SIG_FMT_VGA_1280X960P_85HZ_D002            = 0x04f,
    TVIN_SIG_FMT_VGA_1280X1024P_60HZ_D020           = 0x050,
    TVIN_SIG_FMT_VGA_1280X1024P_60HZ_D020_A         = 0x051,
    TVIN_SIG_FMT_VGA_1280X1024P_75HZ_D025           = 0x052,
    TVIN_SIG_FMT_VGA_1280X1024P_85HZ_D024           = 0x053,
    TVIN_SIG_FMT_VGA_1280X1024P_59HZ_D979           = 0x054,
    TVIN_SIG_FMT_VGA_1280X1024P_72HZ_D005           = 0x055,
    TVIN_SIG_FMT_VGA_1280X1024P_60HZ_D002           = 0x056,
    TVIN_SIG_FMT_VGA_1280X1024P_67HZ_D003           = 0x057,
    TVIN_SIG_FMT_VGA_1280X1024P_74HZ_D112           = 0x058,
    TVIN_SIG_FMT_VGA_1280X1024P_76HZ_D179           = 0x059,
    TVIN_SIG_FMT_VGA_1280X1024P_66HZ_D718           = 0x05a,
    TVIN_SIG_FMT_VGA_1280X1024P_66HZ_D677           = 0x05b,
    TVIN_SIG_FMT_VGA_1280X1024P_76HZ_D107           = 0x05c,
    TVIN_SIG_FMT_VGA_1280X1024P_59HZ_D996           = 0x05d,
    TVIN_SIG_FMT_VGA_1280X1024P_60HZ_D000           = 0x05e,
    TVIN_SIG_FMT_VGA_1360X768P_59HZ_D799            = 0x05f,
    TVIN_SIG_FMT_VGA_1360X768P_60HZ_D015            = 0x060,
    TVIN_SIG_FMT_VGA_1360X768P_60HZ_D015_A          = 0x061,
    TVIN_SIG_FMT_VGA_1360X850P_60HZ_D000            = 0x062,
    TVIN_SIG_FMT_VGA_1360X1024P_60HZ_D000           = 0x063,
    TVIN_SIG_FMT_VGA_1366X768P_59HZ_D790            = 0x064,
    TVIN_SIG_FMT_VGA_1366X768P_60HZ_D000            = 0x065,
    TVIN_SIG_FMT_VGA_1400X1050P_59HZ_D978           = 0x066,
    TVIN_SIG_FMT_VGA_1440X900P_59HZ_D887            = 0x067,
    TVIN_SIG_FMT_VGA_1440X1080P_60HZ_D000           = 0x068,
    TVIN_SIG_FMT_VGA_1600X900P_60HZ_D000            = 0x069,
    TVIN_SIG_FMT_VGA_1600X1024P_60HZ_D000           = 0x06a,
    TVIN_SIG_FMT_VGA_1600X1200P_59HZ_D869           = 0x06b,
    TVIN_SIG_FMT_VGA_1600X1200P_60HZ_D000           = 0x06c,
    TVIN_SIG_FMT_VGA_1600X1200P_65HZ_D000           = 0x06d,
    TVIN_SIG_FMT_VGA_1600X1200P_70HZ_D000           = 0x06e,
    TVIN_SIG_FMT_VGA_1680X1050P_59HZ_D954           = 0x06f,
    TVIN_SIG_FMT_VGA_1680X1080P_60HZ_D000           = 0x070,
    TVIN_SIG_FMT_VGA_1920X1080P_49HZ_D929           = 0x071,
    TVIN_SIG_FMT_VGA_1920X1080P_59HZ_D963_A         = 0x072,
    TVIN_SIG_FMT_VGA_1920X1080P_59HZ_D963           = 0x073,
    TVIN_SIG_FMT_VGA_1920X1080P_60HZ_D000           = 0x074,
    TVIN_SIG_FMT_VGA_1920X1200P_59HZ_D950           = 0x075,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D000_C          = 0x076,
    TVIN_SIG_FMT_VGA_1024X768P_60HZ_D000_D          = 0x077,
    TVIN_SIG_FMT_VGA_1920X1200P_59HZ_D988           = 0x078,
    TVIN_SIG_FMT_VGA_1400X900P_60HZ_D000            = 0x079,
    TVIN_SIG_FMT_VGA_1680X1050P_60HZ_D000           = 0x07a,
    TVIN_SIG_FMT_VGA_800X600P_60HZ_D062             = 0x07b,
    TVIN_SIG_FMT_VGA_800X600P_60HZ_317_B            = 0x07c,
    TVIN_SIG_FMT_VGA_RESERVE8                       = 0x07d,
    TVIN_SIG_FMT_VGA_RESERVE9                       = 0x07e,
    TVIN_SIG_FMT_VGA_RESERVE10                      = 0x07f,
    TVIN_SIG_FMT_VGA_RESERVE11                      = 0x080,
    TVIN_SIG_FMT_VGA_RESERVE12                      = 0x081,
    TVIN_SIG_FMT_VGA_MAX                            = 0x082,
    TVIN_SIG_FMT_VGA_THRESHOLD                      = 0x200,
    //Component Formats
    TVIN_SIG_FMT_COMP_480P_60HZ_D000                = 0x201,
    TVIN_SIG_FMT_COMP_480I_59HZ_D940                = 0x202,
    TVIN_SIG_FMT_COMP_576P_50HZ_D000                = 0x203,
    TVIN_SIG_FMT_COMP_576I_50HZ_D000                = 0x204,
    TVIN_SIG_FMT_COMP_720P_59HZ_D940                = 0x205,
    TVIN_SIG_FMT_COMP_720P_50HZ_D000                = 0x206,
    TVIN_SIG_FMT_COMP_1080P_23HZ_D976               = 0x207,
    TVIN_SIG_FMT_COMP_1080P_24HZ_D000               = 0x208,
    TVIN_SIG_FMT_COMP_1080P_25HZ_D000               = 0x209,
    TVIN_SIG_FMT_COMP_1080P_30HZ_D000               = 0x20a,
    TVIN_SIG_FMT_COMP_1080P_50HZ_D000               = 0x20b,
    TVIN_SIG_FMT_COMP_1080P_60HZ_D000               = 0x20c,
    TVIN_SIG_FMT_COMP_1080I_47HZ_D952               = 0x20d,
    TVIN_SIG_FMT_COMP_1080I_48HZ_D000               = 0x20e,
    TVIN_SIG_FMT_COMP_1080I_50HZ_D000_A             = 0x20f,
    TVIN_SIG_FMT_COMP_1080I_50HZ_D000_B             = 0x210,
    TVIN_SIG_FMT_COMP_1080I_50HZ_D000_C             = 0x211,
    TVIN_SIG_FMT_COMP_1080I_60HZ_D000               = 0x212,
    TVIN_SIG_FMT_COMP_MAX                           = 0x213,
    TVIN_SIG_FMT_COMP_THRESHOLD                     = 0x400,
    //HDMI Formats
    TVIN_SIG_FMT_HDMI_640X480P_60HZ                 = 0x401,
    TVIN_SIG_FMT_HDMI_720X480P_60HZ                 = 0x402,
    TVIN_SIG_FMT_HDMI_1280X720P_60HZ                = 0x403,
    TVIN_SIG_FMT_HDMI_1920X1080I_60HZ               = 0x404,
    TVIN_SIG_FMT_HDMI_1440X480I_60HZ                = 0x405,
    TVIN_SIG_FMT_HDMI_1440X240P_60HZ                = 0x406,
    TVIN_SIG_FMT_HDMI_2880X480I_60HZ                = 0x407,
    TVIN_SIG_FMT_HDMI_2880X240P_60HZ                = 0x408,
    TVIN_SIG_FMT_HDMI_1440X480P_60HZ                = 0x409,
    TVIN_SIG_FMT_HDMI_1920X1080P_60HZ               = 0x40a,
    TVIN_SIG_FMT_HDMI_720X576P_50HZ                 = 0x40b,
    TVIN_SIG_FMT_HDMI_1280X720P_50HZ                = 0x40c,
    TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_A             = 0x40d,
    TVIN_SIG_FMT_HDMI_1440X576I_50HZ                = 0x40e,
    TVIN_SIG_FMT_HDMI_1440X288P_50HZ                = 0x40f,
    TVIN_SIG_FMT_HDMI_2880X576I_50HZ                = 0x410,
    TVIN_SIG_FMT_HDMI_2880X288P_50HZ                = 0x411,
    TVIN_SIG_FMT_HDMI_1440X576P_50HZ                = 0x412,
    TVIN_SIG_FMT_HDMI_1920X1080P_50HZ               = 0x413,
    TVIN_SIG_FMT_HDMI_1920X1080P_24HZ               = 0x414,
    TVIN_SIG_FMT_HDMI_1920X1080P_25HZ               = 0x415,
    TVIN_SIG_FMT_HDMI_1920X1080P_30HZ               = 0x416,
    TVIN_SIG_FMT_HDMI_2880X480P_60HZ                = 0x417,
    TVIN_SIG_FMT_HDMI_2880X576P_60HZ                = 0x418,
    TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_B             = 0x419,
    TVIN_SIG_FMT_HDMI_1920X1080I_100HZ              = 0x41a,
    TVIN_SIG_FMT_HDMI_1280X720P_100HZ               = 0x41b,
    TVIN_SIG_FMT_HDMI_720X576P_100HZ                = 0x41c,
    TVIN_SIG_FMT_HDMI_1440X576I_100HZ               = 0x41d,
    TVIN_SIG_FMT_HDMI_1920X1080I_120HZ              = 0x41e,
    TVIN_SIG_FMT_HDMI_1280X720P_120HZ               = 0x41f,
    TVIN_SIG_FMT_HDMI_720X480P_120HZ                = 0x420,
    TVIN_SIG_FMT_HDMI_1440X480I_120HZ               = 0x421,
    TVIN_SIG_FMT_HDMI_720X576P_200HZ                = 0x422,
    TVIN_SIG_FMT_HDMI_1440X576I_200HZ               = 0x423,
    TVIN_SIG_FMT_HDMI_720X480P_240HZ                = 0x424,
    TVIN_SIG_FMT_HDMI_1440X480I_240HZ               = 0x425,
    TVIN_SIG_FMT_HDMI_1280X720P_24HZ                = 0x426,
    TVIN_SIG_FMT_HDMI_1280X720P_25HZ                = 0x427,
    TVIN_SIG_FMT_HDMI_1280X720P_30HZ                = 0x428,
    TVIN_SIG_FMT_HDMI_1920X1080P_120HZ              = 0x429,
    TVIN_SIG_FMT_HDMI_1920X1080P_100HZ              = 0x42a,
    TVIN_SIG_FMT_HDMI_1280X720P_60HZ_FRAME_PACKING  = 0x42b,
    TVIN_SIG_FMT_HDMI_1280X720P_50HZ_FRAME_PACKING  = 0x42c,
    TVIN_SIG_FMT_HDMI_1280X720P_24HZ_FRAME_PACKING  = 0x42d,
    TVIN_SIG_FMT_HDMI_1280X720P_30HZ_FRAME_PACKING  = 0x42e,
    TVIN_SIG_FMT_HDMI_1920X1080I_60HZ_FRAME_PACKING = 0x42f,
    TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_FRAME_PACKING = 0x430,
    TVIN_SIG_FMT_HDMI_1920X1080P_24HZ_FRAME_PACKING = 0x431,
    TVIN_SIG_FMT_HDMI_1920X1080P_30HZ_FRAME_PACKING = 0x432,
    TVIN_SIG_FMT_HDMI_800X600_00HZ                  = 0x433,
    TVIN_SIG_FMT_HDMI_1024X768_00HZ                 = 0x434,
    TVIN_SIG_FMT_HDMI_720X400_00HZ                  = 0x435,
    TVIN_SIG_FMT_HDMI_1280X768_00HZ                 = 0x436,
    TVIN_SIG_FMT_HDMI_1280X800_00HZ                 = 0x437,
    TVIN_SIG_FMT_HDMI_1280X960_00HZ                 = 0x438,
    TVIN_SIG_FMT_HDMI_1280X1024_00HZ                = 0x439,
    TVIN_SIG_FMT_HDMI_1360X768_00HZ                 = 0x43a,
    TVIN_SIG_FMT_HDMI_1366X768_00HZ                 = 0x43b,
    TVIN_SIG_FMT_HDMI_1600X1200_00HZ                = 0x43c,
    TVIN_SIG_FMT_HDMI_1920X1200_00HZ                = 0x43d,
    TVIN_SIG_FMT_HDMI_1440X900_00HZ                 = 0x43e,
    TVIN_SIG_FMT_HDMI_1400X1050_00HZ                = 0x43f,
    TVIN_SIG_FMT_HDMI_1680X1050_00HZ                = 0x440,
    /* for alternative and 4k2k */
    TVIN_SIG_FMT_HDMI_1920X1080I_60HZ_ALTERNATIVE   = 0x441,
    TVIN_SIG_FMT_HDMI_1920X1080I_50HZ_ALTERNATIVE   = 0x442,
    TVIN_SIG_FMT_HDMI_1920X1080P_24HZ_ALTERNATIVE   = 0x443,
    TVIN_SIG_FMT_HDMI_1920X1080P_30HZ_ALTERNATIVE   = 0x444,
    TVIN_SIG_FMT_HDMI_3840_2160_00HZ                = 0x445,
    TVIN_SIG_FMT_HDMI_4096_2160_00HZ                = 0x446,
    TVIN_SIG_FMT_HDMI_HDR                           = 0x447,
    TVIN_SIG_FMT_HDMI_RESERVE8                      = 0x448,
    TVIN_SIG_FMT_HDMI_RESERVE9                      = 0x449,
    TVIN_SIG_FMT_HDMI_RESERVE10                     = 0x44a,
    TVIN_SIG_FMT_HDMI_RESERVE11                     = 0x44b,
    TVIN_SIG_FMT_HDMI_720X480P_60HZ_FRAME_PACKING   = 0x44c,
    TVIN_SIG_FMT_HDMI_720X576P_50HZ_FRAME_PACKING   = 0x44d,
    TVIN_SIG_FMT_HDMI_640X480P_72HZ                 = 0x44e,
    TVIN_SIG_FMT_HDMI_640X480P_75HZ                 = 0x44f,
    TVIN_SIG_FMT_HDMI_1152X864P_75HZ                = 0x450,
    TVIN_SIG_FMT_HDMI_MAX                           = 0x451,
    TVIN_SIG_FMT_HDMI_THRESHOLD                     = 0x600,
    //Video Formats
    TVIN_SIG_FMT_CVBS_NTSC_M                        = 0x601,
    TVIN_SIG_FMT_CVBS_NTSC_443                      = 0x602,
    TVIN_SIG_FMT_CVBS_PAL_I                         = 0x603,
    TVIN_SIG_FMT_CVBS_PAL_M                         = 0x604,
    TVIN_SIG_FMT_CVBS_PAL_60                        = 0x605,
    TVIN_SIG_FMT_CVBS_PAL_CN                        = 0x606,
    TVIN_SIG_FMT_CVBS_SECAM                         = 0x607,
    TVIN_SIG_FMT_CVBS_NTSC_50                       = 0x608,
    TVIN_SIG_FMT_CVBS_MAX                           = 0x609,
    TVIN_SIG_FMT_CVBS_THRESHOLD                     = 0x800,
    //656 Formats
    TVIN_SIG_FMT_BT656IN_576I_50HZ                  = 0x801,
    TVIN_SIG_FMT_BT656IN_480I_60HZ                  = 0x802,
    //601 Formats
    TVIN_SIG_FMT_BT601IN_576I_50HZ                  = 0x803,
    TVIN_SIG_FMT_BT601IN_480I_60HZ                  = 0x804,
    //Camera Formats
    TVIN_SIG_FMT_CAMERA_640X480P_30HZ               = 0x805,
    TVIN_SIG_FMT_CAMERA_800X600P_30HZ               = 0x806,
    TVIN_SIG_FMT_CAMERA_1024X768P_30HZ              = 0x807,
    TVIN_SIG_FMT_CAMERA_1920X1080P_30HZ             = 0x808,
    TVIN_SIG_FMT_CAMERA_1280X720P_30HZ              = 0x809,
    TVIN_SIG_FMT_BT601_MAX                          = 0x80a,
    TVIN_SIG_FMT_BT601_THRESHOLD                    = 0xa00,
    //HDR Formats
    TVIN_SIG_FMT_HDMI_HDR10                         = 0x80b,
    TVIN_SIG_FMT_HDMI_HDR10PLUS                     = 0x80c,
    TVIN_SIG_FMT_HDMI_HLG                           = 0x80d,
    TVIN_SIG_FMT_HDMI_DOLBY                         = 0x80e,
    TVIN_SIG_FMT_MAX,
} tvin_sig_fmt_t;

typedef enum tvin_trans_fmt {
    TVIN_TFMT_2D = 0,
    TVIN_TFMT_3D_LRH_OLOR,  // Primary: Side-by-Side(Half) Odd/Left picture, Odd/Right p
    TVIN_TFMT_3D_LRH_OLER,  // Primary: Side-by-Side(Half) Odd/Left picture, Even/Right picture
    TVIN_TFMT_3D_LRH_ELOR,  // Primary: Side-by-Side(Half) Even/Left picture, Odd/Right picture
    TVIN_TFMT_3D_LRH_ELER,  // Primary: Side-by-Side(Half) Even/Left picture, Even/Right picture
    TVIN_TFMT_3D_TB,   // Primary: Top-and-Bottom
    TVIN_TFMT_3D_FP,   // Primary: Frame Packing
    TVIN_TFMT_3D_FA,   // Secondary: Field Alternative
    TVIN_TFMT_3D_LA,   // Secondary: Line Alternative
    TVIN_TFMT_3D_LRF,  // Secondary: Side-by-Side(Full)
    TVIN_TFMT_3D_LD,   // Secondary: L+depth
    TVIN_TFMT_3D_LDGD, // Secondary: L+depth+Graphics+Graphics-depth
    /* normal 3D format */
    TVIN_TFMT_3D_DET_TB,
    TVIN_TFMT_3D_DET_LR,
    TVIN_TFMT_3D_DET_INTERLACE,
    TVIN_TFMT_3D_DET_CHESSBOARD,
    TVIN_TFMT_3D_MAX,
} tvin_trans_fmt_t;

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
    SOURCE_ADTV,
    SOURCE_MAX,
} tv_source_input_t;

typedef enum vpp_picture_mode_offset_e {
   VPP_PICTURE_MODE_SDR_OFFSET       = 0,
   VPP_PICTURE_MODE_HDR10_OFFSET     = 20,
   VPP_PICTURE_MODE_HDR10PLUS_OFFSET = 40,
   VPP_PICTURE_MODE_HLG_OFFSET       = 60,
   VPP_PICTURE_MODE_DV_OFFSET        = 80,
} vpp_picture_mode_offset_t;

typedef enum vpp_picture_mode_e {
    VPP_PICTURE_MODE_STANDARD = 0,
    VPP_PICTURE_MODE_BRIGHT,
    VPP_PICTURE_MODE_SOFT,
    VPP_PICTURE_MODE_USER,
    VPP_PICTURE_MODE_MOVIE,
    VPP_PICTURE_MODE_COLORFUL,
    VPP_PICTURE_MODE_MONITOR,
    VPP_PICTURE_MODE_GAME,
    VPP_PICTURE_MODE_SPORTS,
    VPP_PICTURE_MODE_SONY,
    VPP_PICTURE_MODE_SAMSUNG,
    VPP_PICTURE_MODE_SHARP,
    VPP_PICTURE_MODE_DV_BRIGHT,
    VPP_PICTURE_MODE_DV_DARK,
    VPP_PICTURE_MODE_MAX,
} vpp_picture_mode_t;

typedef enum pq_sig_fmt_e {
    PQ_FMT_DEFAUT = 0,
    PQ_FMT_SDR,
    PQ_FMT_HDR,
    PQ_FMT_HDRP,
    PQ_FMT_HLG,
    PQ_FMT_DOBLY,
    PQ_FMT_MAX,
} pq_sig_fmt_t;

typedef struct pq_src_param_s {
    tv_source_input_t pq_source_input;
    pq_sig_fmt_t pq_sig_fmt;
} pq_src_param_t;

typedef enum tvpq_data_type_e {
    TVPQ_DATA_BRIGHTNESS,
    TVPQ_DATA_CONTRAST,
    TVPQ_DATA_SATURATION,
    TVPQ_DATA_HUE,
    TVPQ_DATA_SHARPNESS,
    TVPQ_DATA_VOLUME,

    TVPQ_DATA_MAX,
} tvpq_data_type_t;

typedef enum game_pc_mode_e
{
    MODE_OFF = 0,
    MODE_ON,
    MODE_STABLE,
} game_pc_mode_t;

typedef enum vpp_test_pattern_e {
    VPP_TEST_PATTERN_NONE,
    VPP_TEST_PATTERN_RED,
    VPP_TEST_PATTERN_GREEN,
    VPP_TEST_PATTERN_BLUE,
    VPP_TEST_PATTERN_WHITE,
    VPP_TEST_PATTERN_BLACK,
    VPP_TEST_PATTERN_MAX,
} vpp_test_pattern_t;

typedef enum vpp_color_demomode_e {
    VPP_COLOR_DEMO_MODE_ALLON,
    VPP_COLOR_DEMO_MODE_YOFF,
    VPP_COLOR_DEMO_MODE_COFF,
    VPP_COLOR_DEMO_MODE_GOFF,
    VPP_COLOR_DEMO_MODE_MOFF,
    VPP_COLOR_DEMO_MODE_ROFF,
    VPP_COLOR_DEMO_MODE_BOFF,
    VPP_COLOR_DEMO_MODE_RGBOFF,
    VPP_COLOR_DEMO_MODE_YMCOFF,
    VPP_COLOR_DEMO_MODE_ALLOFF,
    VPP_COLOR_DEMO_MODE_MAX,
} vpp_color_demomode_t;

typedef enum vpp_color_basemode_e {
    VPP_COLOR_BASE_MODE_OFF,
    VPP_COLOR_BASE_MODE_OPTIMIZE,
    VPP_COLOR_BASE_MODE_ENHANCE,
    VPP_COLOR_BASE_MODE_DEMO,
    VPP_COLOR_BASE_MODE_MAX,
} vpp_color_basemode_t;

typedef enum vpp_colorgamut_mode_e {
    VPP_COLORGAMUT_MODE_SRC,
    VPP_COLORGAMUT_MODE_AUTO,
    VPP_COLORGAMUT_MODE_NATIVE,
    VPP_COLORGAMUT_MODE_MAX,
} vpp_colorgamut_mode_t;

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

typedef enum SSM_status_e
{
    SSM_HEADER_INVALID = 0,
    SSM_HEADER_VALID = 1,
    SSM_HEADER_STRUCT_CHANGE = 2,
} SSM_status_t;

typedef enum Dynamic_contrast_mode_e
{
    DYNAMIC_CONTRAST_OFF = 0,
    DYNAMIC_CONTRAST_LOW,
    DYNAMIC_CONTRAST_MID,
    DYNAMIC_CONTRAST_HIGH,
} Dynamic_contrast_mode_t;

typedef enum Dynamic_backlight_status_e
{
    DYNAMIC_BACKLIGHT_OFF = 0,
    DYNAMIC_BACKLIGHT_LOW = 1,
    DYNAMIC_BACKLIGHT_HIGH = 2,
} Dynamic_backlight_status_t;

typedef enum local_contrast_mode_e
{
    LOCAL_CONTRAST_MODE_OFF = 0,
    LOCAL_CONTRAST_MODE_LOW,
    LOCAL_CONTRAST_MODE_MID,
    LOCAL_CONTRAST_MODE_HIGH,
    LOCAL_CONTRAST_MODE_MAX,
} local_contrast_mode_t;

typedef enum hdr_tmo_e
{
    HDR_TMO_OFF = 0,
    HDR_TMO_DYNAMIC,
    HDR_TMO_STATIC,
    HDR_TMO_MAX,
} hdr_tmo_t;

typedef struct noline_params_s {
    int osd0;
    int osd25;
    int osd50;
    int osd75;
    int osd100;
} noline_params_t;

typedef enum vpp_pq_level_e {
    VPP_PQ_LV_OFF,
    VPP_PQ_LV_LOW,
    VPP_PQ_LV_MID,
    VPP_PQ_LV_HIGH,
    VPP_PQ_LV_MAX,
} vpp_pq_level_t;

// ***************************************************************************
// *** struct definitions *********************************************
// ***************************************************************************
typedef struct source_input_param_s {
    tv_source_input_t source_input;
    tvin_sig_fmt_t sig_fmt;
    tvin_trans_fmt_t trans_fmt;
} source_input_param_t;

typedef struct tvin_cutwin_s {
    unsigned short hs;
    unsigned short he;
    unsigned short vs;
    unsigned short ve;
} tvin_cutwin_t;

typedef struct tvpq_data_s {
    int TotalNode;
    int NodeValue;
    int IndexValue;
    int RegValue;
    double step;
} tvpq_data_t;

typedef struct tvpq_sharpness_reg_s {
    int TotalNode;
    am_reg_t Value;
    int NodeValue;
    int IndexValue;
    double step;
} tvpq_sharpness_reg_t;

typedef struct tvpq_sharpness_regs_s {
    int length;
    tvpq_sharpness_reg_t reg_data[50];
} tvpq_sharpness_regs_t;

#define CC_PROJECT_INFO_ITEM_MAX_LEN  (64)

typedef struct tvpq_nonlinear_s {
    int osd0;
    int osd25;
    int osd50;
    int osd75;
    int osd100;
} tvpq_nonlinear_t;

typedef struct tvpq_databaseinfo_s {
    char ToolVersion[32];
    char ProjectVersion[32];
    char GenerateTime[32];
    char ChipVersion[32];
}tvpq_databaseinfo_t;

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
