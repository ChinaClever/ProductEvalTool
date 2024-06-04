/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_corethread.h"
#include "printer_bartender.h"

Power_CoreThread::Power_CoreThread(QObject *parent) :Power_Object(parent)
{

}

void Power_CoreThread::initFunSlot()
{
    mRet = -1; mCurBoxNum = 0;
    mLogs = Power_Logs::bulid(this);
    mRead = Power_DevRead::bulid(this);
    mCtrl = Power_DevCtrl::bulid(this);
    Printer_BarTender::bulid(this);
    mSource = Dev_Source::bulid(this);
    mCfg = TestConfig::bulid()->item;

    mModbus = Rtu_Modbus::bulid(this)->get(2);
    connect(mModbus,&RtuRw::sendNumAndIndexSig, this, &Power_CoreThread::getNumAndIndexSlot);
    connect(mModbus,&RtuRw::sendDelaySig, this, &Power_CoreThread::getDelaySlot);
}

bool Power_CoreThread::hubPort()
{
    QString str = tr("设备 SER 级联口");
    bool ret = mItem->coms.ser3->isOpened();
    if(ret) {
        ret = mRead->readHub();
        if(ret) str += tr("正常");
        else str += tr("错误");
    } else {
        ret = true;
        str += tr("跳过");
    }

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::initDev()
{
    mLogs->updatePro(tr("即将开始"));
    bool ret  = false;
    ret = mRead->readSn();
    mItem->modeId = mDt->devType;

    return ret;
}

void Power_CoreThread::StartErrRange()
{
    bool ret = false; QString str ;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);

    int curValue = b->buzzerStatus;
    int expect = mItem->ip.ip_buzzer;
    if(curValue == expect) ret = true;
    str = tr("始端箱蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->ip.ip_filter;
    if(curValue == expect) ret = true;
    str = tr("始端箱过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);ret = false;
    if(!ret) {
        mRead->SetInfo(mRead->getFilterOid(),QString::number(expect));
    }

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
}

void Power_CoreThread::InsertErrRange()   //比较基本配置信息
{
    bool ret = false; QString str;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);

    int curValue = b->buzzerStatus;
    int expect = mItem->si.si_buzzer;
    if(curValue == expect) ret = true;
    str = tr("插接箱蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->si.si_filter;
    if(curValue == expect) ret = true;
    str = tr("插接箱过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret);
    if(!ret) {
        bool res = Ctrl_SiRtu::bulid()->setBusbarInsertFilter(expect);
        str = tr("插接箱过滤次数设置成功");
        mLogs->updatePro(str,res);
    }
    ret = false;

    curValue = b->iOF;
    expect = mItem->si.si_iOF;
    if(curValue == expect) ret = true;
    str = tr("插接箱iOF辅助触点实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->phaseFlag;
    expect = mItem->si.si_phaseflag;
    if(curValue == expect) ret = true;
    str = tr("插接箱输出位类型实际值：%1 , 期待值：%2！").arg(curValue?tr("三相"):tr("单相")).arg(expect?tr("三相"):tr("单相"));
    mLogs->updatePro(str,ret);ret = false;
}

void Power_CoreThread::BaseErrRange()   //比较基本配置信息
{
    if(mItem->modeId == START_BUSBAR) {
        StartErrRange();
    } else {
        InsertErrRange();
    }
}

bool Power_CoreThread::eleErrRange(int i)
{
    bool ret = mItem->eleCheck;
    QString str = tr("第%1回路，实测电能=%2Kwh").arg(i+1).arg(mBusData->box[mItem->addr-1].data.ele[i]/COM_RATE_ELE);

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::eleErrRange0(int i)
{
    QString str = tr("电能 L%1，实测电能=%2Kwh").arg(i+1).arg(mBusData->box[mItem->addr-1].data.ele[i]/COM_RATE_ELE);
    bool ret = false;
    if(0 != mBusData->box[mItem->addr-1].data.ele[i]) {
        str += tr("错误");
        ret = false;
    } else {
        str += tr("正常");
        ret = true;
    }

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::envErrRange()
{
    bool ret = true;
    for(int i = 0 ; i < 4 ; i++){
        QString str = tr("传感器温度%1，实测温度= %2℃，").arg(i+1).arg(mBusData->box[mItem->addr - 1].env.tem.value[i]);
        if(ret) str += tr("正常");
        else {
            if(mBusData->box[mItem->addr - 1].env.tem.value[i]) {
                str += tr("错误");
            } else {
                str = tr("请插入传感器，实测温度 = 0");
            }
        }
        ret = mLogs->updatePro(str, ret);
    }

    return ret;
}

bool Power_CoreThread::volLoadErrRange(int i)
{
    bool ret = true;
    int a = mSour->line.vol.value[i];
    QString str = tr("负载第%1相，实测电压=%2V").arg(i+1).arg(a/COM_RATE_VOL);

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::curLoadErrRange(int i)
{
    bool ret = true;
    int a = mSour->line.cur.value[i];
    QString str = tr("负载第%1相，实测电流=%2A").arg(i+1).arg(a/COM_RATE_CUR);

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::powLoadErrRange(int i)
{
    bool ret = true;
    int a = mSour->line.cur.value[i];
    QString str = tr("负载第%1相，实测功率=%2kW").arg(i+1).arg(a/COM_RATE_POW);

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::checkLoadErrRange()
{
    int i = 0;
    bool res = true, ret = true;
    for(; i<3; ++i) {
        ret = volLoadErrRange(i); if(!ret) res = false;
        ret = curLoadErrRange(i); if(!ret) res = false;
        ret = powLoadErrRange(i); if(!ret) res = false;
    }

    return res;
}

QString Power_CoreThread::changeMode(int index)
{
    QString str = tr("主机地址 1 / Modbus模式");
    switch(index){
    case 0:str = tr("主机地址 0 / SNMP模式");break;
    case 1:str = tr("主机地址 1 / Modbus模式");break;
    default:str = tr("主机地址 1 / Modbus模式");break;
    }
    return str;
}

bool Power_CoreThread::factorySet()
{
    bool ret = true , res = true;
    if(mItem->modeId == START_BUSBAR && mItem->ip.addr == SNMP){
        mRead->SetInfo(mRead->getConnectModeOid(),"1");//切换成RTU模式
    }
    ret = mCtrl->eleClean();
    mRead->readDevData();
    int i = 0;
    for(; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        ret = eleErrRange0(i); if(!ret) res = false;
    }
    QString str = tr("清除电能");
    if(res) str += tr("成功"); else str += tr("失败");
    mLogs->updatePro(str, res);

    if(mItem->modeId == START_BUSBAR && mItem->ip.addr == SNMP){
        mRead->SetInfo(mRead->getConnectModeOid(),"0");//切换成SNMP模式
        QString str = tr("对比始端箱通讯模式！");
        mLogs->updatePro(str);
        sBoxData* b = &(mPacket->share_mem_get()->box[mItem->addr-1]);
        bool ret = false;
        int curValue = b->workMode;
        int expect = mItem->ip.ip_mode;
        if(curValue == expect) ret = true;
        str = tr("始端箱通讯模式实际值：%1 , 期待值：%2！").arg(changeMode(curValue)).arg(changeMode(expect));
        mLogs->updatePro(str,ret);
    }

    return res;
}

void Power_CoreThread::clearStartEleSlot()
{
    mCtrl->eleClean();

    return;
}

void Power_CoreThread::workResult(bool)
{
    mLogs->updatePro(tr("测试结束"));

    bool res = false;
    QString str = tr("测试结果 ");
    if(mPro->result != Test_Fail) {
        res = true;
        str += tr("通过");
    } else {
        res = false;
        str += tr("失败");
    }
    mPacket->updatePro(str, res);
    mPro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);

    mPro->itemContent << "回路数量：" + mPro->loopNum;
    mPro->itemContent << "模块序列号：" + mPro->moduleSN;
    mPro->itemContent << "设备类型：" + mPro->productType;
    mPro->itemContent << "告警滤波次数：" + QString::number(mBusData->box[mItem->addr-1].alarmTime);

    if(mBusData->box[mItem->addr-1].buzzerStatus) mPro->itemContent << "蜂鸣器：关闭";
    else mPro->itemContent << "蜂鸣器：开启";

    if(mBusData->box[mItem->addr-1].iOF) mPro->itemContent << "断路器IOF触点：配有";
    else mPro->itemContent << "断路器IOF触点：无";

    if(mItem->modeId == INSERT_BUSBAR) {
        mPro->phase = QString::number(mBusData->box[mItem->addr-1].phaseFlag);
        if(mBusData->box[mItem->addr-1].phaseFlag) mPro->itemContent << "相数：三相";
        else mPro->itemContent << "相数：单相";

    }

    if(mItem->modeId == START_BUSBAR) {
        if(mBusData->box[mItem->addr-1].shuntRelease) mPro->itemContent << "分励脱扣器：配有";
        else mPro->itemContent << "分励脱扣器：无";
        if(mBusData->box[mItem->addr-1].lightning) mPro->itemContent << "防雷模块：配有";
        else mPro->itemContent << "防雷模块：无";
        if(mBusData->box[mItem->addr-1].isd) mPro->itemContent << "ISD报警触点：配有";
        else mPro->itemContent << "ISD报警触点：无";
    } else {
        if(mBusData->box[mItem->addr-1].boxType) mPro->itemContent << "盒子类型: 测温模块";
        else mPro->itemContent << "盒子类型：插接箱";
    }

    // mLogs->saveLogs();
    // if(mPro->online) {
    //     sleep(1);
    //     Json_Pack::bulid()->stepData();//全流程才发送记录(http)
    // }

    if(mCfg->work_mode == 3){
        while(1)
        {
            // qDebug()<<"JudgSig"<<mPro->issure;
            msleep(500);
            if(mPro->issure)
            {
                break;

            }
        }
    }

    mLogs->saveLogs();
    if(mPro->online) {
        Json_Pack::bulid()->stepData();//全流程才发送记录(http)
    }

    if(mPro->online) {
        if(mPro->result == Test_Fail) {
            str = tr("数据发送失败");
            res = false;
        }else {
            str = tr("数据发送成功");
            res = true;
        }
        mPacket->updatePro(str, res);
    }

    emit finshSig(res); mPro->step = Test_Over;
}

bool Power_CoreThread::Vol_ctrlOne()
{
    bool ret = true;int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str; QString str1, str2;
    str = tr("打开电压控制L1，关闭电压控制L2");  //b1,b2,b3
    emit TipSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        ret = mRead->readData();
        if(ret) {
            if(loop == 9)
            {
                for(int i =0;i<9;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!b)&&(a >6600)&&(c >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("回路2、5、8电压值等于0，回路1、3、4、6、7、9电压值各大于220V");
                    str = tr("电压控制L2成功 "); mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret);
                    str1.clear(); break;
                }
            }else if(loop == 6) {
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!b)&&(a >4400)&&(c >4400)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("回路2、5电压值等于0，回路1、3、4、6电压值各大于220V");
                    str = tr("电压控制L2成功 ");mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret); str1.clear();break;}
            }else if(loop == 3){
                a = Obj->vol.value[0];
                b = Obj->vol.value[1];
                c = Obj->vol.value[2];
                if((!b)&&(a >2200)&&(c >2200)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("回路2电压值等于0，回路1、3电压值各大于220V");
                    str = tr("电压控制L2成功 "); mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret); str1.clear();break;}
            }            
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str, ret);
            str2 = tr("不满足测试要求"); ret = false;
            str = tr("电压控制L2失败"); mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::Vol_ctrlTwo()
{
    bool ret = true; int flag = 0; QString str1,str2;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    QString str = tr("打开电压控制L2，关闭电压控制L3");//c1,c2,c3
    emit TipSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        ret = mRead->readData();
        if(ret) {
            if(loop == 9)
            {
                for(int i =0;i<9;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!c)&&(a >6600)&&(b >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str, ret);
                    str2 = tr("回路3、6、9电压值等于0，回路1、2、4、5、7、8电压值各大于220V");
                    str = tr("电压控制L3成功 ");mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
                }
            }else if(loop == 6) {
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!c)&&(a> 4400)&&(b>4400)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("回路3、6电压值等于0，回路1、2、4、5电压值各大于220V");
                    str = tr("电压控制L3成功 ");mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
                }
            }else if(loop == 3){
                a = Obj->vol.value[0];
                b = Obj->vol.value[1];
                c = Obj->vol.value[2];
                if((!c)&&(a >2200)&&(b>2200)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("回路3电压值等于0，回路1、2电压值各大于220V");
                    str = tr("电压控制L3成功 ");mLogs->updatePro(str, ret);
                    str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
                }
            }
        }
        flag++;
        if(flag >60) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str2 = tr("不满足测试要求");
            str = tr("电压控制L3失败");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear(); break;
        }
    }

    return ret;
}
bool Power_CoreThread::stepVolTest()
{
    bool ret = true;int flag = 0; QString str1, str2;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str = tr("关闭电压控制L1");
    emit TipSig(str); sleep(1);

    while(1)
    {
        if(mItem->modeId == START_BUSBAR) {
            ret = mRead->checkNet();
            if(!ret) {
                str2 = tr("电压控制L1关闭，网络异常");
                str = tr("电压控制L1成功");mLogs->updatePro(str, !ret);
                mLogs->writeData(str2,str,!ret); str1.clear(); ret = true; break;
            }
        }else {
            ret = mRead->readData();
            if(!ret) {
                str2 = tr("电压控制L1关闭，串口通信异常");
                str = tr("电压控制L1成功");mLogs->updatePro(str, !ret);
                mLogs->writeData(str2,str,!ret);
                str1.clear(); ret = true; break;
            }
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;                
            }
            mLogs->updatePro(str1, !ret); ret = false;
            str2 = tr("电压控制L1断电不成功异常");
            str = tr("电压控制L1失败");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str,ret); str1.clear();
            ret = true; break;
        }
    }
    if(mPro->stopFlag == 0) {   //失败即停止测试
        if(ret) ret = Vol_ctrlOne();
        if(ret) ret = Vol_ctrlTwo();
    }else {
        ret = Vol_ctrlOne();
        ret = Vol_ctrlTwo();
    }

    return ret;
}

