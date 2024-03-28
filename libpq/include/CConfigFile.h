/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_


#define DV_BIN_FILE_DEFAULT_PATH               "/vendor/etc/tvconfig/panel/dv_config.bin"
#define DV_CFG_FILE_DEFAULT_PATH               "/vendor/etc/tvconfig/panel/Amlogic_dv.cfg"

//for pq config
#define CFG_SECTION_PQ                          "PQ"
#define CFG_PQ_DB_CONFIG_PATH                   "pq_db_config_path"
#define CFG_PQ_SETTINGDATA_FILE_PATH            "pq_uiSettingDataFile_path"
#define CFG_PQ_WHITEBALANCE_FILE_PATH           "pq_FactorywhiteBalance_path"
#define CFG_PQ_DV_BIN_PATH                      "pq_dv_bin_path"
#define CFG_PQ_DV_CFG_PATH                      "pq_dv_cfg_path"

#define CFG_BIG_SMALL_DB_ENABLE                 "pq.BigSmallDb.en"
#define CFG_ALL_PQ_MODULE_ENABLE                "pq.AllPQModule.en"
#define CFG_PQ_MODE_CHECK_SOURCE_ENABLE         "pq.pqmodeCheckSource.en"
#define CFG_TVHAL_ENABLE                        "pq.tvhal.en"
#define CFG_BLACK_BLUE_CHROMA_DB_ENABLE         "pq.BlackBlueChromaDb.en"

//di module config
#define CFG_DI_ENABLE                           "pq.di.en"
#define CFG_MCDI_ENABLE                         "pq.mcdi.en"
#define CFG_DEBLOCK_ENABLE                      "pq.deblock.en"
#define CFG_NOISEREDUCTION_ENABLE               "pq.NoiseReduction.en"
#define CFG_DEMOSQUITO_ENABLE                   "pq.DemoSquito.en"
#define CFG_SMOOTHPLUS_ENABLE                   "pq.SmoothPlus.en"
//amvecm module config
#define CFG_SHARPNESS0_ENABLE                   "pq.sharpness0.en"
#define CFG_SHARPNESS1_ENABLE                   "pq.sharpness1.en"
#define CFG_DNLP_ENABLE                         "pq.dnlp.en"
#define CFG_CM2_ENABLE                          "pq.cm2.en"
#define CFG_AMVECM_BASCI_ENABLE                 "pq.amvecm.basic.en"
#define CFG_CONTRAST_RGB_ENABLE                 "pq.contrast.rgb.en"
#define CFG_AMVECM_BASCI_WITHOSD_ENABLE         "pq.amvecm.basic.withOSD.en"
#define CFG_CONTRAST_RGB_WITHOSD_ENABLE         "pq.contrast.rgb.withOSD.en"
#define CFG_WHITEBALANCE_ENABLE                 "pq.whitebalance.en"
#define CFG_GAMMA_ENABLE                        "pq.gamma.en"
#define CFG_LOCAL_CONTRAST_ENABLE               "pq.LocalContrast.en"
#define CFG_BLACKEXTENSION_ENABLE               "pq.BlackExtension.en"
#define CFG_CHROMA_COR_ENABLE                   "pq.chroma.cor.en"
#define CFG_XVYCC_ENABLE                        "pq.xvycc.en"
//overscan module config
#define CFG_DISPLAY_OVERSCAN_ENABLE             "pq.DisplayOverscan.en"
//av decode config
#define CFG_CVD2_ENABLE                         "pq.cvd2.en"
#define CFG_HDRTMO_ENABLE                       "pq.hdrtmo.en"
#define CFG_AI_ENABLE                           "pq.ai.en"
#define CFG_MEMC_ENABLE                         "pq.memc.en"
#define CFG_AAD_ENABLE                          "pq.aad.en"
#define CFG_CABC_ENABLE                         "pq.cabc.en"
#define CFG_BLUESTRETCH_ENABLE                  "pq.BlueStretch.en"
#define CFG_CHROMACORING_ENABLE                 "pq.ChromaCoring.en"
#define CFG_LOCALDIMMING_ENABLE                 "pq.LocalDimming.en"
#define CFG_AISR_ENABLE                         "pq.aisr.en"
#define CFG_NEW_PICTURE_MODE_ENABLE             "pq.new_picture_mode.en"

