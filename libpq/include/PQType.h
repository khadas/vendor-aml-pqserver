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

#include <linux/amlogic/amvecm_ext.h>
#include <linux/amlogic/tvin.h>
#include <linux/amlogic/lcd.h>
#include <linux/amlogic/frc.h>
#include <linux/amlogic/amdv_ioc.h>
#include <linux/amlogic/ldim.h>
#include "cm.h"
#include "ve.h"
#include "ldim.h"
#include "amvecm.h"


#define TVIN_SIG_FMT_HDMI_HDR             TVIN_SIG_FMT_HDMI_1600X900_60HZ

#define MAX_PQ_SRC_INDEX                  (18) //(PQ_SRC_MAX)
#define MAX_PQ_TIMMING_INDEX              (6)  //(PQ_FMT_MAX)

#define MAX_PICTUREMODE_INDEX             (14) //(VPP_PICTURE_MODE_MAX)
#define MAX_COLORTEMP_INDEX               (6)  //(VPP_COLOR_TEMPERATURE_MODE_MAX)

#define MAX_WB_GAMMA_POINT                (21)

#define MAX_PICTUREMODE_SIZE              ((1  * 4) + 4) // > sizeof(vpp_picture_mode_t) + sizeof(int)
#define MAX_PICTUREMODE_PARAM_SIZE        ((30 * 4) + 4) // > sizeof(vpp_pictur_mode_para_t) + sizeof(int)
#define MAX_COLORTEMP_PARAM_SIZE          ((10 * 4) + 4) // > sizeof(tcon_rgb_ogo_s) + sizeof(int)
#define MAX_WB_GAMMA_PARAM_SIZE           ((((MAX_WB_GAMMA_POINT * 3) + 1) * 4) + 4) // > (sizeof(WB_GAMMA_TABLE) + sizeof(int))


#define MAX_LVDS_SSC_PARAM_SIZE              20

#define MAX_COLORCUSTOMIZE_CM_PARAM_SIZE     120
#define MAX_COLORCUSTOMIZE_3DLUT_PARAM_SIZE  80

#define CMS_SAT_MIN  -100
#define CMS_SAT_MAX   127
#define CMS_HUE_MIN  -127
#define CMS_HUE_MAX   127
#define CMS_LUMA_MIN -15
#define CMS_LUMA_MAX  15

#define ALLM_MODE_HDMI           1 //"__u8 allm_mode" bit0=1 bit1=0
#define ALLM_MODE_DLBY           2 //"__u8 allm_mode" bit0=0 bit1=1
#define ALLM_MODE_HDMI_AND_DLBY  3 //"__u8 allm_mode" bit0=1 bit1=1

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

typedef enum vpp_demosquito_mode_e {
    VPP_DEMOSQUITO_MODE_OFF,
    VPP_DEMOSQUITO_MODE_LOW,
    VPP_DEMOSQUITO_MODE_MIDDLE,
    VPP_DEMOSQUITO_MODE_HIGH,
    VPP_DEMOSQUITO_MODE_AUTO,
} vpp_demosquito_mode_t;

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
    VPP_COLOR_TEMPERATURE_MODE_WARMER,
    VPP_COLOR_TEMPERATURE_MODE_COLDER,
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
    int SuperResolution;
    int GammaMidLuminance;
    int CmLevel;
    int HdrTmo;
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
    VPP_GAMMA_CURVE_BT1886,
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
    VPP_PICTURE_MODE_DOLVI_DARK,
    VPP_PICTURE_MODE_DOLVI_BRIGHT,
    VPP_PICTURE_MODE_AMDOLBY_IQ,
    VPP_PICTURE_MODE_MAX,
} vpp_picture_mode_t;

typedef struct _PICTURE_MODE_DEFAULT {
    int Picture;
    int Picture_DOLVI;
    int Picture_HDR;
    int Picture_HLG;
    int Picture_HDRP;
}PICTURE_MODE_DEFAULT;

