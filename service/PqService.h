/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef PQ_SERVICE_H
#define PQ_SERVICE_H

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include "CPQControl.h"
#include <map>

using namespace android;

typedef struct pq_moudle_param_s {
    int moduleId;        //moduleId according to tvcmd.h
    int paramLength;     //length of parambuf
    int paramBuf[20];    //param for action
} pq_moudle_param_t;

class PqService: public CPQControl::ICPQControlObserver,
                 public BBinder {
public:
    PqService();
    ~PqService();
    static PqService *GetInstance();
    int PqServiceHandleMessage();

    enum command {
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
    };

private:
    void ParserPqCommand(const char *commandData);
    int SplitCommand(const char *commandData);
    int SetCmd(pq_moudle_param_t param);
    char* GetCmd(pq_moudle_param_t param);

    char mRetBuf[128] = {0};

    CPQControl *mpPQcontrol;
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
    std::string mPqCommand[20];

private:
    //used to send client binder proxy object pointer to pqserver, to realize service <--> client two-way communication
    int SetClientProxyToServer(sp<IBinder> callBack);
    //used to libpq callback data to pqserver
    virtual void GetCbDataFromLibpq(CPQControlCb &cb_data);

    void SetHdrType(CPQControlCb &cb_data);
    void SetAllmGameMode(CPQControlCb &cb_data);
    void SetFilmMakerMode(CPQControlCb &cb_data);
    void SetRefreshRate(CPQControlCb &cb_data);

    std::map<int, sp<IBinder>> mPqServiceCallBack;

};
#endif
