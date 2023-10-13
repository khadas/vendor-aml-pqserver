/*
 * Copyright (c) 2022 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: CPQControl callback type header file
 */

#ifndef _CPQCB_S_H_
#define _CPQCB_S_H_

#ifdef __cplusplus
extern "C" {
#endif

class CPQControlCb {
public:
    static const int PQ_CB_TYPE_HDRTYPE = 0;


    CPQControlCb(int type) {
        mCbType = type;
    }
    ~CPQControlCb() {

    }
    int getCbType() {
        return mCbType;
    }
private:
    int mCbType;
};

namespace  PQControlCb {
    class HdrTypeCb: public CPQControlCb {
    public:
        HdrTypeCb() : CPQControlCb(CPQControlCb::PQ_CB_TYPE_HDRTYPE)
        {
            mHdrType = 6; //HDRTYPE_SDR
        }
        ~HdrTypeCb() {}
        int mHdrType;
    };
};

#ifdef __cplusplus
}
#endif

#endif
