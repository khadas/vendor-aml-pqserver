/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */


#ifndef _C_CPQCONTROL_H
#define _C_CPQCONTROL_H

#include "SSMAction.h"
#include "CDevicePollCheckThread.h"
#include "CPQdb.h"
#include "PQType.h"
#include "CPQLog.h"
#include "CDynamicBackLight.h"
#include "CConfigFile.h"
#include "COverScandb.h"
#include "UEventObserver.h"
#include "CVdin.h"
#include "CAmdolbyVision.h"
#include "CPQControlCb.h"
#include "GammaOperation.h"

#define PQ_DB_DEFAULT_PATH               "/vendor/etc/tvconfig/pq/pq.db"
#define OVERSCAN_DB_DEFAULT_PATH         "/vendor/etc/tvconfig/pq/overscan.db"
#define PQ_CONFIG_DEFAULT_PATH           "/vendor/etc/tvconfig/pq/pq_default.ini"
#define PQ_SETTINGDATA_DEFAULT_PATH      "/vendor/etc/tvconfig/pq"
#define PQ_WHITEBALANCE_DEFAULT_PATH     "/vendor/etc/tvconfig/pq"

#define LDIM_PATH                        "/dev/aml_ldim"
#define LDIM_CONTROL_PATH                "/sys/class/aml_ldim/func_en"
#define LCD_PATH                         "/dev/lcd0"
#define VPP_DEV_PATH                     "/dev/amvecm"
#define DI_DEV_PATH                      "/dev/di0"
#define AFE_DEV_PATH                     "/dev/tvafe0"
#define SYS_VIDEO_FRAME_HEIGHT           "/sys/class/video/frame_height"
#define SYS_VIDEO_HDR_FMT                "/sys/class/video_poll/primary_src_fmt"
#define SYSFS_VIDEO_EVENT_PATH           "/sys/class/video_poll/status_changed"

#define CROP_PATH                        "/sys/class/video/crop"
#define SCREEN_MODE_PATH                 "/sys/class/video/screen_mode"
#define SYSFS_VIDEO_AXIS_PATH            "/sys/class/video/axis"
#define SYSFS_DEVICE_RESOLUTION          "/sys/class/video/device_resolution"

#define NOLINER_FACTORY                  "/sys/class/video/nonlinear_factor"
#define BACKLIGHT_PATH                   "/sys/class/backlight/aml-bl/brightness"
#define SYSFS_VFM_MAP_PATH               "/sys/class/vfm/map"
#define VIDEO_RGB_SCREEN                 "/sys/class/video/rgb_screen"
#define SSC_PATH                         "/sys/class/lcd/ss"
#define TEST_SCREEN                      "/sys/class/video/test_screen"
#define PQ_SET_RW_INTERFACE              "/sys/class/amvecm/pq_reg_rw"
#define AMVECM_PC_MODE_PATH              "/sys/class/amvecm/pc_mode"
#define SYS_DISPLAY_MODE_PATH            "/sys/class/display/mode"
#define HDMI_OUTPUT_CHECK_PATH           "/sys/class/amhdmitx"    //if this dir exist,is hdmi output
#define DEMOSQUITO_MODULE_CONTROL_PATH   "/sys/module/aml_media/parameters/dnr_dm_en"  //demosquito control
#define DEBLOCK_MODULE_CONTROL_PATH      "/sys/module/aml_media/parameters/dnr_en"     //deblock control
#define NR2_MODULE_CONTROL_PATH          "/sys/module/aml_media/parameters/nr2_en"      //noisereduction control
#define MCDI_MODULE_CONTROL_PATH         "/sys/module/aml_media/parameters/mcen_mode"  //mcdi control
#define VPP_MODULE_COLORGAMUT_PATH       "/sys/module/am_vecm/parameters/primary_policy"
#define VIDEO_AISR_ENABLE                "/sys/module/aml_media/parameters/uvm_open_nn"
#define VDETECT_AIPQ_ENABLE              "/sys/class/vdetect/aipq_enable"
#define DECODER_COMMON_PARAMETERS_DEBUG_VDETECT "/sys/module/decoder_common/parameters/debug_vdetect"
#define AMDOLBY_VISION_HDR10_POLICY      "/sys/module/aml_media/parameters/dolby_vision_hdr10_policy"
#define AML_AUTO_NR_PARAMS               "/sys/class/deinterlace/di0/autonr_param"
#define SYS_VIDEO_FRAME_RATE             "/sys/class/display/frame_rate"
#define HDMI_SET_ALLM_PARAM              "/sys/class/hdmirx/hdmirx0/allm_func_ctrl"
#define HDMI_VRR_ENABLED                 "/sys/class/hdmirx/hdmirx0/vrr_func_ctrl"
#define VPP_AFD_MODULE_ASPECT_MODE       "/sys/class/afd_module/aspect_mode"
#define GAMUT_CONV_ENABLE                "/sys/module/aml_media/parameters/gamut_conv_enable"