typedef enum pq_source_input_e {
    PQ_SRC_DEFAULT = 0,
    PQ_SRC_TV,
    PQ_SRC_AV1,
    PQ_SRC_AV2,
    PQ_SRC_YPBPR1,
    PQ_SRC_YPBPR2,
    PQ_SRC_HDMI1,
    PQ_SRC_HDMI2,
    PQ_SRC_HDMI3,
    PQ_SRC_HDMI4,
    PQ_SRC_VGA,
    PQ_SRC_MPEG,
    PQ_SRC_DTV,
    PQ_SRC_SVIDEO,
    PQ_SRC_IPTV,
    PQ_SRC_DUMMY,
    PQ_SRC_SPDIF,
    PQ_SRC_ADTV,
    PQ_SRC_MAX,
} pq_source_input_t;

typedef enum pq_sig_fmt_e {
    PQ_FMT_DEFAULT = 0,
    PQ_FMT_SDR,
    PQ_FMT_HDR,
    PQ_FMT_HDRP,
    PQ_FMT_HLG,
    PQ_FMT_DOLVI,
    PQ_FMT_MAX,
} pq_sig_fmt_t;

typedef struct pq_src_param_s {
    pq_source_input_t pq_source_input;
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

typedef enum game_pc_mode_e {
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

typedef enum vpp_color_basemode_e {
    VPP_COLOR_BASE_MODE_OFF,
    VPP_COLOR_BASE_MODE_OPTIMIZE,
    VPP_COLOR_BASE_MODE_ENHANCE,
    VPP_COLOR_BASE_MODE_DEMO,
    VPP_COLOR_BASE_MODE_MAX,
} vpp_color_basemode_t;

typedef enum vpp_colorgamut_mode_e {
    VPP_COLORGAMUT_MODE_AUTO,
    VPP_COLORGAMUT_MODE_REC_709,
    VPP_COLORGAMUT_MODE_ADOBE_RGB,
    VPP_COLORGAMUT_MODE_DCI_P3,
    VPP_COLORGAMUT_MODE_BT2020,
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

typedef enum SSM_status_e {
    SSM_HEADER_INVALID = 0,
    SSM_HEADER_VALID = 1,
    SSM_HEADER_STRUCT_CHANGE = 2,
} SSM_status_t;

typedef enum Dynamic_contrast_mode_e {
    DYNAMIC_CONTRAST_OFF = 0,
    DYNAMIC_CONTRAST_LOW,
    DYNAMIC_CONTRAST_MID,
    DYNAMIC_CONTRAST_HIGH,
} Dynamic_contrast_mode_t;

typedef enum Dynamic_backlight_status_e {
    DYNAMIC_BACKLIGHT_OFF = 0,
    DYNAMIC_BACKLIGHT_LOW = 1,
    DYNAMIC_BACKLIGHT_HIGH = 2,
} Dynamic_backlight_status_t;

typedef enum local_contrast_mode_e {
    LOCAL_CONTRAST_MODE_OFF = 0,
    LOCAL_CONTRAST_MODE_LOW,
    LOCAL_CONTRAST_MODE_MID,
    LOCAL_CONTRAST_MODE_HIGH,
    LOCAL_CONTRAST_MODE_MAX,
} local_contrast_mode_t;

typedef enum hdr_tmo_e {
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

typedef enum CTI_param_type_e {
    CVD_YC_DELAY = 0,
    DECODE_CTI,
    SR0_CTI_GAIN0,
    SR0_CTI_GAIN1,
    SR0_CTI_GAIN2,
    SR0_CTI_GAIN3,
    SR1_CTI_GAIN0,
    SR1_CTI_GAIN1,
    SR1_CTI_GAIN2,
    SR1_CTI_GAIN3,
} CTI_param_type_t;

typedef enum decode_luma_e {
    VIDEO_DECODE_BRIGHTNESS,
    VIDEO_DECODE_CONTRAST,
    VIDEO_DECODE_SATURATION,
} decode_luma_t;

typedef struct source_input_param_s {
    tv_source_input_t source_input;
    enum tvin_sig_fmt_e sig_fmt;
    enum tvin_trans_fmt trans_fmt;
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
    struct am_reg_s Value;
    int NodeValue;
    int IndexValue;
    double step;
} tvpq_sharpness_reg_t;

typedef struct tvpq_sharpness_regs_s {
    int length;
    tvpq_sharpness_reg_t reg_data[50];
} tvpq_sharpness_regs_t;

typedef struct tvpq_nonlinear_s {
    int osd0;
    int osd25;
    int osd50;
    int osd75;
    int osd100;
} tvpq_nonlinear_t;

typedef struct tvpq_databaseinfo_s {
    char version[32];
} tvpq_databaseinfo_t;

typedef enum db_version_type_e {
    DB_VER_TYPE_TOOL_VER = 0,
    DB_VER_TYPE_PROJECT_VER,
    DB_VER_TYPE_GENERATE_TIME,
    DB_VER_TYPE_CHIP_VER,
    DB_VER_TYPE_DB_VER,
} db_version_type_t;

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

typedef struct Tconbin_Header_s {
    unsigned int data_check;
    unsigned int ram_data_check;
    unsigned int block_size;
    unsigned short header_size;
    unsigned short ext_header_size;
    unsigned short block_type;
    unsigned short block_ctrl;
    unsigned int block_flag;
    unsigned short init_priority;
    unsigned short chip_id;
    unsigned char name[36];
} Tconbin_Header_t;

typedef enum LD_bin_table_index_e {
    LD_BIN_BL_MAPPING = 0,
    LD_BIN_BL_PROFILE,
    LD_BIN_BL_MAX,
} LD_bin_table_index_t;

typedef enum sharpness_param_type_e {
    H_GAIN_HIGH,
    H_GAIN_LOW,
    V_GAIN_HIGH,
    V_GAIN_LOW,
    D_GAIN_HIGH,
    D_GAIN_LOW,
    HP_DIAG_CORE,
    BP_DIAG_CORE,
    PKGAIN_VSLUMALUT7,
    PKGAIN_VSLUMALUT6,
    PKGAIN_VSLUMALUT5 = 10,
    PKGAIN_VSLUMALUT4,
    PKGAIN_VSLUMALUT3,
    PKGAIN_VSLUMALUT2,
    PKGAIN_VSLUMALUT1,
    PKGAIN_VSLUMALUT0,
} sharpness_param_type_t;

typedef enum sharpness_timing_e {
    SHARPNESS_TIMING_SD = 0,
    SHARPNESS_TIMING_HD,
} sharpness_timing_t;

typedef enum vpp_cms_6color_e {
    VPP_COLOR_6_RED = 0,
    VPP_COLOR_6_GREEN,
    VPP_COLOR_6_BLUE,
    VPP_COLOR_6_CYAN,
    VPP_COLOR_6_MAGENTA,
    VPP_COLOR_6_YELLOW,
    VPP_COLOR_6_MAX,
} vpp_cms_6color_t;

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

typedef enum vpp_cms_14color_e {
    VPP_COLOR_14_BLUE_PURPLE = 0,
    VPP_COLOR_14_PURPLE,
    VPP_COLOR_14_PURPLE_RED,
    VPP_COLOR_14_RED,
    VPP_COLOR_14_SKIN_CHEEKS,
    VPP_COLOR_14_SKIN_HAIR_CHEEKS,
    VPP_COLOR_14_SKIN_YELLOW,
    VPP_COLOR_14_YELLOW,
    VPP_COLOR_14_YELLOW_GREEN,
    VPP_COLOR_14_GREEN,
    VPP_COLOR_14_GREEN_CYAN,
    VPP_COLOR_14_CYAN,
    VPP_COLOR_14_CYAN_BLUE,
    VPP_COLOR_14_BLUE,
    VPP_COLOR_14_MAX,
} vpp_cms_14color_t;

typedef enum vpp_cms_type_e {
    VPP_CMS_TYPE_SAT = 0, //CM
    VPP_CMS_TYPE_HUE,
    VPP_CMS_TYPE_LUMA,

    VPP_CMS_TYPE_RED = 3, //3DLUT
    VPP_CMS_TYPE_GREEN,
    VPP_CMS_TYPE_BLUE,

    VPP_CMS_TYPE_MAX,
} vpp_cms_type_t;

typedef struct vpp_single_color_param_cm_s {
    int sat;
    int hue;
    int luma;
} vpp_single_color_param_cm_t;

typedef struct vpp_single_color_3dlut_param_s {
    int red;
    int green;
    int blue;
} vpp_single_color_param_3dlut_t;

typedef struct vpp_cms_cm_param_e {
    vpp_single_color_param_cm_t data[VPP_COLOR_9_MAX];
} vpp_cms_cm_param_t;

typedef struct vpp_cms_3dlut_param_e {
    vpp_single_color_param_3dlut_t data[VPP_COLOR_6_MAX];
} vpp_cms_3dlut_param_t;

typedef enum vpp_cms_method_e {
    VPP_CMS_METHOD_CM,
    VPP_CMS_METHOD_3DLUT,
    VPP_CMS_METHOD_MAX,
} vpp_cms_method_t;

typedef enum _WB_GAMMA_DIAGRAM {
    PERCENT_0   = 0,
    PERCENT_5   = 13,
    PERCENT_10  = 26,
    PERCENT_15  = 38,
    PERCENT_20  = 51,
    PERCENT_25  = 64,
    PERCENT_30  = 76,
    PERCENT_35  = 90,
    PERCENT_40  = 102,
    PERCENT_45  = 115,
    PERCENT_50  = 128,
    PERCENT_55  = 141,
    PERCENT_60  = 154,
    PERCENT_65  = 166,
    PERCENT_70  = 179,
    PERCENT_75  = 192,
    PERCENT_80  = 204,
    PERCENT_85  = 218,
    PERCENT_90  = 230,
    PERCENT_95  = 243,
    PERCENT_100 = 255,
    PERCENT_100_257 = 256,
    PERCENT_MAX,
} WB_GAMMA_DIAGRAM;

typedef enum _WB_GAMMA_MODE {
    WB_GAMMA_MODE_2POINT,
    WB_GAMMA_MODE_10POINT,
    WB_GAMMA_MODE_11POINT,
    WB_GAMMA_MODE_20POINT,
    WB_GAMMA_MODE_MAX,
} WB_GAMMA_MODE;

typedef struct _WB_GAMMA_TABLE {
    int ENABLE;
    int R_OFFSET[MAX_WB_GAMMA_POINT];
    int G_OFFSET[MAX_WB_GAMMA_POINT];
    int B_OFFSET[MAX_WB_GAMMA_POINT];
} WB_GAMMA_TABLE;

typedef enum _CHANNEL_TYPE {
    RED_CH = 0,
    GREEN_CH,
    BLUE_CH,
    MAX_CH,
} CHANNEL_TYPE;

typedef enum vpp_SuperResolution_Level_e {
    VPP_SUPER_RESOLUTION_OFF,
    VPP_SUPER_RESOLUTION_MID,
    VPP_SUPER_RESOLUTION_HIGHT,
    VPP_SUPER_RESOLUTION_MAX,
} vpp_SuperResolution_Level_t;

typedef enum pq_mode_to_game_e {
    TO_GAME_BY_SIG_CHG_DV_ALLM,
    TO_GAME_BY_SIG_CHG_STS_DIFF_SIG,
    TO_GAME_BY_SIG_CHG_STS_SAME_SIG,
    TO_GAME_BY_MAX,
} pq_mode_to_game_t;

typedef enum pq_wb_rgb_data_path_e {
    WB_RGB_DATA_PATH_NULL = 0,
    WB_RGB_DATA_PATH_CRI,
    WB_RGB_DATA_PATH_SSM,
} pq_wb_rgb_data_path_t;

typedef enum pq_film_maker_mode_e {
    FILM_MAKER_MODE_DISABLE = 0,
    FILM_MAKER_MODE_AUTO,
    FILM_MAKER_MODE_MAX,
} pq_film_maker_mode_t;

#endif
