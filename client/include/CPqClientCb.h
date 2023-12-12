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
    static const int PQ_CB_TYPE_ALLM_GAME_MODE = 1;
    static const int PQ_CB_TYPE_FILM_MAKER_MODE = 2;
    static const int PQ_CB_TYPE_REFRESH_RATE = 3;


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
            mHdrType = 6; //HDRTYPE_SDR
        }
        ~HdrTypeCb() {}
        int mHdrType;
    };

    class AllmGameModeCb: public CPqClientCb {
    public:
        AllmGameModeCb() : CPqClientCb(CPqClientCb::PQ_CB_TYPE_ALLM_GAME_MODE)
        {
            mAllmGameMode = 0; //0: game mode off; 1: game mode on
        }
        ~AllmGameModeCb() {}
        int mAllmGameMode;
    };

    class FilmMakerModeCb: public CPqClientCb {
    public:
        FilmMakerModeCb() : CPqClientCb(CPqClientCb::PQ_CB_TYPE_FILM_MAKER_MODE)
        {
            mFilmMakerMode = 0; //0: filmmaker off; 1: filmmaker on
        }
        ~FilmMakerModeCb() {}
        int mFilmMakerMode;
    };

    class RefreshRateCb: public CPqClientCb {
    public:
        RefreshRateCb() : CPqClientCb(CPqClientCb::PQ_CB_TYPE_REFRESH_RATE)
        {
            mRefreshRate = 60; //60hz
        }
        ~RefreshRateCb() {}
        int mRefreshRate;
    };
};

#ifdef __cplusplus
}
#endif

#endif
