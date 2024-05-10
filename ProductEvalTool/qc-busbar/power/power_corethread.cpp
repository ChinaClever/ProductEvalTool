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
    mModbus = Ad_Modbus::bulid(this);
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
    // if(mItem->modeId == START_BUSBAR)ret = true;
    // else
    ret = mRead->readSn();
    mItem->modeId = mDt->devType;
    qDebug()<<"mItem->modeId"<<mItem->modeId;
    return ret;
}

bool Power_CoreThread::volErrRange(int i)
{
    bool ret = true;
    int b = mBusData->box[mItem->addr - 1].data.vol.value[i];
    QString str = tr("第%1回路，实测电压=%2V ").arg(i+1).arg(b/(COM_RATE_VOL));

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::curErrRange(int i)
{
    bool ret = true;
    int b = mBusData->box[mItem->addr - 1].data.cur.value[i];
    QString str = tr("第%1回路，实测电流=%2A").arg(i+1).arg(b/COM_RATE_CUR);

    return mLogs->updatePro(str, ret);
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
bool Power_CoreThread::pfErrRange(int i)
{
    bool ret = true;
    QString str = tr("第%1回路，实测功率因素=%2").arg(i+1).arg(mBusData->box[mItem->addr-1].data.pf[i]/COM_RATE_PF);

    return mLogs->updatePro(str, ret);
}

bool Power_CoreThread::powErrRange(int i)
{
    bool ret = true;
    int b = mBusData->box[mItem->addr - 1].data.pow.value[i];
    QString str = tr("第%1回路，实测功率=%2kW").arg(i+1)
            .arg(b/COM_RATE_POW);

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

bool Power_CoreThread::checkVolErrRange()
{
    int i = 0;
    bool res = true, ret = true;
    for(; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        ret = volErrRange(i); if(!ret) res = false;
    }

    return res;
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
    if(res) str += tr("成功"); else str += tr("L%1 失败").arg(i);
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
    bool ret = true , res = true;
    ret = mRead->readDevData();

    int i = 0;
    for(; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        ret = eleErrRange0(i); if(!ret) res = false;
    }
    QString str = tr("清除电能");
    if(res) str += tr("成功"); else str += tr("L%1 失败").arg(i);
    mPro->step = Test_Over;
    mLogs->updatePro(str, res);
    emit StepSig(str);

    return;
}

void Power_CoreThread::workResult(bool)
{

    mLogs->updatePro(tr("测试结束"));
    mPro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);

    mPro->itemContent << "回路数量:" + mPro->loopNum;
    mPro->itemContent << "软件版本为:" + mPro->softwareVersion;
    mPro->itemContent << "模块序列号:" + mPro->moduleSN;
    mPro->itemContent << "设备类型:" + mPro->productType;

    if(mItem->modeId != START_BUSBAR) {
        mPro->phase = QString::number(mBusData->box[mItem->addr-1].phaseFlag);
        mPro->itemContent << "相数:" + mPro->phase;
    }
    bool res = false;
    QString str = tr("测试结果 ");
    if(mPro->result != Test_Fail) {
        res = true;
        str += tr("通过");
        mPro->uploadPassResult = 1;
    } else {
        res = false;
        str += tr("失败");
        mPro->uploadPassResult = 0;
    }
    mPacket->updatePro(str, res);
    mLogs->saveLogs();
    sleep(1);
    Json_Pack::bulid()->http_post("busbarreport/add","192.168.1.42");//全流程才发送记录(http)
    if(mPro->result == Test_Fail) res = false;
    emit finshSig(res); mPro->step = Test_Over;
}

bool Power_CoreThread::stepVolTest()
{
    bool ret = true;
    QString str = tr("拨下电压控制开关L1");
    emit StepSig(str);
    while(1)
    {
        if(mItem->modeId == START_BUSBAR) {
            ret = mRead->checkNet();
            if(!ret) break;
        }else {
            ret = mRead->readDevData();
            if(!ret) { str = tr("L1成功");mLogs->updatePro(str, !ret);break;}
        }
    }

    str = tr("拨开电压控制开关L1，拨下控制开关L2");  //b1,b2,b3
    emit StepSig(str);
    while(1)
    {
        int a, b, c;
        ret = mRead->checkNet();
        if(ret) ret = mRead->readData();
        if(mBusData->box[mItem->addr - 1].phaseFlag) {  //三相
            for(int i =0;i<3;i++)
            {
                a = mBusData->box[mItem->addr - 1].data.vol.value[i];
                b = mBusData->box[mItem->addr - 1].data.vol.value[3+i];
                c = mBusData->box[mItem->addr - 1].data.vol.value[6+i];
            }
            if((!b)&&(a)&&(c)) { ret = true; break;}
        }else {
                a = mBusData->box[mItem->addr - 1].data.vol.value[0];
                b = mBusData->box[mItem->addr - 1].data.vol.value[1];
                c = mBusData->box[mItem->addr - 1].data.vol.value[2];
            if((!b)&&(a)&&(c)) { ret = true; break;}
        }
    }

    str = tr("拨开电压控制开关L2，拨下控制开关L3");//c1,c2,c3
    emit StepSig(str);
    while(1)
    {
        int a, b, c;
        ret = mRead->checkNet();
        if(ret) ret = mRead->readData();
        if(mBusData->box[mItem->addr - 1].phaseFlag) {  //三相
            for(int i =0;i<3;i++)
            {
                a = mBusData->box[mItem->addr - 1].data.vol.value[i];
                b = mBusData->box[mItem->addr - 1].data.vol.value[3+i];
                c = mBusData->box[mItem->addr - 1].data.vol.value[6+i];
            }
            if((!c)&&(a)&&(b)) { ret = true; break;}
        }else {
            a = mBusData->box[mItem->addr - 1].data.vol.value[0];
            b = mBusData->box[mItem->addr - 1].data.vol.value[1];
            c = mBusData->box[mItem->addr - 1].data.vol.value[2];
            if((!c)&&(a)&&(b)) { ret = true; break;}
        }
    }

    return ret;
}

bool Power_CoreThread::stepLoadTest()
{
    bool ret = false;
    QString str = tr("拨下电流控制开关L1");  //三相回路电流、功率
    mLogs->updatePro(str, ret); sleep(5);

    ret = mRead->readData();
    for(int i=0; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        curErrRange(i); powErrRange(i);
    }

    str = tr("拨下电流控制开关L2");
    mLogs->updatePro(str, ret); sleep(5);

    ret = mRead->readData();
    for(int i=0; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        curErrRange(i); powErrRange(i);
    }

    str = tr("拨下电流控制开关L3");
    mLogs->updatePro(str, ret); sleep(5);

    ret = mRead->readData();
    for(int i=0; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        curErrRange(i); powErrRange(i);
    }

    return ret;
}

bool Power_CoreThread::stepBreakerTest(int sum)
{
    bool ret = false;
    QString str;
    for(int i =0;i<sum;i++){
        str = tr("拨下断路器开关%1").arg(i+1);  //三相回路电压都为0
        mLogs->updatePro(str, ret); sleep(5);
        ret = mRead->readData();
        checkVolErrRange();
    }

    return ret;
}

void Power_CoreThread::getDelaySlot()
{
    QString str;
    if(mCurBoxNum == 0){
        str = (tr("始端箱未发出命令"));
       mLogs->updatePro(str, true);
    }else if(mCurBoxNum >= 4 && mCurBoxNum <= 20 ){
        str = tr("设置地址%1失败").arg(mCurBoxNum);
    }
    emit StepSig(str);
    mCurBoxNum = 0;
}

void Power_CoreThread::getNumAndIndexSlot(int curnum)
{
    int num = curnum >= 2 ? curnum : 2;
    QString str;
    mCurBoxNum = num;
    if(num > 2){
        QString str = tr("设置地址%1成功").arg(num-1);
    }
    emit StepSig(str);
}

void Power_CoreThread::workDown()
{
    mPro->step = Test_Start;
    bool ret = false;
    ret = initDev(); if(ret) ret = mRead->readDev();
    if(ret) {
        if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(),"0");
        else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(0);  //设置滤波=0

        if(mCfg->work_mode == 2) {
            mModbus->autoSetAddress();                       //自动分配地址
            if(ret) ret = stepVolTest();                     //电压测试
        }else if(mCfg->work_mode == 3) {                     //负载测试
            if(ret) ret = mSource->read();
            else mPro->result = Test_Fail;
            if(ret) ret = checkLoadErrRange();
            if(ret) ret = stepLoadTest();
        }else if(mCfg->work_mode == 4) {                     //断路器测试
            int sum = mBusData->box[mItem->addr-1].loopNum /3;
            if(ret) ret = stepBreakerTest(sum);
        }

        if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(), "5");
        else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(5);  //设置滤波=5
        if(ret) ret = factorySet();
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
    isRun = false;
}
