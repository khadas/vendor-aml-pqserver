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

#if (TV_IPC_TYPE == TV_DBUS)
#include <dbus/dbus.h>
#endif
#include <map>
#include <memory>

#if (TV_IPC_TYPE == TV_BINDER)
#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#endif
#include "common.h"

#if (TV_IPC_TYPE == TV_BINDER)
using namespace android;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (TV_IPC_TYPE == TV_BINDER)
class PqClient : public BBinder{
#else //(TV_IPC_TYPE == TV_DBUS)
class PqClient {
#endif
public:
#if (TV_IPC_TYPE == TV_BINDER)
    enum {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_PQ_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
        CMD_SET_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 2,
        CMD_CLR_PQ_CB = IBinder::FIRST_CALL_TRANSACTION + 3,
        EVT_SRC_CT_CB = IBinder::FIRST_CALL_TRANSACTION + 4,
        EVT_SIG_DT_CB = IBinder::FIRST_CALL_TRANSACTION + 5,
    };
#endif

    PqClient();
    ~PqClient();
    static PqClient *GetInstance();

    int SetPQMode(int mode, int isSave = 0);
    int GetPQMode();
    int SetColorTemperature(int colorTemperatureValue, int isSave = 0);
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

    int FactorySetRGBPattern(int r, int g, int b);
    int FactoryGetRGBPattern();
    int FactorySetGrayPattern(int value);
    int FactoryGetGrayPattern();
    int FactorySetWhiteBalanceRedGain(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceRedGain(int source, int colortemptureMode);
    int FactorySetWhiteBalanceGreenGain(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceGreenGain(int source, int colortemptureMode);
    int FactorySetWhiteBalanceBlueGain(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceBlueGain(int source, int colortemptureMode);
    int FactorySetWhiteBalanceRedPostOffset(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceRedPostOffset(int source, int colortemptureMode);
    int FactorySetWhiteBalanceGreenPostOffset(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceGreenPostOffset(int source, int colortemptureMode);
    int FactorySetWhiteBalanceBluePostOffset(int source, int colortemptureMode, int value);
    int FactoryGetWhiteBalanceBluePostOffset(int source, int colortemptureMode);
private:
#if (TV_IPC_TYPE == TV_DBUS)
    DBusConnection *ClientBusInit();
    DBusConnection *mpDBusConnection = NULL;
#endif
    void SendMethodCall(char *CmdString);
    int SplitRetBuf(const char *commandData);
    char mRetBuf[128] = {0};
    std::string  mRet[10];
#if (TV_IPC_TYPE == TV_BINDER)
    sp<IBinder> mpqServicebinder;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
#endif
};
#ifdef __cplusplus
}
#endif
#endif
