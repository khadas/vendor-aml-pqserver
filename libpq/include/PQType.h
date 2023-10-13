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


#define MAX_PICTUREMODE_PARAM_SIZE        100
#define MAX_LVDS_SSC_PARAM_SIZE           20

#define TVIN_SIG_FMT_HDMI_HDR             TVIN_SIG_FMT_HDMI_1600X900_60HZ

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
    PQ_FMT_DEFAULT = 0,
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

#endif
