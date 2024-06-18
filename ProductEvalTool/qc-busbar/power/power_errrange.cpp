/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "Power_errrange.h"

Power_ErrRange::Power_ErrRange(QObject *parent) : Power_Object(parent)
{
    mLogs = Power_Logs::bulid(this);
}

void Power_ErrRange::initFunSlot()
{
    // mSourceDev = mPacket->getDev(0);
}

Power_ErrRange *Power_ErrRange::bulid(QObject *parent)
{
    static Power_ErrRange* sington = nullptr;
    if(sington == nullptr)
        sington = new Power_ErrRange(parent);
    return sington;
}

bool Power_ErrRange::volErr(int id)
{
    int pass = Test_Fail;
    int value = 0; int exValue = 0; int err = 0;
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        value = mBusData->box[mItem->addr - 1].data.vol.value[id];
        exValue = cth->ip_vol *10.0;
        err = cth->ip_volErr *10.0;
    }else {
        sSiCfg *cth = &(mItem->si);
        value = mBusData->box[mItem->addr - 1].data.vol.value[id];
        exValue = cth->si_vol *10.0;
        err = cth->si_volErr *10.0;
    }
    bool ret = checkErrRange(exValue, value, err);
    if(ret) pass = Test_Pass;
    mBusData->box[mItem->addr - 1].data.vol.status[id] = pass;

    return ret;
}

bool Power_ErrRange::curErr(int id)
{
    int pass = Test_Fail;
    int value = 0; int exValue = 0; int err = 0;
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        value = mBusData->box[mItem->addr - 1].data.cur.value[id];
        exValue = cth->ip_cur *1000.0;
        err = cth->ip_curErr *1000.0;
    }else {
        sSiCfg *cth = &(mItem->si);
        value = mBusData->box[mItem->addr - 1].data.cur.value[id];
        exValue = cth->si_cur *1000.0;
        err = cth->si_curErr *1000.0;
    }

    bool ret = checkErrRange(exValue, value, err);
    if(ret) pass = Test_Pass;
    mBusData->box[mItem->addr - 1].data.cur.status[id] = pass;

    return ret;
}

bool Power_ErrRange::checkErrRange(int exValue, int value, int err)
{
    bool ret = false;
    int min = exValue - err;
    int max = exValue + err;
    if((value>=min) && (value<=max) && value) {
        ret =  true;
    } else {
        qDebug() << "value Err Range" << value << exValue << err;
    }

    return ret;
}


bool Power_ErrRange::curAlarm(int id)
{
    bool ret = true; int min; int max;
    sDataPowUnit *unit = &(mBusData->box[mItem->addr - 1].data.cur);
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        min = cth->ip_curMin*cth->rate;
        max = cth->ip_curMax*cth->rate;
        if(unit->min[id] != min) ret = false;
        if(unit->max[id] != max) ret = false;
    }else {
        sSiCfg *cth = &(mItem->si);
        min = cth->si_curMin*cth->rate;
        max = cth->si_curMax*cth->rate;
        if(unit->min[id] != min) ret = false;
        if(unit->max[id] != max) ret = false;
    }

    return ret;
}

QString Power_ErrRange::changeCurType(int index)
{
    QString str = tr("标准");
    switch(index){
    case 0:str = tr("标准");break;
    case 1:str = tr("250 A");break;
    case 2:str = tr("400 A");break;
    case 3:str = tr("650 A");break;
    case 4:str = tr("800 A");break;
    default:str = tr("标准");break;
    }
    return str;
}

QString Power_ErrRange::changeMode(int index)
{
    QString str = tr("主机地址 1 / Modbus模式");
    switch(index){
    case 0:str = tr("主机地址 0 / SNMP模式");break;
    case 1:str = tr("主机地址 1 / Modbus模式");break;
    default:str = tr("主机地址 1 / Modbus模式");break;
    }
    return str;
}

QString Power_ErrRange::changeBaudType(int index)
{
    QString str = tr("9600");
    switch(index){
    case 1:str = tr("9600");break;
    case 2:str = tr("19200");break;
    case 3:str = tr("38400");break;
    case 4:str = tr("57600");break;
    default:str = tr("9600");break;
    }
    return str;
}

