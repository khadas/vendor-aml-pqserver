/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef __AMVECM_H
#define __AMVECM_H

#include "cm.h"
#include "ldim.h"

#define GAMMA_NUMBER    257
typedef struct tcon_gamma_table_s tcon_gamma_table_t;
typedef struct tcon_rgb_ogo_s     tcon_rgb_ogo_t;

#define AMVECM_IOC_MAGIC               'C'
#define LDIM_IOC_PARA                  _IOW(AMVECM_IOC_MAGIC, 0x50, struct vpu_ldim_param_s)
#define AML_LDIM_IOC_CMD_SET_INFO_NEW  _IOW(AMVECM_IOC_MAGIC, 0x54, struct aml_ldim_pq_s)

#endif /* __AMVECM_H */