// screem mode index value
#define  SCREEN_MODE_NORMAL           0
#define  SCREEN_MODE_FULL_STRETCH     1
#define  SCREEN_MODE_4_3              2
#define  SCREEN_MODE_16_9             3
#define  SCREEN_MODE_NONLINEAR        4
#define  SCREEN_MODE_NORMAL_NOSCALEUP 5
#define  SCREEN_MODE_4_3_IGNORE       6
#define  SCREEN_MODE_4_3_LETTER_BOX   7
#define  SCREEN_MODE_4_3_PAN_SCAN     8
#define  SCREEN_MODE_4_3_COMBINED     9
#define  SCREEN_MODE_16_9_IGNORE      10
#define  SCREEN_MODE_16_9_LETTER_BOX  11
#define  SCREEN_MODE_16_9_PAN_SCAN    12
#define  SCREEN_MODE__16_9_COMBINED   13
#define  SCREEN_MODE_WIDEOPTION_CUSTOM 14
#define  SCREEN_MODE_WIDEOPTION_AFD   15

//NR Param
#define NR_3D_YGAIN_ADDR         (0X371C)
#define NR_3D_CGAIN_ADDR         (0X2DCE)
#define NR_2D_GAIN_ADDR          (0X2DAE)
#define VPP_BLACKEXT_CTRL        (0x1D80)

//Sharpness CTI
#define VPP_CTI_YC_DELAY         (0X7)
#define VPP_DECODE_CTI           (0XB5)
#define VPP_CTI_SR0_GAIN         (0X322F)
#define VPP_CTI_SR1_GAIN         (0X32AF)

#define YC_DELAY_REG_MASK        (0XF)
#define DECODE_CTI_REG_MASK      (0XFFFF)
#define SR0_GAIN0_REG_MASK       (0XFF000000)
#define SR0_GAIN1_REG_MASK       (0X00FF0000)
#define SR0_GAIN2_REG_MASK       (0X0000FF00)
#define SR0_GAIN3_REG_MASK       (0X000000FF)
#define SR1_GAIN0_REG_MASK       (0XFF000000)
#define SR1_GAIN1_REG_MASK       (0X00FF0000)
#define SR1_GAIN2_REG_MASK       (0X0000FF00)
#define SR1_GAIN3_REG_MASK       (0X000000FF)
//Video Decode Luma
#define DECODE_BRI_ADDR          (0X9)
#define DECODE_CON_ADDR          (0X8)
#define DECODE_SAT_ADDR          (0XA)
//Sharpness Advanced
#define SHARPNESS_SD_GAIN                (0x3213)
#define SHARPNESS_SD_HP_DIAG_CORE        (0x320f)
#define SHARPNESS_SD_BP_DIAG_CORE        (0x3210)
#define SHARPNESS_SD_PKGAIN_VSLUMA       (0x327e)
#define SHARPNESS_HD_GAIN                (0x3293)
#define SHARPNESS_HD_HP_DIAG_CORE        (0x328f)
#define SHARPNESS_HD_BP_DIAG_CORE        (0x3290)
#define SHARPNESS_HD_PKGAIN_VSLUMA       (0x32fe)

//memc
#define CPQ_MEMC_SYSFS              "/dev/frc"
#define MEMDEV_CONTRL               _IOW('F', 0x06, unsigned int)

typedef enum db_name_e {
    DB_NAME_PQ = 0,
    DB_NAME_OVERSCAN,
    DB_NAME_MAX,
} db_name_t;

typedef enum rgb_ogo_type_e{
    TYPE_INVALID = -1,
    R_GAIN = 0,
    G_GAIN,
    B_GAIN,
    R_POST_OFFSET,
    G_POST_OFFSET,
    B_POST_OFFSET,
    RGB_TYPE_MAX,
} rgb_ogo_type_t;

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

