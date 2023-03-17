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
        printf("%s cbType %d\n", __FUNCTION__, cbType);

        switch (cbType) {
          case CPqClientCb::PQ_CB_TYPE_HDRTYPE: {
            PqClientCb::HdrTypeCb *hdrTypeCb = (PqClientCb::HdrTypeCb *)(&cb_data);
            hdrtype = hdrTypeCb->mHdrType;
            printf("%s hdrtype %d\n", __FUNCTION__, hdrtype);
            break;
          }
          default:
            printf("%s invalid callback type!\n", __FUNCTION__);
            break;
        }
        return;
    }

    PqClient *mpPqClient;
};


int main(int argc, char **argv) {
    int run = -1;

    printf("%s #### please select cmd####\n", __FUNCTION__);
    printf("%s ##########################\n", __FUNCTION__);
    while (run) {
        char Command[10];
        int conmmand;

        scanf("%s", Command);
        conmmand = atoi(Command);
        printf("%s #### Command: %s %d\n", __FUNCTION__, Command, conmmand);

        switch (Command[0]) {
          case 's': {
            printf("%s register client to pqserver\n", __FUNCTION__);
            sp<ProcessState> proc(ProcessState::self());
            proc->startThreadPool();
            PqCbTest *cbtest = new PqCbTest();
            printf("%s cbtest %p\n", __FUNCTION__, cbtest);
            break;
          }
          case 'g': {
            printf("%s hdrtype %d\n", __FUNCTION__, hdrtype);
            run = 0;
            break;
          }
          default: {
              printf("%s invalid cmd\n", __FUNCTION__);
              break;
          }
        }
        fflush (stdout);
    }

    return 0;
}