//pq param default value
#define CFG_PICTUREMODE_DEF                     "pq.PictureMode.def"
#define CFG_DV_PICTUREMODE_DEF                  "pq.DvPictureMode.def"
#define CFG_COLORTEMPTUREMODE_DEF               "pq.ColorTemperature.def"
#define CFG_COLORDEMOMODE_DEF                   "pq.ColorDemoMode.def"
#define CFG_COLORBASEMODE_DEF                   "pq.ColorBaseMode.def"
#define CFG_COLORSPACE_DEF                      "pq.ColorSpace.def"
#define CFG_RGBGAIN_R_DEF                       "pq.RGBGainR.def"
#define CFG_RGBGAIN_G_DEF                       "pq.RGBGainG.def"
#define CFG_RGBGAIN_B_DEF                       "pq.RGBGainB.def"
#define CFG_RGBPOSTOFFSET_R_DEF_DEF             "pq.RGBPostOffsetR.def"
#define CFG_RGBPOSTOFFSET_G_DEF_DEF             "pq.RGBPostOffsetG.def"
#define CFG_RGBPOSTOFFSET_B_DEF_DEF             "pq.RGBPostOffsetB.def"
#define CFG_GAMMALEVEL_DEF                      "pq.GammaLevel.def"
#define CFG_DISPLAYMODE_DEF                     "pq.DisplayMode.def"
#define CFG_AUTOASPECT_DEF                      "pq.AutoAspect.def"
#define CFG_43STRETCH_DEF                       "pq.43Stretch.def"
#define CFG_DNLPLEVEL_DEF                       "pq.DnlpLevel.def"
#define CFG_DNLPGAIN_DEF                        "pq.DnlpGain.def"
#define CFG_EYEPROJECTMODE_DEF                  "pq.EyeProtectionMode.def"
#define CFG_DDRSSC_DEF                          "pq.DDRSSC.def"
#define CFG_LVDSSSC_DEF                         "pq.LVDSSSC.def"
#define CFG_LOCALCONTRASTMODE_DEF               "pq.LocalContrastMode.def"
#define CFG_MEMCMODE_DEF                        "pq.MemcMode.def"
#define CFG_MEMCDEBLURLEVEL_DEF                 "pq.MemcDeblurLevel.def"
#define CFG_MEMCDEJUDDERLEVEL_DEF               "pq.MemcDeJudderLevel.def"
#define CFG_DEBLOCKMODE_DEF                     "pq.DeblockMode.def"
#define CFG_DEMOSQUITOMODE_DEF                  "pq.DemoSquitoMode.def"
#define CFG_MCDI_DEF                            "pq.McDiMode.def"
#define CFG_BLACKSTRETCH_DEF                    "pq.BlackStretch.def"
#define CFG_BLUESTRETCH_DEF                     "pq.BlueStretch.def"
#define CFG_CHMROMACORING_DEF                   "pq.ChromaCoring.def"
#define CFG_LOCALDIMMING_DEF                    "pq.LocalDimming.def"
#define CFG_DYNAMICBACKLIGHT_DEF                "pq.DynamicBacklight.def"
#define CFG_FMM_MODE_DEF                        "pq.FilmMakerMode.def"

//for backlight
#define CFG_SECTION_BACKLIGHT                   "BACKLIGHT"
#define CFG_LCD_HDR_INFO_ENABLE                 "lcd.hdrinfo.en"
#define CFG_AUTOBACKLIGHT_THTF                  "auto.backlight.thtf"
#define CFG_AUTOBACKLIGHT_LUTMODE               "auto.backlight.lutmode"
#define CFG_AUTOBACKLIGHT_LUTHIGH               "auto.backlight.lutvalue_high"
#define CFG_AUTOBACKLIGHT_LUTLOW                "auto.backlight.lutvalue_low"

//For HDMI
#define CFG_SECTION_HDMI                        "HDMI"
#define CFG_EDID_VERSION_DEF                    "hdmi.edid.version.def"
#define CFG_HDCP_SWITCHER_DEF                   "hdmi.hdcp.switcher.def"
#define CFG_COLOR_RANGE_MODE_DEF                "hdmi.color.range.mode.def"
#define CFG_HDMI_OUT_WITH_FBC_ENABLE            "hdmi.out.with.fbc.en"
#define CGF_HDMI_AUTOSWITCH_ALLM_EN             "hdmi.autoswitch.allm.en"
#define CGF_HDMI_AUTOSWITCH_ITCONTENT_EN        "hdmi.autoswitch.itcontent.en"
#define CGF_HDMI_AUTOSWITCH_DVI_EN              "hdmi.autoswitch.dvi.en"

