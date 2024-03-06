/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __VE_H
#define __VE_H

#define CABC_O_BL_CV_MAX       15
#define CABC_MAXBIN_BL_CV_MAX  15

#define CAAD_SENSOR_INPUT_MAX  3
#define CAAD_LUT_Y_GAIN_MAX    20
#define CAAD_LUT_RG_GAIN_MAX   20
#define CAAD_LUT_BG_GAIN_MAX   20
#define CAAD_GAIN_LUT_MAX      50
#define CAAD_XY_LUT_MAX        40

typedef enum hdr_lut_e {
    HDR_oo_init_lut = 35,
    HDR_lut_res,
} hdr_lut_t;

typedef enum dnlp_curve_e {
    scurv_low = 1000,
    scurv_mid1,
    scurv_mid2,
    scurv_hgh1,
    scurv_hgh2,
    curv_var_lut49,
    curv_wext_gain,
    adp_thrd = 1013,
    reg_blk_boost_12,
    reg_adp_ofset_20,
    reg_mono_protect,
    reg_trend_wht_expand_lut8,
    c_hist_gain = 1019,
    s_hist_gain,
} dnlp_curve_t;

typedef enum lc_curve_parm_num_e {
    LC_SATURATION_NUM = 1,
    LC_YMINVAL_LMT_NUM = 2,
    LC_YPKBV_YMAXVAL_LMT_NUM = 4,
    LC_YPKBV_RATIO_NUM = 8,
    LC_YMAXVAL_LMT_NUM = 16,
    LC_YPKBV_LMT_NUM = 32,
} lc_curve_parm_num_t;

typedef struct cabc_param_s {
    int cabc_param_cabc_en;
    int cabc_param_hist_mode;
    int cabc_param_tf_en;
    int cabc_param_sc_flag;
    int cabc_param_bl_map_mode;
    int cabc_param_bl_map_en;
    int cabc_param_temp_proc;
    int cabc_param_max95_ratio;
    int cabc_param_hist_blend_alpha;
    int cabc_param_init_bl_min;
    int cabc_param_init_bl_max;
    int cabc_param_tf_alpha;
    int cabc_param_sc_hist_diff_thd;
    int cabc_param_sc_apl_diff_thd;
    int cabc_param_patch_bl_th;
    int cabc_param_patch_on_alpha;
    int cabc_param_patch_bl_off_th;
    int cabc_param_patch_off_alpha;
    int cabc_param_o_bl_cv_len;
    int cabc_param_o_bl_cv[CABC_O_BL_CV_MAX];
    int cabc_param_maxbin_bl_cv_len;
    int cabc_param_maxbin_bl_cv[CABC_MAXBIN_BL_CV_MAX];
} cabc_param_t;

typedef enum aad_curve_e {
    LUT_Y_gain = 400,
    LUT_RG_gain,
    LUT_BG_gain,
    gain_lut,
    xy_lut,
    sensor_input,
} add_curve_t;

typedef struct aad_param_s {
    int aad_param_cabc_aad_en;
    int aad_param_aad_en;
    int aad_param_tf_en;
    int aad_param_force_gain_en;
    int aad_param_sensor_mode;
    int aad_param_mode;
    int aad_param_dist_mode;
    int aad_param_tf_alpha;
    int aad_param_sensor_input_len;
    int aad_param_sensor_input[CAAD_SENSOR_INPUT_MAX];
    int aad_param_LUT_Y_gain_len;
    int aad_param_LUT_Y_gain[CAAD_LUT_Y_GAIN_MAX];
    int aad_param_LUT_RG_gain_len;
    int aad_param_LUT_RG_gain[CAAD_LUT_RG_GAIN_MAX];
    int aad_param_LUT_BG_gain_len;
    int aad_param_LUT_BG_gain[CAAD_LUT_BG_GAIN_MAX];
    int aad_param_gain_lut_len;
    int aad_param_gain_lut[CAAD_GAIN_LUT_MAX];
    int aad_param_xy_lut_len;
    int aad_param_xy_lut[CAAD_XY_LUT_MAX];
} aad_param_t;

typedef struct color_primary_t {
    unsigned int src[8];
    unsigned int dest[8];
} color_primary_t;

typedef struct _PANLE_INFO {
    unsigned int hdr_support;
    unsigned int features;
    unsigned int primaries_r_x;
    unsigned int primaries_r_y;
    unsigned int primaries_g_x;
    unsigned int primaries_g_y;
    unsigned int primaries_b_x;
    unsigned int primaries_b_y;
    unsigned int white_point_x;
    unsigned int white_point_y;
    unsigned int luma_max;
    unsigned int luma_min;
    unsigned int luma_avg;
} PANLE_INFO;

#endif  // _VE_H
