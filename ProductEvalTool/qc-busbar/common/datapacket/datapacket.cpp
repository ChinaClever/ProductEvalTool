/*
 *
 *  Created on: 2020年10月1日
 *      Author: Lzy
 */
#include "datapacket.h"

sDataPacket::sDataPacket()
{
    pro = new sProgress();
    for(int i=0; i<DEV_NUM; ++i) {
        dev[i] = new sDevData;
        clear(i);
    }
}

void sDataPacket::clear(int id)
{
    sDevData *ptr = dev[id];
    ptr->devType.ac = 1;
    ptr->devType.ip[0] = 0;
    ptr->devType.devType = 0;
    ptr->devType.version = 0;
    // ptr->devType.sn.clear();
    ptr->devType.dev_type.clear();
    memset(&(ptr->line), 0, sizeof(sObjData));
    memset(&(ptr->env), 0, sizeof(sEnvData));

    pro->step = 0;
    pro->result = 0;
    pro->flag = 0;
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
    pro->sureItem.clear();

    pro->moduleSN.clear();
}


sDataPacket *sDataPacket::bulid()
{
    static sDataPacket* sington = nullptr;
    if(sington == nullptr)
        sington = new sDataPacket();
    return sington;
}

void sDataPacket::init()
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
    pro->safeData.clear();
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
    // pro->type = 0;
    pro->oning = false;

    pro->gndParm.clear();
    pro->irParm.clear();
    pro->acwParm.clear();
    pro->safe_result.clear();
}

bool sDataPacket::updatePro(const QString &str, bool pass, int sec)
{
    pro->pass << pass;
    pro->itPass << pass;

    pro->itemName << str;
    pro->status << str;

    if(pass) pass = delay(sec);
    else pro->result = Test_Fail;

    return pass;
}

bool sDataPacket::delay(int s)
{
    bool ret = true;
    for(int i=0; i<s; ++i) {
        if((pro->step < Test_Over)  || (pro->step > Test_End)){
            QThread::msleep(200);
        } else {
            ret = false;
            break;
        }
    }

    return ret;
}

bool sDataPacket::delayMs(int secs)
{
    bool ret = true;
    for(int i=0; i<secs; ++i) {
        if((pro->step < Test_Over)  || (pro->step > Test_End)){
            QThread::msleep(1);
        } else {
            ret = false;
            break;
        }
    }

    return ret;
}
/**
 * 获取共享内存
 */

sBusData *sDataPacket::share_mem_get()
{
    static sBusData *ptr = nullptr;
    if(!ptr) {
        ptr = new sBusData();
    }

    return ptr;
}

/**
 * 共享内存初始化
 */
void sDataPacket::share_mem_init()
{
    sBusData *shared = share_mem_get();//指向shm
}

/**
 * 把共享内存从当前进程中分离
 */
void sDataPacket::share_mem_free()
{
    sBusData *shm = share_mem_get();
    delete shm;
}

/**
 * 删除共享内存
 */
void sDataPacket::share_mem_del()
{
    share_mem_free();
}


/***
  * 获取共享内存
  */
sBusData *get_share_mem()
{
    static sBusData *shm = NULL;
    if(shm == NULL) {
        shm = sDataPacket::bulid()->share_mem_get();
//        sDataPacket::bulid()->share_mem_init();
    }

    return shm;
}
