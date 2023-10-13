/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __CAMDOLBY_VISION_H
#define __CAMDOLBY_VISION_H

#include "PQType.h"

#define DV_DEV_PATH                "/dev/amdolby_vision"

#define HDR10_POLICY_PATH          "/sys/module/amdolby_vision/parameters/dolby_vision_hdr10_policy"

/* === HDR10 === */
/* bit0: follow sink 0: bypass hdr10 to vpp 1: process hdr10 by dolby core */
/* bit1: follow source 0: bypass hdr10 to vpp 1: process hdr10 by dolby core */
/* === HDR10+ === */
/* bit2: 0: bypass hdr10+ to vpp, 1: process hdr10+ as hdr10 by dolby core */
/* === HLG -- TV core 1.6 only === */
/* bit3: follow sink 0: bypass hlg to vpp, 1: process hlg by dolby core */
/* bit4: follow source 0: bypass hlg to vpp, 1: process hlg by dolby core */
/* === SDR === */
/* bit5: 0: bypass SDR to vpp, 1: process SDR by dolby core */
/* set by policy_process */
#define HDR_BY_DV_F_SINK 0x1
#define HDR_BY_DV_F_SRC  0x2
#define HDRP_BY_DV       0x4
#define HLG_BY_DV_F_SINK 0x8
#define HLG_BY_DV_F_SRC  0x10
#define SDR_BY_DV        0x20

// ***************************************************************************
// *** ENUM definitions **********
// ***************************************************************************
typedef enum dolby_pq_mode_e {
    DOLBY_PQ_MODE_INVALID = -1,
    DOLBY_PQ_MODE_DARK_DV = 0,
    DOLBY_PQ_MODE_BRIGHT_DV,
    DOLBY_PQ_MODE_DYNAMIC_DV,
    DOLBY_PQ_MODE_STANDARD_DV,
    DOLBY_PQ_MODE_PC_DV,
    DOLBY_PQ_MODE_CUSTOMER_DV,
    DOLBY_PQ_MODE_STORE_DV,
    DOLBY_PQ_MODE_DARK_HDR10,
    DOLBY_PQ_MODE_BRIGHT_HDR10,
    DOLBY_PQ_MODE_DYNAMIC_HDR10,
    DOLBY_PQ_MODE_STANDARD_HDR10,
    DOLBY_PQ_MODE_PC_HDR10,
    DOLBY_PQ_MODE_CUSTOMER_HDR10,
    DOLBY_PQ_MODE_STORE_HDR10,
    DOLBY_PQ_MODE_DARK_HLG,
    DOLBY_PQ_MODE_BRIGHT_HLG,
    DOLBY_PQ_MODE_DYNAMIC_HLG,
    DOLBY_PQ_MODE_STANDARD_HLG,
    DOLBY_PQ_MODE_PC_HLG,
    DOLBY_PQ_MODE_CUSTOMER_HLG,
    DOLBY_PQ_MODE_STORE_HLG,
    DOLBY_PQ_MODE_DARK_SDR,
    DOLBY_PQ_MODE_BRIGHT_SDR,
    DOLBY_PQ_MODE_DYNAMIC_SDR,
    DOLBY_PQ_MODE_STANDARD_SDR,
    DOLBY_PQ_MODE_PC_SDR,
    DOLBY_PQ_MODE_CUSTOMER_SDR,
    DOLBY_PQ_MODE_STORE_SDR,
    DOLBY_PQ_MODE_MAX,
} dolby_pq_mode_t;


class CDolbyVision {
public:
    CDolbyVision(const char *binFilePath, const char *cfgFilePath);
    ~CDolbyVision();
    int SetAmdolbyCfgFile(const char *binFilePath, const char *cfgFilePath);
    int SetAmdolbyPQMode(dolby_pq_mode_t mode);
    dolby_pq_mode_t GetAmdolbyPQMode(void);
    int SetAmdolbyPQParam(dolby_pq_mode_t mode, enum pq_item_e iteamID, int value);
    int GetAmdolbyPQParam(dolby_pq_mode_t mode, enum pq_item_e iteamID);
    int SetAmdolbyPQFullParam(struct dv_full_pq_info_s fullInfo);
    int GetAmdolbyPQFullParam(struct dv_full_pq_info_s *fullInfo);
    bool isSourceCallAmdolbyCore(enum hdr_type_e hdrType);
    dolby_pq_mode_t MappingPQModeToAmdolbyVisionPQMode(enum hdr_type_e hdrType, vpp_picture_mode_t pq_mode);
    int SetAmdolbyPQDarkDetail(int mode);

private:
    int DV_OpenModule();
    int DV_CloseModule();
    int DV_DeviceIOCtl(int request, ...);
    int ReadSysfs(const char *path, char *buf, int count);

private:
    int mDVDevFd;
};
#endif /*__CAMDOLBY_VISION_H*/
