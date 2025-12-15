/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_devread.h"
#include "dev_source.h"
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
    mSource = Dev_Source::bulid(this);
    mItem = Cfg::bulid()->item;
    mErr = Power_ErrRange::bulid(this);
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
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        exValue = cth->ip_cur *1000.0;
        err = cth->ip_curErr *1000.0;
    }else {
        sSiCfg *cth = &(mItem->si);
        exValue = cth->si_cur *1000.0;
        err = cth->si_curErr *1000.0;
    }

    bool ret = true;
    if( mItem->modeId == START_BUSBAR ){
        ret = mSiRtu->readPduData();
        if(ret) {
            ret = checkNet();
            if(ret) ret = mIpSnmp->readPduData();
            }
    }else{
        ret = mSiRtu->readPduData();
    }

    return ret;
}

bool Power_DevRead::readDevBus()
{
    bool ret = mPacket->delay(3);
    if(ret) {
        if( mItem->modeId == 2 ){

            ret = checkNet();
            if(ret) {
                QString str = mIpSnmp->readBreakValue();
                if(str == "2")
                {
                     str = tr("光栅告警，附近请勿站人！"); mLogs->updatePro(str, false);
                    ret = false;
                }else if(str == "1")
                {
                    // str = tr("防雷状态正常"); mLogs->updatePro(str, true);
                    ret = true;
                }
            }else{
                QString str = tr("始端箱通讯失败"); mLogs->updatePro(str, false);
                ret = false;
            }
        }
    }

    return ret;
}


bool Power_DevRead::readDevBasicType()
{
    bool ret = true;
    if(ret) ret = mSource->read();
    return ret;
}


