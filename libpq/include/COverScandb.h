/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */


#ifndef COVERSCANDB_H_
#define COVERSCANDB_H_

#include "CSqlite.h"
#include "PQType.h"

class COverScandb: public CSqlite {
public:
    COverScandb();
    ~COverScandb();
    int openOverScanDB(const char *);
    bool GetOverScanDbVersion(std::string& ToolVersion, std::string& ProjectVersion, std::string& dbversion, std::string& GenerateTime);
    int PQ_GetOverscanParams(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t *cutwin_t);
    int PQ_SetOverscanParams(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t cutwin_t);
    int PQ_ResetAllOverscanParams(void);
    int PQ_GetPQModeParams(tv_source_input_t source_input, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    int PQ_SetPQModeParams(tv_source_input_t source_input, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    int PQ_SetPQModeParamsByName(const char *name, tv_source_input_t source_input, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    int PQ_ResetAllPQModeParams(void);
    bool CheckIdExistInDb(const char *Id, const char *TableName);

private:
    std::string GetTableName(const char *GeneralTableName, source_input_param_t source_input_param);
    void PQ_GetOverscanTableName(vpp_display_mode_t dmode, char *table_name, int size);

public:
    hdr_type_t mHdrType = HDR_TYPE_SDR;
    unsigned int mOverScanDbVersion;
};
#endif
