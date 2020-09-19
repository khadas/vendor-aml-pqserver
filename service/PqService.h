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

using namespace android;

typedef struct pq_moudle_param_s {
    int moudleId;        //moudleId according to tvcmd.h
    int paramLength;     //length of parambuf
    int paramBuf[10];    //param for action
} pq_moudle_param_t;

class PqService: public BBinder{
public:
    PqService();
    ~PqService();
    static PqService *GetInstance();
    int PqServiceHandleMessage();

    enum command {
        CMD_START = IBinder::FIRST_CALL_TRANSACTION,
        CMD_PQ_ACTION = IBinder::FIRST_CALL_TRANSACTION + 1,
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
    std::string mPqCommand[10];
};
#endif
