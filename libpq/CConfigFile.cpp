/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "CConfigFile"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "CPQLog.h"
#include "CConfigFile.h"
#include "Minizip.h"

CConfigFile *CConfigFile::mInstance = NULL;
CConfigFile *CConfigFile::GetInstance()
{
    if (NULL == mInstance) {
        mInstance = new CConfigFile();
    }

    return mInstance;
}

CConfigFile::CConfigFile()
{
    mpFirstSection = NULL;
    mpFileName[0] = '\0';
    mpConfigFile = NULL;
    mpFirstLine = NULL;
}

CConfigFile::~CConfigFile()
{
    LOGD("%s\n", __FUNCTION__);
    FreeAllMem();
}

bool CConfigFile::isFileExist(const char *file_name)
{
    struct stat tmp_st;
    int ret = -1;

    ret = stat(file_name, &tmp_st);
    if (ret != 0 ) {
       LOGE("%s don't exist!\n",file_name);
       return false;
    } else {
       return true;
    }
}

int CConfigFile::LoadFromFile(const char *filename)
{
    char lineStr[MAX_CONFIG_FILE_LINE_LEN];
    LINE *pCurLINE = NULL;
    SECTION *pCurSection = NULL;

    FreeAllMem();

    if (filename == NULL) {
        LOGE("%s: config file path is null!\n", __FUNCTION__);
        return -1;
    }

    LOGD("LoadFromFile name = %s\n", filename);
    if (strlen(filename) <= sizeof(mpFileName)/sizeof(char)) {
        strcpy(mpFileName, filename);
    }
    if ((mpConfigFile = fopen (mpFileName, "r")) == NULL) {
        LOGE("open %s fail!!!\n", filename);
        return -1;
    }

    while (fgets (lineStr, MAX_CONFIG_FILE_LINE_LEN, mpConfigFile) != NULL) {
        allTrim(lineStr);

        LINE *pLINE = new LINE();
        pLINE->pKeyStart = pLINE->Text;
        pLINE->pKeyEnd = pLINE->Text;
        pLINE->pValueStart = pLINE->Text;
        pLINE->pValueEnd = pLINE->Text;
        pLINE->pNext = NULL;
        pLINE->type = getLineType(lineStr);
        //LOGD("getline=%s len=%d type=%d", lineStr, strlen(lineStr), pLINE->type);
        strcpy(pLINE->Text, lineStr);
        pLINE->LineLen = strlen(pLINE->Text);

        //head
        if (mpFirstLine == NULL) {
            mpFirstLine = pLINE;
        } else {
            pCurLINE->pNext = pLINE;
        }

        pCurLINE = pLINE;

        switch (pCurLINE->type) {
        case LINE_TYPE_SECTION: {
            SECTION *pSec = new SECTION();
            pSec->pLine = pLINE;
            pSec->pNext = NULL;
            if (mpFirstSection == NULL) { //first section
                mpFirstSection = pSec;
            } else {
                pCurSection->pNext = pSec;
            }
            pCurSection = pSec;
            break;
        }
        case LINE_TYPE_KEY: {
            char *pM = strchr(pCurLINE->Text, '=');
            pCurLINE->pKeyStart = pCurLINE->Text;
            pCurLINE->pKeyEnd = pM - 1;
            pCurLINE->pValueStart = pM + 1;
            pCurLINE->pValueEnd = pCurLINE->Text + pCurLINE->LineLen - 1;
            break;
        }
        case LINE_TYPE_COMMENT:
        default:
            break;
        }
    }

    fclose (mpConfigFile);
    mpConfigFile = NULL;
    return 0;
}

int CConfigFile::SaveToFile(const char *filename)
{
    const char *filepath = NULL;
    FILE *pFile = NULL;

    if (filename == NULL) {
        if (strlen(mpFileName) == 0) {
            LOGE("error save file is null.\n");
            return -1;
        } else {
            filepath = mpFileName;
        }
    } else {
        filepath = filename;
    }
    //LOGD("Save to file name = %s", file);

    if ((pFile = fopen (filepath, "wb")) == NULL) {
        LOGD("Save to file open error = %s", filepath);
        return -1;
    }

    LINE *pCurLine = NULL;
    for (pCurLine = mpFirstLine; pCurLine != NULL; pCurLine = pCurLine->pNext) {
        fprintf (pFile, "%s\r\n", pCurLine->Text);
    }

    fflush(pFile);
    fsync(fileno(pFile));
    fclose(pFile);
    return 0;
}

