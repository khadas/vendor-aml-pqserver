/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *  @author   Tellen Yu
 *  @version  1.0
 *  @date     2016/09/06
 *  @par function description:
 *  - 1 process uevent for system control
 */

#define LOG_MOUDLE_TAG "PQ"
#define LOG_CLASS_TAG "UEventObserver"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <pthread.h>

#include "UEventObserver.h"

UEventObserver::UEventObserver()
    :mFd(-1) {

    mFd = ueventInit();
    mMatchStr.num = 0;
    mMatchStr.strList.buf = NULL;
    mMatchStr.strList.next = NULL;

    addMatch(FRAME_RATE_VDIN0_UEVENT);
    addMatch(FRAME_RATE_VDIN1_UEVENT);
    addMatch(FRAME_RATE_VDIN0_UEVENT_2);
}

UEventObserver::~UEventObserver() {
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
}

int UEventObserver::ueventInit() {
    struct sockaddr_nl addr;
    int sz = 64*1024;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    //addr.nl_pid = pthread_self() << 16 | getpid();
    addr.nl_pid    = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
        return 0;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(s);
        return 0;
    }

    return s;
}

int UEventObserver::ueventGetFd() {
    return mFd;
}

int UEventObserver::ueventNextEvent(char* buffer, int buffer_length) {
    while (1) {
        struct pollfd fds;
        int nr;

        fds.fd = mFd;
        fds.events = POLLIN;
        fds.revents = 0;
        nr = poll(&fds, 1, -1);

        if (nr > 0 && (fds.revents & POLLIN)) {
            int count = recv(mFd, buffer, buffer_length, 0);
            if (count > 0) {
                return count;
            }
        }
    }

    // won't get here
    return 0;
}

bool UEventObserver::isMatch(const char* buffer, int length,
    uevent_data_t* ueventData, const char *matchStr) {
    bool matched = false;

    // Consider all zero-delimited fields of the buffer.
    const char* field = buffer;
    const char* end = buffer + length + 1;
    do {
        if (!strcmp(field, matchStr)) {
            LOGD("Matched uevent message with pattern: %s\n", matchStr);

            strcpy(ueventData->matchName, matchStr);
            matched = true;
        }
        else if (strstr(field, "DEVTYPE=")) {
            strcpy(ueventData->switchName, field + strlen("DEVTYPE="));
        }
        field += strlen(field) + 1;
    } while (field != end);

    if (matched) {
        ueventData->len = length;
        memcpy(ueventData->buf, buffer, length);
        return matched;
    }

    return matched;
}

bool UEventObserver::isMatch(const char* buffer, int length, uevent_data_t* ueventData) {
    bool matched = false;

    match_node_t *strItem = &mMatchStr.strList;
    //the first node is null
    for (size_t i = 0; i < (unsigned int)mMatchStr.num; i++) {
        const char *matchStr = strItem->buf;
        strItem = strItem->next;
        matched = isMatch(buffer, length, ueventData, matchStr);
        if (matched)
            break;
    }

    return matched;
}

void UEventObserver::waitForNextEvent(uevent_data_t* ueventData) {
    char buffer[1024];

    for (;;) {
        int length = ueventNextEvent(buffer, sizeof(buffer) - 1);
        if (length <= 0) {
            LOGE("Received uevent message length: %d", length);
            return;
        }
        buffer[length] = '\0';

        ueventPrint(buffer, length);
        if (isMatch(buffer, length, ueventData))
            //LOGD("Received uevent message is match\n");
            return;
    }
}