//for auto nr
#define CFG_SECTION_AUTO_NR                     "AUTO_NR"
#define CFG_AUTO_NR_MOTION_TH                   "auto.nr.motion_th"
#define CFG_AUTO_NR_MOTION_LP_YGAIN             "auto.nr.motion_lp_ygain"
#define CFG_AUTO_NR_MOTION_HP_YGAIN             "auto.nr.motion_hp_ygain"
#define CFG_AUTO_NR_MOTION_LP_CGAIN             "auto.nr.motion_lp_cgain"
#define CFG_AUTO_NR_MOTION_HP_CGAIN             "auto.nr.motion_hp_cgain"
#define CFG_AUTO_NR_APL_GAIN                    "auto.nr.apl_gain"

//for CMS
#define CFG_SECTION_CMS                         "CMS"
#define CFG_CMS_CM_READ                         "color.customize.cm.red"
#define CFG_CMS_CM_GREEN                        "color.customize.cm.green"
#define CFG_CMS_CM_BLUE                         "color.customize.cm.blue"
#define CFG_CMS_CM_CYAN                         "color.customize.cm.cyan"
#define CFG_CMS_CM_PURPLE                       "color.customize.cm.purple"
#define CFG_CMS_CM_YELLOW                       "color.customize.cm.yellow"
#define CFG_CMS_CM_SKIN                         "color.customize.cm.skin"
#define CFG_CMS_CM_YELLOW_GREEN                 "color.customize.cm.yellow.green"
#define CFG_CMS_CM_BLUE_GREEN                   "color.customize.cm.blue.green"
#define CFG_CMS_3DLUT_EN                        "color.customize.3dlut.en"
#define CFG_CMS_3DLUT_RED                       "color.customize.3dlut.red"
#define CFG_CMS_3DLUT_GREEN                     "color.customize.3dlut.green"
#define CFG_CMS_3DLUT_BLUE                      "color.customize.3dlut.blue"
#define CFG_CMS_3DLUT_CYAN                      "color.customize.3dlut.cyan"
#define CFG_CMS_3DLUT_MAGENTA                   "color.customize.3dlut.magenta"
#define CFG_CMS_3DLUT_YELLOW                    "color.customize.3dlut.yellow"

static const int MAX_CONFIG_FILE_LINE_LEN = 512;

typedef enum _LINE_TYPE {
    LINE_TYPE_SECTION = 0,
    LINE_TYPE_KEY,
    LINE_TYPE_COMMENT,
} LINE_TYPE;

typedef struct _LINE {
    LINE_TYPE type;
    char  Text[MAX_CONFIG_FILE_LINE_LEN];
    int LineLen;
    char *pKeyStart;
    char *pKeyEnd;
    char *pValueStart;
    char *pValueEnd;
    struct _LINE *pNext;
} LINE;

typedef struct _SECTION {
    LINE *pLine;
    struct _SECTION *pNext;
} SECTION;


class CConfigFile {
public:
    CConfigFile();
    ~CConfigFile();
    static CConfigFile *GetInstance();
    bool isFileExist(const char *file_name);
    void GetDvFilePath(char *bin_file_path, char *cfg_file_path);
    int LoadFromFile(const char *filename);
    int SaveToFile(const char *filename = NULL);
    int SetString(const char *section, const char *key, const char *value);
    int SetInt(const char *section, const char *key, int value);
    int SetFloat(const char *section, const char *key, float value);
    const char *GetString(const char *section, const char *key, const char *def_value);
    int GetInt(const char *section, const char *key, int def_value);
    float GetFloat(const char *section, const char *key, float def_value);
    const char *GetDatabaseFilePath(const char *section, const char *key, const char *def_value);
    const char *GetSettingDataFilePath(const char *section, const char *key, const char *def_value);
    const char *GetWhiteBalanceFilePath(const char *section, const char *key, const char *def_value);
    const char *GetFilePath(const char *section, const char *key, const char *def_value);

private:
    LINE_TYPE getLineType(char *Str);
    void allTrim(char *Str);
    SECTION *getSection(const char *section);
    LINE *getKeyLineAtSec(SECTION *pSec, const char *key);
    void FreeAllMem();
    int InsertSection(SECTION *pSec);
    int InsertKeyLine(SECTION *pSec, LINE *line);

    static CConfigFile *mInstance;
    char mpFileName[256];
    FILE *mpConfigFile;
    LINE *mpFirstLine;
    SECTION *mpFirstSection;
};
#endif //end of CONFIG_FILE_H_
