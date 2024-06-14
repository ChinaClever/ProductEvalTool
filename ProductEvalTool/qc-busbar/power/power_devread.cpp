/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_devread.h"

Power_DevRead::Power_DevRead(QObject *parent) : Power_Object(parent)
{

}

void Power_DevRead::initFunSlot()
{
    mRtu = nullptr;
    mSiRtu = Dev_SiRtu::bulid(this);
    mIpRtu = Dev_IpRtu::bulid(this);
    mLogs = Power_Logs::bulid(this);
    mSn = Sn_SerialNum::bulid(this);
    mIpSnmp = Dev_IpSnmp::bulid(this);
    mItem = Cfg::bulid()->item;
    mErr = Power_ErrRange::bulid(this);
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        exValue = cth->ip_cur *1000.0;
        err = cth->ip_curErr *1000.0;
    }else {
        sSiCfg *cth = &(mItem->si);
        exValue = cth->si_cur *1000.0;
        err = cth->si_curErr *1000.0;
    }
}

Power_DevRead *Power_DevRead::bulid(QObject *parent)
{
    static Power_DevRead* sington = nullptr;
    if(sington == nullptr)
        sington = new Power_DevRead(parent);
    return sington;
}

bool Power_DevRead::readSn()
{
    bool ret = true;
    ret = mSn->snEnter();
//     if(ret) ret = readDevData();

    return ret;
}

bool Power_DevRead::readData()
{
    bool ret = true;
    if( mItem->modeId == START_BUSBAR ){
            // if(mItem->ip.ip_mode == 1){//RTU模式地址为1
                ret = mSiRtu->readPduData();
            // }else ret = mSiRtu->readRtuData();
        if(ret) {
            ret = checkNet();
            if(ret) ret = mIpSnmp->readPduData();
            }
    }else{
        ret = mSiRtu->readPduData();
    }

    return ret;
}

bool Power_DevRead::readDev()
{
    bool ret = mPacket->delay(5);
    if(ret) {
        if( mItem->modeId == START_BUSBAR ){
            QString str = tr("始端箱串口RTU通讯");
            for(int i=0; i<5; ++i) {
                // if(mItem->ip.ip_mode == 1){//RTU模式地址为1
                    ret = mSiRtu->readPduData();
                // }else ret = mSiRtu->readRtuData();
                if(ret) break; else if(!mPacket->delay(8)) break;
            }
            if(ret) str += tr("成功");
            else{ str += tr("失败"); mPro->result = Test_Fail;}
            mLogs->updatePro(str, ret);

            if(ret) {
                ret = checkNet();
                if(ret) ret = mIpSnmp->readPduData();
                str = tr("始端箱SNMP通讯");
                if(ret) str += tr("成功");
                else{ str += tr("失败"); mPro->result = Test_Fail;}
                mLogs->updatePro(str, ret);
            }
        }else{
            for(int i=0; i<5; ++i) {
                ret = mSiRtu->readPduData();
                if(ret) break; else if(!mPacket->delay(3)) break;
            }
            QString str = tr("插接箱串口RTU通讯");
            if(ret) str += tr("成功");
            else{ str += tr("失败"); mPro->result = Test_Fail;}
            mLogs->updatePro(str, ret);
        }
    }

    return ret;
}

QString Power_DevRead::getConnectModeOid()
{
    return mIpSnmp->getConnectModeOid();
}

QString Power_DevRead::setShuntReleaseCtrlOid()
{
    return mIpSnmp->setShuntReleaseCtrlOid();
}

QString Power_DevRead::getFilterOid()
{
    return mIpSnmp->getFilterOid();
}

bool Power_DevRead::SetInfo(QString o , QString val)
{
    return mIpSnmp->SetInfo(o,val);
}

bool Power_DevRead::checkNet()
{
    QString ip;
    bool ret = false;
    QString str = tr("网络检测");
    for(int k=0; k<3; ++k) {
        if(!ret) {
            msleep(50);
            ip = "192.168.1.163";
            ret = cm_pingNet(ip);
        }
        if(ret) break;
    }

    if(ret) str += tr("正常");else str += tr("异常");

    // mLogs->updatePro(str, ret);
    return ret;
}

bool Power_DevRead::readSnmp()
{
    bool ret = true;
    if(!mItem->ip.modbus) {
        QString str = tr("SNMP通讯");
        ret = mIpSnmp->readPduData();
        if(!ret) {
            mIpSnmp->resetSnmp();
            ret = mIpSnmp->readPduData();
        }
        if(ret) str += tr("成功");
        else str += tr("失败");
        mLogs->updatePro(str, ret);
    }

    return ret;
}

