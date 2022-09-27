/*
 * Copyright (c) 2022 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file (same as CPQControlCb.h)
 */

#ifndef _CPQCB_C_H_
#define _CPQCB_C_H_

#ifdef __cplusplus
extern "C" {
#endif

class CPqClientCb {
public:
    static const int PQ_CB_TYPE_HDRTYPE = 0;


    CPqClientCb(int type) {
        mCbType = type;
    }
    ~CPqClientCb() {

    }
    int getCbType() {
        return mCbType;
    }
private:
    int mCbType;
};

namespace  PqClientCb {
    class HdrTypeCb: public CPqClientCb {
    public:
        HdrTypeCb() : CPqClientCb(CPqClientCb::PQ_CB_TYPE_HDRTYPE)
        {
            mHdrType = 6; //HDR_TYPE_SDR
        }
        ~HdrTypeCb() {}
        int mHdrType;
    };
};

#ifdef __cplusplus
}
#endif

#endif
