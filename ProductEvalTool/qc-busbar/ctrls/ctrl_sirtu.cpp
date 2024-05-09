/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "ctrl_sirtu.h"

Ctrl_SiRtu::Ctrl_SiRtu(QObject *parent) : Ctrl_Object(parent)
{
}

Ctrl_SiRtu *Ctrl_SiRtu::bulid(QObject *parent)
{
    static Ctrl_SiRtu* sington = nullptr;
    if(sington == nullptr)
        sington = new Ctrl_SiRtu(parent);
    return sington;
}

bool Ctrl_SiRtu::setCurTh(int i)
{
    bool ret = true;
    ushort reg = 0x1008 + 2*i;
    if(DC == mDt->ac) reg = 0x1016;

    sDataUnit *unit = &(mDev->line.cur);
    ushort value = mItem->cTh.cur_max;
    if((mDt->lines == 2) && i) {
        if(value == 630&&value == 6300) value = (value/10 +1)/2 * 10; // 解决单项二路阈值问题
        else value /= 2;
    }
    if(unit->max[i] != value) {
        ret = sentRtuCmd(reg++, value); if(!ret) return ret;
    } else reg++;

    value = mItem->cTh.cur_min;
    if(unit->min[i] != value) {
        ret = sentRtuCmd(reg++, value); if(!ret) return ret;
    } else reg++;

    return ret;
}

bool Ctrl_SiRtu::setVolTh(int i)
{
    bool ret = true;
    ushort reg = 0x1002 + 2*i;
    if(DC == mDt->ac) reg = 0x1014;

    sDataUnit *unit = &(mDev->line.vol);
    ushort value = mItem->cTh.vol_max;
    if(unit->max[i] != value) {
        ret = sentRtuCmd(reg++, value); if(!ret) return ret;
    } else reg++;

    value = mItem->cTh.vol_min;
    if(unit->min[i] != value) {
        ret = sentRtuCmd(reg++, value); if(!ret) return ret;
    } else reg++;

    return ret;
}

// 表示行业标准 Modbus RTU 模式
bool Ctrl_SiRtu::setModel()
{
    sRtuSetItem it;
    it.addr = mDev->id;
    it.fn = 0x06;
    it.reg = 0x1019;
    it.data = 1;

    return mModbus->write(it);
}

void Ctrl_SiRtu::eleClean()
{
    if(mItem->modeId == START_BUSBAR){
        for(int i = 0 ; i < START_LINE_NUM ; i++){
            setBusbarStartEle(i+1); // 清除电能
            mPacket->delay(6);//12
        }
    }else{
        for(int i = 0 ; i < mBusData->box[mItem->addr - 1].loopNum ; i++){
            setBusbarInsertEle(i+1); // 清除电能
            mPacket->delay(7);
        }
    }
}

bool Ctrl_SiRtu::factorySet()
{
    return sentRtuCmd(0x1001, 1); // 清除电能
}

//bool Ctrl_SiRtu::sentRtuCmd(ushort reg, ushort value)
//{
//    sRtuSetItem it;
//    it.addr = mDev->id;
//    it.fn = 0x10;
//    it.reg = reg;
//    it.data = value;
//    if(mItem->si.si_mod) it.fn = 0x06;

//    return mModbus->write(it);
//}

bool Ctrl_SiRtu::sentRtuCmd(ushort reg, ushort value, uchar fn)
{
    bool ret = true;
    Rtu_Sent_Single_Ushort_V3 it;
    it.addr = mItem->addr;
    it.fn = fn;
    it.reg = reg;
    it.val = value;
    it.num = 1;

    for(int i=0; i<3; ++i) {
        ret = mModbus->write(it);
        if(ret) break; else sDataPacket::bulid()->delay(2+i);
    }

    return ret;
}

bool Ctrl_SiRtu::rtu_sent_ushortV3_buff(uchar addr, ushort reg, uint num,  uint val1, uint val2 )
{
    bool ret = true;
    Rtu_Sent_Ushort_V3 msg;

    msg.addr = addr;
    msg.fn   = 0x10;
    msg.reg  = reg;
    msg.num  = num;
    msg.val1 = val1;
    msg.val2 = val2;
    for(int i=0; i<3; ++i) {
        ret = mModbus->write(msg);
        if(ret) break; else sDataPacket::bulid()->delay(2+i);
    }
    return ret;
}

bool Ctrl_SiRtu::rtu_sent_uintV3_buff(uchar addr, ushort reg, uint num,  uint val1, uint val2)
{
    bool ret = true;
    Rtu_Sent_Uint_V3 msg;

    msg.addr = addr;
    msg.fn   = 0x10;
    msg.reg  = reg;
    msg.num  = num;
    msg.val1 = (val1 >> 16) & (0xffff);
    msg.val2 = val1 & (0xffff);
    msg.val3 = (val2 >> 16) & (0xffff);
    msg.val4 = val2 & (0xffff);

    for(int i=0; i<3; ++i) {
        ret = mModbus->write(msg);
        if(ret) break; else sDataPacket::bulid()->delay(2+i);
    }

    return ret;
}

bool Ctrl_SiRtu::setBusbarStartEle(int index)//清始端箱电能
{
    bool ret = true;
    ret = sentRtuCmd(StartEle_3031+(index-1)*110, 1);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertBaud(int val)
{
    bool ret = true;
    ret = sentRtuCmd(PlugBaudRate, val);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertiOF(int val)
{
    bool ret = true;
    ret = sentRtuCmd(SetPlugiOF, val);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertBuzzer(int val)
{
    bool ret = true;
    ret = sentRtuCmd(SetPlugBuzzer, val);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertFilter(int val)
{
    bool ret = true;
    ret = sentRtuCmd(SetPlugAlarmTime, val);

    return ret;
}

bool Ctrl_SiRtu::setBusbarStartPow(int val1 , int val2)//test
{
    bool ret = true;
    ret = rtu_sent_uintV3_buff(mItem->addr,StartTotalPowMIN_1, 4 , val1 , val2);

    return ret;
}

bool Ctrl_SiRtu::setBusbarStartLineVol(int index , int val1 , int val2)//test
{
    bool ret = true;
    ret = rtu_sent_ushortV3_buff(mItem->addr,StartLineVoltageMIN_L1+(index-1)*10, 2 , val1 , val2);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertEle(int index)
{
    bool ret = true;
    ret = sentRtuCmd(SetPlugEle_1+(index-1)*15, 1);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertTem(int index , int val1 , int val2)
{
    bool ret = true;
    ret = rtu_sent_ushortV3_buff(mItem->addr, PlugTemperatureMIN_1+(index-1)*2, 2 , val1 , val2);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertCur(int index , int val1 , int val2)
{
    bool ret = true;
    ret = rtu_sent_ushortV3_buff(mItem->addr, PlugCurrentMIN_L1+(index-1)*8, 2 , val1 , val2);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertVol(int index , int val1 , int val2)
{
    bool ret = true;
    ret = rtu_sent_ushortV3_buff(mItem->addr, PlugVoltageMIN_L1+(index-1)*8, 2 , val1 , val2);

    return ret;
}

bool Ctrl_SiRtu::setBusbarInsertPow(int index , int val1 , int val2)
{
    bool ret = true;
    ret = rtu_sent_uintV3_buff(mItem->addr,PlugPowerMIN_L1_1+(index-1)*8, 4 , val1 , val2);

    return ret;
}
