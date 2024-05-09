/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "dev_sirtu.h"

#define SI_RTU_DC_LEN 0x1B // 直流数据长度
#define SI_RTU_THREE_LEN 0x51  // 三相数据长度//单相时，L2和L3表示C1和C2

Dev_SiRtu::Dev_SiRtu(QObject *parent) : Dev_Object(parent)
{
    mRtuPkt = new Rtu_recv; //传输数据结构
    mBusData = get_share_mem();
}

Dev_SiRtu *Dev_SiRtu::bulid(QObject *parent)
{
    static Dev_SiRtu* sington = nullptr;
    if(sington == nullptr)
        sington = new Dev_SiRtu(parent);
    return sington;
}


void Dev_SiRtu::initRtuItem(Rtu_Sent &it)
{
    it.addr = mItem->addr;
    it.fn = 0x03;
    it.reg = 0;
    it.len = mItem->addr==1?RTU_SENT_LEN_V303:RTU_SENT_LEN_V30;
}


/**
  * 功　能：长度 校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：0 正确
  */
int Dev_SiRtu::recvLine(int len)
{
    int ret = 0;
    switch (len) {
    case SI_RTU_DC_LEN: ret = 1; break;
    case SI_RTU_THREE_LEN: ret = 3; break;
    default: qDebug() << "SI rtu recv len Err!!" << len; break;
    }

    return ret;
}


int Dev_SiRtu::recvDcData(uchar *ptr, int line)
{
    sObjData *obj = &(mDev->line);
    ptr = toShort(ptr, line, obj->vol.value);
    ptr = toShort(ptr, line, obj->cur.value);
    ptr = toShort(ptr, line, obj->aPow);

    ptr = toInt(ptr, line, obj->ele);
    ptr = toThreshold(ptr, line, obj->vol);
    ptr = toThreshold(ptr, line, obj->cur);

    sEnvData *env = &(mDev->env);
    env->tem.value[0] = *ptr++; env->tem.max[0] = 40;
//    env->hum.value[0] = *ptr++; env->hum.max[0] = 99;
//    env->size = env->tem.size = env->hum.size = 1;

    mDev->br = *(ptr++); // 波特率
    //ptr = toShort(ptr, line, obj->pow);
    ptr = toChar(ptr, line, obj->pf); // 功率因数
    ptr = toChar(ptr, line, obj->sw); // 开关状态
    obj->size = *(ptr++);
    mDev->hz = *(ptr++);

    if(obj->size == 0)  obj->size = 1;
    obj->vol.size = obj->cur.size = obj->size;

    return SI_RTU_DC_LEN;
}



/**
  * 功　能：读取电参数 数据
  * 入口参数：ptr -> 缓冲区
  * 出口参数：pkt -> 结构体
  * 返回值：12 正确
  */
int Dev_SiRtu::recvAcData(uchar *ptr, int line)
{
    sObjData *obj = &(mDev->line);
    ptr = toShort(ptr, line, obj->vol.value);
    ptr = toShort(ptr, line, obj->cur.value);
    ptr = toShort(ptr, line, obj->aPow);

    ptr = toInt(ptr, line, obj->ele);
    ptr = toThreshold(ptr, line, obj->vol);
    ptr = toThreshold(ptr, line, obj->cur);

//    sEnvData *env = &(mDev->env);
//    env->size = env->tem.size = env->hum.size = 1;
//    env->tem.value[0] = *ptr++;
//    env->hum.value[0] = *ptr++;

//    ptr = toEnvTh(ptr, env->tem);
//    ptr = toEnvTh(ptr, env->hum);

    obj->size = *(ptr++);
    mDev->br = *(ptr++); // 波特率
    //ptr = toShort(ptr, line, obj->pow);
    ptr = toChar(ptr, line, obj->pf); // 功率因数
    ptr = toChar(ptr, line, obj->sw); // 开关状态
    ptr += 3;
    mDev->hz = *(ptr++);

    mDt->lines = obj->size;
    if(obj->size > 1)  obj->size = 3;
    obj->vol.size = obj->cur.size = obj->size;

    return SI_RTU_THREE_LEN;
}



bool Dev_SiRtu::recvPacket(uchar *buf, int len)
{
    bool ret = true;
    int line = recvLine(len);
    switch (line) {
    case 1: recvDcData(buf, line); break;
    case 3: recvAcData(buf, line); break;
    default: ret = false; break;
    }

    return ret;
}

