/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef CPQDB_H_
#define CPQDB_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CSqlite.h"
#include "PQType.h"
#include "CPQLog.h"
#include <string>

#define PQ_DB_CODE_VERSION        20201211

typedef enum code_db_version_e {
    PQ_DB_CODE_VERSION_0 = 20191113,
    PQ_DB_CODE_VERSION_1 = 20201029,  //split hdr10/hdr10plus/hlg/dv
    PQ_DB_CODE_VERSION_2 = 20201211,  //add picture mode 5,add general gamma,remove picture mode table from overscan.db to pq.db
} code_db_version_t;

//pq db table name
#define PQ_DB_VERSION_TABLE_NAME                "PQ_VersionTable"
#define PQ_DB_LDIM_TABLE_NAME                   "LD_HDMI"
#define PQ_DB_GENERALPICTUREMODE_TABLE_NAME     "GeneralPictureMode5Table"
#define PQ_DB_GENERALGAMMA_TABLE_NAME           "GeneralGammaTable"
#define PQ_DB_LCD_HDRINFO_TABLE_NAME            "Panel"

typedef struct database_attribute_s {
    std::string ToolVersion;
    std::string ProjectVersion;
    std::string GenerateTime;
    std::string ChipVersion;
    std::string dbversion;
} database_attribute_t;


