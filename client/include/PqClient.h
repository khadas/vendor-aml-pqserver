/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef PQCLIENT_H
#define PQCLIENT_H

#include <map>
#include <memory>
#include <array>

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include "common.h"
#include "CPqClientCb.h"

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

class PqClient : public BBinder {
public:
    enum {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_PQ_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
        CMD_SET_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 2,
        CMD_CLR_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 3,
        CMD_HDR_DT_CB = IBinder::FIRST_CALL_TRANSACTION + 4,
        CMD_ALLM_GAME_CB = IBinder::FIRST_CALL_TRANSACTION + 5,
        CMD_FMM_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 6,
        CMD_REFRESH_RATE_CB = IBinder::FIRST_CALL_TRANSACTION + 7,

        CMD_PQ_SET_DDR_SSC,
        CMD_PQ_GET_DDR_SSC,

        CMD_PQ_SET_LVDS_SSC_LEVEL,
        CMD_PQ_GET_LVDS_SSC_LEVEL,
        CMD_PQ_SET_LVDS_SSC_FREP,
        CMD_PQ_GET_LVDS_SSC_FREP,
        CMD_PQ_SET_LVDS_SSC_MODE,
        CMD_PQ_GET_LVDS_SSC_MODE,
        CMD_PQ_SET_COLORTEMPERATURE_USER_PARAM,
        CMD_PQ_GET_COLORTEMPERATURE_USER_PARAM,
        CMD_PQ_RESET_PICTURE_MODE,
        CMD_PQ_RESET_ALL_PICTURE_MODE,
        CMD_PQ_GET_PICTURE_MODE_PARAM,
        CMD_PQ_GET_DEFAULT_PICTURE_MODE_PARAM,
        CMD_PQ_GET_DEFAULT_ALL_PQ_PARAM,
        CMD_PQ_COPY_VALUES_FROM_PICTURE_MODE,
        CMD_PQ_COPY_PICTURE_MODE_PARAM_TO_ALL_SOURCE,
    };

    PqClient();
    ~PqClient();
    static PqClient *GetInstance();
    int SetPQMode(int mode, int isSave = 0);
    int GetPQMode();
    int GetLastPQMode();
    int SetColorTemperature(int colorTemperatureValue, int isSave);
    int GetColorTemperature(void);
    int SetColorTemperatureUserParam(tvpq_rgb_ogo_t *pData);
    tvpq_rgb_ogo_t GetColorTemperatureUserParam(void);
    int SetBrightness(int brightnessValue, int isSave = 0);
    int GetBrightness();
    int SetContrast(int contrastValue, int isSave = 0);
    int GetContrast();
    int SetSaturation(int saturationValue, int isSave = 0);
    int GetSaturation();
    int SetHue(int hueValue, int isSave = 0);
    int GetHue();
    int SetSharpness(int sharpnessValue, int is_enable, int isSave = 0);
    int GetSharpness();
    int SetNoiseReductionMode(int nr_mode, int isSave = 0);
    int GetNoiseReductionMode();
    int SetEyeProtectionMode(int source_input, int enable, int isSave = 0);
    int GetEyeProtectionMode();
    int SetGammaValue(int gamma_curve, int isSave);
    int GetGammaValue();
    int SetDisplayMode(int isUsed, int DisplayMode, int isSave = 0);
    int GetDisplayMode();
    int SetBacklight(int backlightValue, int isSave = 0);
    int GetBacklight();
    int SetDynamicBacklight(int backlightValue, int isSave = 0);
    int GetDynamicBacklight();
    int SetLocalContrastMode(int mode, int isSave = 0);
    int GetLocalContrastMode();
    int SetColorBaseMode(int mode, int isSave = 0);
    int GetColorBaseMode();
    int setCurrentSourceInfo(int sourceInput, int sigFmt, int transFmt);
    source_input_param_t getCurrentSourceInfo();
    int SetColorGamutMode(int mode, int isSave);
    int GetColorGamutMode();
    int GetSourceHDRType();
    int SetDynamicContrastMode(int mode, int isSave);
    int GetDynamicContrastMode();
    int SSMRecovery(void);
    int HasMemcFunc();
    int SetMemcMode(int mode, int isSave);
    int GetMemcMode();
    int SetMemcDeBlurLevel(int mode, int isSave);
    int GetMemcDeBlurLevel();
    int SetMemcDeJudderLevel(int mode, int isSave);
    int GetMemcDeJudderLevel();
    int SetDecontourMode(int mode, int isSave);
    int GetDecontourMode();
    int SetDeBlockMode(int mode, int isSave);
    int GetDeBlockMode();
    int SetDeMosquitoMode(int mode, int isSave);
    int GetDeMosquitoMode();
    int SetBlackStretch(int mode, int isSave);
    int GetBlackStretch();
    int SetBlueStretch(int mode, int isSave);
    int GetBlueStretch();
    int HasAipqFunc();
    int SetAipqEnable(int mode);
    int GetAipqEnable();
    int HasAisrFunc();
    int SetAiSrEnable(int mode);
    int GetAiSrEnable();
    int SetLdim(int mode, int isSave);
    int GetLdim();
    int SetMpegNr(int mode, int isSave);
    int GetMpegNr();
    int ResetPictureUiSetting(void);
    int SetColorCustomize(int color, int type, int value, int isSave);
    vpp_single_color_param_cm_t GetColorCustomize(int color);
    int SetColorCustomizeEnable(int enable);
    int GetColorCustomizeEnable(void);
    int SetWhitebalanceGammaEnable(int enable);
    int GetWhitebalanceGammaEnable(void);
    int SetWhitebalanceGammaMode(int mode);
    int GetWhitebalanceGammaMode(void);
    int SetColorCustomizeBy3DLut(int color, int type, int value, int isSave);
    vpp_single_color_param_3dlut_t GetColorCustomizeBy3DLut(int color);
    int ResetColorCustomize(int mode);