void Dev_SiRtu::loopObjData(sObjectData *loop, int id, RtuRecvLine *data)
{
    loop->vol.value[id] = data->vol.svalue;
    loop->vol.crMin[id] = loop->vol.min[id] = data->vol.smin;
    loop->vol.crMax[id] = loop->vol.max[id] = data->vol.smax;
    loop->vol.upalarm[id] = data->vol.supalarm;

    loop->cur.value[id] = data->cur.svalue;
    loop->cur.crMin[id] = loop->cur.min[id] = data->cur.smin;
    loop->cur.crMax[id] = loop->cur.max[id] = data->cur.smax;
    loop->cur.upalarm[id] = data->cur.supalarm;

    loop->pow.value[id] = data->pow.ivalue;
    loop->pow.crMin[id] = loop->pow.min[id] = data->pow.imin;
    loop->pow.crMax[id] = loop->pow.max[id] = data->pow.imax;
    loop->pow.upalarm[id] = data->pow.iupalarm;
    loop->ele[id] = data->ele;
    loop->pf[id] = data->pf;
    loop->sw[id] = data->sw;
    loop->apPow[id] = data->apPow;
    loop->reactivePower[id] = data->reactivePower;
    //    loop->ratedCur[id] = data->curAlarm; ////

    //loop->wave[id] = data->wave;
}

void Dev_SiRtu::loopData(sBoxData *box, Rtu_recv *pkt)
{
    sObjectData *loop = &(box->data);
    box->loopNum = loop->lineNum = pkt->lineNum;

    for(int i=0; i<loop->lineNum; i++)
    {
        RtuRecvLine *data = &(pkt->data[i]);
        loopObjData(loop, i, data);
    }
}

void Dev_SiRtu::envData(sEnvData *env, Rtu_recv *pkt)
{
    for(int i=0; i<SENSOR_NUM; ++i)
    {
        env->tem.value[i] = pkt->env[i].tem.value;
        env->tem.crMin[i] = env->tem.min[i] = pkt->env[i].tem.min;
        env->tem.crMax[i] = env->tem.max[i] = pkt->env[i].tem.max;
        env->tem.upalarm[i] =  pkt->env[i].tem.upalarm;
    }
}

void Dev_SiRtu::initData(sBoxData *box, Rtu_recv *pkt)
{
    box->dc = pkt->dc;
    box->version = pkt->version;
    box->lightning = pkt->lightning;
    box->lpsState = pkt->lpsState;
    box->proNum = pkt->proNum;
    box->curSpecification = pkt->curSpecification;
    box->workMode = pkt->workMode; // 通讯协议版本
    box->baudRate = pkt->baudRate;
    box->buzzerStatus = pkt->buzzerStatus;
    box->alarmTime = pkt->alarmTime;
    box->iOF = pkt->iOF;
    box->isd = pkt->isd;
    box->reState = pkt->reState;
    box->phaseFlag = pkt->phaseFlag;
//    box->boxType = pkt->boxType;
}

void Dev_SiRtu::thdDataV3(Rtu_recv *pkt)
{
    sBoxData *box = &(mBusData->box[pkt->addr-1]);

    box->lpsState = pkt->lpsState;
    for(int i=0; i<3; ++i) {
        box->data.pl[i] = pkt->pl[i];
    }

    if(pkt->addr == 1) {
        for(int line = 0 ; line < RTU_LINE_NUM ; ++line){
            box->data.volThd[line] = pkt->volThd[line][0];
            box->data.curThd[line] = pkt->curThd[line][0];
            for(int i=0; i<32; ++i){
                mBusData->thdData.curThd[line][i] = pkt->curThd[line][i];
                mBusData->thdData.volThd[line][i] = pkt->volThd[line][i];
            }
        }

    } else {
        ushort *thd = box->data.curThd;
        for(int i=0; i<3; ++i) {
            thd[i] = pkt->thd[i];
        }
    }
}