bool Power_DevRead::readDev()
{
    bool ret = mPacket->delay(5);
    if(ret) {
        if( mItem->modeId == START_BUSBAR ){
            QString str = tr("始端箱串口RTU通讯");
            for(int i=0; i<5; ++i) {
                ret = mSiRtu->readPduData();
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

QString Power_DevRead::getBreakCtrlOid()
{
    return mIpSnmp->getBreakOid();
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

bool Power_DevRead::readDevData()
{
    bool ret = true;

    switch (mItem->modeId) {
    case START_BUSBAR:  mRtu = mIpSnmp; break;
    case INSERT_BUSBAR:  mRtu = mSiRtu; break;
    case TEMPERATURE_BUSBAR:  mRtu = mSiRtu; break;
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
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相九路电流、功率
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }            
            mLogs->updatePro(str1, ret); str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-B1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

           mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3 and check that the reading of C current is 0A and A/B current is normal");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口1-C1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-C1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开负载输入端L3");
    emit StepSig(str); sleep(3);

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
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 1, A1/B1/C1 voltage is 0V, A2/B2/C2/A3/B3/C3 voltage is normal;");
    QString eng3;
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
            a += Obj->vol.value[i];
            b += Obj->vol.status[3+i];
            c += Obj->vol.status[6+i];  
        }

        if((a ==0)&&(b == 3)&&(c == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }            
            mLogs->updatePro(str1, ret); str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >35) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口2，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 2 current check");

    while(1){
        int a=0, b=0, c = 0;
        if(ret) {
            ret = readData();
            for(int i =3;i<6;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
            if(a &&b &&c) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >80) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str5); emit CurImageSig(1); sleep(5);

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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }           
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-C2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

             mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开负载输入端L3");
    emit StepSig(str); sleep(3);

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
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 2, A2/B2/C2 voltage is 0V, A1/B1/C1/A3/B3/C3 voltage is normal;");
    QString eng3;
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
            b += Obj->vol.value[3+i];
            c += Obj->vol.status[6+i];
        }
        if((b == 0)&&(a == 3)&&(c == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }            
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >35) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str2 = tr("输出插座3接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口3，打开负载输入端L1、L2、L3");
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Connect output socket 3 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 3 current check");

    while(1){
        int a=0, b=0, c = 0;
        if(ret) {
            ret = readData();
            for(int i =6;i<9;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[6]; b = Obj->cur.status[7]; c = Obj->cur.status[8];
            if(a &&b &&c) {
               ret = true; break;
            }
        }
        flag++;
        if(flag >80) {
            ret = false;
            str = tr("输出口3 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(5);
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-A3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }           
            mLogs->updatePro(str, ret); ret = false;
            str = tr("输出口3-A3检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-B3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-B3检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
       if(flag >70) {
           str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
           emit StepSig(str);
       }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C3检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    // str = tr("打开负载输入端L3");
    // emit StepSig(str); sleep(3);

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
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 3, A3/B3/C3 voltage is 0V, A1/B1/C1/A2/B2/C2 voltage is normal;");
    QString eng3;
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
            c += Obj->vol.value[6+i];
        }
        if((c == 0)&&(a == 3)&&(b == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);           
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >35) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }           
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器3");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Load_NineLoop()
{
    bool ret = true; QString str1; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    QString str = tr("请准备输出口1，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1){
        int a=0, b=0, c=0;
        if(ret) {
            ret = readData();
            for(int i =0;i<3;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
            if(a &&b &&c) {

                ret = true; break;
            }
        }
        flag++;
        if(flag >80) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str2 = tr("请检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str2);
    emit StepSig(str2); emit CurImageSig(1); sleep(5);


    if(ret) ret = NineInsertOne_CtrlOne();
    if(ret) ret = NineInsertOne_CtrlTwo();
    if(ret) ret = NineInsertOne_CtrlThree();

    if(ret) ret = NineInsertTwo_CtrlOne();
    if(ret) ret = NineInsertTwo_CtrlTwo();
    if(ret) ret = NineInsertTwo_CtrlThree();

    if(ret) ret = NineInsertThree_CtrlOne();
    if(ret) ret = NineInsertThree_CtrlTwo();
    if(ret) ret = NineInsertThree_CtrlThree();

    emit CurImageSig(4);

    return ret;
}

bool Power_DevRead::Break_NineLoop()
{
    bool ret = true;
    ret = NineInsertOne_BreakerOne();
    if(ret) ret = NineInsertOne_BreakerTwo();
    if(ret) ret = NineInsertOne_BreakerThree();
    // emit CurImageSig(4);

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;

            str = tr("输出口1-A1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
       if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
       }
        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口1-B1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-C1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-C1检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开负载输入端L3");
    emit StepSig(str); sleep(3);

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
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 1, A1/B1/C1 voltage is 0V, A2/B2/C2 voltage is normal;");
    QString eng3;
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
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str); sleep(3);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口2，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 2 current check");

    while(1){
        int a=0, b=0, c=0;
        if(ret) {
            ret = readData();
            for(int i =3;i<6;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
            if(a &&b &&c) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >90) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5);  emit CurImageSig(1);sleep(5);
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >70) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >90) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口2-C2检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    // str = tr("打开负载输入端L3");
    // emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::SixInsertOne_BreakerTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器2，A2/B2/C2电压为0V, A1/B1/C1电压正常；");
    QString str = tr("关闭插接箱的断路器2，打开插接箱的断路器1");
    emit StepSig(str);
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker 2, A2/B2/C2 voltage is 0V, A1/B1/C1 voltage is normal;");
    QString eng3;
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
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器2");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Load_SixLoop()
{
    bool ret = true; QString str1; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    QString str = tr("请准备输出口1，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1){
        int a=0, b=0, c=0;
        if(ret) {
            ret = readData();
            for(int i =0;i<3;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
            if(a &&b &&c) {
                ret = true; break;
            }
        }
        flag++;
        if(flag >80) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str3 = tr("请检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(5);

    if(ret) ret = SixInsertOne_CtrlOne();
    if(ret) ret = SixInsertOne_CtrlTwo();
    if(ret) ret = SixInsertOne_CtrlThree();

    if(ret) ret = SixInsertTwo_CtrlOne();
    if(ret) ret = SixInsertTwo_CtrlTwo();
    if(ret) ret = SixInsertTwo_CtrlThree();

    emit CurImageSig(4);

    return ret;
}

bool Power_DevRead::Break_SixLoop()
{
    bool ret = true;
    ret = SixInsertOne_BreakerOne();
    if(ret) ret = SixInsertOne_BreakerTwo();
    emit CurImageSig(4);

    return ret;
}

bool Power_DevRead::Three_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.value[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) && b && c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-A检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >60) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >70) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;

            if(mItem->modeId == START_BUSBAR)
            {
                str = tr("输出口-A检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->ip.ip_cur).arg(mItem->ip.ip_curErr);
            }else {
                str = tr("输出口-A检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            }
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L2 and check that the reading of B current is 0A and A/C current is normal;");
    QString eng3;
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.value[1]; c = Obj->cur.status[2];
        if((!b) && a && c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-B检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >60) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >70) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;

            if(mItem->modeId == START_BUSBAR)
                str = tr("输出口-B检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->ip.ip_cur).arg(mItem->ip.ip_curErr);
            else str = tr("输出口-B检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
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
    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Disconnect the load input circuit breaker L3, check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.value[2];
        if((!c) && b && a) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-C检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >60) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >70) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            if(mItem->modeId == START_BUSBAR)
                str = tr("输出口-C检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->ip.ip_cur).arg(mItem->ip.ip_curErr);
            else str = tr("输出口-C检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    // str = tr("打开负载输入端L3");  //三相回路电流、功率
    // emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_Breaker()
{
    bool ret = true; int flag = 0; QString str1,str;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器，A/B/C电压为0V；");
    str = tr("关闭插接箱的断路器");
    emit StepSig(str);
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect the circuit breaker, with A/B/C voltage of 0V;");
    QString eng3;
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
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器检测成功 ");
            mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器检测失败 ");
            mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_One()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座1接入负载，断开负载输入端断路器L1，检查读取A电流为0A，B/C电流为正常；");
    QString str = tr("请准备输出口1，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座1电流检查");
    QString eng2 = tr("Connect output socket 1 to the load, disconnect the load input circuit breaker L1, and read that the current of A is 0A, while the current of B/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 1 current check");

    while(1)
    {
        if(ret) {
            ret = readData();
            Obj->cur.status[0] = mErr->checkErrRange(exValue, Obj->cur.value[0], err);
            if(Obj->cur.status[0]){
                    ret = true;
                    break;
                }
        }
        flag++;
        if(flag >90) {
            ret = false;
            str = tr("输出口1 无电流");mLogs->updatePro(str, ret);
                break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L1，并检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(5);

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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >50) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开负载输入端L1");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_OneBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器A，A电压为0V，B/C电压正常；");
    QString str3; QString str4 = tr("断路器检查");
    str = tr("关闭插接箱的断路器1");  //三相回路电流、功率
    emit StepSig(str);
    QString eng2 = tr("Disconnect circuit breaker A, A voltage is 0V, B/C voltage is normal;");
    QString eng3;
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
        if(((!a) &&(b == 1)&&(c == 1) && loop == 3) || ((!a) &&(b == 1)&& loop == 2)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器1检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器1检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器1");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_Two()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座2接入负载，断开负载输入端断路器L1，检查读取B电流为0A，A/C电流为正常；");
    QString str = tr("请准备输出口2，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座2电流检查");
    QString eng2 = tr("Connect output socket 2 to the load, disconnect the load input circuit breaker L1, check and read that the current of B is 0A, and the current of A/C is normal;");
    QString eng3;
    QString eng4 = tr("Socket 2 current check");

    while(1)
    {
        if(ret) {
            ret = readData();
            Obj->cur.status[1] = mErr->checkErrRange(exValue, Obj->cur.value[1], err);
            if(Obj->cur.status[1]) {
               ret = true; break;}
        }
        flag++;
        if(flag >90) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
                break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L1，检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(5);

    flag = 0;
    str = tr("关闭负载输入端L1");  //单相回路电流、功率
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
                    str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                    str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                    str1 += str; eng3 += str3;
                }
                mLogs->updatePro(str1, ret);
                str = tr("输出口2-B检测成功 ");mLogs->updatePro(str, ret);
                mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                str1.clear(); break;

        }
        if(flag >50) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret);  mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开负载输入端L1");
    emit StepSig(str); sleep(3);

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
    QString str3; QString str4 = tr("断路器检查");
    QString eng2 = tr("Disconnect circuit breaker B, B voltage is 0V, A/C voltage is normal;");
    QString eng3;
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
        if(((a == 1)&&(!b) &&(c == 1)&& loop == 3) || ((a==1) &&(!b)&& loop == 2)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器2检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器2检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器2");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_Three()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("输出插座3接入负载，断开负载输入端断路器L1，检查读取C电流为0A，A/B电流为正常；");
    QString str = tr("请准备输出口3，打开负载输入端L1、L2、L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    QString str3; QString str4 = tr("插座3电流检查");
    QString eng2 = tr("Connect the output socket 3 to the load, disconnect the load input circuit breaker L1, and check that the reading of C current is 0A, and the A/B current is normal;");
    QString eng3;
    QString eng4 = tr("Socket 3 current check");

    while(1)
    {
        if(ret) {
            ret = readData();
            Obj->cur.status[2] = mErr->checkErrRange(exValue, Obj->cur.value[2], err);
            if(Obj->cur.status[2]) {
               ret = true; break;}
            }
        flag++;
        if(flag >90) {
            ret = false;
            str = tr("输出口3 无电流");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str5 = tr("相位转换旋钮转到L1，检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str5);
    emit StepSig(str5); emit CurImageSig(1); sleep(5);

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
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;

        }
        if(flag >50) {
            str = tr("电流超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str3 = tr("%1 current %2A，").arg(temp).arg(QString::number((Obj->cur.value[i]/COM_RATE_CUR),'f',3));
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C检测失败，超出误差范围，设置的电流 %1A，误差 %2A").arg(mItem->si.si_cur).arg(mItem->si.si_curErr);
            mLogs->updatePro(str, ret);

            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    // str = tr("打开负载输入端L1");
    // emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Three_ThreeBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("断开断路器C，C电压为0V，A/B电压正常；");
    QString str3; QString str4 = tr("断路器检查");
    str = tr("打开插接箱断路器2，关闭插接箱断路器3");  //三相回路电流、功率
    emit StepSig(str);
    QString eng2 = tr("Disconnect circuit breaker C, C voltage is 0V, A/B voltage is normal;");
    QString eng3;
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

        if((a == 1) &&(b == 1)&&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
        if(flag >30) {
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电压和误差是否设置合适");
            emit StepSig(str);
        }

        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1 voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            mLogs->writeData(str2, str1, str4, ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
            str1.clear(); break;
        }
    }
    str = tr("打开插接箱的断路器3");
    emit StepSig(str); sleep(3);

    return ret;
}

bool Power_DevRead::Load_ThreeLoop()
{
    bool ret = true; QString str; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    sBoxData *StartBox = &(mBusData->box[mItem->addr - 1]);
    QString str3 = tr("符合要求"); QString str4 = tr("始端箱功率检查");
    if((mItem->modeId == START_BUSBAR) || (mBusData->box[mItem->addr-1].phaseFlag == 1)) {
        str = tr("请准备输出口，打开负载输入端L1、L2、L3");  //三相回路电流、功率
        emit StepSig(str); emit CurImageSig(2);

        while(1){
            int a=0, b=0, c=0;
            ret = readData();
            for(int i =0;i<3;i++) {
                Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
            }
            a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
            if(a && b && c) {
                ret = true; break;
            }
            flag++;
            if(flag >80) {
                ret = false;
                str = tr("该输出口 无电流");mLogs->updatePro(str, ret);
                 break;
            }
        }
        QString str5 = tr("请检测该输出口位置的极性测试是否合格?");
        // emit PloarSig(str5);
        emit StepSig(str5); emit CurImageSig(1); sleep(5);

        if(mItem->modeId == START_BUSBAR) { //判断始端箱三相功率值之和是否等于总功率误差不超过10kw
            flag = 0; bool res = false;uint toTal = 0; uint sum = 0;
            while(1)
            {
                ret = readData();
                toTal = StartBox->totalPow.ivalue; sum = 0;
                int toTalerr = toTal * (mItem->ip.ip_powErr/100.0);

                for(int i =0;i<START_LINE_NUM;i++)
                    sum += StartBox->data.pow.value[i];

                res = mErr->checkErrRange(toTal, sum, toTalerr);
                if(res) break;

                flag++;
                if(flag >10) {
                    res = false;
                    break;
                }
            }

            QString str1 = tr("总功率与三相功率之和在误差%1范围内");
            str = tr("总功率为 %1kw，三相功率之和为 %2kw").arg(toTal /COM_RATE_POW).arg(sum /COM_RATE_POW);
            if(!res) {
                str += tr("误差较大"); str3 = tr("不符合要求");
            }
            mLogs->updatePro(str, res); //mLogs->writeData(str1,str3,str4,res);
        }

        if(ret) ret = Three_CtrlOne();
        if(ret) ret = Three_CtrlTwo();
        if(ret) ret = Three_CtrlThree();

    }else if((mItem->modeId != START_BUSBAR) && (mBusData->box[mItem->addr-1].phaseFlag == 0)) {    //单相三回路三个输出位

        if(mItem->si.si_testItem == 0 || mItem->si.si_testItem == 1){
            if(ret) ret = Three_One();
        }
        if(mItem->si.si_testItem == 0 || mItem->si.si_testItem == 2){
            if(ret) ret = Three_Two();
        }
        if( (mItem->si.si_testItem == 0 || mItem->si.si_testItem == 3 ) && mBusData->box[mItem->addr-1].loopNum == 3){
            if(ret) ret = Three_Three();
        }

    }
    emit CurImageSig(4);

    return ret;
}
bool Power_DevRead::Three_Break()
{
    bool ret = true;

    ret = Three_OneBreaker();
    if(ret) ret = Three_TwoBreaker();
    if(ret && mBusData->box[mItem->addr-1].loopNum > 2) ret = Three_ThreeBreaker();
    // emit CurImageSig(4);

    return ret;
}