class CPQControl: public UEventObserver::IUEventObserverCallBack,
                  public CDevicePollCheckThread::IDevicePollCheckObserver,
                  public CDynamicBackLight::IDynamicBackLightObserver,
                  public SSMAction::ISSMActionObserver {
public:
    CPQControl();
    ~CPQControl();
    static CPQControl *GetInstance();
    void CPQControlInit(void);
    void CPQControlUnInit(void);
    virtual void onUevent(uevent_data_t ueventData);
    virtual void onVframeSizeChange();
    virtual void onTXStatusChange();
    virtual void resetAllUserSettingParam();
    virtual void resetPQUiSetting(void);
    virtual void resetPQTableSetting(void);
    virtual void Set_Backlight(int value);
    virtual void GetDynamicBacklighConfig(int *thtf, int *lut_mode, int *heigh_param, int *low_param);
    virtual void GetDynamicBacklighParam(dynamic_backlight_Param_t *DynamicBacklightParam);
    int ResetPQModeSetting(tv_source_input_t source_input, vpp_picture_mode_t pq_mode);
    int LoadPQSettings();
    int LoadPQUISettings();
    int LoadPQTableSettings(void);
    int LoadCpqLdimRegs(void);
    int Cpq_LoadRegs(am_regs_t regs);
    int Cpq_LoadDisplayModeRegs(struct ve_pq_load_s regs);
    int DI_LoadRegs(struct am_pq_parm_s di_regs );
    int Cpq_LoadBasicRegs(source_input_param_t source_input_param, vpp_picture_mode_t pqMode);
    int Cpq_SetDIModuleParam(source_input_param_t source_input_param);
    int ResetLastPQSettingsSourceType(void);
    int BacklightInit(void);
    //PQ mode
    int SetPQMode(int pq_mode, int is_save);
    int GetPQMode(void);
    int SavePQMode(int pq_mode);
    int GetLastPQMode(void);
    int SaveLastPQMode(int pq_mode);
    int Cpq_SetVppPCMode(game_pc_mode_t pcStatus);
    int Cpq_SetVdinPCMode(game_pc_mode_t pcStatus);
    int Cpq_SetPCMode(game_pc_mode_t pcStatus);
    int Cpq_SetVdinGameMode(game_pc_mode_t gameStatus);
    int Cpq_SetGameMode(game_pc_mode_t gameStatus);
    int Cpq_SetPQMode(vpp_picture_mode_t pq_mode, source_input_param_t source_input_param);
    int SetPQParams(source_input_param_t source_input_param, vpp_picture_mode_t pq_mode, vpp_pq_para_t pq_para);
    int GetPQParams(source_input_param_t source_input_param, vpp_picture_mode_t pq_mode, vpp_pq_para_t *pq_para);
    void SetPcGameMode(vpp_picture_mode_t pq_mode);
    int Set_PictureMode(vpp_picture_mode_t pq_mode);
    int SetPQPictureMode(vpp_picture_mode_t pq_mode);
    int SetFacColorParams(source_input_param_t source_input_param, vpp_picture_mode_t pqMode);

    //color Temperature
    int SetColorTemperature(int temp_mode, int is_save);
    int GetColorTemperature(void);
    int SaveColorTemperature(int value);
    int Cpq_SetColorTemperature(int level);
    int Cpq_SetWhitebalance(int level, int gamma);

    int SetColorTempParams(int ColorTemp, tcon_rgb_ogo_t *param);
    tvpq_rgb_ogo_t GetColorTempParams(int ColorTemp);

    //Brightness
    int SetBrightness(int value, int is_save);
    int GetBrightness(void);
    int SaveBrightness(int value);
    int Cpq_SetBrightnessBasicParam(source_input_param_t source_input_param);
    int Cpq_SetBrightness(int value, source_input_param_t source_input_param);
    int Cpq_SetVideoBrightness(int value);
    //Contrast
    int SetContrast(int value, int is_save);
    int GetContrast(void);
    int SaveContrast(int value);
    int Cpq_SetContrastBasicParam(source_input_param_t source_input_param);
    int Cpq_SetContrast(int value, source_input_param_t source_input_param);
    int Cpq_SetVideoContrast(int value);
    //Saturation
    int SetSaturation(int value, int is_save);
    int GetSaturation(void);
    int SaveSaturation(int value);
    int Cpq_SetSaturationBasicParam(source_input_param_t source_input_param);
    int Cpq_SetSaturation(int value, source_input_param_t source_input_param);
    //Hue
    int SetHue(int value, int is_save);
    int GetHue(void);
    int SaveHue(int value);
    int Cpq_SetHueBasicParam(source_input_param_t source_input_param);
    int Cpq_SetHue(int value, source_input_param_t source_input_param);
    int Cpq_SetVideoSaturationHue(int satVal, int hueVal);
    void video_set_saturation_hue(signed char saturation, signed char hue, signed long *mab);
    void video_get_saturation_hue(signed char *sat, signed char *hue, signed long *mab);
    //Sharpness
    int SetSharpness(int value, int is_enable, int is_save);
    int GetSharpness(void);
    int SaveSharpness(int value);
    int Cpq_SetSharpness(int value, source_input_param_t source_input_param);

    int Cpq_SetSharpness0Variable(int value, source_input_param_t source_input_param);
    int Cpq_SetSharpness1Variable(int value, source_input_param_t source_input_param);

    int SetSuperResolution(int value, int is_save);
    int GetSuperResolution(void);
    int SaveSuperResolution(int value);
    int Cpq_SetSuperResolution(int value);

    int Cpq_SetSharpness0FixedParam(int mode, source_input_param_t source_input_param);
    int Cpq_SetSharpness0VariableParam(source_input_param_t source_input_param);
    int Cpq_SetSharpness1FixedParam(int mode, source_input_param_t source_input_param);
    int Cpq_SetSharpness1VariableParam(source_input_param_t source_input_param);
    void InitAutoNr(void);
    //NoiseReductionMode
    int SetNoiseReductionMode(int nr_mode, int is_save);
    int GetNoiseReductionMode(void);
    int SaveNoiseReductionMode(int nr_mode);
    int Cpq_SetNoiseReductionMode(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param);
    //GammaValue
    int SetGammaValue(vpp_gamma_curve_t gamma_curve, int is_save);
    int GetGammaValue();
    int SaveGammaValue(int gamma);

    //Memc
    bool hasMemcFunc();
    int Memc_enable(int enable);
    int SetMemcMode(int memc_mode, int is_save);
    int GetMemcMode(void);
    int SaveMemcMode(vpp_memc_mode_t memc_mode);
    int Cpq_SetMemcMode(int memc_mode, source_input_param_t source_input_param);

    int SetMemcDeBlurLevel(int level, int is_save);
    int GetMemcDeBlurLevel(void);
    int SaveMemcDeBlurLevel(int level);
    int Cpq_SetMemcDeBlurLevel(int level, source_input_param_t source_input_param);

    int SetMemcDeJudderLevel(int level, int is_save);
    int GetMemcDeJudderLevel(void);
    int SaveMemcDeJudderLevel(int level);
    int Cpq_SetMemcDeJudderLevel(int level, source_input_param_t source_input_param);

    //Displaymode
    int SetDisplayMode(vpp_display_mode_t display_mode, int is_save);
    int GetDisplayMode(void);
    int SaveDisplayMode(vpp_display_mode_t mode);
    int Cpq_SetDisplayModeAllTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode);
    int Cpq_SetDisplayModeOneTiming(tv_source_input_t source_input, vpp_display_mode_t display_mode);
    int Cpq_SetVideoScreenMode(int value);
    int Cpq_GetScreenModeValue(vpp_display_mode_t display_mode);
    int Cpq_SetVideoCrop(int Voffset0, int Hoffset0, int Voffset1, int Hoffset1);
    int Cpq_SetNonLinearFactor(int value);
    int SetCurrentAspectRatioInfo(enum tvin_aspect_ratio_e aspectRatioInfo);
    //LCD HDR INFO
    int SetLCDhdrinfo(void);
    int Cpq_SetHdrInfo(const struct lcd_optical_info_s *plcd_hdrinfo);

    //Backlight
    int SetBacklight(int value, int is_save);
    int GetBacklight(void);
    int SaveBacklight(int value);
    int Cpq_SetBackLight(int value);
    void Cpq_GetBacklight(int *value);
    int SetDynamicBacklight(Dynamic_backlight_status_t mode, int is_save);
    int GetDynamicBacklight(void);
    int DynamicBackLightInit(void);
    int GetVideoPlayStatus(void);
    //smooth plus
    int SetSmoothPlusMode(int smoothplus_mode, int is_save);
    int GetSmoothPlusMode(void);
    int SaveSmoothPlusMode(int smoothplus_mode);
    int Cpq_SetSmoothPlusMode(vpp_smooth_plus_mode_t smoothplus_mode, source_input_param_t source_input_param);
    //local contrast
    int SetLocalContrastMode(local_contrast_mode_t mode, int is_save);
    int GetLocalContrastMode(void);
    int SaveLocalContrastMode(local_contrast_mode_t mode);
    int Cpq_SetLocalContrastMode(local_contrast_mode_t mode);
    //MpegNr
    int SetMpegNr(vpp_pq_level_t mode, int is_save);
    int GetMpegNr(void);
    int SaveMpegNr(vpp_pq_level_t mode);
    int Cpq_SetMpegNr(vpp_pq_level_t mode, source_input_param_t source_input_param);
    //DI deblock
    int SetDeblockMode(vpp_deblock_mode_t mode, int is_save);
    int GetDeblockMode(void);
    int SaveDeblockMode(vpp_deblock_mode_t mode);
    int Cpq_SetDeblockMode(vpp_deblock_mode_t deblock_mode, source_input_param_t source_input_param);
    //DI demosquito
    int SetDemoSquitoMode(vpp_demosquito_mode_t mode, int is_save);
    int GetDemoSquitoMode(void);
    int SaveDemoSquitoMode(vpp_demosquito_mode_t mode);
    int Cpq_SetDemoSquitoMode(vpp_demosquito_mode_t DeMosquito_mode, source_input_param_t source_input_param);
    //DI MCDI
    int SetMcDiMode(vpp_mcdi_mode_e mode, int is_save);
    int GetMcDiMode(void);
    int SaveMcDiMode(vpp_mcdi_mode_e mode);
    int Cpq_SetMcDiMode(vpp_mcdi_mode_e McDi_mode, source_input_param_t source_input_param);
    //color base
    int SetColorBaseMode(vpp_color_basemode_t basemode, int isSave);
    vpp_color_basemode_t GetColorBaseMode(void);
    int SaveColorBaseModeToSSM(int value, vpp_picture_mode_t pq_mode, tv_source_input_t srcInput);
    int SaveColorBaseModeAllSrc(int value, vpp_picture_mode_t pq_mode);
    int SaveColorBaseMode(vpp_color_basemode_t basemode);
    int Cpq_SetColorBaseMode(vpp_color_basemode_t basemode, source_input_param_t source_input_param);
    //color customize
    int SetColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value, int isSave);
    vpp_single_color_param_cm_t GetColorCustomize(vpp_cms_color_t color);
    int SetColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value, int isSave);
    vpp_single_color_param_3dlut_t GetColorCustomizeBy3DLut(vpp_cms_6color_t color);
    int ResetColorCustomize(vpp_cms_method_t mode);
    int SetColorTuneEnable(int enable);
    int GetColorTuneEnable(void);

    int SetWbGammaEnable(int enable);
    int GetWbGammaEnable(void);
    int SetWbGammaMode(int mode);
    int GetWbGammaMode(void);

    //BlackExtension
    int SetBlackExtensionParam(source_input_param_t source_input_param);
    int Cpq_SetCABC(const struct db_cabc_param_s *pCABC);
    int SetCabc(void);
    int Cpq_SetAAD(const struct db_aad_param_s *pAAD);
    int SetAad(void);
    //dynamic contrast
    int SetDnlpMode(Dynamic_contrast_mode_t mode, int is_save);
    int GetDnlpMode();
    int SaveDnlpMode(Dynamic_contrast_mode_t mode);
    int Cpq_SetDnlpMode(Dynamic_contrast_mode_t level, source_input_param_t source_input_param);
    int Cpq_SetDNLPStatus(enum dnlp_state_e status);

    //FilmMaker mode
    int SetFilmMakerMode(pq_film_maker_mode_t mode, int is_save);
    int GetFilmMakerMode(void);

    //Factory
    int FactoryResetPQMode(void);
    int FactoryResetColorTemp(void);
    int FactorySetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode, int brightness );
    int FactoryGetPQMode_Brightness(source_input_param_t source_input_param, int pq_mode );
    int FactorySetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode, int contrast );
    int FactoryGetPQMode_Contrast(source_input_param_t source_input_param, int pq_mode );
    int FactorySetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode, int saturation );
    int FactoryGetPQMode_Saturation(source_input_param_t source_input_param, int pq_mode );
    int FactorySetPQMode_Hue(source_input_param_t source_input_param, int pq_mode, int hue );
    int FactoryGetPQMode_Hue(source_input_param_t source_input_param, int pq_mode );
    int FactorySetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode, int sharpness );
    int FactoryGetPQMode_Sharpness(source_input_param_t source_input_param, int pq_mode );

    int FactorySetColorTemp_Rgain ( int source_input, int colortemp_mode, int rgain );
    int FactorySaveColorTemp_Rgain ( int source_input, int colortemp_mode, int rgain );
    int FactoryGetColorTemp_Rgain ( int source_input, int colortemp_mode );
    int FactorySetColorTemp_Ggain ( int source_input, int colortemp_mode, int ggain );
    int FactorySaveColorTemp_Ggain ( int source_input, int colortemp_mode, int ggain );
    int FactoryGetColorTemp_Ggain ( int source_input, int colortemp_mode );
    int FactorySetColorTemp_Bgain ( int source_input, int colortemp_mode, int bgain );
    int FactorySaveColorTemp_Bgain ( int source_input, int colortemp_mode, int bgain );
    int FactoryGetColorTemp_Bgain ( int source_input, int colortemp_mode );
    int FactorySetColorTemp_Roffset ( int source_input, int colortemp_mode, int roffset );
    int FactorySaveColorTemp_Roffset ( int source_input, int colortemp_mode, int roffset );
    int FactoryGetColorTemp_Roffset ( int source_input, int colortemp_mode );
    int FactorySetColorTemp_Goffset ( int source_input, int colortemp_mode, int goffset );
    int FactorySaveColorTemp_Goffset ( int source_input, int colortemp_mode, int goffset );
    int FactoryGetColorTemp_Goffset ( int source_input, int colortemp_mode );
    int FactorySetColorTemp_Boffset ( int source_input, int colortemp_mode, int boffset );
    int FactorySaveColorTemp_Boffset ( int source_input, int colortemp_mode, int boffset );
    int FactoryGetColorTemp_Boffset ( int source_input, int colortemp_mode );

    int FactorySetRGBGainOffset(int colortemp_mode, tcon_rgb_ogo_t params);
    int FactoryGetRGBGainOffset(int colortemp_mode, tcon_rgb_ogo_t *params);

    int FactoryResetNonlinear(void);
    int FactorySetParamsDefault(void);
    int FactorySetNolineParams(source_input_param_t source_input_param, int type, noline_params_t noline_params);
    noline_params_t FactoryGetNolineParams(source_input_param_t source_input_param,          int type);
    int FactorySetHdrMode(int mode);
    int FactoryGetHdrMode(void);
    int FactorySetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t cutwin_t);
    tvin_cutwin_t FactoryGetOverscanParam(source_input_param_t source_input_param, vpp_display_mode_t dmode);
    int FactorySetGamma(int gamma_r_value, int gamma_g_value, int gamma_b_value);
    int FcatorySSMRestore(void);

    int Cpq_SetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo);
    int Cpq_GetRGBOGO(const struct tcon_rgb_ogo_s *rgbogo);
    int Cpq_LoadGamma(vpp_gamma_curve_t gamma_curve, vpp_color_temperature_mode_t level);
    int Cpq_SetGammaTbl_R(unsigned short red[GAMMA_NUMBER]);
    int Cpq_SetGammaTbl_G(unsigned short green[GAMMA_NUMBER]);
    int Cpq_SetGammaTbl_B(unsigned short blue[GAMMA_NUMBER]);
    int Cpq_SetGammaOnOff(int onoff);
    int FactorySetDNLPCurveParams(source_input_param_t source_input_param, int level, int final_gain);
    int FactoryGetDNLPCurveParams(source_input_param_t source_input_param, int level);
    int FactorySetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr, int val);
    int FactoryGetNoiseReductionParams(source_input_param_t source_input_param, vpp_noise_reduction_mode_t nr_mode, int addr);
    int SetEyeProtectionMode(tv_source_input_t source_input, int enable, int is_save);
    int GetEyeProtectionMode(tv_source_input_t source_input);
    int Cpq_SSMReadNTypes(int id, int data_len, int offset);
    int Cpq_SSMWriteNTypes(int id, int data_len, int data_buf, int offset);
    int Cpq_GetSSMActualAddr(int id);
    int Cpq_GetSSMActualSize(int id);
    int SSMRecovery(void);
    int Cpq_GetSSMStatus();
    enum hdr_type_e Cpq_GetSourceHDRType(tv_source_input_t source_input);
    int SetFlagByCfg(void);
    int SetPLLValues(source_input_param_t source_input_param);
    int SetCVD2Values(void);
    int SetCurrentSourceInputInfo(source_input_param_t source_input_param);
    int SetCurrentSource(tv_source_input_t source_input);
    source_input_param_t GetCurrentSourceInputInfo();
    int GetHistParam(struct ve_hist_s *hist);
    bool isFileExist(const char *file_name);

    int GetRGBPattern();
    int SetRGBPattern(int r, int g, int b);
    int FactorySetDDRSSC (int step);
    int FactoryGetDDRSSC(void);
    int FactorySetLVDSSSCLevel(int level);
    int FactoryGetLVDSSSCLevel(void);
    int FactorySetLVDSSSCFrep(int step);
    int FactoryGetLVDSSSCFrep(void);
    int FactorySetLVDSSSCMode(int mode);
    int FactoryGetLVDSSSCMode(void);
    int GetLVDSSSCParams(struct aml_lcd_ss_ctl_s *param);
    int SaveLVDSSSCParams(struct aml_lcd_ss_ctl_s *param);
    int SetDecodeLumaParamsCheckVal(int param_type, int val);
    int FactorySetDecodeLumaParams(source_input_param_t source_input_param, int param_type, int val);
    int FactoryGetDecodeLumaParams(source_input_param_t source_input_param, int param_type);

    int SetGrayPattern(int value);
    int GetGrayPattern();
    //HDR
    int SetHDRMode(int mode);
    int GetHDRMode(void);
    int GetSourceHDRType();
    tvpq_databaseinfo_t GetDBVersionInfo(db_name_t name, db_version_type_t type);
    int SetCurrentHdrInfo (int hdrInfo);
    int SetDtvKitSourceEnable(bool isEnable);
    //AI
    void AipqInit();
    int SetAipqEnable(bool isEnable);
    int GetAipqEnable(void);
    void enableAipq(bool isEnable);
    int AiParamLoad(void);
    //AI SR
    bool hasAisrFunc();
    int SetAiSrEnable(bool isEnable);
    int GetAiSrEnable(void);
    int SaveAiSrEnable(bool enable);
    int Cpq_SetAiSrEnable(bool enable);
    //color gamut
    int SetColorGamutMode(int value, int is_save);
    int GetColorGamutMode(void);
    int SaveColorGamutMode(vpp_colorgamut_mode_t value);
    int Cpq_SetColorGamutMode(vpp_colorgamut_mode_t value, source_input_param_t source_input_param);
    int Cpq_SetColorGamutEnable(bool enable);

    //HDR tone mapping
    int SetHDRTMData(int *reGain);
    //HDR TMO
    int SetHDRTMOMode(hdr_tmo_t mode, int is_save);
    int GetHDRTMOMode();
    int SaveHDRTMOMode(hdr_tmo_t mode);
    int Cpq_SetHDRTMOMode(int mode);

    //black/bule/chroma stretch
    int SetBlackStretch(int level, int is_save);
    int GetBlackStretch(void);
    int SaveBlackStretch(int level);
    int Cpq_BlackStretch(int level, source_input_param_t source_input_param);

    int SetBlueStretch(int level, int is_save);
    int GetBlueStretch(void);
    int SaveBlueStretch(int level);
    int Cpq_BlueStretch(int level, source_input_param_t source_input_param);

    int SetChromaCoring(int level, int is_save);
    int GetChromaCoring(void);
    int SaveChromaCoring(int level);
    int Cpq_ChromaCoring(int level, source_input_param_t source_input_param);

    int SetLocalDimming(int level, int is_save);
    int GetLocalDimming(void);
    int SaveLocalDimming(int level);
    int Cpq_SetLocalDimming(vpp_pq_level_t level);

    int SetAmdolbyDarkDetail(int mode, int is_save);
    int GetAmdolbyDarkDetail(void);
    int SaveAmdolbyDarkDetail(int value);
    int Cpq_SetAmdolbyDarkDetail(int mode);

    int Cpq_SetAmdolbyPQMode(int mode);

    void resetCurSrcPqUiSetting(void);
    int SetSharpnessParamsCheckVal(int param_type, int val);
    int MatchSharpnessRegAddr(int param_type, int isHd);
    int FactorySetSharpnessParams(source_input_param_t source_input_param, sharpness_timing_t source_timing, int param_type, int val);
    int FactoryGetSharpnessParams(source_input_param_t source_input_param, sharpness_timing_t source_timing, int param_type);
    int SetHdmiColorRangeMode(int pq_type, int isEnable);
    int GetHdmiColorRangeMode(int pq_type);
    int DBGammaBlend(tcon_gamma_table_t *wb_gamma, unsigned int *index_gamma);

    bool SetPictureModeDataBySrcTimming(vpp_pictur_mode_para_t *params, pq_source_input_t src, pq_sig_fmt_t timming, vpp_picture_mode_t picmode);
    bool GetPictureModeDataBySrcTimming(vpp_pictur_mode_para_t *params, pq_source_input_t src, pq_sig_fmt_t timming, vpp_picture_mode_t picmode);

    bool SetColorTemperatureDataBySrcTimming(tcon_rgb_ogo_t *params, pq_source_input_t src, pq_sig_fmt_t timming, int level);
    bool GetColorTemperatureDataBySrcTimming(tcon_rgb_ogo_t *params, pq_source_input_t src, pq_sig_fmt_t timming, int level);

    int SetWhitebalanceGamma(int channel, int point, int offset);
    int GetWhitebalanceGamma(int channel, int point);

    int FactorySetWhitebalanceGamma(int colortemp, int channel, int point, int offset);
    int FactoryGetWhitebalanceGamma(int colortemp, int channel, int point);