bool Power_CoreThread::stepLoadTest()
{
    bool ret = false;
    if(mBusData->box[mItem->addr-1].loopNum == 9) {
        ret = mRead->Load_NineLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 6) {
        ret = mRead->Load_SixLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 3) {
        ret = mRead->Load_ThreeLoop();
    }

    return ret;
}

void Power_CoreThread::getDelaySlot()
{
    QString str; bool ret = false;
    if(mCurBoxNum == 0){
        str = (tr("始端箱未发出命令"));
    }else if(mCurBoxNum >= 2 && mCurBoxNum <= 3 ){
        str = tr("设置地址失败");
    }
    emit TipSig(str);
    mLogs->updatePro(str, ret);

    if(mCurBoxNum == 2) str = tr("插接箱 IN口接错");
    if(mCurBoxNum == 3) str = tr("插接箱 OUT口接错");
    emit TipSig(str);
    mLogs->updatePro(str, ret);
    mLogs->writeData("测试插接箱地址设成2",str,ret);
    mCurBoxNum = 0;
}

void Power_CoreThread::getNumAndIndexSlot(int curnum)
{
    int num = curnum >= 2 ? curnum : 2;
    bool ret = false;
    mCurBoxNum = num;
    QString str = tr("级联测试成功，设置地址成功");
    if(num > 3){
        ret = true;
        emit TipSig(str);
        mLogs->updatePro(str, ret);
        mLogs->writeData("测试插接箱地址设成2",str,ret);
    }
}

