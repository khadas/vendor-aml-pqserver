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
static int allmgamemode = -1;
static int filmmakermode = -1;
static int refreshrate = -1;

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
          case CPqClientCb::PQ_CB_TYPE_ALLM_GAME_MODE: {
            PqClientCb::AllmGameModeCb *allmGameCb = (PqClientCb::AllmGameModeCb *)(&cb_data);
            allmgamemode = allmGameCb->mAllmGameMode;
            printf("%s allmgamemode %d\n", __FUNCTION__, allmgamemode);
            break;
          }
          case CPqClientCb::PQ_CB_TYPE_FILM_MAKER_MODE: {
            PqClientCb::FilmMakerModeCb *filmMakerCb = (PqClientCb::FilmMakerModeCb *)(&cb_data);
            filmmakermode = filmMakerCb->mFilmMakerMode;
            printf("%s filmmakermode %d\n", __FUNCTION__, filmmakermode);
            break;
          }
          case CPqClientCb::PQ_CB_TYPE_REFRESH_RATE: {
            PqClientCb::RefreshRateCb *refreshRateCb = (PqClientCb::RefreshRateCb *)(&cb_data);
            refreshrate = refreshRateCb->mRefreshRate;
            printf("%s refreshrate %d\n", __FUNCTION__, refreshrate);
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
    printf("%s #### s register client to pqserver####\n", __FUNCTION__);
    printf("%s #### 0 get hdr type####\n", __FUNCTION__);
    printf("%s #### 1 get allm game mode####\n", __FUNCTION__);
    printf("%s #### 2 get film maker mode####\n", __FUNCTION__);
    printf("%s #### 3 get refresh rate####\n", __FUNCTION__);
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
          case '0': {
            printf("%s hdrtype %d\n", __FUNCTION__, hdrtype);
            run = 0;
            break;
          }
          case '1': {
            printf("%s allmgamemode %d\n", __FUNCTION__, allmgamemode);
            run = 0;
            break;
          }
          case '2': {
            printf("%s filmmakermode %d\n", __FUNCTION__, filmmakermode);
            run = 0;
            break;
          }
          case '3': {
            printf("%s refreshrate %d\n", __FUNCTION__, refreshrate);
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
