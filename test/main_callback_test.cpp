#define LOG_MODULE_TAG "PQ"
#define LOG_CLASS_TAG "PQClientCb_Test"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "PqClient.h"
#include "CPqClientLog.h"


static int hdrtype = -1;

class PqCbTest: public PqClient::PqClientIObserver {
public:
    PqCbTest()
    {
        mpPqClient = PqClient::GetInstance();
        mpPqClient->RegisterObserverToPqClient(this);
    }

    ~PqCbTest()
    {

    }

    void GetPqCbData(CPqClientCb &cb_data) {
        int cbType = cb_data.getCbType();
        LOGD("%s cbType %d\n", __FUNCTION__, cbType);

        switch (cbType) {
          case CPqClientCb::PQ_CB_TYPE_HDRTYPE: {
            PqClientCb::HdrTypeCb *hdrTypeCb = (PqClientCb::HdrTypeCb *)(&cb_data);
            hdrtype = hdrTypeCb->mHdrType;
            LOGD("%s hdrtype %d\n", __FUNCTION__, hdrtype);
            break;
          }
          default:
            LOGD("%s invalid callback type!\n", __FUNCTION__);
            break;
        }
        return;
    }

    PqClient *mpPqClient;
};


int main(int argc, char **argv) {
    int run = -1;

    LOGD("%s #### please select cmd####\n", __FUNCTION__);
    LOGD("%s ##########################\n", __FUNCTION__);
    while (run) {
        char Command[10];
        int conmmand;

        scanf("%s", Command);
        conmmand = atoi(Command);
        LOGD("%s #### Command: %s %d\n", __FUNCTION__, Command, conmmand);

        switch (Command[0]) {
          case 's': {
            LOGD("%s register client to pqserver\n", __FUNCTION__);
            sp<ProcessState> proc(ProcessState::self());
            proc->startThreadPool();
            PqCbTest *cbtest = new PqCbTest();
            LOGD("%s cbtest %p\n", __FUNCTION__, cbtest);
            break;
          }
          case 'g': {
            LOGD("%s hdrtype %d\n", __FUNCTION__, hdrtype);
            run = 0;
            break;
          }
          default: {
              LOGD("%s invalid cmd\n", __FUNCTION__);
              break;
          }
        }
        fflush (stdout);
    }

    return 0;
}