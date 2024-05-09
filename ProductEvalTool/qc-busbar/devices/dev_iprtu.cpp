/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "dev_iprtu.h"

#define IP_LINE_NUM 3
#define IP_RTU_ONE_LEN 49  // 单相数据长度
#define IP_RTU_THREE_LEN 58  // 三相数据长度


Dev_IpRtu::Dev_IpRtu(QObject *parent) : Dev_Object(parent)
{
    m_Line = 1;
}

Dev_IpRtu *Dev_IpRtu::bulid(QObject *parent)
{
    static Dev_IpRtu* sington = nullptr;
    if(sington == nullptr)
        sington = new Dev_IpRtu(parent);
    return sington;
}


int Dev_IpRtu::recvLine(int len)
{
    int ret = 0;
    switch (len/2) {
    case IP_RTU_ONE_LEN: ret = 1; break;
    case IP_RTU_THREE_LEN: ret = 3; break;
    default: qDebug() << "IP rtu recv len Err!!" << len; break;
    }

    return ret;
}


void Dev_IpRtu::initRtuItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0;
    it.num = IP_RTU_ONE_LEN;// V1


    if(mDev->devType.version >= 3 && mDev->devType.version <= 5) {
        it.num = IP_RTU_THREE_LEN;  // V3
    }
    qDebug()<<"mDev->version   "<<mDev->version <<" IP_PDUV3_BYTE  "<<IP_PDUV3_BYTE;
    if(mDev->devType.version == IP_PDUV3_BYTE) it.num = IP_RTU_THREE_LEN;  // V3
}


uchar *Dev_IpRtu::getSwitch(uchar *ptr, int line, uchar *value)
{
    ptr = getShort(ptr, line, value);
    for(int i=0; i<line; ++i) {
        if(value[i] == 2) value[i] = 1; else value[i] = 0;
    }

    return ptr;
}


/**
  * 功　能：读取电参数 数据
  * 入口参数：ptr -> 缓冲区
  * 出口参数：pkt -> 结构体
  * 返回值：12 正确
  */
int Dev_IpRtu::recvDataV1(uchar *ptr)
{
    uchar *ret = ptr;
//    int line = IP_LINE_NUM;

//    sObjData *obj = &(mDev->line);
//    ptr =  toShort(ptr, line, obj->vol.value);
//    ptr =  toShort(ptr, line, obj->cur.value);
//    ptr =  toShort(ptr, line, obj->pow);
//    ptr =  toShort(ptr, line, obj->hz);
//    ptr =  toInt(ptr, line, obj->ele);

//    sEnvData *env = &(mDev->env);
//    env->size = env->tem.size = env->hum.size = 1;
//    ptr =  toShort(ptr, 1, env->tem.value);
//    ptr =  toShort(ptr, 1, env->hum.value);
//    env->tem.max[0] = 40; env->hum.max[0] = 99;

//    ptr = toThreshold(ptr, line, obj->vol);
//    ptr = toThreshold(ptr, line, obj->cur);
//    ptr += 8 + (2*2*line + 4); // 报警标志位

//    ptr = getSwitch(ptr, 2, obj->sw);
//    obj->size = getShort(ptr); ptr +=2;
//    mDev->version = getShort(ptr); ptr +=2;
//    mDev->br = getShort(ptr); ptr +=2;
//    mDev->reserve = getShort(ptr); ptr +=2;

//    mDt->lines = obj->size;
//    if(obj->size == 2)  obj->size = 3;
//    obj->vol.size = obj->cur.size = obj->size;

    return ptr-ret;
}


/**
  * 功　能：读取电参数 数据
  * 入口参数：ptr -> 缓冲区
  * 出口参数：pkt -> 结构体
  * 返回值：12 正确
  */
