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
    ret = mRead->readSn();
    mItem->modeId = mDt->devType;

    mPro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);

    mPro->itemContent << "回路数量:" + mPro->loopNum;
    mPro->itemContent << "软件版本为:" + mPro->softwareVersion;
    mPro->itemContent << "模块序列号:" + mPro->moduleSN;
    mPro->itemContent << "设备类型:" + mPro->productType;

    if(mItem->modeId != START_BUSBAR) {
        mPro->phase = QString::number(mBusData->box[mItem->addr-1].phaseFlag);
        mPro->itemContent << "相数:" + mPro->phase;
    }

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
    bool ret = true , res = false;
    ret = mRead->readDevData();

    int i = 0;
    for(; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        ret = eleErrRange0(i); if(ret) res = true;
    }
    QString str = tr("清除电能");
    if(res) str += tr("成功"); else str += tr("L%1 失败").arg(i);
    mPro->step = Test_Over;
    mLogs->updatePro(str, res);
    emit TipSig(str);

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
    bool ret = true;int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str = tr("关闭电压控制L1"); QString str1;
    emit TipSig(str);
    while(1)
    {
        if(mItem->modeId == START_BUSBAR) {
            ret = mRead->checkNet();
            if(!ret) break;
        }else {
            ret = mRead->readData();
            if(!ret) {
                str = tr("电压控制L1成功");mLogs->updatePro(str, !ret);
                mLogs->writeData(str); str1.clear(); break;
            }
        }
        flag++;
        if(flag >100) {
            for(int i =0;i<loop;i++)
            {
                str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);

                mLogs->updatePro(str, ret);
            }
            ret = false; str = tr("电压控制L1失败");mLogs->updatePro(str, ret);
            str1 += str; mPro->itemData << str1; str1.clear(); break;
        }
    }

    str = tr("打开电压控制L1，关闭控制L2");  //b1,b2,b3
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
//              if((!b)&&(a)&&(c)) { ret = true; str = tr("电压控制L2");mLogs->updatePro(str, !ret); break;}
                if((b < 300)&&(a >6600)&&(c >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret);
                        str1 += str;
                    }
                    str = tr("电压控制L2成功"); str1 += str; mPro->itemData << str1;
                    mLogs->updatePro(str, ret); str1.clear(); break;
                }
            }else if(loop == 6) {
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
//                if((!b)&&(a)&&(c)) { ret = true; str = tr("电压控制L2");mLogs->updatePro(str, !ret); break;}
                if((b < 300)&&(a >6600)&&(c >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电压控制L2成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear();break;}
            }else if(loop == 3){
                a = Obj->vol.value[0];
                b = Obj->vol.value[1];
                c = Obj->vol.value[2];
//            if((!b)&&(a)&&(c)) { ret = true; str = tr("电压控制L2成功");mLogs->updatePro(str, !ret);break;}
                if((b < 300)&&(a >6600)&&(c >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电压控制L2成功"); mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear();break;}
            }
            flag++;
            if(flag >100) {
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("电压控制L2失败"); mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }
    }
    flag = 0;
    str = tr("打开电压控制L2，关闭电压控制L3");//c1,c2,c3
    emit TipSig(str);
    while(1)
    {
        int a=0, b=0, c=0;
        if(ret) {
            ret = mRead->readData();
            if(loop == 9)
            {
                for(int i =0;i<9;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                qDebug()<<"b"<<b<<c<<a;
                if((c < 300)&&(a >6600)&&(b >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电压控制L3成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;}
            }else if(loop == 6) {
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.value[i];
                    b += Obj->vol.value[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((c < 300)&&(a> 6600)&&(c >6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电压控制L3成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;}
            }else if(loop == 3){
                a = Obj->vol.value[0];
                b = Obj->vol.value[1];
                c = Obj->vol.value[2];
                if((c < 300)&&(a >6600)&&(b>6600)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        mLogs->updatePro(str, ret); str1 += str;
                    }
                    str = tr("电压控制L3成功");mLogs->updatePro(str, ret);
                    str1 += str; mPro->itemData << str1; str1.clear(); break;}
            }
            flag++;
            if(flag >100) {
                for(int i =0;i<loop;i++)
                {
                    str = tr("回路%1电压 %2V ").arg(i+1).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    mLogs->updatePro(str, ret); str1 += str;
                }
                ret = false;
                str = tr("电压控制L3失败");mLogs->updatePro(str, ret);
                str1 += str; mPro->itemData << str1; str1.clear(); break;
            }
        }else break;
    }
    return ret;
}

bool Power_CoreThread::stepLoadTest()
{
    bool ret = true;
    if(mBusData->box[mItem->addr-1].loopNum == 9) {
        ret = mRead->Load_NineLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 6) {
        ret = mRead->Load_SixLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 3) {
        ret = mRead->Load_ThreeLoop();
    }

    return ret;
}

bool Power_CoreThread::stepBreakerTest()
{
    bool ret = true;
    if(mBusData->box[mItem->addr-1].loopNum == 9) {
        ret = mRead->Break_NineLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 6) {
        ret = mRead->Break_SixLoop();
    } else if(mBusData->box[mItem->addr-1].loopNum == 3) {
        ret = mRead->Break_ThreeLoop();
    }

    return ret;
}

void Power_CoreThread::getDelaySlot()
{
    QString str; bool ret = false;
    if(mCurBoxNum == 0){
        str = (tr("始端箱未发出命令"));
        ret = true;
    }else if(mCurBoxNum >= 4 && mCurBoxNum <= 20 ){
        str = tr("设置地址%1失败").arg(mCurBoxNum);
        ret = false;
    }
    mLogs->updatePro(str, ret);
    mCurBoxNum = 0;
}

void Power_CoreThread::getNumAndIndexSlot(int curnum)
{
    int num = curnum >= 2 ? curnum : 2;
    QString str; bool ret = false;
    mCurBoxNum = num;
    if(num > 2){
        QString str = tr("设置地址%1成功").arg(num-1);
        ret = false;
    }
    mLogs->updatePro(str, ret);
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
            // mModbus->autoSetAddress();                       //自动分配地址
            if(ret) ret = stepVolTest();                     //电压测试
        }else if(mCfg->work_mode == 3) {                     //负载测试
          // if(ret) ret = mSource->read();
//            else mPro->result = Test_Fail;
//            if(ret) ret = checkLoadErrRange();
            if(ret) ret = stepLoadTest();
            if(ret) ret = factorySet();
        }else if(mCfg->work_mode == 4) {                     //断路器测试
            if(ret) ret = stepBreakerTest();
        }

        if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(), "5");
        else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(5);  //设置滤波=5

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
