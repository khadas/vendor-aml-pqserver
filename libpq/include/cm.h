/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef _TVOUT_CM_H
#define _TVOUT_CM_H

typedef struct am_regs_s  am_regs_t;

typedef struct cm_regmap_s {
    unsigned long reg[50];
} cm_regmap_t;

/*pq_timing:
 *SD/HD/FHD/UHD for DTV/MEPG,
 *NTST_M/NTST_443/PAL_I/PAL_M/PAL_60/PAL_CN/SECAM/NTST_50 for AV/ATV
 */
typedef enum ve_pq_timing_type_e {
    SIG_TIMING_TYPE_SD = 0,
    SIG_TIMING_TYPE_HD,
    SIG_TIMING_TYPE_FHD,
    SIG_TIMING_TYPE_UHD,
    SIG_TIMING_TYPE_NTSC_M,
    SIG_TIMING_TYPE_NTSC_443,
    SIG_TIMING_TYPE_PAL_I,
    SIG_TIMING_TYPE_PAL_M,
    SIG_TIMING_TYPE_PAL_60,
    SIG_TIMING_TYPE_PAL_CN,
    SIG_TIMING_TYPE_SECAM,
    SIG_TIMING_TYPE_NTSC_50,
    SIG_TIMING_TYPE_MAX,
} ve_pq_timing_type_t;

/*
 *src_timing: load status: bit31 ->1: load/save all crop; 0: load one according to timing
              screen mode: bit24~bit30 ->dispiay mode
              source: bit16~bit23 -> source
              timing: bit0 ~bit15 -> sd/hd/fhd/uhd
 *value1: bit0 ~bit15 -> hs
          bit16~bit31 -> he
 *value2: bit0~ bit15 -> vs
          bit16~bit31 -> ve
 */
typedef struct ve_pq_table_s {
    unsigned int src_timing;
    unsigned int value1;
    unsigned int value2;
    unsigned int reserved1;
    unsigned int reserved2;
} ve_pq_table_t;

#endif  // _TVOUT_CM_H