bool Dev_SiRtu::readPduData()
{
    Rtu_Sent it;
    uchar recv[MODBUS_RTU_SIZE] = {0};

    initRtuItem(it);
    sRtuItem pkt;
    pkt.fn = it.fn;
    pkt.crc = it.crc;
    pkt.num = it.len;
    pkt.reg = it.reg;
    pkt.addr = it.addr = mItem->addr;

    int len = mModbus->read(pkt, recv);
    sBoxData *box = &(mBusData->box[mItem->addr-1]); //共享内存
    bool ret = rtu_recv_packetV3(recv, len , mRtuPkt);
    if(ret && mItem->addr == mRtuPkt->addr) { //回收地址和发送地址同
        box->boxOffLineAlarm = 1;
        loopData(box, mRtuPkt); //更新数据
        envData(&(box->env), mRtuPkt);
        initData(box, mRtuPkt);
        box->rate.svalue = mRtuPkt->rate.svalue;
        box->rate.smin = mRtuPkt->rate.smin;
        box->rate.smax = mRtuPkt->rate.smax;
        box->rate.supalarm = mRtuPkt->rate.supalarm;
        box->reCur.svalue = mRtuPkt->reCur.svalue;
        box->reCur.smin = mRtuPkt->reCur.smin;
        box->reCur.smax = mRtuPkt->reCur.smax;
        box->reCur.supalarm = mRtuPkt->reCur.supalarm;
        box->zeroLineCur.svalue = mRtuPkt->zeroLineCur.svalue;
        box->zeroLineCur.smin = mRtuPkt->zeroLineCur.smin;
        box->zeroLineCur.smax = mRtuPkt->zeroLineCur.smax;
        box->zeroLineCur.supalarm = mRtuPkt->zeroLineCur.supalarm;

        box->volUnbalance = mRtuPkt->volUnbalance;
        box->curUnbalance = mRtuPkt->curUnbalance;

        box->data.totalPow.value[0] = mRtuPkt->totalPow.ivalue;
        box->data.totalPow.min[0] = mRtuPkt->totalPow.imin;
        box->data.totalPow.max[0] = mRtuPkt->totalPow.imax;
        thdDataV3(mRtuPkt);
    }
    return ret;
}

bool Dev_SiRtu::readRtuData()//SNMP read RTU
{
    uchar recv[MODBUS_RTU_SIZE] = {0};
    int len = mModbus->read(recv);
    return len?true:false;
}

/**
  * 功　能：长度 校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：0 正确
  */
int Dev_SiRtu::rtu_recv_len(uchar *buf, int len , int rtn)
{
    int ret = 0;
    //int rtn = RTU_SENT_LEN_V25+6;

    if(len < rtn) {  //判断是否为交流数据
        ret = -1;
        //        qDebug() << "rtu recv Err: len too short!!" << len  << rtn;
    } else if(len > rtn) {
        ret = -2;
        //        qDebug() << "rtu recv Err: len too long!!" << len << rtn ;
    } else {
        len = buf[2]*256 + buf[3];
        if(len != rtn - 6 ) {
            ret = -3;
            qDebug() << "rtu recv len Err!!"<< len << rtn  << rtn - 6;
        }
    }

    return ret;
}

/**
  * 功　能：读取数据包头部
  * 入口参数：ptr -> 缓冲区
  * 出口参数：pkt -> 结构体
  * 返回值：0 正确
  */
int Dev_SiRtu::rtu_recv_head(uchar *ptr,  Rtu_recv *pkt)
{
    pkt->addr = *(ptr++);// 从机地址码
    pkt->fn = *(ptr++);  /*功能码*/

    pkt->len = (*ptr)*256 +  *(ptr+1);  ptr += 2;/*数据长度*/
    return 4;
}

