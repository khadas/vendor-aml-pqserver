/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "SSMAction"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "SSMAction.h"
#include "CPQLog.h"

pthread_mutex_t ssm_r_w_op_mutex = PTHREAD_MUTEX_INITIALIZER;

SSMAction *SSMAction::mInstance;
SSMHandler *SSMAction::mSSMHandler = NULL;
SSMAction *SSMAction::getInstance()
{
    if (NULL == mInstance)
        mInstance = new SSMAction();
    return mInstance;
}

SSMAction::SSMAction()
{
}

SSMAction::~SSMAction()
{
    if (m_dev_fd > 0) {
        close(m_dev_fd);
        m_dev_fd = -1;
    }

    if (mSSMHandler != NULL) {
        delete mSSMHandler;
        mSSMHandler = NULL;
    }
}

void SSMAction::init(char *settingDataPath, char *whiteBalanceDataPath)
{
    //check file PATH
    char filePath0[64] = {0};
    sprintf(filePath0, "%s/cri_data", whiteBalanceDataPath);
    strcpy(mWhiteBalanceFilePath, filePath0);
    LOGD("%s: mWhiteBalanceFilePath:%s\n", __FUNCTION__, mWhiteBalanceFilePath);

    char filePath1[64] = {0};
    sprintf(filePath1, "%s/ssm_data", settingDataPath);
    mSSMDataFilePath = filePath1;
    LOGD("%s: mSSMDataFilePath:%s\n", __FUNCTION__, mSSMDataFilePath);

    char filePath2[64] = {0};
    sprintf(filePath2, "%s/SSMHandler", settingDataPath);
    mSSMHandlerFilePath = filePath2;
    LOGD("%s: mSSMHandlerFilePath:%s\n", __FUNCTION__, mSSMHandlerFilePath);

    //check file exist or not
    bool ssm_data_FileExist    = isFileExist(mSSMDataFilePath);
    bool ssm_handler_FileExist = isFileExist(mSSMHandlerFilePath);
    bool cri_data_FileExist    = isFileExist(mWhiteBalanceFilePath);
    LOGD("%s, ssm_handler_FileExist:%d cri_data_FileExist:%d\n", __FUNCTION__, ssm_handler_FileExist, cri_data_FileExist);
    //open SSM handler
    mSSMHandler = SSMHandler::GetSingletonInstance(mSSMHandlerFilePath);
    //open ssm file
    if (!ssm_data_FileExist) {
        LOGD("%s, %s don't exist,create and open\n", __FUNCTION__, mSSMDataFilePath);
        m_dev_fd = open(mSSMDataFilePath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    } else {
        LOGD("open %s file\n",mSSMDataFilePath);
        m_dev_fd = open(mSSMDataFilePath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    }

    if (m_dev_fd < 0) {
        LOGE("%s, Open %s failed error: %s\n", __FUNCTION__, mSSMDataFilePath, strerror(errno));
    } else {
        LOGD("%s, Open %s success\n", __FUNCTION__, mSSMDataFilePath);
    }
    //ssm check
    if (mSSMHandler != NULL) {
        SSM_status_t SSM_status = (SSM_status_t)GetSSMStatus();
        LOGD("%s, Verify SSMHeader status= %d\n", __FUNCTION__, SSM_status);
        if (DeviceMarkCheck() < 0 || SSM_status == SSM_HEADER_INVALID) {
            if (mpObserver != NULL) {
                EraseAllData();
                mpObserver->resetAllUserSettingParam();
                //SSMRestoreDefault(0, true);
                mSSMHandler->SSMRecreateHeader();
                RestoreDeviceMarkValues();
            } else {
                LOGE("%s: SSMActionObserver is NULL\n", __FUNCTION__);
            }
        } else {
            LOGD("%s, Verify ssm_data and ssmhandler is ok\n", __FUNCTION__);
        }
    }
}

bool SSMAction::isFileExist(const char *file_name)
{
    struct stat tmp_st;
    int ret = -1;

    ret = stat(file_name, &tmp_st);
    if (ret != 0 ) {
       LOGE("%s, %s don't exist\n", __FUNCTION__, file_name);
       return false;
    } else {
       return true;
    }
}

//Mark r/w values
#define CC_DEF_CHARACTER_CHAR_VAL                   (0x8A)
static const int SSM_MARK_01_VALUE = 0x90;
static const int SSM_MARK_02_VALUE = 0xCE;
static const int SSM_MARK_03_VALUE = 0xDF;

int SSMAction::SaveBurnWriteCharaterChar(int rw_val)
{
    int value = rw_val;
    return SSMWriteNTypes(SSM_RSV_W_CHARACTER_CHAR_START, 1, &value, 0);
}

int SSMAction::ReadBurnWriteCharaterChar()
{
    int tmp_val = 0;

    if (SSMReadNTypes(SSM_RSV_W_CHARACTER_CHAR_START, 1, &tmp_val, 0) < 0) {
        return -1;
    }

    return tmp_val;
}

int SSMAction::DeviceMarkCheck()
{
    int i = 0, failed_count = 0;
    int mark_offset[3] = { 0, 0, 0 };
    unsigned char mark_values[3] = { 0, 0, 0 };
    int tmp_ch = 0;

    //read temp one byte
    SSMReadNTypes(0, 1, &tmp_ch, 0);

    mark_offset[0] = SSM_MARK_01_START;
    mark_offset[1] = SSM_MARK_02_START;
    mark_offset[2] = SSM_MARK_03_START;

    mark_values[0] = SSM_MARK_01_VALUE;
    mark_values[1] = SSM_MARK_02_VALUE;
    mark_values[2] = SSM_MARK_03_VALUE;

    if (ReadBurnWriteCharaterChar() != CC_DEF_CHARACTER_CHAR_VAL) {
        SaveBurnWriteCharaterChar(CC_DEF_CHARACTER_CHAR_VAL);
    }

    failed_count = 0;
    for (i = 0; i < 3; i++) {
        tmp_ch = 0;
        if (SSMReadNTypes(mark_offset[i], 1, &tmp_ch, 0) < 0) {
            LOGE("%s, Read Mark failed\n", __FUNCTION__);
            break;
        }

        if ((unsigned char)tmp_ch != mark_values[i]) {
            failed_count += 1;
            LOGE(
                "%s, Mark[%d]'s offset = %d, Mark[%d]'s Value = %d, read value = %d\n",
                __FUNCTION__, i, mark_offset[i], i, mark_values[i], tmp_ch);
        }
    }

    if (failed_count >= 3) {
        return -1;
    }

    return 0;
}

int SSMAction::RestoreDeviceMarkValues()
{
    int i;
    int mark_offset[3] = {
        (int) SSM_MARK_01_START,
        (int) SSM_MARK_02_START,
        (int) SSM_MARK_03_START,
    };

    int mark_values[3] = {SSM_MARK_01_VALUE, SSM_MARK_02_VALUE, SSM_MARK_03_VALUE};

    for (i = 0; i < 3; i++) {
        if (SSMWriteNTypes(mark_offset[i], 1, &mark_values[i], 0) < 0) {
            LOGD("SSMRestoreDeviceMarkValues Write Mark failed.\n");
            break;
        }
    }

    if (i < 3) {
        return -1;
    }

    return 0;
}

int SSMAction::WriteBytes(int offset, int size, int *buf)
{
    int wr_size;

    lseek(m_dev_fd, offset, SEEK_SET);
    wr_size = write(m_dev_fd, buf, size);
    if (wr_size < 0) {
        LOGE("write error = %s\n", strerror(errno));
    }

    return 0;
}
int SSMAction::ReadBytes(int offset, int size, int *buf)
{
    int rd_size;

    lseek(m_dev_fd, offset, SEEK_SET);
    rd_size = read(m_dev_fd, buf, size);
    if (rd_size < 0) {
        LOGE("read error = %s\n", strerror(errno));
    }

    return 0;
}
int SSMAction::EraseAllData()
{
    int ret;

    ret = ftruncate(m_dev_fd, 0);
    if (ret < 0) {
        LOGE("ftruncate error = %s\n", strerror(errno));
    }

    lseek (m_dev_fd, 0, SEEK_SET);

    return 0;
}

int SSMAction::GetSSMActualAddr(int id)
{
    return mSSMHandler->SSMGetActualAddr(id);
}

int SSMAction::GetSSMActualSize(int id)
{
    return mSSMHandler->SSMGetActualSize(id);
}

int SSMAction::GetSSMStatus(void)
{
    return (int)mSSMHandler->SSMVerify();
}

int SSMAction::SSMWriteNTypes(int id, int data_len, int *data_buf, int offset)
{
    //LOGD("%s: id = %d, len = %d, offset = %d\n", __FUNCTION__, id, data_len, offset);
    pthread_mutex_lock(&ssm_r_w_op_mutex);
    if (data_buf == NULL) {
        LOGE("data_buf is NULL.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    if (0 == data_len) {
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    unsigned int actualAddr = mSSMHandler->SSMGetActualAddr(id) + offset;
    //LOGD("%s: actualAddr = %u, data = %d.\n", __FUNCTION__, actualAddr, *data_buf);
    if (WriteBytes(actualAddr, data_len, data_buf) < 0) {
        LOGE("device WriteNBytes error.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ssm_r_w_op_mutex);
    return 0;
}

int SSMAction::SSMReadNTypes(int id, int data_len, int *data_buf, int offset)
{
    //LOGD("%s: id = %d, len = %d, offset = %d\n", __FUNCTION__, id, data_len, offset);
    pthread_mutex_lock(&ssm_r_w_op_mutex);
    if (data_buf == NULL) {
        LOGE("data_buf is NULL.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    if (0 == data_len) {
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }

    unsigned int actualAddr = mSSMHandler->SSMGetActualAddr(id) + offset;
    if (ReadBytes(actualAddr, data_len, data_buf) < 0) {
        LOGE("device ReadNBytes error.\n");
        pthread_mutex_unlock(&ssm_r_w_op_mutex);
        return -1;
    }
    pthread_mutex_unlock(&ssm_r_w_op_mutex);

    //LOGD("%s: actualAddr = %u, data = %d.\n", __FUNCTION__, actualAddr, *data_buf);
    return 0;
}

bool SSMAction::SSMRecovery()
{
    bool ret = true;

    //erase ssm data
    EraseAllData();

    //recovery ssm data from db
    if (mpObserver != NULL) {
        mpObserver->resetAllUserSettingParam();
        ret = mSSMHandler->SSMRecreateHeader();
        RestoreDeviceMarkValues();
    } else {
        LOGE("%s: mpObserver is NULL\n", __FUNCTION__);
    }

    return ret;
}

int SSMAction::SSMRestoreDefault(int id, bool resetAll)
{
    int i = 0, tmp_val = 0;
    int offset_r = 0, offset_g = 0, offset_b = 0, gain_r = 1024, gain_g = 1024, gain_b = 1024;
    int8_t std_buf[6] = { 0, 0, 0, 0, 0, 0 };
    int8_t warm_buf[6] = { 0, 0, -8, 0, 0, 0 };
    int8_t cold_buf[6] = { -8, 0, 0, 0, 0, 0 };
    int tmp[2] = {0, 0};

    if (resetAll || VPP_DATA_POS_RGB_GAIN_R_START == id)
        SSMSaveRGBGainRStart(0, gain_r);

    if (resetAll || VPP_DATA_POS_RGB_GAIN_G_START == id)
        SSMSaveRGBGainGStart(0, gain_g);

    if (resetAll || VPP_DATA_POS_RGB_GAIN_B_START == id)
        SSMSaveRGBGainBStart(0, gain_b);

    if (resetAll || VPP_DATA_POS_RGB_POST_OFFSET_R_START == id)
        SSMSaveRGBPostOffsetRStart(0, offset_r);

    if (resetAll || VPP_DATA_POS_RGB_POST_OFFSET_G_START== id)
        SSMSaveRGBPostOffsetGStart(0, offset_g);

    if (resetAll || VPP_DATA_POS_RGB_POST_OFFSET_B_START == id)
        SSMSaveRGBPostOffsetBStart(0, offset_b);

    if (resetAll || VPP_DATA_RGB_START == id) {
        for (i = 0; i < 6; i++) {
            SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_STANDARD * 6, std_buf[i]); //0~5
            SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_WARM * 6, warm_buf[i]); //6~11
            SSMSaveRGBValueStart(i + VPP_COLOR_TEMPERATURE_MODE_COLD * 6, cold_buf[i]); //12~17
        }
    }

    for (i = SOURCE_INVALID; i < SOURCE_MAX; i++) {//invalid souce setting also need clear when restore default
        if (resetAll || VPP_DATA_COLOR_SPACE_START == id) {
            if (i == SOURCE_TYPE_HDMI) {
                SSMSaveColorSpaceStart ( VPP_COLOR_SPACE_AUTO);
            }
        }

        tmp_val = 50;
        if (resetAll || VPP_DATA_POS_BRIGHTNESS_START == id)
            SSMSaveBrightness(i, tmp_val);

        if (resetAll || VPP_DATA_POS_CONTRAST_START == id)
            SSMSaveContrast(i, tmp_val);

        if (resetAll || VPP_DATA_POS_SATURATION_START == id)
            SSMSaveSaturation(i, tmp_val);

        if (resetAll || VPP_DATA_POS_HUE_START == id)
            SSMSaveHue(i, tmp_val);

        if (resetAll || VPP_DATA_POS_SHARPNESS_START == id)
            SSMSaveSharpness(i, tmp_val);

        tmp_val = VPP_PICTURE_MODE_STANDARD;
        if (resetAll || VPP_DATA_POS_PICTURE_MODE_START == id)
            SSMSavePictureMode(i, tmp_val);

        tmp_val = VPP_DISPLAY_MODE_169;
        if (resetAll || VPP_DATA_POS_DISPLAY_MODE_START == id)
            SSMSaveDisplayMode(i, tmp_val);

        tmp_val = VPP_NOISE_REDUCTION_MODE_AUTO;
        if (resetAll || VPP_DATA_POS_NOISE_REDUCTION_START == id)
            SSMSaveNoiseReduction(i, tmp_val);

        tmp_val = VPP_GAMMA_CURVE_DEFAULT;
        if (resetAll || VPP_DATA_GAMMA_VALUE_START == id)
            SSMSaveGammaValue(i, tmp_val);

        tmp_val = 1;
        if (resetAll || CUSTOMER_DATA_POS_AUTO_ASPECT == id)
            SSMSaveAutoAspect(i, tmp_val);

        tmp_val = 0;
        if (resetAll || CUSTOMER_DATA_POS_43_STRETCH == id)
            SSMSave43Stretch(i, tmp_val);

        tmp_val = DYNAMIC_CONTRAST_MID;
        if (resetAll || VPP_DATA_DNLP_MODE_START == id)
            SSMSaveDnlpMode(i, tmp_val);

        tmp_val = 0;
        if (resetAll || VPP_DATA_DNLP_GAIN_START == id)
            SSMSaveDnlpGainValue(i, tmp_val);
    }

    if (resetAll || VPP_DATA_POS_DDR_SSC_START == id)
        SSMSaveDDRSSC(0);

    if (resetAll || VPP_DATA_POS_LVDS_SSC_START == id)
        SSMSaveLVDSSSC(tmp);

    if (resetAll || VPP_DATA_EYE_PROTECTION_MODE_START == id)
       SSMSaveEyeProtectionMode(0);

    return 0;
}

//PQ mode
int SSMAction::SSMSavePictureMode(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_PICTURE_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadPictureMode(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_PICTURE_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveLastPictureMode(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_LAST_PICTURE_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadLastPictureMode(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_LAST_PICTURE_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//Color Temperature
int SSMAction::SSMSaveColorTemperature(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_COLOR_TEMPERATURE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadColorTemperature(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_COLOR_TEMPERATURE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveColorDemoMode(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_COLOR_DEMO_MODE_START, 1, &tmp_val);
}

int SSMAction::SSMReadColorDemoMode(unsigned char *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_COLOR_DEMO_MODE_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveColorBaseMode(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_COLOR_BASE_MODE_START, 1, &tmp_val);
}

int SSMAction::SSMReadColorBaseMode(unsigned char *rw_val)
{
    int tmp_val = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_COLOR_BASE_MODE_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveColorGamutMode(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_COLORGAMUT_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadColorGamutMode(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_COLORGAMUT_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBGainRStart(int offset, unsigned int rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_RGB_GAIN_R_START, 4, &tmp_val, offset);
}

int SSMAction::SSMReadRGBGainRStart(int offset, unsigned int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_RGB_GAIN_R_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBGainGStart(int offset, unsigned int rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_RGB_GAIN_G_START, 4, &tmp_val, offset);
}

int SSMAction::SSMReadRGBGainGStart(int offset, unsigned int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_RGB_GAIN_G_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBGainBStart(int offset, unsigned int rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_RGB_GAIN_B_START, 4, &tmp_val, offset);
}

int SSMAction::SSMReadRGBGainBStart(int offset, unsigned int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_RGB_GAIN_B_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBPostOffsetRStart(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_RGB_POST_OFFSET_R_START, 4, &rw_val, offset);
}

int SSMAction::SSMReadRGBPostOffsetRStart(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_RGB_POST_OFFSET_R_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBPostOffsetGStart(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_RGB_POST_OFFSET_G_START, 4, &rw_val, offset);
}

int SSMAction::SSMReadRGBPostOffsetGStart(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_RGB_POST_OFFSET_G_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveRGBPostOffsetBStart(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_RGB_POST_OFFSET_B_START, 4, &rw_val, offset);
}

int SSMAction::SSMReadRGBPostOffsetBStart(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_RGB_POST_OFFSET_B_START, 4, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMReadRGBOGOValue(int offset, int size, unsigned char data_buf[])
{
    int tmp_off = 0, val = 0, max = 0, ret = -1;

    val = offset + size;
    max = SSM_CR_RGBOGO_LEN + SSM_CR_RGBOGO_CHKSUM_LEN;
    if (val > max) {
        LOGE("Out of max_size!!!\n");
        return ret;
    }

    tmp_off = SSM_RGBOGO_FILE_OFFSET + offset;

    ret = ReadDataFromFile(mWhiteBalanceFilePath, tmp_off, size, data_buf);

    return ret;
}

int SSMAction::SSMSaveRGBOGOValue(int offset, int size, unsigned char data_buf[])
{
    int tmp_off = 0, val = 0, max = 0, ret = -1;

    val = offset + size;
    max = SSM_CR_RGBOGO_LEN + SSM_CR_RGBOGO_CHKSUM_LEN;
    if (val > max) {
        LOGE("Out of max_size!!!\n");
        return ret;
    }

    tmp_off = SSM_RGBOGO_FILE_OFFSET + offset;

    ret = SaveDataToFile(mWhiteBalanceFilePath, tmp_off, size, data_buf);

    return ret;
}

int SSMAction::SSMSaveRGBValueStart(int offset, int8_t rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_RGB_START, 1, &tmp_val, offset);
}

int SSMAction::SSMReadRGBValueStart(int offset, int8_t *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_RGB_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveColorSpaceStart(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_COLOR_SPACE_START, 1, &tmp_val);
}

int SSMAction::SSMReadColorSpaceStart(unsigned char *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_COLOR_SPACE_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::ReadDataFromFile(const char *file_name, int offset, int nsize, unsigned char data_buf[])
{
    int device_fd = -1;
    int ret = 0;
    int bytesRead = 0;

    if (data_buf == NULL) {
        LOGE("data_buf is NULL!!!\n");
        return -1;
    }

    if (file_name == NULL) {
        LOGE("%s is NULL!\n",file_name);
        return -1;
    }

    //device_fd = open(file_name, O_RDONLY);
    device_fd = open(file_name, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (device_fd < 0) {
        LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return -1;
    }

    if ( lseek(device_fd, offset, SEEK_SET) < 0 ){
        LOGE("lseek file \"%s\" error(%s).\n", file_name, strerror(errno));
        ret = -1;
    }else if ( (bytesRead = read(device_fd, data_buf, nsize)) <= 0 ) {
        LOGE("read file \"%s\" bytesRead[%d] error(%s).\n", file_name, bytesRead, strerror(errno));
        ret = -1;
    }
    
    close(device_fd);
    device_fd = -1;

    return ret;
}

int SSMAction::SaveDataToFile(const char *file_name, int offset, int nsize, unsigned char data_buf[])
{
    int device_fd = -1;
    int wr_size;

    if (data_buf == NULL) {
        LOGE("%s, data_buf is NULL!!!\n", __FUNCTION__);
        return -1;
    }

    if (file_name == NULL) {
        LOGE("%s IS NULL!\n",file_name);
        return -1;
    }

    device_fd = open(file_name, O_RDWR | O_SYNC);
    if (device_fd < 0) {
        LOGE("open file \"%s\" error(%s).\n", file_name, strerror(errno));
        return -1;
    }

    lseek(device_fd, offset, SEEK_SET);
    wr_size = write(device_fd, data_buf, nsize);
    if (wr_size < 0) {
        LOGE("write error = %s\n", strerror(errno));
    }

    fsync(device_fd);

    close(device_fd);
    device_fd = -1;

    return 0;
}


//Brightness
int SSMAction::SSMSaveBrightness(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_BRIGHTNESS_START, 1, &rw_val, offset);
}


int SSMAction::SSMReadBrightness(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_BRIGHTNESS_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}


//constract
int SSMAction::SSMSaveContrast(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_CONTRAST_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadContrast(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_CONTRAST_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//saturation
int SSMAction::SSMSaveSaturation(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_SATURATION_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadSaturation(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_SATURATION_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//hue
int SSMAction::SSMSaveHue(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_HUE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadHue(int offset, int *rw_val)
{
    int ret = 0;
    int tmp_val = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_HUE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//Sharpness
int SSMAction::SSMSaveSharpness(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_SHARPNESS_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadSharpness(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_SHARPNESS_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}


//NoiseReduction
int SSMAction::SSMSaveNoiseReduction(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_NOISE_REDUCTION_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadNoiseReduction(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_NOISE_REDUCTION_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//SmoothPlus
int SSMAction::SSMSaveSmoothPlus(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_SMOOTH_PLUS_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadSmoothPlus(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_SMOOTH_PLUS_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//Gamma
int SSMAction::SSMSaveGammaValue(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_GAMMA_VALUE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadGammaValue(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_GAMMA_VALUE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//EdgeEnhance
int SSMAction::SSMSaveEdgeEnhanceStatus(int offset, int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_EDGE_ENHANCER, 1, &rw_val, offset);
}



int SSMAction::SSMReadEdgeEnhanceStatus(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_EDGE_ENHANCER, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//mpeg NR
int SSMAction::SSMSaveMpegNoiseReduction(int offset, int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_MPEG_NOISE_REDUCTION, 1, &rw_val, offset);
}

int SSMAction::SSMReadMpegNoiseReduction(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_MPEG_NOISE_REDUCTION, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//Dynamic contrast
int SSMAction::SSMSaveDynamicContrast(int offset, int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_DYNAMIC_CONTRAST, 1, &rw_val, offset);
}

int SSMAction::SSMReadDynamicContrast(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_DYNAMIC_CONTRAST, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

//Dynamic Backlight
int SSMAction::SSMSaveDynamicBacklightMode(int rw_val)
{
    return SSMWriteNTypes(CUSTOMER_DATA_POS_DYNAMIC_BACKLIGHT, 1, &rw_val);
}

int SSMAction::SSMReadDynamicBacklightMode(int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_DYNAMIC_BACKLIGHT, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDnlpMode(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_DNLP_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadDnlpMode(int offset, int *rw_val)
{
    int ret = 0;
    int tmp_val = 0;
    ret = SSMReadNTypes(VPP_DATA_DNLP_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDnlpGainValue(int offset __unused, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_DNLP_GAIN_START, 1, &rw_val);
}

int SSMAction::SSMReadDnlpGainValue(int offset __unused, int *rw_val)

{
    int ret = 0;
    int tmp_val = 0;
    ret = SSMReadNTypes(VPP_DATA_DNLP_GAIN_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveEyeProtectionMode(int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_EYE_PROTECTION_MODE_START, 1, &rw_val);
}

int SSMAction::SSMReadEyeProtectionMode(int *rw_val)
{
    int ret = 0;
    int tmp_val = 0;
    ret = SSMReadNTypes(VPP_DATA_EYE_PROTECTION_MODE_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDDRSSC(unsigned char rw_val)
{
    int tmp_val = rw_val;
    return SSMWriteNTypes(VPP_DATA_POS_DDR_SSC_START, 1, &tmp_val);
}

int SSMAction::SSMReadDDRSSC(unsigned char *rw_val)
{
    int tmp_val;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_DDR_SSC_START, 1, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveLVDSSSC(int *rw_val)
{
    int tmp_val;
    int ret = 0;
    ret = SSMWriteNTypes(VPP_DATA_POS_LVDS_SSC_START, 2, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMReadLVDSSSC(int *rw_val)
{
    int tmp_val;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_LVDS_SSC_START, 2, &tmp_val);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDisplayMode(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_DISPLAY_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadDisplayMode(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_DISPLAY_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMReadBackLightVal(int *rw_val)
{
    int tmp_ret = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_BACKLIGHT_START, 1, &tmp_ret);
    *rw_val = tmp_ret;

    return ret;
}

int SSMAction::SSMSaveBackLightVal(int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_BACKLIGHT_START, 1, &rw_val);
}

int SSMAction::SSMSaveAutoAspect(int offset, int rw_val) {
    return SSMWriteNTypes(CUSTOMER_DATA_POS_AUTO_ASPECT, 1, &rw_val, offset);
}

int SSMAction::SSMReadAutoAspect(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_AUTO_ASPECT, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSave43Stretch(int offset, int rw_val) {
    return SSMWriteNTypes(CUSTOMER_DATA_POS_43_STRETCH, 1, &rw_val, offset);
}

int SSMAction::SSMRead43Stretch(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(CUSTOMER_DATA_POS_43_STRETCH, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMEdidRestoreDefault(int rw_val) {
    int ret = 0;
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI1_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI2_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI3_EDID_START, 1, &rw_val);
    ret |= SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI4_EDID_START, 1, &rw_val);
    return ret;
}

int SSMAction::SSMHdcpSwitcherRestoreDefault(int rw_val) {
    return SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI_HDCP_SWITCHER_START, 1, &rw_val);
}

int SSMAction::SSMSColorRangeModeRestoreDefault(int rw_val) {
    return SSMWriteNTypes(CUSTOMER_DATA_POS_HDMI_COLOR_RANGE_START, 1, &rw_val);
}

int SSMAction::SSMSaveLocalContrastMode(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_LOCAL_CONTRAST_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadLocalContrastMode(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_LOCAL_CONTRAST_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDeblockMode(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_DEBLOCK_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadDeblockMode(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_DEBLOCK_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveDemoSquitoMode(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_DEMOSQUITO_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadDemoSquitoMode(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_DEMOSQUITO_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveMcDiMode(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_MCDI_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadMcDiMode(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_MCDI_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMReadAipqEnableVal(int *rw_val)
{
    int tmp_ret = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_AIPQ_ENABLE_START, 1, &tmp_ret);
    *rw_val = tmp_ret;

    return ret;
}

int SSMAction::SSMSaveAipqEnableVal(int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_AIPQ_ENABLE_START, 1, &rw_val);
}

int SSMAction::SSMReadAiSrEnable(int *rw_val)
{
    int tmp_ret = 0;
    int ret = 0;

    ret = SSMReadNTypes(VPP_DATA_POS_AISR_ENABLE_START, 1, &tmp_ret);
    *rw_val = tmp_ret;

    return ret;
}

int SSMAction::SSMSaveAiSrEnable(int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_AISR_ENABLE_START, 1, &rw_val);
}

int SSMAction::SSMSaveHdrTmoVal(int offset, int rw_val)
{
    return SSMWriteNTypes(VPP_DATA_POS_HDR_TMO_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadHdrTmoVal(int offset, int *rw_val)
{
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_HDR_TMO_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveMemcMode(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_MEMC_MODE_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadMemcMode(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_MEMC_MODE_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveMemcDeblurLevel(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_MEMC_DEBLUR_LEVEL_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadMemcDeblurLevel(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_MEMC_DEBLUR_LEVEL_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveMemcDeJudderLevel(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_MEMC_DEJUDDER_LEVEL_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadMemcDeJudderLevel(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_MEMC_DEJUDDER_LEVEL_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveBlackStretch(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_BLACK_STRETCH_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadBlackStretch(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_BLACK_STRETCH_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveBlueStretch(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_BLUE_STRETCH_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadBlueStretch(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_BLUE_STRETCH_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveChromaCoring(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_CHROMA_CORING_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadChromaCoring(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_CHROMA_CORING_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSaveLocalDimming(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_LOCAL_DIMMING_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadLocalDimming(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_LOCAL_DIMMING_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSavePictureModeParamsFlag(int offset, int rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_PICTURE_MODE_PARAM_CRC_START, 1, &rw_val, offset);
}

int SSMAction::SSMReadPictureModeParamsFlag(int offset, int *rw_val) {
    int tmp_val = 0;
    int ret = 0;
    ret = SSMReadNTypes(VPP_DATA_POS_PICTURE_MODE_PARAM_CRC_START, 1, &tmp_val, offset);
    *rw_val = tmp_val;

    return ret;
}

int SSMAction::SSMSavePictureModeParams(int offset, int size, int *rw_val) {
    return SSMWriteNTypes(VPP_DATA_POS_PICTURE_MODE_PARAM_START, size, rw_val, offset);
}

int SSMAction::SSMReadPictureModeParams(int offset, int size, int *rw_val) {
    return SSMReadNTypes(VPP_DATA_POS_PICTURE_MODE_PARAM_START, size, rw_val, offset);
}