class CPQdb: public CSqlite {
public:
    CPQdb();
    ~CPQdb();
    int PQ_GetBlackExtensionParams(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetSharpness0FixedParams(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetSharpness1FixedParams(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_SetSharpness0VariableParams(source_input_param_t source_input_param);
    int PQ_SetSharpness1VariableParams(source_input_param_t source_input_param);
    int LoadVppBasicParam(tvpq_data_type_t data_type, source_input_param_t source_input_param);
    int PQ_GetCM2Params(vpp_color_management2_t basemode, source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetXVYCCParams(vpp_xvycc_mode_t xvycc_mode, source_input_param_t source_input_param, am_regs_t *regs, am_regs_t *regs_1);
    int PQ_GetDIParams(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetMCDIParams(vpp_mcdi_mode_t mcdi_mode, source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetDeblockParams(vpp_deblock_mode_t mode, source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetNR2Params(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetDemoSquitoParams(source_input_param_t source_input_param, am_regs_t *regs);
    int getDIRegValuesByValue(const char *name, const char *f_name, const char *f2_name, const int val, const int val2, am_regs_t *regs);
    int PQ_GetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, source_input_param_t source_input_param, tcon_rgb_ogo_t *params);
    int PQ_SetColorTemperatureParams(vpp_color_temperature_mode_t Tempmode, source_input_param_t source_input_param, tcon_rgb_ogo_t params);
    int PQ_ResetAllColorTemperatureParams(void);
    int PQ_SetNoLineAllBrightnessParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllBrightnessParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_GetBrightnessParams(source_input_param_t source_input_param, int level, int *params);
    int PQ_SetBrightnessParams(source_input_param_t source_input_param, int level, int params);
    int PQ_SetNoLineAllContrastParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllContrastParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_GetContrastParams(source_input_param_t source_input_param, int level, int *params);
    int PQ_SetContrastParams(source_input_param_t source_input_param, int level, int params);
    int PQ_SetNoLineAllSaturationParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllSaturationParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_GetSaturationParams(source_input_param_t source_input_param, int level, int *params);
    int PQ_SetSaturationParams(source_input_param_t source_input_param, int level, int params);
    int PQ_SetNoLineAllHueParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllHueParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_GetHueParams(source_input_param_t source_input_param, int level, int *params);
    int PQ_SetHueParams(source_input_param_t source_input_param, int level, int params);
    int PQ_SetNoLineAllSharpnessParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllSharpnessParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_GetSharpness0Params(source_input_param_t source_input_param, int level, am_regs_t *regs);
    int PQ_GetSharpness1Params(source_input_param_t source_input_param, int level, am_regs_t *regs);
    int PQ_SetSharpnessParams(source_input_param_t source_input_param, int level, am_regs_t regs);
    int PQ_SetNoLineAllVolumeParams(tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_GetNoLineAllVolumeParams(tv_source_input_t source_input, int *osd0, int *osd25, int *osd50, int *osd75, int *osd100);
    int PQ_ResetAllNoLineParams(void);
    int PQ_GetNoiseReductionParams(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param, int reg_addr);
    int PQ_SetNoiseReductionParams(vpp_noise_reduction_mode_t nr_mode, source_input_param_t source_input_param, int reg_addr, int value);
    int PQ_GetDNLPParams(source_input_param_t source_input_param, Dynamic_contrast_mode_t mode, ve_dnlp_curve_param_t *newParams);
    int PQ_SetDNLPGains(source_input_param_t source_input_param, Dynamic_contrast_mode_t level, int final_gain);
    int PQ_GetDNLPGains(source_input_param_t source_input_param, Dynamic_contrast_mode_t level);
    int PQ_GetLocalContrastNodeParams(source_input_param_t source_input_param, local_contrast_mode_t mode, ve_lc_curve_parm_t *Params);
    int PQ_GetLocalContrastRegParams(source_input_param_t source_input_param, local_contrast_mode_t mode, am_regs_t *regs);
    int PQ_GetOverscanParams(source_input_param_t source_input_param, vpp_display_mode_t dmode, tvin_cutwin_t *cutwin_t);
    int PQ_SetOverscanParams(source_input_param_t source_input_param, tvin_cutwin_t cutwin_t);
    int PQ_ResetAllOverscanParams(void);
    int PQ_GetPQModeParams(source_input_param_t source_input_param, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    int PQ_SetPQModeParams(tv_source_input_t source_input, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    int PQ_ResetAllPQModeParams(void);
    int PQ_GetGammaTableR(int panel_id, source_input_param_t source_input_param, tcon_gamma_table_t *gamma_r);
    int PQ_GetGammaTableG(int panel_id, source_input_param_t source_input_param, tcon_gamma_table_t *gamma_g);
    int PQ_GetGammaTableB(int panel_id, source_input_param_t source_input_param, tcon_gamma_table_t *gamma_b);
    int PQ_GetGammaSpecialTable(vpp_gamma_curve_t gamma_curve, const char *f_name, tcon_gamma_table_t *gamma_r);
    int PQ_GetGammaParams(source_input_param_t source_input_param, vpp_gamma_curve_t gamma_curve, const char *f_name, tcon_gamma_table_t *gamma_value);
    int PQ_GetPLLParams(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetCVD2Params(source_input_param_t source_input_param, am_regs_t *regs);
    int PQ_GetLCDHDRInfoParams(source_input_param_t source_input_param, lcd_optical_info_t *newParams);
    int PQ_GetLDIMParams(source_input_param_t source_input_param, aml_ldim_info_s *newParams);
    int openPqDB(const char *db_path);
    int reopenDB(const char *db_path);
    bool PQ_GetDataBaseAttribute(database_attribute_t *DbAttribute);
    int getRegValues(const char *table_name, am_regs_t *regs);
    int getRegValuesByValue(const char *name, const char *f_name, const char *f2_name, const int val, const int val2, am_regs_t *regs);
    int getRegValuesByValue_long(const char *name, const char *f_name, const char *f2_name, const int val, const int val2, am_regs_t *regs, am_regs_t *regs_1);
    bool PQ_GetLDIM_Regs(vpu_ldim_param_s *vpu_ldim_param);
    int GetFileAttrIntValue(const char *fp, int flag);
    bool CheckHdrStatus(const char *tableName);
    bool CheckPQModeTableInDb(void);
    bool CheckIdExistInDb(const char *Id, const char *TableName);

private:
    std::string GetTableName(const char *GeneralTableName, source_input_param_t source_input_param);
    int CaculateLevelParam(tvpq_data_t *pq_data, int nodes, int level);
    am_regs_t CaculateLevelRegsParam(tvpq_sharpness_regs_t *pq_regs, int level, int sharpness_number);
    int GetNonlinearMapping(tvpq_data_type_t data_type, tv_source_input_t source_input, int level, int *params);
    int GetNonlinearMappingByOSDFac(tvpq_data_type_t data_type, tv_source_input_t source_input, int *params);
    int SetNonlinearMapping(tvpq_data_type_t data_type, tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int loadSharpnessData(const char *table_name, int sharpness_number);
    int PQ_GetGammaTable(int panel_id, source_input_param_t source_input_param, const char *f_name, tcon_gamma_table_t *val);
    int SetNonlinearMappingByName(const char *name, tvpq_data_type_t data_type, tv_source_input_t source_input, int osd0, int osd25, int osd50, int osd75, int osd100);
    int PQ_SetPQModeParamsByName(const char *name, tv_source_input_t source_input, vpp_picture_mode_t pq_mode, vpp_pq_para_t *params);
    const char *getSharpnessTableName(source_input_param_t source_input_param, int isHd);
    am_regs_t MergeSameAddrVal(am_regs_t regs);

    tvpq_data_t pq_bri_data[15];
    tvpq_data_t pq_con_data[15];
    tvpq_data_t pq_sat_data[15];
    tvpq_data_t pq_hue_data[15];
    tvpq_sharpness_regs_t pq_sharpness0_reg_data[10];
    tvpq_sharpness_regs_t pq_sharpness1_reg_data[10];
    int bri_nodes;
    int con_nodes;
    int hue_nodes;
    int sat_nodes;
    int sha0_nodes;
    int sha1_nodes;
public:
    bool mHdrStatus = false;
    hdr_type_t mHdrType = HDR_TYPE_SDR;
    output_type_t mOutPutType = OUTPUT_TYPE_LVDS;
    colortemperature_type_t mColorTemperatureMode = LVDS_STD;
    unsigned int mDbVersion;
};
#endif