private:
    int VPPOpenModule(void);
    int VPPCloseModule(void );
    int VPPDeviceIOCtl(int request, ...);
    int DIOpenModule(void);
    int DICloseModule(void);
    int DIDeviceIOCtl(int request, ...);
    int LCDLdimOpenModule(void);
    int LCDLdimCloseModule(void);
    int LCDLdimDeviceIOCtl(int request, ...);
    int MEMCOpenModule(void);
    int MEMCCloseModule(void);
    int MEMCDeviceIOCtl(int request, ...);
    int LCDOpenModule(void);
    int LCDCloseModule(void);
    int LCDDeviceIOCtl(int request, ...);
    int VDINOpenModule(void);
    int VDINCloseModule(void);
    int VDINDeviceIOCtl(int request, ...);
    int AFEDeviceIOCtl ( int request, ... );
    int Cpq_GetAllmAndVrrInfo(void);
    void stopVdin(void);
    void onSigStatusChange(void);
    void onAllmOrVrrStatusChange(void);
    int SetCurrenSourceInfo(struct tvin_parm_s sig_info);
    enum tvin_sig_fmt_e getVideoResolutionToFmt();
    int GetWindowStatus(void);
    int Cpq_SetXVYCCMode(vpp_xvycc_mode_t xvycc_mode, source_input_param_t source_input_param);
    int pqWriteSys(const char *path, const char *val);
    int pqReadSys(const char *path, char *buf, int count);
    void pqTransformStringToInt(const char *buf, int *val);
    unsigned int GetSharpnessRegVal(int addr);
    int Cpq_GetInputVideoFrameHeight(tv_source_input_t source_input);
    output_type_t CheckOutPutMode(tv_source_input_t source_input);
    pq_source_input_t CheckPQSource(tv_source_input_t source);
    pq_sig_fmt_t CheckPQTimming(enum hdr_type_e hdr_type);
    bool isPqDatabaseMachChip();
    bool CheckPQModeTableInDb(void);
    int Cpq_SetVadjEnableStatus(int isvadj1Enable, int isvadj2Enable);
    bool IsAllmVrrUiFuncCtrl(int *allm_func_ctrl, int *vrr_func_ctrl);
    bool IsAllmVrrGameMode(void);
    int Cpq_SetPqModeToGame(pq_mode_to_game_t mode);
    int Cpq_SetPqModeToMonitor(int ui_fmm, int drv_fmm);

    int GetBaseGammaData(int level, GAMMA_TABLE *pData);
    int GetWBGammaData(int level, GAMMA_TABLE *pData);
    int GetGammaPowerData(int level, GAMMA_TABLE *pData);
    double GetGammaPower(vpp_gamma_curve_t mode);
    int CubeInterpolationProcess(interpolation_info_t output, unsigned short *gamma, int num_points);

    bool SetPictureMode(vpp_picture_mode_t PictureMode);
    bool GetPictureMode(vpp_picture_mode_t *PictureMode);
    bool SetLastPictureMode(vpp_picture_mode_t PictureMode);
    bool GetLastPictureMode(vpp_picture_mode_t *PictureMode);
    bool ResetPictureMode(void);
    bool ResetPictureModeBySrc(void);
    bool ResetPictureModeAll(void);

    bool SetPictureModeData(vpp_pictur_mode_para_t *params, vpp_picture_mode_t picmode);
    bool GetPictureModeData(vpp_pictur_mode_para_t *params, vpp_picture_mode_t picmode);
    bool ResetPictureModeData(void);
    bool ResetPictureModeDataBySrc(void);
    bool ResetPictureModeDataAll(void);

    bool SetColorTemperatureData(tcon_rgb_ogo_t *params, int level);
    bool GetColorTemperatureData(tcon_rgb_ogo_t *params, int level);
    bool ResetColorTemperatureData(void);
    bool ResetColorTemperatureDataBySrc(void);
    bool ResetColorTemperatureDataAll(void);

    bool FactoryGetWhitebalanceRGBGainOffsetData(tcon_rgb_ogo_t *pData, int level);
    bool FactorySetWhitebalanceRGBGainOffsetData(tcon_rgb_ogo_t *pData, int level);
    bool CheckCriDataWhitebalanceRGBGainOffsetData(void);

    bool SetWhitebalanceGammaData(WB_GAMMA_TABLE *params, int level);
    bool GetWhitebalanceGammaData(WB_GAMMA_TABLE *params, int level);
    bool ResetWhitebalanceGammaData(void);
    bool ResetWhitebalanceGammaDataBySrc(void);
    bool ResetWhitebalanceGammaDataAll(void);

    bool FactorySetWhitebalanceGammaData(WB_GAMMA_TABLE_DATA *params, int level);
    bool FactoryGetWhitebalanceGammaData(WB_GAMMA_TABLE_DATA *params, int level);
    bool CheckCriDataWhitebalanceGammaData(void);

    //color customize
    int SaveColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value);
    int Cpq_SetColorCustomize(vpp_cms_color_t color, vpp_cms_type_t type, int value);
    int Cpq_SetColorCustomizeEnable(int enable);

    int SaveColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value);
    int Cpq_SetColorCustomizeBy3DLut(vpp_cms_6color_t color, vpp_cms_type_t type, int value);
    int Cpq_GetColorCustomizeDefValue(vpp_cms_cm_param_t *pCmsCm, vpp_cms_3dlut_param_t *pCms3DLut);


    int GetDriverValueMap(vpp_cms_type_t type, int value);

    //FilmMaker mode
    int SaveFilmMakerMode(pq_film_maker_mode_t mode);

    bool mInitialized                                   = false;
    //cfg
    bool mbCpqCfg_separate_db_enable                    = false;
    bool mbCpqCfg_pq_enable                             = false;
    bool mbCpqCfg_amvecm_basic_enable                   = false;
    bool mbCpqCfg_amvecm_basic_withOSD_enable           = false;
    bool mbCpqCfg_contrast_rgb_enable                   = false;
    bool mbCpqCfg_contrast_rgb_withOSD_enable           = false;
    bool mbCpqCfg_blackextension_enable                 = false;
    bool mbCpqCfg_sharpness0_enable                     = false;
    bool mbCpqCfg_sharpness1_enable                     = false;
    bool mbCpqCfg_di_enable                             = false;
    bool mbCpqCfg_mcdi_enable                           = false;
    bool mbCpqCfg_deblock_enable                        = false;
    bool mbCpqCfg_nr_enable                             = false;
    bool mbCpqCfg_demoSquito_enable                     = false;
    bool mbCpqCfg_gamma_enable                          = false;
    bool mbCpqCfg_cm2_enable                            = false;
    bool mbCpqCfg_whitebalance_enable                   = false;
    bool mbCpqCfg_dnlp_enable                           = false;
    bool mbCpqCfg_xvycc_enable                          = false;
    bool mbCpqCfg_display_overscan_enable               = false;
    bool mbCpqCfg_cvd2_enable                           = false;
    bool mbCpqCfg_local_contrast_enable                 = false;
    bool mbCpqCfg_hdmi_out_with_fbc_enable              = false;
    bool mbCpqCfg_pq_mode_check_source_enable           = false;
    bool mbCpqCfg_pq_mode_check_hdr_enable              = false;
    bool mbCpqCfg_pq_param_check_source_enable          = false;
    bool mbCpqCfg_pq_param_check_hdr_enable             = false;
    bool mbCpqCfg_ldim_enable                           = false;
    bool mbCpqCfg_lcd_hdrinfo_enable                    = false;
    bool mbAllmModeCfg_enable                           = false;
    bool mbItcontentModeCfg_enable                      = false;
    bool mbDviModeCfg_enable                            = false;
    bool mbCpqCfg_ai_enable                             = false;
    bool mbCpqCfg_aad_enable                            = false;
    bool mbCpqCfg_cabc_enable                           = false;
    bool mbCpqCfg_smoothplus_enable                     = false;
    bool mbCpqCfg_hdrtmo_enable                         = false;
    bool mbCpqCfg_memc_enable                           = false;
    bool mbCpqCfg_separate_black_blue_chorma_db_enable  = false;
    bool mbCpqCfg_bluestretch_enable                    = false;
    bool mbCpqCfg_chromacoring_enable                   = false;
    bool mbCpqCfg_LocalDimming_enable                   = false;
    bool mbCpqCfg_aisr_enable                           = false;
    bool mbCpqCfg_new_picture_mode_enable               = false;

    bool mbDtvKitEnable                                 = false;
    bool mbDatabaseMatchChipStatus                      = false;
    bool mbVideoIsPlaying                               = false;//video don't playing
    bool mbResetPicture                                 = false;
    bool mbFilmmakerModeFlag                            = false;

    CPQdb *mPQdb                                        = NULL;
    COverScandb *mpOverScandb                           = NULL;
    SSMAction *mSSMAction                               = NULL;
    CConfigFile *mPQConfigFile                          = NULL;
    CVdin *mpVdin                                       = NULL;
    CDolbyVision *mDolbyVision                          = NULL;

    int mAmvideoFd                                      = -1;
    int mDiFd                                           = -1;
    int mLdFd                                           = -1;
    int mMemcFd                                         = -1;
    int mLcdFd                                          = -1;

    static CPQControl *mInstance;
    CDevicePollCheckThread mCDevicePollCheckThread;
    UEventObserver mUEventObserver;
    CDynamicBackLight mDynamicBackLight;

    tcon_rgb_ogo_t rgbfrompq[3];
    source_input_param_t mCurrentSourceInputInfo;
    struct tvin_parm_s mCurrentSignalInfo;
    tvin_inputparam_t mCurrentTvinInfo;

    game_pc_mode_t mGamemode                            = MODE_OFF;
    enum tvin_aspect_ratio_e mCurrentAfdInfo            = TVIN_ASPECT_NULL;
    tv_source_input_t mSourceInputForSaveParam          = SOURCE_MPEG;
    pq_source_input_t CurSource                         = PQ_SRC_MPEG;
    pq_sig_fmt_t CurTimming                             = PQ_FMT_SDR;
    enum hdr_type_e mCurrentHdrType                     = HDRTYPE_NONE;

    int mColorTuneEnable                                = 1;
    int _unused                                         = -1;

    bool aAllmSrcFmtFlag[PQ_SRC_MAX][PQ_FMT_MAX];
    bool aFmmSrcFmtFlag[PQ_SRC_MAX][PQ_FMT_MAX];


//for callback to upper client
public:
    class ICPQControlObserver {
    public:
        ICPQControlObserver() {};
        virtual ~ICPQControlObserver() {};
        virtual void GetCbDataFromLibpq(CPQControlCb &cb_data) = 0;
    };

    void setObserver (ICPQControlObserver *pOb) {
        mpObserver = pOb;
    };

private:
    ICPQControlObserver *mpObserver; //to get upper pqserver object

    int mPreAllmGameMode = 0;
    int mPreFilmMakerMode[PQ_SRC_MAX][PQ_FMT_MAX];
    int mPreRefreshRate = 0;

    int Cpq_SetHdrType(int data);
    int Cpq_SetAllmStatus(void);
    int Cpq_SetFilmMakerMode(void);
    int Cpq_SetRefreshRate(void);
};
#endif
