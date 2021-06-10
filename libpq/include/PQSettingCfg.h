#ifndef __PQ_SETTING_CFG__H__
#define __PQ_SETTING_CFG__H__

enum{
    CHKSUM_PROJECT_ID_OFFSET = 0,
    CHKSUM_MAC_ADDRESS_OFFSET = 1,
    CHKSUM_HDCP_KEY_OFFSET = 2,
    CHKSUM_BARCODE_OFFSET = 3,
    SSM_RSV_W_CHARACTER_CHAR_START = 10,
    SSM_CR_START = 11,
    SSM_MARK_01_START = 12,
    SSM_MARK_02_START = 13,
    SSM_MARK_03_START = 14,
    SSM_RSV0 = 20,
    SSM_RW_FBMF_START = 21,
    SSM_RW_DEF_HDCP_START = 22,
    SSM_RW_POWER_CHANNEL_START = 23,
    SSM_RW_LAST_SOURCE_INPUT_START = 24,
    SSM_RW_SYS_LANGUAGE_START = 25,
    SSM_RW_AGING_MODE_START = 26,
    SSM_RW_PANEL_TYPE_START = 27,
    SSM_RW_POWER_ON_MUSIC_SWITCH_START = 28,
    SSM_RW_POWER_ON_MUSIC_VOL_START = 29,
    SSM_RW_SYS_SLEEP_TIMER_START = 30,
    SSM_RW_INPUT_SRC_PARENTAL_CTL_START = 31,
    SSM_RW_PARENTAL_CTL_SWITCH_START = 32,
    SSM_RW_PARENTAL_CTL_PASSWORD_START = 33,
    SSM_RW_SEARCH_NAVIGATE_FLAG_START = 34,
    SSM_RW_INPUT_NUMBER_LIMIT_START = 35,
    SSM_RW_SERIAL_ONOFF_FLAG_START = 36,
    SSM_RW_STANDBY_MODE_FLAG_START = 37,
    SSM_RW_HDMIEQ_MODE_START = 38,
    SSM_RW_LOGO_ON_OFF_FLAG_START = 39,
    SSM_RW_HDMIINTERNAL_MODE_START = 40,
    SSM_RW_DISABLE_3D_START = 41,
    SSM_RW_GLOBAL_OGO_ENABLE_START = 42,
    SSM_RW_LOCAL_DIMING_START = 43,
    SSM_RW_VDAC_2D_START = 44,
    SSM_RW_VDAC_3D_START = 45,
    SSM_RW_NON_STANDARD_START = 46,
    SSM_RW_ADB_SWITCH_START = 47,
    SSM_RW_SERIAL_CMD_SWITCH_START = 48,
    SSM_RW_CA_BUFFER_SIZE_START = 49,
    SSM_RW_NOISE_GATE_THRESHOLD_START = 50,
    SSM_RW_DTV_TYPE_START = 51,
    SSM_RW_UI_GRHPHY_BACKLIGHT_START = 52,
    SSM_RW_FASTSUSPEND_FLAG_START = 53,
    SSM_RW_BLACKOUT_ENABLE_START = 54,
    SSM_RW_PANEL_ID_START = 55,
    SSM_RSV_1 = 80,
    SSM_RSV_2 = 150,
    VPP_DATA_POS_COLOR_DEMO_MODE_START = 151,
    VPP_DATA_POS_COLOR_BASE_MODE_START = 152,
    VPP_DATA_POS_TEST_PATTERN_START = 153,
    VPP_DATA_POS_DDR_SSC_START = 154,
    VPP_DATA_POS_LVDS_SSC_START = 155,
    VPP_DATA_POS_DREAM_PANEL_START = 156,
    VPP_DATA_POS_BACKLIGHT_REVERSE_START = 157,
    VPP_DATA_POS_BRIGHTNESS_START = 158,
    VPP_DATA_POS_CONTRAST_START = 159,
    VPP_DATA_POS_SATURATION_START = 160,
    VPP_DATA_POS_HUE_START = 161,
    VPP_DATA_POS_SHARPNESS_START = 162,
    VPP_DATA_POS_COLOR_TEMPERATURE_START = 163,
    VPP_DATA_POS_NOISE_REDUCTION_START = 164,
    VPP_DATA_POS_SCENE_MODE_START = 165,
    VPP_DATA_POS_PICTURE_MODE_START = 166,
    VPP_DATA_POS_DISPLAY_MODE_START = 167,
    VPP_DATA_POS_BACKLIGHT_START = 168,
    VPP_DATA_POS_RGB_GAIN_R_START = 169,
    VPP_DATA_POS_RGB_GAIN_G_START = 170,
    VPP_DATA_POS_RGB_GAIN_B_START = 171,
    VPP_DATA_POS_RGB_POST_OFFSET_R_START = 172,
    VPP_DATA_POS_RGB_POST_OFFSET_G_START = 173,
    VPP_DATA_POS_RGB_POST_OFFSET_B_START = 174,
    VPP_DATA_POS_DBC_START = 175,
    VPP_DATA_PROJECT_ID_START = 176,
    VPP_DATA_POS_DNLP_START = 177,
    VPP_DATA_POS_PANORAMA_START = 178,
    VPP_DATA_APL_START = 179,
    VPP_DATA_APL2_START = 180,
    VPP_DATA_BD_START = 181,
    VPP_DATA_BP_START = 182,
    VPP_DATA_RGB_START = 183,
    VPP_DATA_COLOR_SPACE_START = 184,
    VPP_DATA_USER_NATURE_SWITCH_START = 185,
    VPP_DATA_GAMMA_VALUE_START = 186,
    VPP_DATA_DBC_BACKLIGHT_START = 187,
    VPP_DATA_DBC_STANDARD_START = 188,
    VPP_DATA_DBC_ENABLE_START = 189,
    VPP_DATA_POS_FBC_BACKLIGHT_START = 190,
    VPP_DATA_POS_FBC_ELECMODE_START = 191,
    VPP_DATA_POS_FBC_COLORTEMP_START = 192,
    VPP_DATA_POS_FBC_N310_BACKLIGHT_START = 193,
    VPP_DATA_POS_FBC_N310_COLORTEMP_START = 194,
    VPP_DATA_POS_FBC_N310_LIGHTSENSOR_START = 195,
    VPP_DATA_POS_FBC_N310_MEMC_START = 196,
    VPP_DATA_POS_FBC_N310_DREAMPANEL_START = 197,
    VPP_DATA_POS_FBC_N310_MULTI_PQ_START = 198,
    VPP_DATA_POS_N311_VBYONE_SPREAD_SPECTRUM_START = 199,
    VPP_DATA_POS_N311_BLUETOOTH_VAL_START = 200,
    VPP_DATA_EYE_PROTECTION_MODE_START = 201,
    VPP_DATA_POS_COLORGAMUT_START = 202,
    SSM_RSV3 = 250,
    TVIN_DATA_POS_SOURCE_INPUT_START = 251,
    TVIN_DATA_CVBS_STD_START = 252,
    TVIN_DATA_POS_3D_MODE_START = 253,
    TVIN_DATA_POS_3D_LRSWITCH_START = 254,
    TVIN_DATA_POS_3D_DEPTH_START = 255,
    TVIN_DATA_POS_3D_TO2D_START = 256,
    TVIN_DATA_POS_3D_TO2DNEW_START = 257,
    SSM_RSV4 = 280,
    CUSTOMER_DATA_POS_HDMI1_EDID_START = 281,
    CUSTOMER_DATA_POS_HDMI2_EDID_START = 282,
    CUSTOMER_DATA_POS_HDMI3_EDID_START = 283,
    CUSTOMER_DATA_POS_HDMI4_EDID_START = 284,
    CUSTOMER_DATA_POS_HDMI_HDCP_SWITCHER_START = 285,
    CUSTOMER_DATA_POS_HDMI_COLOR_RANGE_START = 287,
    CUSTOMER_DATA_POS_DYNAMIC_BACKLIGHT = 288,
    CUSTOMER_DATA_POS_EDGE_ENHANCER = 289,
    CUSTOMER_DATA_POS_MPEG_NOISE_REDUCTION = 290,
    CUSTOMER_DATA_POS_DYNAMIC_CONTRAST = 291,
    CUSTOMER_DATA_POS_AUTO_ASPECT = 292,
    CUSTOMER_DATA_POS_43_STRETCH = 293,
    VPP_DATA_DNLP_MODE_START = 294,
    VPP_DATA_DNLP_GAIN_START = 295,
    VPP_DATA_POS_LOCAL_CONTRAST_MODE_START = 296,
    VPP_DATA_DOLBY_MODE_START = 297,
    VPP_DATA_POS_SDR_PICTURE_MODE_START = 298,
    VPP_DATA_POS_HDR10_PICTURE_MODE_START = 299,
    VPP_DATA_POS_HDR10PLUS_PICTURE_MODE_START = 300,
    VPP_DATA_POS_HLG_PICTURE_MODE_START = 301,
    VPP_DATA_POS_DV_PICTURE_MODE_START = 302,
    VPP_DATA_POS_LAST_PICTURE_MODE_START = 303,
};
#endif
