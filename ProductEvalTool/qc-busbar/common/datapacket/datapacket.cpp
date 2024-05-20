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
    ptr->devType.sn.clear();
    ptr->devType.dev_type.clear();
    memset(&(ptr->line), 0, sizeof(sObjData));
    memset(&(ptr->env), 0, sizeof(sEnvData));

    pro->step = 0;
    pro->result = 0;
    pro->pass.clear();
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
    pro->num = 0;
    pro->ir.clear();
    pro->acw.clear();
    pro->gnd.clear();
    pro->itemData.clear();
    pro->itemRequest.clear();
    if(pro->work_mode ==0) {
        pro->itemRequest = "交流耐压 <5mA,绝缘电阻 >10MΩ";
    }else if(pro->work_mode ==1) {
        pro->itemRequest = "接地电阻 <100mΩ";
    }else if(pro->work_mode ==2) {
        pro->itemRequest = "级联测试：IN/OUT接口正常通讯,电压测试：测试回路电压值为0,待测回路电压值>220V";
    }else if(pro->work_mode ==3) {
        pro->itemRequest = "断路器：测试回路电压值为0，待测回路电压值>220V;负载测试：测试回路电流值为0、功率值为0，待测回路电流值、功率值不为0";
    }
}

bool sDataPacket::updatePro(const QString &str, bool pass, int sec)
{
    pro->num ++;
    pro->pass << pass;
    pro->itPass << pass;

    pro->itemName << str;
    pro->status << str;

    QString str1 = QString::number(pro->num) + "、"+ str;
    pro->itemContent << str1;

    if(pass) pass = delay(sec);
    else pro->result = Test_Fail;
    // if(!pass) BaseLogs::bulid()->appendLogItem(str, pass);
    return pass;
}


bool sDataPacket::delay(int s)
{
    bool ret = true;
    for(int i=0; i<s; ++i) {
        if((pro->step < Test_Over)  || (pro->step > Test_End)){
            QThread::msleep(1000);
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
        sDataPacket::bulid()->share_mem_init();
    }

    return shm;
}
