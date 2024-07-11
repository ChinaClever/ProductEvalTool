#include "datapacket_english.h"

datapacket_English::datapacket_English()
{
    pro = new sProgress();
    for(int i=0; i<DEV_NUM; ++i) {
        dev[i] = new sDevData;
        clear(i);
    }
}

void datapacket_English::clear(int id)
{
    sDevData *ptr = dev[id];
    ptr->devType.ac = 1;
    ptr->devType.ip[0] = 0;
    ptr->devType.devType = 0;
    ptr->devType.version = 0;
    ptr->devType.sn.clear();
    ptr->devType.dev_type.clear();
    memset(&(ptr->line), 0, sizeof(sObjData));
    memset(&(ptr->env), 0, sizeof(sEnvData));

    pro->step = 0;
    pro->result = 0;
    pro->pass.clear();
    pro->stepResult.clear();
    pro->itPass.clear();
    pro->item.clear();
    pro->status.clear();
    pro->startTime = QTime::currentTime();

    pro->productType.clear();
    pro->no.clear();
    pro->itemName.clear();
    pro->uploadPass.clear();
    pro->softwareVersion.clear();
}


datapacket_English *datapacket_English::bulid()
{
    static datapacket_English* sington = nullptr;
    if(sington == nullptr)
        sington = new datapacket_English();
    return sington;
}

void datapacket_English::init()
{
    clear();
    pro->step = Test_Start;
    pro->result = Test_Info;
    pro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    pro->product_sn.clear();
    pro->test_item.clear();
    pro->itemContent.clear();
    pro->Service.clear();
    pro->stopFlag = false;
    pro->online = 0;
    pro->num = 0;
    pro->ir.clear();
    pro->acw.clear();
    pro->gnd.clear();
    pro->itemData.clear();
    pro->itemRequest.clear();
    pro->stepRequest.clear();
    pro->stepResult.clear();
    pro->issure = 0;
    pro->dev_name.clear();
    pro->order_id.clear();
    pro->order_num = 0;
    pro->test_num = 0;
    pro->Lan_flag = 0;
    pro->stepNum = 0;
    pro->stepNumEng = 0;
}
