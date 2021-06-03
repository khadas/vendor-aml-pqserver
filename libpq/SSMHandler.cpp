/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "SSMHandler"

#include "SSMHandler.h"

SSMHandler* SSMHandler::mSSMHandler = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

SSMHandler* SSMHandler::GetSingletonInstance(const char *SSMHandlerPath)
{

    pthread_mutex_lock(&mutex);

    if (!mSSMHandler) {
        mSSMHandler = new SSMHandler();
        mSSMHandler->mSSMHandlerPath = SSMHandlerPath;
        if (mSSMHandler && !mSSMHandler->Construct()) {
            delete mSSMHandler;
            mSSMHandler = NULL;
        }
    }

    pthread_mutex_unlock(&mutex);

    return mSSMHandler;
}

SSMHandler::SSMHandler()
{
    unsigned int sum = 0;

    memset (&mSSMHeader_section1, 0, sizeof (SSMHeader_section1_t));

    for (unsigned int i = 1; i < gSSMHeader_section1.count; i++) {
        sum += gSSMHeader_section2[i-1].size;

        gSSMHeader_section2[i].addr = sum;
    }
}

SSMHandler::~SSMHandler()
{
    if (mFd > 0) {
        close(mFd);
        mFd = -1;
    }
}

bool SSMHandler::Construct()
{
    bool ret = true;

    mFd = open(mSSMHandlerPath, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (-1 == mFd) {
        ret = false;
        LOGD ("%s, Open %s failure\n", __FUNCTION__, mSSMHandlerPath);
    }

    return ret;
}

SSM_status_t SSMHandler::SSMSection1Verify()
{
    SSM_status_t ret = SSM_HEADER_VALID;

    lseek (mFd, 0, SEEK_SET);
    ssize_t ssize = read(mFd, &mSSMHeader_section1, sizeof (SSMHeader_section1_t));

    if (ssize != sizeof (SSMHeader_section1_t) ||
        mSSMHeader_section1.magic != gSSMHeader_section1.magic ||
        mSSMHeader_section1.count != gSSMHeader_section1.count ||
        mSSMHeader_section1.version != gSSMHeader_section1.version) {
        ret = SSM_HEADER_INVALID;
    }

    return ret;
}

SSM_status_t SSMHandler::SSMSection2Verify(SSM_status_t SSM_status)
{
    return SSM_status;
}

int SSMHandler::SSMSaveCurrentHeader(current_ssmheader_section2_t *header_cur)
{
    std::vector<SSMHeader_section2_t> vsection2;
    unsigned int size = 0;
    int rd_size;

    LOGD ("%s --- line:%d\n", __FUNCTION__, __LINE__);

    lseek (mFd, sizeof (SSMHeader_section1_t), SEEK_SET);

    for (unsigned int i = 0; i < gSSMHeader_section1.count; i++) {
        SSMHeader_section2_t temp;

        rd_size = read(mFd, &temp, sizeof (SSMHeader_section2_t));
        if (rd_size < 0) {
            LOGE("read error = %s\n", strerror(errno));
        }
        vsection2.push_back(temp);
        size += temp.size;
    }
    header_cur->size = size;
    header_cur->header_section2_data = &vsection2[0];

    //if (header_cur->size < 0) {
    //    LOGE ("SSMSaveCurrentHeader error!\n");
    //    return -1;
    //}

    return 0;
}

bool SSMHandler::SSMRecreateHeader()
{
    bool ret = true;
    int wr_size;
    int size;

    LOGD ("%s ---.\n", __FUNCTION__);

    size = ftruncate(mFd, 0);
    if (size < 0) {
        LOGE("ftruncate error = %s\n", strerror(errno));
    }
    lseek (mFd, 0, SEEK_SET);

    //cal Addr and write
    size = write(mFd, &gSSMHeader_section1, sizeof (SSMHeader_section1_t));
    if (size < 0) {
        LOGE("write error = %s\n", strerror(errno));
    }

    wr_size = write(mFd, gSSMHeader_section2, gSSMHeader_section1.count * sizeof (SSMHeader_section2_t));
    if (wr_size < 0) {
        LOGE("write error = %s\n", strerror(errno));
    }

    return ret;
}

unsigned int SSMHandler::SSMGetActualAddr(int id)
{
    return gSSMHeader_section2[id].addr;
}

unsigned int SSMHandler::SSMGetActualSize(int id)
{
    return gSSMHeader_section2[id].size;
}

SSM_status_t SSMHandler::SSMVerify()
{
    return  SSMSection2Verify(SSMSection1Verify());
}

SSMHandler& SSMHandler::operator = (const SSMHandler& obj)
{
    return const_cast<SSMHandler&>(obj);
}
