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
    if(mItem->modeId == START_BUSBAR) {
        sIpCfg *cth = &(mItem->ip);
        exValue = cth->ip_cur;
        err = exValue*(cth->ip_curErr/1000.0);
    }else {
        sSiCfg *cth = &(mItem->si);
        exValue = cth->si_cur;
        err = exValue*(cth->si_curErr/1000.0);
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
            //     ret = mSiRtu->readPduData();
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

            for(int i=0; i<5; ++i) {
                if(mItem->ip.ip_mode == 1){//RTU模式地址为1
                    ret = mSiRtu->readPduData();
                }else ret = mSiRtu->readRtuData();
                if(ret) break; else if(!mPacket->delay(3)) break;
            }


            if(ret) {
                ret = checkNet();
                if(ret) ret = mIpSnmp->readPduData();                               
            }
        }else{
            for(int i=0; i<5; ++i) {
                ret = mSiRtu->readPduData();
                if(ret) break; else if(!mPacket->delay(3)) break;
            }           
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
    case 2: str = "B"+QString::number(index / 3+1);
    case 3: str = "C"+QString::number(index / 3+1);
    }
    return str;
}

bool Power_DevRead::NineInsertOne_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1电流值、功率值为0，B1、C1电流值及功率值不为0");
    QString str = tr("关闭负载输入端L1");  //三相九路电流、功率
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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i] /COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B2电流值、功率值为0，A1、C1电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A，功率%3kw ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret); str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;           
            str = tr("输出口1-B1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C3电流值、功率值为0，A1、B1电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口1-C1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-C1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1、B1、C1电压值为0，A2、B2、C2、A3、B3、C3电压值不为0");
    QString str = tr("关闭插接箱的断路器1，打开插接箱的断路器2、3");
    emit StepSig(str);

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
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret); str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A2电流值、功率值等于0，B2、C2电流值及功率值不为0");
    QString str = tr("请准备输出口2");  //三相回路电流、功率
    emit StepSig(str);

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
    QString str3 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(4);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B2电流值、功率值等于0，A2、C2电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }           
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertTwo_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C2电流值、功率值等于0，A2、B2电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-C2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A2、B2、C2电压值为0，A1、B1、C1、A3、B3、C3电压值为0");
    QString str = tr("打开插接箱的断路器1、3，关闭插接箱的断路器2");
    emit StepSig(str);

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
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }            
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A3电流值、功率值为0，B2、C3电流值及功率值不为0");
    QString str = tr("请准备输出口3");
    emit StepSig(str);

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
    QString str3 = tr("请检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(4);
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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-A3检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }           
            mLogs->updatePro(str, ret); ret = false;
            str = tr("输出口3-A3检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B3电流值、功率值为0，A3、C3电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-B3检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-B3检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertThree_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C3电流值、功率值为0，A3、B3电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);

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
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C3检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C3检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::NineInsertOne_BreakerThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1、B1、C1、A2、B2、C2电压值为0，A3、B3、C3电压值不为0");
    QString str = tr("打开插接箱的断路器1、2，关闭插接箱的断路器3");
    emit StepSig(str);

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
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);           
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }           
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
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
    QString str2 = tr("A1电流值为0，B1、C1电流值在误差范围内");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("回路%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B1电流值、功率值为0，A1、C1电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
       if((!b)&&(c)&&(a)){
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-B1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口1-B1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }

    }

    return ret;
}