bool Power_DevRead::checkIpVersion()
{
    bool ret = !isRun;
    if(ret) {
        QString str = tr("设备版本检查");
        if(mDt->version != mItem->ip.version) {
            if( mItem->ip.version ==  IP_PDUV1_HUADA || mDt->version ==  IP_PDUV3){
                str += tr("正常");
            }
            else{
                str += tr("出错 期望版本V%1，实际版本V%2").arg(mItem->ip.version).arg(mDt->version);
                ret = false;
            }
        } else {
            str += tr("正常");
        }
        mLogs->updatePro(str, ret);
    }

    return ret;
}

bool Power_DevRead::checkIpLine()
{
    bool ret = !isRun;
    if(ret && mItem->ip.lines) {
        QString str = tr("设备相数检查");
        if(mDt->lines != mItem->ip.lines) {
            str += tr("出错 期望相数L=%1，实际相数L=%2").arg(mItem->ip.lines).arg(mDt->lines);
            ret = false;
        } else {
            str += tr("正常");
        }
        mLogs->updatePro(str, ret);
    }
    if(ret) ret = checkIpVersion();

    return ret;
}

bool Power_DevRead::checkSiLine()
{
    bool ret = !isRun;
    if(ret) {
        QString str = tr("设备相数检查");
        if(mDt->lines != mItem->si.lines) {
            str += tr("出错 期望相数L=%1，实际相数L=%2").arg(mItem->si.lines).arg(mDt->lines);
            ret = false;
        } else {
            str += tr("正常");
        }
        mLogs->updatePro(str, ret);
    }

    return ret;
}

bool Power_DevRead::readNet()
{
    bool ret = true;
    if(IP_PDU == mDt->devType) {
        if(ret) ret = checkIpLine();
    } else {
        ret = checkSiLine();
    }

    return ret;
}

bool Power_DevRead::readDevData()
{
    bool ret = true;

    switch (mItem->modeId) {
    case START_BUSBAR:  mRtu = mIpSnmp; break;
    case INSERT_BUSBAR:  mRtu = mSiRtu; break;
    case TEMPER_BUSBAR:  mRtu = mSiRtu; break;
    default: ret = false; break;
    }
    ret = mRtu->readPduData();
    return ret;
}

QString Power_DevRead::trans(int index)
{
    QString str;
    int ca = index % 3 + 1;
    switch(ca)
    {
    case 1: str = "A"+QString::number(index / 3+1);
        break;
    case 2: str = "B"+QString::number(index / 3+1);
        break;
    case 3: str = "C"+QString::number(index / 3+1);
        break;
    }

    return str;
}

