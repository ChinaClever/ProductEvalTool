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

bool Power_ErrRange::powErr(int id)
{
    int pass = Test_Fail;
    int value = mBusData->box[mItem->addr - 1].data.pow.value[id];
    int exValue = mSourceDev->line.pow[id];
    if(mItem->modeId == START_BUSBAR) exValue*=15;
    int err = exValue * (mItem->err.powErr/1000.0);

    bool ret = checkErrRange(exValue, value, err);
    if(ret) pass = Test_Pass;
    mBusData->box[mItem->addr - 1].data.pow.status[id] = pass;

    return ret;
}

bool Power_ErrRange::temErr(int id)
{
    bool ret = false;
    int pass = Test_Fail;
    int value = mBusData->box[mItem->addr - 1].env.tem.value[id];

    if((value>0)&&(value<100))ret=true;
    if(ret) pass = Test_Pass;
    mBusData->box[mItem->addr - 1].env.tem.status[id] = pass;

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


bool Power_ErrRange::temAlarm(int id)
{
    bool ret = true;
    sObjCfg *cth = mItem->modeId == START_BUSBAR ?(&(mItem->ip_cfg)):(&(mItem->si_cfg));
    sDataValue *unit = &(mBusData->box[mItem->addr - 1].env.tem);
    int min = cth->tem.min*cth->tem.rate;
    int max = cth->tem.max*cth->tem.rate;
    if(unit->min[id] != min) ret = false;
    if(unit->max[id] != max) ret = false;

    return ret;
}
bool Power_ErrRange::temEnvAlarm(int id)
{
    bool ret = true;
    sSiCfg *cth = &(mItem->si);
    sDataValue *unit = &(mBusData->box[mItem->addr - 1].env.tem);

    QString str = tr("温度传感器%1最小阈值：实际值 %2℃，期望值 %3℃").arg(id+1).arg(unit->min[id]).arg(cth->temMin[id]);
    mLogs->updatePro(str);
    if(unit->min[id] != cth->temMin[id]) ret = false;
    str = tr("温度传感器%1最大阈值：实际值 %2℃，期望值 %3℃").arg(id+1).arg(unit->max[id]).arg(cth->temMax[id]);
    mLogs->updatePro(str);
    if(unit->max[id] != cth->temMax[id]) ret = false;

    return ret;
}
bool Power_ErrRange::powAlarm(int id)
{
    bool ret = true;
    sObjCfg *cth = mItem->modeId == START_BUSBAR ?(&(mItem->ip_cfg)):(&(mItem->si_cfg));
    sDataPowUnit *unit = &(mBusData->box[mItem->addr - 1].data.pow);
    uint min = cth->pow.min*cth->pow.rate;
    uint max = cth->pow.max*cth->pow.rate;
    if(unit->min[id] != min) ret = false;
    if(unit->max[id] != max) ret = false;

    return ret;
}

bool Power_ErrRange::lineVolAlarm(int id)
{
    bool ret = true;
    sObjCfg *cth = &(mItem->ip_cfg);
    sDataValue *unit = &(mBusData->box[mItem->addr - 1].data.lineVol);
    uint min = cth->linevol.min*cth->linevol.rate;
    uint max = cth->linevol.max*cth->linevol.rate;
    if(unit->min[id] != min) ret = false;
    if(unit->max[id] != max) ret = false;

    return ret;
}

bool Power_ErrRange::hzAlarm()
{
    bool ret = true;
    sObjCfg *cth = &(mItem->ip_cfg);
    sRtuUshortUnit *unit = &(mBusData->box[mItem->addr - 1].rate);
    uint min = cth->hz.min*cth->hz.rate;
    uint max = cth->hz.max*cth->hz.rate;
    if(unit->smin != min) ret = false;
    if(unit->smax != max) ret = false;

    return ret;
}

bool Power_ErrRange::totalPowAlarm()
{
    bool ret = true;
    sObjCfg *cth = &(mItem->ip_cfg);
    sRtuULLintUnit *unit = &(mBusData->box[mItem->addr - 1].totalPow);
    uint min = cth->totalpow.min*cth->totalpow.rate;
    uint max = cth->totalpow.max*cth->totalpow.rate;
    if(unit->imin != min) ret = false;
    if(unit->imax != max) ret = false;

    return ret;
}

bool Power_ErrRange::zeroLineCurAlarm()
{
    bool ret = true;
    sObjCfg *cth = mItem->modeId == START_BUSBAR ?(&(mItem->ip_cfg)):(&(mItem->si_cfg));
//    sObjCfg *cth = &(mItem->ip_cfg);
    sRtuCurUintUnit *unit = &(mBusData->box[mItem->addr - 1].zeroLineCur);
    // uint min = cth->zerocur.min*cth->zerocur.rate;
    uint max = cth->zerocur.max*cth->zerocur.rate;
    // if(unit->smin != min) ret = false;
    if(unit->smax != max) ret = false;

    return ret;
}

bool Power_ErrRange::residualAlarm()
{
    bool ret = true;
    sObjCfg *cth = &(mItem->ip_cfg);
    sRtuUshortUnit *unit = &(mBusData->box[mItem->addr - 1].reCur);
    uint max = cth->recur.max*cth->recur.rate;
    if(unit->smax != max) ret = false;

    return ret;
}

void Power_ErrRange::compareInsertValue()
{
    compareInsertInfo();
    mPro->step = Test_vert;
}

void Power_ErrRange::compareInsertInfo()
{
    QString str = tr("对比插接箱基本信息！");
    mLogs->updatePro(str);
    sBoxData* b = &(mPacket->share_mem_get()->box[mItem->addr-1]);
    bool ret = false;
    int curValue = b->baudRate;
    int expect = mItem->si.si_baud+1;
    if(curValue == expect) ret = true;
    str = tr("插接箱波特率实际值：%1 , 期待值：%2！").arg(changeBaudType(curValue)).arg(changeBaudType(expect));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->buzzerStatus;
    expect = mItem->si.si_buzzer;
    if(curValue == expect) ret = true;
    str = tr("插接箱蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->si.si_filter;
    if(curValue == expect) ret = true;
    str = tr("插接箱过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->iOF;
    expect = mItem->si.si_iOF;
    if(curValue == expect) ret = true;
    str = tr("插接箱iOF触点实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->phaseFlag;
    expect = mItem->si.si_phaseflag;
    if(curValue == expect) ret = true;
    str = tr("插接箱单相或三相实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;
}
void Power_ErrRange::compareEnvInfo()
{
    QString str = tr("对比温度传感器基本信息！");
    mLogs->updatePro(str);
    sBoxData* b = &(mPacket->share_mem_get()->box[mItem->addr-1]);
    bool ret = false;
    int curValue = b->buzzerStatus;
    int expect = mItem->si.tem_buzzer;
    if(curValue == expect) ret = true;
    str = tr("温度传感器蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->si.tem_filter;
    if(curValue == expect) ret = true;
    str = tr("温度传感器过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;
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

void Power_ErrRange::compareStartMac()
{
    bool ret = false;
    if( mBusData->box[mItem->addr-1].mac.contains("2c:26:5f:") ){
        ret = true;
        QString info = tr("始端箱MAC Address :%1 设置成功！")
                        .arg(mBusData->box[mItem->addr-1].mac);
        mLogs->updatePro(info,ret);
    }else{
        QString info = tr("始端箱MAC Address :%1 设置失败！")
                        .arg(mBusData->box[mItem->addr-1].mac);
        mLogs->updatePro(info,ret);
    }
}

void Power_ErrRange::compareStartValue()
{
    compareStartInfo();
    compareStartMac();
}

void Power_ErrRange::compareStartInfo()
{
    QString str = tr("对比始端箱基本信息！");
    mLogs->updatePro(str);
    sBoxData* b = &(mPacket->share_mem_get()->box[mItem->addr-1]);
    bool ret = false;
    int curValue = b->curSpecification;
    int expect = mItem->ip.ip_curtype;
    if(curValue == expect) ret = true;
    str = tr("始端箱电流规格实际值：%1 , 期待值：%2！").arg(changeCurType(curValue)).arg(changeCurType(expect));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->workMode;
    expect = mItem->ip.ip_mode;
    if(curValue == expect) ret = true;
    str = tr("始端箱通讯模式实际值：%1 , 期待值：%2！").arg(changeMode(curValue)).arg(changeMode(expect));
    mLogs->updatePro(str,ret);ret = false;

    curValue = mPacket->share_mem_get()->boxNum;
    expect = mItem->ip.ip_num;
    if(curValue == expect) ret = true;
    str = tr("始端箱级联数目实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->buzzerStatus;
    expect = mItem->ip.ip_buzzer;
    if(curValue == expect) ret = true;
    str = tr("始端箱蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->ip.ip_filter;
    if(curValue == expect) ret = true;
    str = tr("始端箱过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->lightning;
    expect = mItem->ip.ip_lightning;
    if(curValue == expect) ret = true;
    str = tr("始端箱防雷模块实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->iOF;
    expect = mItem->ip.ip_iOF;
    if(curValue == expect) ret = true;
    str = tr("始端箱iOF辅助触点实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->isd;
    expect = mItem->ip.ip_ISD;
    if(curValue == expect) ret = true;
    str = tr("始端箱ISD报警触点实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->shuntRelease;
    expect = mItem->ip.ip_shunt;
    if(curValue == expect) ret = true;
    str = tr("始端箱分励脱扣实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->reState;
    expect = mItem->ip.ip_residual;
    if(curValue == expect) ret = true;
    str = tr("始端箱剩余电流实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;
}