bool Power_DevRead::SixInsertOne_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C1电流值、功率值为0，A1、B1电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("回路%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-C1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-C1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_BreakerOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1、B1、C1电压值为0，A2、B2、C2电压值不为0");
    QString str = tr("关闭插接箱的断路器1，打开插接箱的断路器2");
    emit StepSig(str);

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
            b += Obj->vol.status[3+i];
        }
        if((a == 6)&&(b == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlOne()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A2电流值、功率值为0，B2、C2电流值及功率值不为0");
    QString str = tr("请准备输出口2");  //三相回路电流、功率
    emit StepSig(str);

    while(1){
        int a=0; int d=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[3]; d = Obj->pow.value[3];
            if(a) {
                if(d) ret = true; break;
            }
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口2 无电流功率");mLogs->updatePro(str, ret);
            break;
        }
    }
    QString str3 = tr("请检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3);  emit CurImageSig(1);sleep(4);
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
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-A2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-A2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B2电流值、功率值为0，A2、C2电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
        if((!b)&&(c)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-B2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertTwo_CtrlThree()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C2电流值、功率值为0，A2、B2电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1;
        for(int i =3;i<6;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[3]; b = Obj->cur.status[4]; c = Obj->cur.status[5];
        if((!c)&&(b)&&(a)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口2-C2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);  ret = false;
            str = tr("输出口2-C2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::SixInsertOne_BreakerTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A2、B2、C2电压值为0，A1、B1、C1电压值不为0");
    QString str = tr("关闭插接箱的断路器2，打开插接箱的断路器1");
    emit StepSig(str); emit CurImageSig(2);

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
            b += Obj->vol.status[3+i];
        }
        if((b == 6)&&(a == 3)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
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
        int a=0; int d=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[0];
            d = Obj->pow.value[0];
            if(a) {
                if(d) ret = true; break;
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
    QString str2 = tr("A电流值、功率值为0，B、C电流值及功率值不为0");
    QString str = tr("关闭负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&b &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-A检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str = tr("输出口-A检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_CtrlTwo()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B电流值、功率值为0，A、C电流值及功率值不为0");
    QString str = tr("打开负载输入端L1，关闭负载输入端L2");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!b) &&a &&c) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-B检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-B检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_CtrlThree()
{
    bool ret = true; int flag = 0;QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C1电流值、功率值为0，A1、B1电流值及功率值不为0");
    QString str = tr("打开负载输入端L2，关闭负载输入端L3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 0;
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!c) &&b &&a) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口-C检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口-C检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Breaker()
{
    bool ret = true; int flag = 0; QString str1,str;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1、B1、C1电压值为0");
    if(mItem->modeId == START_BUSBAR)
        str = tr("关闭始端箱的断路器");
    else {str = tr("关闭插接箱的断路器");}
    emit StepSig(str);

    while(1)
    {
        int a=0,b=0,c=0;
        ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.status[1]; c = Obj->vol.status[2];

        if((a == 2)&&(b == 2) &&(c == 2)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            if(mItem->modeId == START_BUSBAR) str = tr("始端箱的断路器检测成功 ");
            else str = tr("插接箱的断路器检测成功 ");
            mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            if(mItem->modeId == START_BUSBAR) str = tr("始端箱的断路器检测失败 ");
            else str = tr("插接箱的断路器检测失败 ");
            mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_One()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("回路A电流值、功率值为0");
    QString str = tr("请准备输出口1");  //三相回路电流、功率
    emit StepSig(str);

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
    QString str3 = tr("相位转换旋钮转到L1，并检测输出口1位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1，关闭插接箱的断路器1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&(!b) &&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口1-A检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口1-A检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_OneBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("A1电压值为0");

    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.status[1]; c = Obj->vol.status[2];

        if((a == 2) &&(b == 1)&&(c == 1)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器1检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器1检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Two()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B1电流值、功率值为0");
    QString str = tr("请准备输出口2,打开负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

    while(1)
    {
        int a=0; int d=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[1];
            d = Obj->pow.value[1];
            if(a) {
                if(d) ret = true; break;}
        }
        flag++;
        if(flag >60) {
            ret = false;
            str = tr("输出口2 无电流");mLogs->updatePro(str, ret);
                break;
        }
    }
    QString str3 = tr("相位转换旋钮转到L2，检测输出口2位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1，打开插接箱断路器1，关闭插接箱断路器2");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    while(1)
    {
        int a=0, b=0, c = 0;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&(!b) &&(!c)) {

                ret = true;
                for(int i =0;i<loop;i++)
                {
                    QString temp = trans(i);
                    str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    str1 += str;
                }
                mLogs->updatePro(str1, ret);
                str = tr("输出口2-B检测成功 ");mLogs->updatePro(str, ret);
                str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口2-B检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_TwoBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("B1电压值为0");

    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.status[1]; c = Obj->vol.status[2];

        if((a == 1)&&(b == 2) &&(c == 1)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱的断路器2检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱的断路器2检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_Three()
{
    bool ret = true; int flag = 0; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C电流值、功率值为0");
    QString str = tr("请准备输出口3，打开负载输入端L1");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);

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
    QString str3 = tr("相位转换旋钮转到L2，检测输出口3位置的极性测试是否合格?");
    // emit PloarSig(str3);
    emit StepSig(str3); emit CurImageSig(1); sleep(4);

    flag = 0;
    str = tr("关闭负载输入端L1，打开插接箱断路器2，关闭插接箱断路器3");  //三相回路电流、功率
    emit StepSig(str); emit CurImageSig(2);
    while(1)
    {
        int a=0, b=0, c = 1;
        ret = readData();
        for(int i =0;i<3;i++) {
            Obj->cur.status[i] = mErr->checkErrRange(exValue, Obj->cur.value[i], err);
        }
        a = Obj->cur.status[0]; b = Obj->cur.status[1]; c = Obj->cur.status[2];
        if((!a) &&(!b) &&(!c)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("输出口3-C检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;

        }
        flag++;
        if(flag >80) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电流%2A ").arg(temp).arg(Obj->cur.value[i]/COM_RATE_CUR);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("输出口3-C检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Three_ThreeBreaker()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str2 = tr("C1电压值为0");

    while(1)
    {
        int a=0,b=0,c=0;
        if(ret) ret = readData();
        for(int i =0;i<loop;i++)
        {
            mErr->volErr(i);
        }
        a = Obj->vol.status[0]; b = Obj->vol.status[1]; c = Obj->vol.status[2];

        if((a == 2) &&(b == 1)&&(c == 2)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            str = tr("插接箱断路器3检测成功 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V ").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret); ret = false;
            str = tr("插接箱断路器3检测失败 ");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_ThreeLoop()
{
    bool ret = true; QString str; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    sBoxData *StartBox = &(mBusData->box[mItem->addr - 1]);
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
        QString str3 = tr("请检测输出口2位置的极性测试是否合格?");
        // emit PloarSig(str3);
        emit StepSig(str3); emit CurImageSig(1); sleep(4);

        if(mItem->modeId == START_BUSBAR) { //判断始端箱三相功率值之和是否等于总功率误差不超过10kw
            uint toTal = StartBox->totalPow.ivalue;
            uint sum = 0; bool res = false;

            QString str1 = tr("总功率与三相功率之和在误差范围内");
            int toTalerr = toTal * (mItem->err.powErr/1000.0);

            for(int i =0;i<START_LINE_NUM;i++)
                sum += StartBox->data.pow.value[i];

            res = mErr->checkErrRange(toTal, sum, toTalerr);
            str = tr("总功率为 %1kw，三相功率之和为 %2kw").arg(toTal /COM_RATE_POW).arg(sum/COM_RATE_POW);
            if(!res) str= tr("误差较大：");
            mLogs->updatePro(str, res); mLogs->writeData(str1,str,res);
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
