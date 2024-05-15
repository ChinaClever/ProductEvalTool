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
    // if(ret) ret = readDevData();

    return ret;
}

bool Power_DevRead::readData()
{
    bool ret = true;
    if( mItem->modeId == START_BUSBAR ){
            if(mItem->ip.ip_mode == 1){//RTU模式地址为1
                ret = mSiRtu->readPduData();
            }else ret = mSiRtu->readRtuData();        
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

            for(int i=0; i<6; ++i) {
                if(mItem->ip.ip_mode == 1){//RTU模式地址为1
                    ret = mSiRtu->readPduData();
                }else ret = mSiRtu->readRtuData();
                if(ret) break; else if(!mPacket->delay(1)) break;
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
            for(int i=0; i<6; ++i) {
                ret = mSiRtu->readPduData();
                if(ret) break; else if(!mPacket->delay(1)) break;
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
        //ret = checkNet();
        if(ret) ret = checkIpLine();
        //if(ret) ret = readSnmp();
        //if(ret) Ctrl_IpRtu::bulid(this)->start();
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

bool Power_DevRead::Load_NineLoop()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    str = tr("请准备插接位1");  //三相回路电流、功率
    emit StepSig(str);
    mLogs->updatePro(str, ret);
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
    }
    str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    emit StepSig(str);

    while(1)
    {
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
        ret = readData();

        a = Obj->cur.value[0];
        b = Obj->cur.value[1];
        c = Obj->cur.value[2];

        d = Obj->pow.value[0];
        e = Obj->pow.value[1];
        f = Obj->pow.value[2];

        if((!a) &&b &&c)
            if((!d) &&e &&f) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i] /COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位1-电流控制L1成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >100) {            
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L2");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
        ret = readData();

        a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
        d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

        if(((!b)&&(c)&&(a)) &&((!e)&&(f)&&(d))) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("插接位1-电流控制L2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >100) {           
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L3");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0 ,e=0 ,f = 1;
        a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
        d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

        if((!c)&&(b)&&(a))
            if((!f)&&(e)&&(d)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位1-电流控制L3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >100) {            
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    str = tr("请准备插接位2");  //三相回路电流、功率
    emit StepSig(str);
    while(1){
        int a=0; int d=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[3];
            d = Obj->pow.value[3];
            if(a) {
                if(d) ret = true; break;
            }
        }
    }

    str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;

        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if((!a) &&b &&c)
            if((!d) &&e &&f) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位2-电流控制L1成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L2");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;

        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if(((!b)&&(c)&&(a)) &&((!e)&&(f)&&(d))) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("插接位2-电流控制L2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >100) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L3");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0 ,e=0 ,f = 1;
        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if((!c)&&(b)&&(a))
            if((!f)&&(e)&&(d)) {
                ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
                str = tr("插接位2-电流控制L3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >100) {            
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    str = tr("请准备插接位3");  //三相回路电流、功率
    emit StepSig(str);
    while(1){
        int a=0; int d=0;
        if(ret) {
            ret = readData();
            a = Obj->cur.value[6];
            d = Obj->pow.value[6];
            if(a) {
                if(d) ret = true; break;
            }
        }
    }
    str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    emit StepSig(str);

    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;        
        a = Obj->cur.value[6]; b = Obj->cur.value[7]; c = Obj->cur.value[8];
        d = Obj->pow.value[6]; e = Obj->pow.value[7]; f = Obj->pow.value[8];

        if((!a) &&b &&c)
            if((!d) &&e &&f) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位3-电流控制L1成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >200) {          
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位3电流控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L2");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
        a = Obj->cur.value[6]; b = Obj->cur.value[7]; c = Obj->cur.value[8];
        d = Obj->pow.value[6]; e = Obj->pow.value[7]; f = Obj->pow.value[8];

        if((!b) &&(c) &&(a)) {
            if((!e) &&(f) &&(d)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位3电流控制L2成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}}

        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位3电流控制L2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L3");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0 ,e=0 ,f = 1;       
        a = Obj->cur.value[6]; b = Obj->cur.value[7]; c = Obj->cur.value[8];
        d = Obj->pow.value[6]; e = Obj->pow.value[7]; f = Obj->pow.value[8];

        if((!c)&&(b)&&(a))
            if((!f)&&(e)&&(d)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位3电流控制L3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >200) {           
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位3电流控制L3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_SixLoop()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    str = tr("请准备插接位1");  //三相回路电流、功率
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
    }

    str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;         
        a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
        d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

        if((!a) &&b &&c)
            if((!d) &&e &&f){
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位1-电流控制L1成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}

        flag++;
        if(flag >100) {           
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L2");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;

        a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
        d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

        if(((!b)&&(c)&&(a)) &&((!e)&&(f)&&(d))) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("插接位1-电流控制L2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }

        flag++;
        if(flag >200) {            
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }

    }

    flag = 0;
    str = tr("拨下电流控制开关L3");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0, c = 1; int d=0 ,e=0 ,f = 1;
        ret = readData();
        a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
        d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

        if((!c)&&(b)&&(a))
            if((!f)&&(e)&&(d)){
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位1-电流控制L3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位1-电流控制L3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    str = tr("请准备插接位2");  //三相回路电流、功率
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
    }

    str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if((!a) &&b &&c)
            if((!d) &&e &&f){
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位2-电流控制L1成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }

        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L2");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if(((!b)&&(c)&&(a)) &&((!e)&&(f)&&(d))) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("插接位2-电流控制L2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }

        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("拨下电流控制开关L3");
    emit StepSig(str);
    while(1)
    {
        ret = readData();
        int a=0, b=0, c = 1; int d=0 ,e=0 ,f = 1;       
        a = Obj->cur.value[3]; b = Obj->cur.value[4]; c = Obj->cur.value[5];
        d = Obj->pow.value[3]; e = Obj->pow.value[4]; f = Obj->pow.value[5];

        if((!c)&&(b)&&(a))
            if((!f)&&(e)&&(d)){
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("插接位2-电流控制L3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }

        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("插接位2-电流控制L3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Load_ThreeLoop()
{
    bool ret = true; int flag = 0; QString str; QString str1;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    if(mItem->modeId == START_BUSBAR) {
        str = tr("请准备插接位");  //三相回路电流、功率
        emit StepSig(str);
        while(1){
            int a=0; int d=0;
            ret = readData();
            a = Obj->cur.value[0];
            d = Obj->pow.value[0];
            if(a) {
                if(d) ret = true; break;
            }
        }

        flag = 0;
        str = tr("拨下电流控制开关L1");  //三相回路电流、功率
        emit StepSig(str);
        while(1)
        {
            ret = readData();
            int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
            a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
            d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

            if((!a) &&b &&c)
                if((!d) &&e &&f){
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电流控制L1成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            flag++;
            if(flag >100) {
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("电流控制L1失败");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }

        flag = 0;
        str = tr("拨下电流控制开关L2");  //三相回路电流、功率
        emit StepSig(str);
        while(1)
        {
            ret = readData();
            int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;            
            a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
            d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

            if((!b) &&a &&c)
                if((!e) &&d &&f) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电流控制L2成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            flag++;
            if(flag >100) {
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("电流控制L2失败");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }

        flag = 0;
        str = tr("拨下电流控制开关L3");  //三相回路电流、功率
        emit StepSig(str);
        while(1)
        {
            ret = readData();
            int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;           
            a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
            d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

            if((!c) &&b &&a)
                if((!f) &&e &&d) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电流控制L3成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            flag++;
            if(flag >100) {
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("电流控制L3失败");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }
    }else{
        if(mBusData->box[mItem->addr-1].phaseFlag) {    //三相
            str = tr("请准备插接位");  //三相回路电流、功率
            emit StepSig(str);
            while(1){
                int a=0; int d=0;
                ret = readData();
                a = Obj->cur.value[0];
                d = Obj->pow.value[0];
                if(a) {
                    if(d) ret = true; break;
                }
            }

            str = tr("拨下电流控制开关L1");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                ret = readData();
                int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
                a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
                d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

                if((!a) &&b &&c)
                    if((!d) &&e &&f) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L1成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }
                flag++;
                if(flag >100) {
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L1失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }

            flag = 0;
            str = tr("拨下电流控制开关L2");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                ret = readData();
                int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
                a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
                d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

                if((!b) &&a &&c)
                    if((!e) &&d &&f) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L2成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }

                flag++;
                if(flag >100) {
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L2失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }

            flag = 0;
            str = tr("拨下电流控制开关L3");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                ret = readData();
                int a=0, b=0, c = 1; int d=0, e=0 ,f = 1;
                a = Obj->cur.value[0]; b = Obj->cur.value[1]; c = Obj->cur.value[2];
                d = Obj->pow.value[0]; e = Obj->pow.value[1]; f = Obj->pow.value[2];

                if((!c) &&b &&a)
                    if((!f) &&e &&d) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L3成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }
                flag++;
                if(flag >100) {
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L3失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }
        }else{  //单相三回路三个输出位
            str = tr("请准备插接位1");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                int a=0; int d=0;
                if(ret) {
                    ret = readData();
                    a = Obj->cur.value[0];
                    d = Obj->pow.value[0];
                    if(!a)
                        if(!d) {
                            ret = true;
                            for(int i =0;i<loop;i++)
                            {
                                str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                                mLogs->updatePro(str, ret);
                            }
                            break;
                        }
                }
            }

            flag = 0;
            str = tr("拨下电流控制开关L1");  //三相回路电流、功率
            emit StepSig(str);

            while(1)
            {
                int a=0; int d=0;
                ret = readData();
                a = Obj->cur.value[0];
                d = Obj->pow.value[0];

                if(!a)
                    if(!d) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L1成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }

                flag++;
                if(flag >100) {
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L1失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }

            flag = 0;
            str = tr("请准备插接位2");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                int a=0; int d=0;
                if(ret) {
                    ret = readData();
                    a = Obj->cur.value[1];
                    d = Obj->pow.value[1];
                    if(!a) {
                        if(!d) ret = true; break;}
                }
            }

            str = tr("拨下电流控制开关L2");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                int b=0; int e=0;
                ret = readData();
                b = Obj->cur.value[1]; e = Obj->pow.value[1];
                if(!b)
                    if(!e) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L2成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }

                flag++;
                if(flag >100) {
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L2失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }

            flag = 0;
            str = tr("请准备插接位3");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                int a=0; int d=0;
                if(ret) {
                    ret = readData();
                    a = Obj->cur.value[2];
                    d = Obj->pow.value[2];
                    if(!a) {
                        if(!d) ret = true; break;}
                    }
            }

            str = tr("拨下电流控制开关L3");  //三相回路电流、功率
            emit StepSig(str);
            while(1)
            {
                int c = 1; int f = 1;
                ret = readData();
                c = Obj->cur.value[2];
                f = Obj->pow.value[2];

                if(!c)
                    if(!f) {
                        ret = true;
                        for(int i =0;i<loop;i++)
                        {
                            str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                            mLogs->updatePro(str, ret); str1 += str;
                        }
                        str = tr("电流控制L3成功");mLogs->updatePro(str, ret);
                        str1 += str; mPro->itemData << str1; str1.clear(); break;
                    }

                flag++;
                if(flag >100) {                    
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1 电流 %2A，功率 %3kw ").arg(i+1).arg(Obj->cur.value[i]/COM_RATE_CUR).arg(Obj->pow.value[i]/COM_RATE_POW);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    ret = false;
                    str = tr("电流控制L3失败");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;
                }
            }
        }
    }

    return ret;
}

bool Power_DevRead::Break_NineLoop()
{
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    bool ret = true; int flag = 0; QString str; QString str1;
    str = tr("关闭断路器控制1");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        if(ret) ret = readData();
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.value[i];
            b += Obj->vol.value[3+i];
            c += Obj->vol.value[6+i];
        }
        if((!a)&&(b >6600)&&(c >6600)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("断路器控制1成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
        flag++;
        if(flag >200) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("断路器控制1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("打开断路器控制1，关闭断路器控制2");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        if(ret) ret = readData();
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.value[i];
            b += Obj->vol.value[3+i];
            c += Obj->vol.value[6+i];
        }
        if((!b)&&(a>6600)&&(c>6600)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("断路器控制2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
        flag++;
        if(flag >200) {          
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("断路器控制2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("打开断路器控制2，关闭断路器控制3");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        if(ret) ret = readData();
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.value[i];
            b += Obj->vol.value[3+i];
            c += Obj->vol.value[6+i];
        }
        if((!c)&&(a>6600)&&(b>6600)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("断路器控制3成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >200) {      
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("断路器控制3失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Break_SixLoop()
{
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    bool ret = true; int flag = 0; QString str; QString str1;
    str = tr("打开断路器控制1");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0;
        if(ret) ret = readData();
        for(int i =0;i<3;i++)
        {
            a += Obj->vol.value[i];
            b += Obj->vol.value[3+i];
        }
        if((!a)&&(b>6600)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("断路器控制1成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >100) {            
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("断路器控制1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    flag = 0;
    str = tr("打开断路器控制2");
    emit StepSig(str);
    while(1)
    {
        int a=0, b=0;
        if(ret) ret = readData();
        for(int i =0;i<3;i++)
        {
            a += mBusData->box[mItem->addr - 1].data.vol.value[i];
            b += mBusData->box[mItem->addr - 1].data.vol.value[3+i];
        }
        if((!b)&&(a>6600)) {
            ret = true;
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            str = tr("断路器控制2成功");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;}
        flag++;
        if(flag >100) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                mLogs->updatePro(str, ret); str1 += str;
            }
            ret = false;
            str = tr("断路器控制2失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    return ret;
}

bool Power_DevRead::Break_ThreeLoop()
{
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    bool ret = true; int flag = 0; QString str; QString str1;

    if(mBusData->box[mItem->addr-1].phaseFlag ==0) {    //单相三回路三个输出位
        str = tr("打开断路器控制1");
        emit StepSig(str);
        while(1)
        {
            int a=0;
            if(ret) ret = readData();
            a = Obj->vol.value[0];

            if(a <100) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("断路器控制1成功 ");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
            flag++;
            if(flag >100) {                
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("断路器控制1失败");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }

        flag = 0;
        str = tr("打开断路器控制1，关闭断路器控制2");
        emit StepSig(str);
        while(1)
        {
            int a=0;
            if(ret) ret = readData();
            a = Obj->vol.value[1];

            if(a <100) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("断路器控制2成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}
            flag++;
            if(flag >100) {               
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("断路器控制2成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }

        flag = 0;
        str = tr("打开断路器2，拨关闭断路器3");
        emit StepSig(str);
        while(1)
        {
            int a=0;
            if(ret) ret = readData();
            a = Obj->vol.value[2];

            if(a <100) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("断路器控制3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
            flag++;
            if(flag >100) {                
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("断路器控制3成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }
    }else {
        flag = 0;
        str = tr("关闭断路器");
        emit StepSig(str);
        while(1)
        {
            int a=0,b=0,c=0;
            if(ret) ret = readData();
            a = Obj->vol.value[0]; b = Obj->vol.value[1]; c = Obj->vol.value[2];

            if((a <100)&&(b <100) &&(c <100)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    str = tr("A%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                str = tr("断路器控制成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;}
            flag++;
            if(flag >100) {               
                for(int i =0;i<loop;i++)
                {
                    str = tr("A%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("断路器控制成功");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
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