int CConfigFile::SetString(const char *section, const char *key, const char *value)
{
    SECTION *pNewSec = NULL;
    LINE *pNewSecLine = NULL;
    LINE *pNewKeyLine = NULL;

    SECTION *pSec = getSection(section);
    if (pSec == NULL) {
        pNewSec = new SECTION();
        pNewSecLine = new LINE();
        pNewKeyLine = new LINE();

        pNewKeyLine->type = LINE_TYPE_KEY;
        pNewSecLine->type = LINE_TYPE_SECTION;


        sprintf(pNewSecLine->Text, "[%s]", section);
        pNewSec->pLine = pNewSecLine;

        InsertSection(pNewSec);

        int keylen = strlen(key);
        sprintf(pNewKeyLine->Text, "%s=%s", key, value);
        pNewKeyLine->LineLen = strlen(pNewKeyLine->Text);
        pNewKeyLine->pKeyStart = pNewKeyLine->Text;
        pNewKeyLine->pKeyEnd = pNewKeyLine->pKeyStart + keylen - 1;
        pNewKeyLine->pValueStart = pNewKeyLine->pKeyStart + keylen + 1;
        pNewKeyLine->pValueEnd = pNewKeyLine->Text + pNewKeyLine->LineLen - 1;

        InsertKeyLine(pNewSec, pNewKeyLine);

    } else { //find section
        LINE *pLine = getKeyLineAtSec(pSec, key);
        if (pLine == NULL) { //, not find key
            pNewKeyLine = new LINE();
            pNewKeyLine->type = LINE_TYPE_KEY;

            int keylen = strlen(key);
            sprintf(pNewKeyLine->Text, "%s=%s", key, value);
            pNewKeyLine->LineLen = strlen(pNewKeyLine->Text);
            pNewKeyLine->pKeyStart = pNewKeyLine->Text;
            pNewKeyLine->pKeyEnd = pNewKeyLine->pKeyStart + keylen - 1;
            pNewKeyLine->pValueStart = pNewKeyLine->pKeyStart + keylen + 1;
            pNewKeyLine->pValueEnd = pNewKeyLine->Text + pNewKeyLine->LineLen - 1;

            InsertKeyLine(pSec, pNewKeyLine);
        } else { //all find, change it
            sprintf(pLine->Text, "%s=%s", key, value);
            pLine->LineLen = strlen(pLine->Text);
            pLine->pValueEnd = pLine->Text + pLine->LineLen - 1;
        }
    }

    //save
    SaveToFile(NULL);
    return 0;
}

int CConfigFile::SetInt(const char *section, const char *key, int value)
{
    char tmp[64];
    sprintf(tmp, "%d", value);
    SetString(section, key, tmp);
    return 0;
}

const char *CConfigFile::GetString(const char *section, const char *key, const char *def_value)
{
    SECTION *pSec = getSection(section);
    if (pSec == NULL) {
        return def_value;
    }
    LINE *pLine = getKeyLineAtSec(pSec, key);
    if (pLine == NULL) {
        return def_value;
    }
    return pLine->pValueStart;
}

int CConfigFile::GetInt(const char *section, const char *key, int def_value)
{
    const char *num = GetString(section, key, NULL);
    if (num != NULL) {
        return atoi(num);
    }
    return def_value;
}

int CConfigFile::SetFloat(const char *section, const char *key, float value)
{
    char tmp[64];
    sprintf(tmp, "%.2f", value);
    SetString(section, key, tmp);
    return 0;
}

float CConfigFile::GetFloat(const char *section, const char *key, float def_value)
{
    const char *num = GetString(section, key, NULL);
    if (num != NULL) {
        return atof(num);
    }
    return def_value;
}

LINE_TYPE CConfigFile::getLineType(char *Str)
{
    LINE_TYPE type = LINE_TYPE_COMMENT;
    if (strchr(Str, '#') != NULL) {
        type = LINE_TYPE_COMMENT;
    } else if ( (strstr (Str, "[") != NULL) && (strstr (Str, "]") != NULL) ) { /* Is Section */
        type = LINE_TYPE_SECTION;
    } else {
        if (strstr (Str, "=") != NULL) {
            type = LINE_TYPE_KEY;
        } else {
            type = LINE_TYPE_COMMENT;
        }
    }
    return type;
}

void CConfigFile::FreeAllMem()
{
    //line
    LINE *pCurLine = NULL;
    LINE *pNextLine = NULL;
    for (pCurLine = mpFirstLine; pCurLine != NULL;) {
        pNextLine = pCurLine->pNext;
        delete pCurLine;
        pCurLine = pNextLine;
    }
    mpFirstLine = NULL;
    //section
    SECTION *pCurSec = NULL;
    SECTION *pNextSec = NULL;
    for (pCurSec = mpFirstSection; pCurSec != NULL;) {
        pNextSec = pCurSec->pNext;
        delete pCurSec;
        pCurSec = pNextSec;
    }
    mpFirstSection = NULL;
}

int CConfigFile::InsertSection(SECTION *pSec)
{
    //insert it to sections list ,as first section
    pSec->pNext = mpFirstSection;
    mpFirstSection = pSec;
    //insert it to lines list, at first
    pSec->pLine->pNext = mpFirstLine;
    mpFirstLine = pSec->pLine;
    return 0;
}

int CConfigFile::InsertKeyLine(SECTION *pSec, LINE *line)
{
    LINE *line1 = pSec->pLine;
    LINE *line2 = line1->pNext;
    line1->pNext = line;
    line->pNext = line2;
    return 0;
}