    /* to ssmdata*/
    /*channel: R channel(0), G channel(1), B channel(2)*/
    /*point:  gray (%0 ~ 100% ) %5 per step. 0 ~ 21*/
    /*offset:  -1023 ~ 1023*/
    int SetWhitebalanceGamma(int channel, int point, int offset);
    int GetWhitebalanceGamma(int channel, int point);
    int SetSuperResolution(int mode, int isSave);
    int GetSuperResolution(void);
    int SetHdrToneMappingMode(int mode, int isSave);
    int GetHdrToneMappingMode(void);
    tvpq_databaseinfo_t GetDbVersionInfo(int mode, int type);
    int SetFilmMakerMode(int mode, int isSave);
    int GetFilmMakerMode(void);

    int FactoryResetPQMode(void);
    int FactoryResetColorTemp(void);
    int FactoryWhiteBalanceSetColorTemperature(int sourceInput, int sigFmt, int transFmt, int colorTemperatureValue, int isSave = 0);
    int FactoryWhiteBalanceGetColorTemperature();
    int FactorySetPQMode_Brightness(int sourceInput, int sigFmt, int transFmt, int pq_mode, int brightness);
    int FactoryGetPQMode_Brightness(int sourceInput, int sigFmt, int transFmt, int pq_mode);
    int FactorySetPQMode_Contrast(int sourceInput, int sigFmt, int transFmt, int pq_mode, int contrast);
    int FactoryGetPQMode_Contrast(int sourceInput, int sigFmt, int transFmt, int pq_mode);
    int FactorySetPQMode_Saturation(int sourceInput, int sigFmt, int transFmt, int pq_mode, int saturation);
    int FactoryGetPQMode_Saturation(int sourceInput, int sigFmt, int transFmt, int pq_mode);
    int FactorySetPQMode_Hue(int sourceInput, int sigFmt, int transFmt, int pq_mode, int hue);
    int FactoryGetPQMode_Hue(int sourceInput, int sigFmt, int transFmt, int pq_mode);
    int FactorySetPQMode_Sharpness(int sourceInput, int sigFmt, int transFmt, int pq_mode, int sharpness);
    int FactoryGetPQMode_Sharpness(int sourceInput, int sigFmt, int transFmt, int pq_mode);
    int FactorySetOverscanParams(int sourceInput, int sigFmt, int transFmt, int dmode, tvin_cutwin_t cutwin_t);
    tvin_cutwin_t FactoryGetOverscanParams(int sourceInput, int sigFmt, int transFmt, int dmode);
    int FactorySetWhiteBalanceRedGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceRedGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);
    int FactorySetWhiteBalanceGreenGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceGreenGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);
    int FactorySetWhiteBalanceBlueGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceBlueGain(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);
    int FactorySetWhiteBalanceRedPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceRedPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);
    int FactorySetWhiteBalanceGreenPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceGreenPostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);
    int FactorySetWhiteBalanceBluePostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceBluePostOffset(int sourceInput, int sigFmt, int transFmt, int colortemptureMode);

    int FactorySetRGBPattern(int r, int g, int b);
    int FactoryGetRGBPattern();
    int FactorySetGrayPattern(int value);
    int FactoryGetGrayPattern();

    int FactoryGetSharpnessParams(int sourceInput, int sigFmt, int transFmt, int SDHD, int param_type);
    int FactorySetSharpnessParams(int sourceInput, int sigFmt, int transFmt, int SDHD, int param_type, int val);

    int FactorySetDDRSSC(int step);
    int FactoryGetDDRSSC(void);
    int FactorySetLVDSSSCLevel(int level);
    int FactoryGetLVDSSSCLevel(void);
    int FactorySetLVDSSSCFrep(int step);
    int FactoryGetLVDSSSCFrep(void);
    int FactorySetLVDSSSCMode(int mode);
    int FactoryGetLVDSSSCMode(void);
    int FactorySetNolineParams(source_input_param_t source_input_param, int type, noline_params_t noline_params);
    int FactorySetBrightnessNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    int FactorySetContrastNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    int FactorySetSaturationNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    int FactorySetHueNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    int FactorySetSharpnessNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    int FactorySetVolumeNolineParams(source_input_param_t source_input_param, noline_params_t noline_params);
    noline_params_t FactoryGetNolineParams(source_input_param_t source_input_param, int type);
    noline_params_t FactoryGetBrightnessNolineParams(source_input_param_t source_input_param);
    noline_params_t FactoryGetContrastNolineParams(source_input_param_t source_input_param);
    noline_params_t FactoryGetSaturationNolineParams(source_input_param_t source_input_param);
    noline_params_t FactoryGetHueNolineParams(source_input_param_t source_input_param);
    noline_params_t FactoryGetSharpnessNolineParams(source_input_param_t source_input_param);
    noline_params_t FactoryGetVolumeNolineParams(source_input_param_t source_input_param);
    int FactorySetColorParams(int tempMode, tvpq_rgb_ogo_t rgbogo);
    int FactoryGetColorParams(int tempMode, tvpq_rgb_ogo_t *rgbogo);
    int FactorySetDecodeLumaParams(int sourceInput, int sigFmt, int transFmt, int paramType, int value);
    int FactoryGetDecodeLumaParams(int sourceInput, int sigFmt, int transFmt, int paramType);
    int FactorySetHdmiColorRangeMode(int pq_type, int isEnable);
    int FactoryGetHdmiColorRangeMode(int pq_type);

    /*to cri data*/
    /*level: STANDARD(0), WARM(1), COLD(2), USER(3), WARMER(4), COLDER(5)*/
    /*channel: R channel(0), G channel(1), B channel(2)*/
    /*point:  gray (%0 ~ 100% ) %5 per step. 0 ~ 21*/
    /*offset:  -1023 ~ 1023*/
    int FactorySetWhitebalanceGamma(int level, int channel, int point, int offset);
    int FactoryGetWhitebalanceGamma(int level, int channel, int point);

    // This method will copy the current values of
    // parameters from the given pq mode to the currently
    // active mode.
    int CopyValuesFromPqMode(int pq_mode);

    // This method will reset the current picture mode
    // to the values set during PQ tuning process
    // Only affects the currently active source/format
    int ResetCurrentPictureMode(void);

    // This method will restore all the settings to the
    // values set during PQ tuning process
    // Affects all pq modes, all sources, all formats
    int ResetAllPictureSettings(void);

    pq_mode_parameters GetDefaultParameterValues(int pq_mode);

    pq_mode_parameters GetCurrentParameterValues(int pq_mode);

    all_pq_parameters GetALLDefaultPQParameterValues(void);

    // This method will apply the current values of
    // parameters from the currently active mode
    // to all the sources
    int ApplyUserSettingsToAllSources(void);

    int GetWBGammaPointNum(int mode);

private:
    void SendMethodCall(char *CmdString);
    int SplitRetBuf(const char *commandData);

    char mRetBuf[128] = {0};
    std::array<std::string, 10> mRet;
    sp<IBinder> mpqServicebinder;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);

//for callback method
public:
    class PqClientIObserver {
    public:
        PqClientIObserver() {};
        virtual ~PqClientIObserver() {};
        virtual void GetPqCbData(CPqClientCb &cb_data) = 0;
    };

    //used to register pqclient observer by upper client
    int RegisterObserverToPqClient(PqClientIObserver *observer);

private:
    int TransactCbData(CPqClientCb &cb_data);
    static int GetHdrTypeFromPqserver(const void *param);
    static int GetAllmGameModeFromPqserver(const void *param);
    static int GetFilmMakerFromPqserver(const void* param);
    static int GetRefreshRateFromPqserver(const void* param);

    std::map<int, PqClientIObserver *> mPqClientObserver;
    int mpqServicebinderId;
};
#ifdef __cplusplus
}
#endif
#endif