int Dev_SiRtu::rtu_start_recv_init(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    msg->proNum = (*ptr) * 256 + *(ptr+1); ptr+=2; len+=2;// 项目ID
    msg->version = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;// 软件版本
    msg->dc = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[交直流]
    if( msg->dc ) msg->dc = 0;
    else msg->dc = 1;
    msg->curSpecification = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[电流规格]
    msg->workMode = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[始端箱的工作模式]
    msg->baudRate = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[波特率]
    msg->buzzerStatus = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[蜂鸣器]
    msg->alarmTime = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->lightning = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->iOF = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[iOF触点]
    msg->isd = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[isd触点]

    msg->shuntRelease = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->reState = (*ptr) * 256 + *(ptr+1);ptr+=2;len+=2;
    msg->lpsState = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_line_data(uchar *ptr, Rtu_recv *msg , int index)
{
    RtuRecvLine *p = &(msg->data[index]);
    uint len = 0;
    p->lineVol.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->lineVol.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.ivalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.ivalue  <<= 16; // 左移16位
    p->pow.ivalue += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;// 读取低16位有功功率
    p->pow.iupalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->reactivePower = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->reactivePower  <<= 16; // 左移16位
    p->reactivePower += (*ptr) * 256 + *(ptr+1);  ptr += 2; len+=2;// 读取低16位无功功率
    p->apPow = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->apPow  <<= 16; // 左移16位
    p->apPow += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;// 读取低16位视在功率
    p->pf = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->ele = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->ele  <<= 16; // 左移16位
    p->ele += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;// 读取低16位电能
    msg->pl[index] = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_other_data(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    msg->totalApPow = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->totalApPow  <<= 16; // 左移16位
    msg->totalApPow += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;
    msg->totalPow.ivalue  = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->totalPow.ivalue  <<= 16; // 左移16位
    msg->totalPow.ivalue  += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;
    msg->totalPow.iupalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->reCur.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->reCur.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->zeroLineCur.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->zeroLineCur.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->volUnbalance = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->curUnbalance = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->breaker = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->rate.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->rate.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_env_data(uchar *ptr, Rtu_recv *msg , int index)
{
    uint len = 0;
    msg->env[index].tem.value = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->env[index].tem.alarm= (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_thd_data(uchar *ptr, Rtu_recv *msg , int index)
{
    uint len = 0;
    for(int i = 0 ; i < 32 ; ++i ){
        msg->volThd[index][i] = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    }
    for(int i = 0 ; i < 32 ; ++i ){
        msg->curThd[index][i] = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    }
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_some_alarm_data(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    for(int i = 0 ; i < RTU_TH_NUM ; ++i) // 读取温度阈值数据
    {
        msg->env[i].tem.min = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
        msg->env[i].tem.max = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    }
    msg->reCur.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->zeroLineCur.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->zeroLineCur.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->totalPow.imin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->totalPow.imin  <<= 16; // 左移16位
    msg->totalPow.imin  += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;
    msg->totalPow.imax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->totalPow.imax  <<= 16; // 左移16位
    msg->totalPow.imax  += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;
    msg->rate.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->rate.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_start_recv_last_alarm_data(uchar *ptr, Rtu_recv *msg, int index)
{
    RtuRecvLine *p = &(msg->data[index]);
    uint len = 0;
    p->lineVol.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->lineVol.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.imin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.imin <<= 16; // 左移16位
    p->pow.imin += (*ptr) * 256 + *(ptr+1);  ptr += 2; len+=2;// 读取低16位有功功率最小值
    p->pow.imax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.imax <<= 16; // 左移16位
    p->pow.imax += (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;// 读取低16位有功功率最大值
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_init(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    msg->version = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;// 软件版本
    msg->proNum = (*ptr) * 256 + *(ptr+1); ptr+=2; len+=2;// 项目ID
    msg->lineNum = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//回路数
    //msg->addr = (*ptr) * 256 + *(ptr+1);
    ptr+=2;len+=2;//地址
    msg->baudRate = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[波特率]
    msg->iOF = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[iOF触点]
    msg->buzzerStatus = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;//[蜂鸣器]
    msg->alarmTime = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
//    msg->boxType = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    ptr+=2;len+=2;//盒子类型
    msg->phaseFlag = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_loop_data(uchar *ptr, Rtu_recv *msg , int index)
{
    RtuRecvLine *p = &(msg->data[index]);
    uint len = 0;
    p->vol.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.svalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.supalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    p->pow.ivalue = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.ivalue  <<= 16; // 左移16位
    p->pow.ivalue += (*ptr) * 256 + *(ptr+1); ptr+=2; len+=2;// 读取低16位有功功率

    p->pow.iupalarm = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    p->reactivePower = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->reactivePower  <<= 16; // 左移16位
    p->reactivePower += (*ptr) * 256 + *(ptr+1);  ptr+=2; len+=2;// 读取低16位无功功率

    p->apPow = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->apPow  <<= 16; // 左移16位
    p->apPow += (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;// 读取低16位视在功率

    p->pf = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->sw = (*ptr) * 256 + *(ptr+1) - 1; ptr+=2;len+=2;

    p->ele = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->ele  <<= 16; // 左移16位
    p->ele += (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;// 读取低16位电能
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_thd_pl_data(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    for(int i = 0 ; i < RTU_LINE_NUM ; ++i){
        msg->thd[i] = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    }
    for(int i = 0 ; i < RTU_LINE_NUM ; ++i){
        msg->pl[i]= (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    }
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_env_data(uchar *ptr, Rtu_recv *msg , int index)
{
    uint len = 0;
    msg->env[index].tem.value = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->env[index].tem.alarm= (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_env_alarm_data(uchar *ptr, Rtu_recv *msg , int index)
{
    uint len = 0;
    msg->env[index].tem.min = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    msg->env[index].tem.max= (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_loop_alarm_data(uchar *ptr, Rtu_recv *msg , int index)
{
    RtuRecvLine *p = &(msg->data[index]);
    uint len = 0;
    p->vol.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->vol.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.smin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->cur.smax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    p->pow.imin = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.imin  <<= 16;
    p->pow.imin += (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;

    p->pow.imax = (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    p->pow.imax  <<= 16;
    p->pow.imax += (*ptr) * 256 + *(ptr+1); ptr+=2;len+=2;
    return len; //3.0.0版本
}

int Dev_SiRtu::rtu_plug_recv_zero_data(uchar *ptr, Rtu_recv *msg)
{
    uint len = 0;
    msg->zeroLineCur.svalue = (* ptr )*256 + *(ptr + 1); ptr+=2; len+=2;
    msg->zeroLineCur.salarm= (* ptr )*256 + *(ptr + 1); ptr+=2; len+=2;
    msg->zeroLineCur.smin= (* ptr )*256 + *(ptr + 1); ptr+=2; len+=2;
    msg->zeroLineCur.smax= (* ptr )*256 + *(ptr + 1); ptr+=2; len+=2;
    return len;
}

bool Dev_SiRtu::rtu_recv_packetV3(uchar *buf, int len, Rtu_recv *pkt)
{
    bool ret = false;
    int rtn = rtu_recv_len(buf, len , (mItem->addr == 0x01 ?RTU_SENT_LEN_V303:RTU_SENT_LEN_V30)*2+6);  //判断回收的数据是否完全
    if(rtn == 0) {
        uchar *ptr=buf;
        ptr += rtu_recv_head(ptr, pkt); //指针偏移0
        if( pkt->addr == 0x01 ){//始端箱??
            // ptr += rtu_start_recv_init(ptr , pkt);
            // ptr += 5*2;//保留
            // for(int i = 0 ; i < RTU_LINE_NUM ; ++i) // 读取相 数据
            //     ptr += rtu_start_recv_line_data(ptr , pkt , i);
            // ptr += rtu_start_recv_other_data(ptr , pkt);
            // for(int i = 0 ; i < RTU_TH_NUM ; ++i) // 读取温度 数据
            //     ptr += rtu_start_recv_env_data(ptr , pkt , i);
            // for(int i = 0 ; i < RTU_LINE_NUM ; ++i) // 读取滤波 数据
            //     ptr += rtu_start_recv_thd_data(ptr , pkt , i);
            // ptr += rtu_start_recv_some_alarm_data(ptr , pkt);
            // for(int i = 0 ; i < RTU_LINE_NUM ; ++i) // 读取滤波 数据
            //     ptr += rtu_start_recv_last_alarm_data(ptr , pkt , i);
            // int state = 1;
            // if( pkt->breaker == 1 ) state = 0;
            // for(int i = 0 ; i < RTU_LINE_NUM ; ++i)
            //     pkt->data[i].sw = state;// 更新始端箱断路器状态
            // pkt->lineNum = 3;
        }
        else{//插接箱
            ptr += rtu_plug_recv_init(ptr , pkt);
            ptr += 6*2;//保留
            for(int i = 0 ; i < RTU_LOOP_NUM ; ++i) // 读取loop 数据
                ptr += rtu_plug_recv_loop_data(ptr , pkt , i);
            ptr += rtu_plug_recv_thd_pl_data(ptr , pkt);
            for(int i = 0 ; i < RTU_TH_NUM ; ++i) // 读取温度 数据
                ptr += rtu_plug_recv_env_data(ptr , pkt , i);
            for(int i = 0 ; i < RTU_TH_NUM ; ++i) // 读取温度 alarm数据
                ptr += rtu_plug_recv_env_alarm_data(ptr , pkt , i);
            for(int i = 0 ; i < RTU_LOOP_NUM ; ++i) // 读取loop alarm数据
                ptr += rtu_plug_recv_loop_alarm_data(ptr , pkt , i);
#if ZHIJIANGINSERTBOXZERO==1
            ptr += rtu_plug_recv_zero_data(ptr , pkt);
#endif

        }
        pkt->crc = (buf[(pkt->addr == 0X01 ?RTU_SENT_LEN_V303:RTU_SENT_LEN_V30)*2+6-1]*256) + buf[(pkt->addr == 0X01 ?RTU_SENT_LEN_V303:RTU_SENT_LEN_V30)*2+6-2]; // RTU_SENT_LEN_V23*2+5
        ret = rtu_recv_crc(buf, len, pkt); //校验码
    }
    return ret;
}

bool Dev_SiRtu::rtu_recv_crc(uchar *buf, int len, Rtu_recv *msg)
{
    bool ret = true;
    int rtn = len-2;

    ushort crc = rtu_crc(buf, rtn);
    if(crc != msg->crc) {
        ret = false;
        qDebug() << "rtu recv crc Err!";
    }

    return ret;
}



