SECTION *CConfigFile::getSection(const char *section)
{
    //section
    for (SECTION *psec = mpFirstSection; psec != NULL; psec = psec->pNext) {
        if (strncmp((psec->pLine->Text) + 1, section, strlen(section)) == 0)
            return psec;
    }
    return NULL;
}

LINE *CConfigFile::getKeyLineAtSec(SECTION *pSec, const char *key)
{
    //line
    for (LINE *pline = pSec->pLine->pNext; (pline != NULL && pline->type != LINE_TYPE_SECTION); pline = pline->pNext) {
        if (pline->type == LINE_TYPE_KEY) {
            if (strncmp(pline->Text, key, strlen(key)) == 0) {
                return pline;
            }
        }
    }
    return NULL;
}

void CConfigFile::allTrim(char *Str)
{
    char *pStr;
    pStr = strchr (Str, '\n');
    if (pStr != NULL) {
        *pStr = 0;
    }
    int Len = strlen(Str);
    if ( Len > 0 ) {
        if ( Str[Len - 1] == '\r' ) {
            Str[Len - 1] = '\0';
        }
    }
    pStr = Str;
    while (*pStr != '\0') {
        if (*pStr == ' ') {
            char *pTmp = pStr;
            while (*pTmp != '\0') {
                *pTmp = *(pTmp + 1);
                pTmp++;
            }
        } else {
            pStr++;
        }
    }
    return;
}

const char *CConfigFile::GetDatabaseFilePath(const char *section, const char *key, const char *def_value)
{
    return GetString(section, key, def_value);
}

const char *CConfigFile::GetSettingDataFilePath(const char *section, const char *key, const char *def_value)
{
    return GetString(section, key, def_value);
}

const char *CConfigFile::GetWhiteBalanceFilePath(const char *section, const char *key, const char *def_value)
{
    return GetString(section, key, def_value);
}

const char *CConfigFile::GetFilePath(const char *section, const char *key, const char *def_value)
{
    return GetString(section, key, def_value);
}

void CConfigFile::GetDvFilePath(char *bin_file_path, char *cfg_file_path)
{
    //read dv file path from pq_default.ini
    const char *pqdvbinPath    = NULL;
    pqdvbinPath = GetString(CFG_SECTION_PQ, CFG_PQ_DV_BIN_PATH, DV_BIN_FILE_DEFAULT_PATH);

    if (isFileExist(pqdvbinPath)) {
        strcpy(bin_file_path, pqdvbinPath);
    } else if (isFileExist(DV_BIN_FILE_DEFAULT_PATH)) {
        strcpy(bin_file_path, DV_BIN_FILE_DEFAULT_PATH);
    } else {
        LOGE("no dv_config.bin in %s\n", DV_BIN_FILE_DEFAULT_PATH);
    }

    const char *pqdvcfgPath = NULL;
    pqdvcfgPath = GetString(CFG_SECTION_PQ, CFG_PQ_DV_CFG_PATH, DV_CFG_FILE_DEFAULT_PATH);

    if (isFileExist(pqdvcfgPath)) {
        strcpy(cfg_file_path, pqdvcfgPath);
    } else if (isFileExist(DV_CFG_FILE_DEFAULT_PATH)) {
        strcpy(cfg_file_path, DV_CFG_FILE_DEFAULT_PATH);
    } else {
        LOGE("no Amlogic_dv.cfg in %s\n", DV_CFG_FILE_DEFAULT_PATH);
    }
}

void CConfigFile::GePqDbFilePath(char *db_file_path)
{
    if (!isFileExist(db_file_path)) {
        LOGD("%s there is no %s, decompress pq.bin to pq.db\n", __FUNCTION__, db_file_path);

        char bin_file_path[128] = {0};
        char ch = '.';
        char *ptr = NULL;

        ptr = strchr(db_file_path, ch);
        if (ptr != NULL) {
            strncpy(bin_file_path, db_file_path, ptr - db_file_path);
            strcat(bin_file_path, ".bin\0");
        } else {
            LOGE("%s strchr failed\n", __FUNCTION__);
        }
        LOGD("%s bin_file_path is %s\n", __FUNCTION__, bin_file_path);

        if (!isFileExist(bin_file_path)) {
            LOGE("%s %s is not exit\n", __FUNCTION__, bin_file_path);
        } else {
            //decompress pq.bin to pq.db
            Minizip *pMiniz   = NULL;
            pMiniz = new Minizip();
            if (pMiniz->CheckAndUpdateUncompressFile(db_file_path, bin_file_path) != 0) {
                LOGE("%s decompress %s failed\n", __FUNCTION__, bin_file_path);
            } else {
                LOGD("%s decompress %s to %s success\n", __FUNCTION__, bin_file_path, db_file_path);
            }

            pMiniz->freeAll();
            delete pMiniz;
            pMiniz = NULL;
        }
    } else {
        LOGD("%s there is %s, no need decompress pq.bin\n", __FUNCTION__, db_file_path);
    };
}