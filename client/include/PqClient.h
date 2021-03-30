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

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include "common.h"

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

class PqClient : public BBinder{
public:

    enum {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_PQ_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
        CMD_SET_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 2,
        CMD_CLR_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 3,
        EVT_SRC_CT_CB = IBinder::FIRST_CALL_TRANSACTION + 4,
        EVT_SIG_DT_CB = IBinder::FIRST_CALL_TRANSACTION + 5,
    };

    PqClient();
    ~PqClient();
    static PqClient *GetInstance();

    int SetPQMode(int mode, int isSave = 0);
    int GetPQMode();
    int SetColorTemperature(int colorTemperatureValue, int isSave, int rgb_type, int value);
    int GetColorTemperature();
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
    int FactorySetOverscanParams(int sourceInput, int sigFmt, int transFmt, tvin_cutwin_t cutwin_t);
    tvin_cutwin_t FactoryGetOverscanParams(int sourceInput, int sigFmt, int transFmt);
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

private:
    void SendMethodCall(char *CmdString);
    int SplitRetBuf(const char *commandData);
    char mRetBuf[128] = {0};
    std::string  mRet[10];

    sp<IBinder> mpqServicebinder;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
};
#ifdef __cplusplus
}
#endif
#endif