void UEventObserver::addMatch(const char *matchStr) {
    match_node_t *strItem = &mMatchStr.strList;
    //the first node is null
    if (NULL == strItem->buf) {
        strItem->buf = (char *)malloc(strlen(matchStr) + 1);
        strcpy(strItem->buf, matchStr);
        strItem->next = NULL;

        mMatchStr.num++;
        return;
    }

    while (NULL != strItem->buf) {
        if (!strcmp(strItem->buf, matchStr)) {
            LOGE("have added uevent : %s before\n", matchStr);
            return;
        }

        //the last node
        if (NULL == strItem->next) {
            LOGD("no one match the uevent : %s, add it to list\n", matchStr);
            break;
        }
        strItem = strItem->next;
    }

    match_node_t *newNode = (match_node_t *)malloc(sizeof(match_node_t));
    newNode->buf = (char *)malloc(strlen(matchStr) + 1);
    strcpy(newNode->buf, matchStr);
    newNode->next = NULL;

    //add the new node to the list
    strItem->next = newNode;

    mMatchStr.num++;
}

void UEventObserver::removeMatch(const char *matchStr) {
    match_node_t *headItem = &mMatchStr.strList;
    match_node_t *curItem = headItem;
    match_node_t *preItem = curItem;
    while (NULL != curItem->buf) {
        if (!strcmp(curItem->buf, matchStr)) {
            LOGD("find the match uevent : %s, remove it\n", matchStr);
            match_node_t *tmpNode = curItem->next;
            free(curItem->buf);
            curItem->buf = NULL;
            //head item do not need free
            if (curItem != headItem) {
                free(curItem);
                curItem = NULL;
            }

            preItem->next = tmpNode;
            mMatchStr.num--;
            return;
        }

        //the last node
        if (NULL == curItem->next) {
            LOGE("can not find the match uevent : %s\n", matchStr);
            return;
        }
        preItem = curItem;
        curItem = curItem->next;
    }
}

void UEventObserver::ueventPrint(char* ueventBuf, int len) {
    //change@/devices/platform/vdin@0/vdin/vdin0/vdin0event ACTION=change
    //DEVPATH=/devices/platform/vdin@0/vdin/vdin0/vdin0event SUBSYSTEM=extcon
    //NAME=vdin0_extcon STATE=VDIN=1 DEVTYPE=vdin0event SEQNUM=3319
    char printBuf[1024] = {0};
    memcpy(printBuf, ueventBuf, len);
    for (int i = 0; i < len; i++) {
        if (printBuf[i] == 0x0)
            printBuf[i] = ' ';
    }

    //LOGD("Received uevent message: %s\n", printBuf);
}

int UEventObserver::StartUEventThead()
{
    int ret;
    pthread_t  id;
    ret = pthread_create(&id, NULL, UeventThread, this);
    if (ret != 0) {
        LOGE("Create UeventThread error :%d!\n", ret);
    }

    return ret;
}

void* UEventObserver::UeventThread(void* data)
{
    UEventObserver *pThis = (UEventObserver *)data;
    pThis->UeventThreadLoop();
    return NULL;
}

// uevent prcessed in this loop
void* UEventObserver::UeventThreadLoop(void) {
    uevent_data_t ueventData;

    while (true) {
        memset(&ueventData, 0, sizeof(uevent_data_t));
        waitForNextEvent(&ueventData);
        //LOGD("Received ueventData.matchName: %s\n", ueventData.matchName);

        // for new uevent handle
   #if 0 //compare uevent whole path
        if (!strcmp(ueventData.matchName, FRAME_RATE_VDIN0_UEVENT)
            || !strcmp(ueventData.matchName, FRAME_RATE_VDIN1_UEVENT)
            || !strcmp(ueventData.matchName, FRAME_RATE_VDIN0_UEVENT_2)) {
            if (mpUeventObserverCallBack == NULL) {
                LOGE("%s: mpUeventObserverCallBack is null!\n", __FUNCTION__);
            } else {
                mpUeventObserverCallBack->onUevent(ueventData);
            }
        }
    #else //compare uevent sub path
        if ((strstr(ueventData.matchName, "vdin/vdin0event") != NULL)
           || (strstr(ueventData.matchName, "vdin/vdin1event") != NULL)) {
           if (mpUeventObserverCallBack == NULL) {
                LOGE("%s: mpUeventObserverCallBack is null!\n", __FUNCTION__);
            } else {
                mpUeventObserverCallBack->onUevent(ueventData);
            }
        }
    #endif
    }

    return NULL;
}
