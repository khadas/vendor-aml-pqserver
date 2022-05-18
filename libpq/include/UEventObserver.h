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

 #ifndef _SYSTEM_CONTROL_UEVENT_OBSERVER_H
#define _SYSTEM_CONTROL_UEVENT_OBSERVER_H

#include "CPQLog.h"

#define FRAME_RATE_VDIN0_UEVENT                 "change@/devices/platform/vdin@0/vdin/vdin0event"
#define FRAME_RATE_VDIN1_UEVENT                 "change@/devices/platform/vdin@1/vdin/vdin1event"
#define FRAME_RATE_VDIN0_UEVENT_2               "change@/devices/platform/soc/soc:vdin0/vdin/vdin0event"

typedef struct uevent_data {
    int len;
    char buf[1024];
    char matchName[256];
    char switchName[64];
    char switchState[64];
} uevent_data_t;

typedef struct _match_node{
    char *buf;
    struct _match_node *next;
} match_node_t;

typedef struct {
    int num;             //match string item number
    match_node_t strList;
} match_item_t;

// ----------------------------------------------------------------------------
class UEventObserver
{
public:
    UEventObserver();
    ~UEventObserver();

    void addMatch(const char *matchStr);
    void removeMatch(const char *matchStr);
    void waitForNextEvent(uevent_data_t* ueventData);
    int ueventGetFd();
    int StartUEventThead();

    class IUEventObserverCallBack {
    public:
        IUEventObserverCallBack() {};
        virtual ~IUEventObserverCallBack() {};
        virtual void onUevent(uevent_data_t ueventData) {};
    };
    void setUeventObserverCallBack (IUEventObserverCallBack *pOb) {
        mpUeventObserverCallBack = pOb;
    };

private:
    int ueventInit();
    bool isMatch(const char* buffer, int length, uevent_data_t* ueventData, const char *matchStr);
    bool isMatch(const char* buffer, int length, uevent_data_t* ueventData);
    int ueventNextEvent(char* buffer, int buffer_length);
    void ueventPrint(char* ueventBuf, int len);
    static void* UeventThread(void* data);
    void* UeventThreadLoop(void);

    IUEventObserverCallBack *mpUeventObserverCallBack = NULL;

    int mFd;

    match_item_t mMatchStr;
};
// ----------------------------------------------------------------------------
#endif /*_SYSTEM_CONTROL_UEVENT_OBSERVER_H*/