int Dev_IpRtu::recvDataV3(uchar *ptr)
{
    uchar *ret = ptr;
    int line = IP_LINE_NUM;

    sObjData *obj = &(mDev->line);
    if(mDt->version != IP_PDUV3_BYTE){
        ptr =  toShort(ptr, line, obj->vol.value);
        ptr =  toShort(ptr, line, obj->cur.value);
        ptr =  getShort(ptr, line, obj->pf);
        //ptr =  toShort(ptr, line, obj->pow);
        ptr =  toShort(ptr, line, obj->aPow);
        ptr =  toInt(ptr, line, obj->ele);
        mDev->hz = getShort(ptr); ptr +=2;
        ptr = getSwitch(ptr, line, obj->sw); // 开关状态

//        sEnvData *env = &(mDev->env);
//        env->size = env->tem.size = env->hum.size = 1;
//        ptr =  toShort(ptr, 1, env->tem.value);
//        ptr =  toShort(ptr, 1, env->hum.value);
//        env->tem.max[0] = 40; env->hum.max[0] = 99;

        ptr = toThreshold(ptr, line, obj->vol);
        ptr = toThreshold(ptr, line, obj->cur);
        ptr += 8 + (2*2*line + 2 + 2); // 报警

        obj->size = getShort(ptr); ptr +=2;
        mDev->version = getShort(ptr); ptr +=2;
        mDev->br = getShort(ptr); ptr +=2;
        ptr = toChar(ptr, 8, mDev->devType.ip);
        mDev->reserve = getShort(ptr); ptr +=2;

        mDt->lines = obj->size;
        if(obj->size == 2)  obj->size = 3;
        obj->vol.size = obj->cur.size = obj->size;
    }else{
        ptr =  toShort(ptr, line, obj->vol.value);
        ptr =  toShort(ptr, line, obj->cur.value);
        ptr =  getShort(ptr, line, obj->pf);
        //ptr =  toShort(ptr, line, obj->pow);
        ptr =  calcaPow(ptr, line, obj->aPow, obj->vol.value , obj->cur.value);
        ptr =  toInt(ptr, line, obj->ele);
        mDev->hz = getShort(ptr); ptr +=2;
        ptr = getSwitch(ptr, line, obj->sw); // 开关状态

//        sEnvData *env = &(mDev->env);
//        env->size = env->tem.size = env->hum.size = 1;
//        ptr =  toShort(ptr, 1, env->tem.value);
//        ptr =  toShort(ptr, 1, env->hum.value);
//        env->tem.max[0] = 40; env->hum.max[0] = 99;

        ptr = toThreshold10(ptr, line, obj->vol);
        ptr = toCurThreshold(ptr, line, obj->cur);
        ptr += 8 + (2*2*line + 2 + 2); // 报警

        obj->size = getShort(ptr); ptr +=2;
        mDev->version = getShort(ptr); ptr +=2;
        mDev->br = getShort(ptr); ptr +=2;
        ptr = toChar(ptr, 8, mDev->devType.ip);
        mDev->reserve = getShort(ptr); ptr +=2;

        mDt->lines = obj->size;
        if(obj->size == 2)  obj->size = 3;
        obj->vol.size = obj->cur.size = obj->size;
    }

    return ptr-ret;
}


bool Dev_IpRtu::recvPacket(uchar *buf, int len)
{
    bool ret = true;
    int line = recvLine(len);
    switch (line) {
    case 1: recvDataV1(buf); break;
    case 3: recvDataV3(buf); break;
    default: ret = false; break;
    }

    return ret;
}

bool Dev_IpRtu::readPduData()
{
    sRtuItem it;
    uchar recv[MODBUS_RTU_SIZE] = {0};
    if(IP_PDUV3_ECC == mDt->version){
        initRtuLineNumItem(it);
        mModbus->read(it, recv);
        recvLineNumPacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuBaseValueItem(it);
        mModbus->read(it, recv);
        recvBaseValuePacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuCurRangeItem(it);
        mModbus->read(it, recv);
        recvCurRangePacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuVolRangeItem(it);
        mModbus->read(it, recv);
        recvVolRangePacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuCurTemHumItem(it);
        mModbus->read(it, recv);
        recvCurTemHumPacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuTemRangeItem(it);
        mModbus->read(it, recv);
        recvTemRangePacket(recv);
        memset(recv , 0 , strlen((char*)recv));

        initRtuHumRangeItem(it);
        mModbus->read(it, recv);
        return recvHumRangePacket(recv);
    }else{
        initRtuItem(it);
        int len = mModbus->read(it, recv);
        return recvPacket(recv, len);
    }
}