void Power_CoreThread::workDown()
{
    mPro->step = Test_Start;
    bool ret = false;
    ret = initDev(); if(ret) ret = mRead->readDev();
    if(ret) {
        if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(),"0");
        else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(0); //设置滤波=0

        if(mCfg->work_mode == 2) {
            QString str = tr("请打开插接箱测试治具电源");
            emit TipSig(str); sleep(15);

            if(mItem->modeId != START_BUSBAR)
                mModbus->autoSetAddress();                       //自动分配地址

            str = tr("请关闭插接箱测试治具电源");
            emit TipSig(str); sleep(3);

            if(ret) ret = stepVolTest();                     //电压测试

        }else if(mCfg->work_mode == 3) {
          // if(ret) ret = mSource->read();
          // else mPro->result = Test_Fail;
          // if(ret) ret = checkLoadErrRange();
           if(ret) ret = stepLoadTest();                       //负载+断路器测试
           BaseErrRange();                                     //对比始端箱/插接箱基本信息
           if(ret) ret = factorySet();                       //清除电能
           emit JudgSig();
        }
    }

    if(!ret) mPro->result = Test_Fail;

    workResult(ret);
}

void Power_CoreThread::noloadHomeSlot(int ret)
{
    mRet = ret;
}

void Power_CoreThread::run()
{
    if(isRun) return;
    isRun = true;

    workDown();
    mPro->step = Test_Over;
    isRun = false;
}
