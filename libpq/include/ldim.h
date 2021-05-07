/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __LDIM_H__
#define __LDIM_H__

struct vpu_ldim_param_s {
	/* beam model */
	int rgb_base;
	int boost_gain;
	int lpf_res;
	int fw_ld_th_sf; /* spatial filter threshold */
	/* beam curve */
	int ld_vgain;
	int ld_hgain;
	int ld_litgain;
	int ld_lut_vdg_lext;
	int ld_lut_hdg_lext;
	int ld_lut_vhk_lext;
	int ld_lut_hdg[32];
	int ld_lut_vdg[32];
	int ld_lut_vhk[32];
	/* beam shape minor adjustment */
	int ld_lut_vhk_pos[32];
	int ld_lut_vhk_neg[32];
	int ld_lut_hhk[32];
	int ld_lut_vho_pos[32];
	int ld_lut_vho_neg[32];
	/* remapping */
	int lit_idx_th;
	int comp_gain;
};
typedef enum ldim_lut_e {
    bl_remap_curve = 1000,
    LD_remap_LUT_0 = 1100,
    LD_remap_LUT_1,
    LD_remap_LUT_2,
    LD_remap_LUT_3,
    LD_remap_LUT_4,
    LD_remap_LUT_5,
    LD_remap_LUT_6,
    LD_remap_LUT_7,
    LD_remap_LUT_8,
    LD_remap_LUT_9,
    LD_remap_LUT_10,
    LD_remap_LUT_11,
    LD_remap_LUT_12,
    LD_remap_LUT_13,
    LD_remap_LUT_14,
    LD_remap_LUT_15,
    fw_ld_whist = 1200
} ldim_lut_t;

//////////for customer backlight local dimming
typedef struct aml_ldim_info_s {
    unsigned int func_en;
    unsigned int remapping_en;
    unsigned int alpha;
    unsigned int LPF_method;
    unsigned int lpf_gain;
    unsigned int lpf_res;
    unsigned int side_blk_diff_th;
    unsigned int bbd_th;
    unsigned int boost_gain;
    unsigned int rgb_base;
    unsigned int Ld_remap_bypass;
    unsigned int LD_TF_STEP_TH;
    unsigned int TF_BLK_FRESH_BL;
    unsigned int TF_FRESH_BL;
    unsigned int fw_LD_ThTF_l;
    unsigned int fw_rgb_diff_th;
    unsigned int fw_ld_thist;
    unsigned int bl_remap_curve[16];
    unsigned int Reg_LD_remap_LUT[16][32];
    unsigned int fw_ld_whist[16];

    //unsigned int RGBmapping_demo;
    //unsigned int frm_width;
    //unsigned int frm_height;
    //unsigned int Blk_Vnum;
    //unsigned int Blk_Hnum;
    //unsigned int Backlit_Mode;
    //unsigned int BL_matrix[LD_BLKREGNUM];
    //unsigned int SF_matrix[row][col];
    //unsigned int TF_step_method;
    //unsigned int fw_print_frequent;
    //unsigned int Dbprint_lv;
    //unsigned int Histgram[row * col][16];
    //unsigned int max_rgb[row * col][3];
    //unsigned int brightness_bypass;
    //unsigned int litgain;
}aml_ldim_info_t;

/* **********************************
 * LCD HDR info define
 * **********************************
 */
typedef struct lcd_optical_info_s {
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
} lcd_optical_info_t;

#endif