uchar* Dev_IpRtu::calcaPow(uchar *ptr, int line, ushort *value , ushort *vol, ushort *cur)
{
    for(int i=0; i<line; ++i) {
        value[i] =  vol[i]*cur[i]/1000.0;  ptr += 2;
    }
    return ptr;
}

uchar* Dev_IpRtu::toCurThreshold(uchar *ptr, int line,sDataUnit &unit)
{
    for(int i=0; i<line; ++i) {
        ptr = toShort10(ptr, 1, &unit.min[i] , 10);
        ptr = toShort10(ptr, 1, &unit.max[i] , 10);
        ptr += 2;
    }

    return ptr;
}


void Dev_IpRtu::initRtuBaseValueItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x23;
    it.num = 0x08;
}

void Dev_IpRtu::initRtuLineNumItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x015A;
    it.num = 0x01;
}

void Dev_IpRtu::initRtuCurRangeItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x03EB;
    it.num = 0x02;
}

void Dev_IpRtu::initRtuVolRangeItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x0475;
    it.num = 0x02;
}

void Dev_IpRtu::initRtuCurTemHumItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x01F4;
    it.num = 0x02;
}

void Dev_IpRtu::initRtuTemRangeItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x0453;
    it.num = 0x02;
}

void Dev_IpRtu::initRtuHumRangeItem(sRtuItem &it)
{
    it.addr = mDev->id;
    it.fn = 0x03;
    it.reg = 0x0463;
    it.num = 0x02;
}

int Dev_IpRtu::recvLineNumPacket(uchar *buf)
{
//    sObjData *obj = &(mDev->line);
//    sEnvData *envobj = &(mDev->env);
//    obj->size = m_Line =  (*buf) * 256 + *(buf+1);
//    mDt->lines = 2;
//    obj->vol.size = obj->cur.size = obj->size;
//    envobj->size = envobj->tem.size = envobj->hum.size = 1;
    return 2;
}

int Dev_IpRtu::recvBaseValuePacket(uchar *buf)
{
    sObjData *obj = &(mDev->line);
    int len = 0;
    obj->vol.value[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->cur.value[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->pow[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    buf += 2; len += 2;//无功功率不解析
    obj->pf[0] =  ((*buf) * 256 + *(buf+1))/10;  buf += 2; len += 2;
    obj->ele[0] =  (*buf) * 256 + *(buf+1);  buf += 2;len += 2; // 读取电能高8位
    obj->ele[0] <<= 16; // 左移8位
    obj->ele[0] +=  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;// 读取电能底8位
    return len;
}

int Dev_IpRtu::recvCurRangePacket(uchar *buf)
{
    sObjData *obj = &(mDev->line);
    int len = 0;
    obj->cur.max[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->cur.min[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    return len;
}

int Dev_IpRtu::recvVolRangePacket(uchar *buf)
{
    sObjData *obj = &(mDev->line);
    int len = 0;
    obj->vol.max[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->vol.min[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    return len;
}

int Dev_IpRtu::recvTemRangePacket(uchar *buf)
{
    sEnvData *obj = &(mDev->env);
    int len = 0;
    obj->tem.max[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->tem.min[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    obj->tem.max[0] -= 40;
    obj->tem.min[0] -= 40;
    return len;
}

int Dev_IpRtu::recvHumRangePacket(uchar *buf)
{
    sEnvData *obj = &(mDev->env);
    int len = 0;
//    obj->hum.max[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
//    obj->hum.min[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    return len;
}

int Dev_IpRtu::recvCurTemHumPacket(uchar *buf)
{
    sEnvData *obj = &(mDev->env);
    int len = 0;
    obj->tem.value[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
//    if(obj->tem.value[0])obj->tem.value[0] -= 40;
//    obj->hum.value[0] =  (*buf) * 256 + *(buf+1);  buf += 2; len += 2;
    return len;
}