bool Power_DevRead::NineInsertOne_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相九路电流、功率
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i] /COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];

        if(((!b)&&(c)&&(a))) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret); str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;           
            str = tr("输出口1-B1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3 and check that the reading of C current is 0A and A/B current is normal");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口1-C1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-C1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器1， A1/B1/C1电压为0V，A2/B2/C2/A3/B3/C3电压正常；");
    QString str = tr("关闭插接箱的断路器1，打开插接箱的断路器2、3");
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 1, A1/B1/C1 voltage is 0V, A2/B2/C2/A3/B3/C3 voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0, b=0, c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.status[i];
            b += Obj->vol.status[3+i];
            c += Obj->vol.status[6+i];
        }
        if((a ==6)&&(b == 3)&&(c == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret); str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口2");  //三相回路电流、功率
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1){
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[3];
            if(a) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str5); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];

        if((!b)&&(c)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }           
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];

        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-C2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器2，A2/B2/C2电压为0V, A1/B1/C1/A3/B3/C3电压正常；");
    QString str = tr("打开插接箱的断路器1、3，关闭插接箱的断路器2");
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 2, A2/B2/C2 voltage is 0V, A1/B1/C1/A3/B3/C3 voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0, b=0, c=0;
        ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.status[i];
            b += Obj->vol.status[3+i];
            c += Obj->vol.status[6+i];
        }
        if((b == 6)&&(a == 3)&&(c == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座3接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口3");
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Connect output socket 3 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 3 current check");

    while(1){
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[6];
            if(a) {
               ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口3 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(4);
    flag = 0;
    str = tr("关闭负载输入端L1");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =6;i<9;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[6]; b = Obj->cur.status[7]; c = Obj->cur.status[8];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-A3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }           
            mLogs->updatePro(str, ret); ret = false;
            str = tr("输出口3-A3检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 3 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =6;i<9;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[6]; b = Obj->cur.status[7]; c = Obj->cur.status[8];

        if((!b) &&(c) &&(a)) {           
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-B3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-B3检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求");mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 3 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =6;i<9;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[6]; b = Obj->cur.status[7]; c = Obj->cur.status[8];
       if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C3检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器3，A3/B3/C3电压为0V, A1/B1/C1/A2/B2/C2电压正常；");
    QString str = tr("打开插接箱的断路器1、2，关闭插接箱的断路器3");
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 3, A3/B3/C3 voltage is 0V, A1/B1/C1/A2/B2/C2 voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c=0;
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.status[i];
            b += Obj->vol.status[3+i];
            c += Obj->vol.status[6+i];
        }
        if((c == 6)&&(a == 3)&&(b == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);           
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }           
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_NineLoop()
{
    bool ret = true; QString str1; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    QString str = tr("请准备输出口1");  //三相回路电流、功率
    emit StepSig(str);

    while(1){
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[0];
            if(a) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str2 = tr("请检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str2);
    emit StepSig(str2); emit CurImageSig(1); sleep(4);

    if(mPro->stopFlag == 0) {           //插接位1电流控制1
        if(ret) ret = NineInsertOne_CtrlOne();
        if(ret) ret = NineInsertOne_CtrlTwo();
        if(ret) ret = NineInsertOne_CtrlThree();
        if(ret) ret = NineInsertOne_BreakerOne();
        if(ret) ret = NineInsertTwo_CtrlOne();
        if(ret) ret = NineInsertTwo_CtrlTwo();
        if(ret) ret = NineInsertTwo_CtrlThree();
        if(ret) ret = NineInsertOne_BreakerTwo();
        if(ret) ret = NineInsertThree_CtrlOne();
        if(ret) ret = NineInsertThree_CtrlTwo();
        if(ret) ret = NineInsertThree_CtrlThree();
        if(ret) ret = NineInsertOne_BreakerThree();
    }else {
        ret = NineInsertOne_CtrlOne();
        ret = NineInsertOne_CtrlTwo();
        ret = NineInsertOne_CtrlThree();
        ret = NineInsertOne_BreakerOne();
        ret = NineInsertTwo_CtrlOne();
        ret = NineInsertTwo_CtrlTwo();
        ret = NineInsertTwo_CtrlThree();
        ret = NineInsertOne_BreakerTwo();
        ret = NineInsertThree_CtrlOne();
        ret = NineInsertThree_CtrlTwo();
        ret = NineInsertThree_CtrlThree();
        ret = NineInsertOne_BreakerThree();
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.value[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("回路%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.value[1]; c = Obj->cur.status[2];
       if((!b)&&(c)&&(a)){
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口1-B1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }

    }

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.value[2];
        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("回路%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-C1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-C1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_BreakerOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器1，A1/B1/C1电压为0V, A2/B2/C2电压正常；");
    QString str = tr("关闭插接箱的断路器1，打开插接箱的断路器2");
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 1, A1/B1/C1 voltage is 0V, A2/B2/C2 voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0; int b=0;
        ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.value[i];
            b += Obj->vol.status[3+i];
        }
        if((!a)&&(b == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口2");  //三相回路电流、功率
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1){
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[3];
            if(a) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口2 无电流功率");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5);  emit CurImageSig(1);sleep(4);
    flag = 0;
    str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.value[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.value[4]; c = Obj->cur.status[5];
        if((!b)&&(c)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.value[5];
        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口2-C2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_BreakerTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器2，A2/B2/C2电压为0V, A1/B1/C1电压正常；");
    QString str = tr("关闭插接箱的断路器2，打开插接箱的断路器1");
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 2, A2/B2/C2 voltage is 0V, A1/B1/C1 voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0, b=0;
        ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.status[i];
            b += Obj->vol.value[3+i];
        }
        if((!b)&&(a == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_SixLoop()
{
    bool ret = true; QString str1; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    QString str = tr("请准备输出口1");  //三相回路电流、功率
    emit StepSig(str);

    while(1){
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[0];
            if(a) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str3 = tr("请检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(3);
    if(mPro->stopFlag == 0) {           //插接位1电流控制1
        if(ret) ret = SixInsertOne_CtrlOne();
        if(ret) ret = SixInsertOne_CtrlTwo();
        if(ret) ret = SixInsertOne_CtrlThree();
        if(ret) ret = SixInsertOne_BreakerOne();
        if(ret) ret = SixInsertTwo_CtrlOne();
        if(ret) ret = SixInsertTwo_CtrlTwo();
        if(ret) ret = SixInsertTwo_CtrlThree();
        if(ret) ret = SixInsertOne_BreakerTwo();
    }else {
        ret = SixInsertOne_CtrlOne();
        ret = SixInsertOne_CtrlTwo();
        ret = SixInsertOne_CtrlThree();
        ret = SixInsertOne_BreakerOne();
        ret = SixInsertTwo_CtrlOne();
        ret = SixInsertTwo_CtrlTwo();
        ret = SixInsertTwo_CtrlThree();
        ret = SixInsertOne_BreakerTwo();
    }

    return ret;
}

bool Power_DevRead::Three_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.value[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-A检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str = tr("输出口-A检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L2，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.value[1]; c = Obj->cur.status[2];
        if((!b) &&a &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-B检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-B检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_CtrlThree()
{
    bool ret = true; int flag = 0;QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开负载输入端断路器L3，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.value[2];
        if((!c) &&b &&a) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-C检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-C检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Breaker()
{
    bool ret = true; int flag = 0; QString str1,str;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器，A/B/C电压为0V；");
    if(mItem->modeId == START_BUSBAR)
        str = tr("关闭始端箱的断路器");
    else {str = tr("关闭插接箱的断路器");}
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect the circuit breaker, with A/B/C voltage of 0V;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0,b=0,c=0;
        ret = readData();

        a = Obj->vol.value[0]; b = Obj->vol.value[1]; c = Obj->vol.value[2];
        if((!a)&&(!b) &&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            if(mItem->modeId == START_BUSBAR) str = tr("始端箱的断路器检测成功 ");
            else str = tr("插接箱的断路器检测成功 ");
            mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            if(mItem->modeId == START_BUSBAR) str = tr("始端箱的断路器检测失败 ");
            else str = tr("插接箱的断路器检测失败 ");
            mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_One()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，读取读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口1");  //三相回路电流、功率
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[0];
            if(a){
                    ret = true;
                    break;
                }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
                break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L1，并检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.value[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&(!b) &&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_OneBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器A，A电压为0V，B/C电压正常；");
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    str = tr("关闭插接箱的断路器1");  //三相回路电流、功率
    emit StepSig(str);
    QString eng2 = tr("Disconnect circuit breaker A, A voltage is 0V, B/C voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.value[0]; b = Obj->vol.status[1]; c = Obj->vol.status[2];

        if((!a) &&(b == 1)&&(c == 1)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器1检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Two()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("请准备输出口2,打开负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, check and read that the current of B is 0A, and the current of A/C is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[1];            
            if(a) {
               ret = true; break;}
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
                break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L2，检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L2");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.value[1]; c = Obj->cur.status[2];
        if((!a) &&(!b) &&(!c)) {

                ret = true;
                for(int i =0;i<loop;i++)
                {
                    QString temp = trans(i);
                    str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    str1 += str;
                }
                mLogs->updatePro(str1, ret);
                str = tr("输出口2-B检测成功 ");mLogs->updatePro(str, ret);
                mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                str1.clear(); break;

        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_TwoBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器B,B电压为0V，A/C电压正常；");
    str = tr("打开插接箱断路器1，关闭插接箱断路器2");  //三相回路电流、功率
    emit StepSig(str);
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker B, B voltage is 0V, A/C voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");

    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.value[1]; c = Obj->vol.status[2];

        if((a == 1)&&(!b) &&(c == 1)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器2检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Three()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座3接入负载，断开负载输入端断路器L1，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("请准备输出口3，打开负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3 = tr("符合要求"); QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Connect the output socket 3 to the load, disconnect the load input circuit breaker L1, and check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Socket 3 current check");

    while(1)
    {
        int a=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[2];
            if(a) {
               ret = true; break;}
            }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口3 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L2，检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.value[2];
        if((!a) &&(!b) &&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_ThreeBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器C，C电压为0V，A/B电压正常；");
    QString str3 = tr("符合要求"); QString str4 = tr("断路器检查");
    str = tr("打开插接箱断路器2，关闭插接箱断路器3");  //三相回路电流、功率
    emit StepSig(str);
    QString eng2 = tr("Disconnect circuit breaker C, C voltage is 0V, A/B voltage is normal;");
    QString eng3 = tr("Meet a requirement");
    QString eng4 = tr("Circuit breaker inspection");
    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.status[1]; c = Obj->vol.value[2];

        if((a == 2) &&(b == 1)&&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str3, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            str3 = tr("不符合要求"); mLogs->writeData(str2, str3, str4, ret);
            eng3 = tr("Not Satisfiable"); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_ThreeLoop()
{
    bool ret = true; QString str; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    sBoxData *StartBox = &(mBusData->box[mItem->addr - 1]);
    QString str3 = tr("符合要求"); QString str4 = tr("始端箱功率检查");
    if((mItem->modeId == START_BUSBAR) || (mBusData->box[mItem->addr-1].phaseFlag == 1)) {
        str = tr("请准备输出口");  //三相回路电流、功率
        emit StepSig(str);

        while(1){
            int a=0;
            ret = readData();
            a = Obj->cur.value[0];
            if(a) {
                ret = true; break;
            }
            flag++;
            if(flag >60) {
                ret = false;
                str = tr("该输出口 无电流");mLogs->updatePro(str, ret);
                 break;
            }
        }
        QString str5 = tr("请检测输出口2位置的极性测试是否合格?");
        // emit PloarSig(str5);
        emit StepSig(str5); emit CurImageSig(1); sleep(4);

        if(mItem->modeId == START_BUSBAR) { //判断始端箱三相功率值之和是否等于总功率误差不超过10kw
            uint toTal = StartBox->totalPow.ivalue;
            uint sum = 0; bool res = false;

            QString str1 = tr("总功率与三相功率之和在误差范围内");
            int toTalerr = toTal * (mItem->err.powErr/1000.0);

            for(int i =0;i<START_LINE_NUM;i++)
                sum += StartBox->data.pow.value[i];

            res = mErr->checkErrRange(toTal, sum, toTalerr);
            str = tr("总功率为 %1kw，三相功率之和为 %2kw").arg(toTal /COM_RATE_POW).arg(sum/COM_RATE_POW);
            if(!res) {
                str= tr("误差较大："); str3 = tr("不符合要求");
            }
            mLogs->updatePro(str, res); mLogs->writeData(str1,str3,str4,res);
        }
        if(mPro->stopFlag == 0) {           //插接位1电流控制1
            if(ret) ret = Three_CtrlOne();
            if(ret) ret = Three_CtrlTwo();
            if(ret) ret = Three_CtrlThree();
            if(ret) ret = Three_Breaker();
        }else {
            ret = Three_CtrlOne();
            ret = Three_CtrlTwo();
            ret = Three_CtrlThree();
            ret = Three_Breaker();
        }
    }else if(mBusData->box[mItem->addr-1].phaseFlag == 0) {    //单相三回路三个输出位
        if(mPro->stopFlag == 0) {           //插接位1电流控制1
            if(ret) ret = Three_One();
            if(ret) ret = Three_OneBreaker();
            if(ret) ret = Three_Two();
            if(ret) ret = Three_TwoBreaker();
            if(ret) ret = Three_Three();
            if(ret) ret = Three_ThreeBreaker();
        }else {
            ret = Three_One();
            ret = Three_OneBreaker();
            ret = Three_Two();
            ret = Three_TwoBreaker();
            ret = Three_Three();
            ret = Three_ThreeBreaker();
        }
    }

    return ret;
}

bool Power_DevRead::readHub()
{
    bool ret = true;
    mRtu->setModbus(0);
    for(int i=0; i<3; ++i) {
        ret = mRtu->readPduData();
        if(ret) break; else mRtu->changeBaudRate();
    }
    mRtu->setModbus(2);

    return ret;
}

void Power_DevRead::run()
{
    if(isRun) return;
    isRun = true;

    mPacket->init();
    mPro->step = Collect_Start;

    bool ret  = readSn();
    if(ret) {
        QString str = tr("设备数据读取");
        ret = mRtu->readPduData();
        if(ret) str += tr("成功");
        else str += tr("失败");
        mPacket->updatePro(str, ret);
    }
    mPacket->updatePro(tr("已结束"), ret);
    mPro->step = Test_End;

    isRun = false;
}
