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

    mErr = Power_ErrRange::bulid(this);
    mSiRtu = Dev_SiRtu::bulid(this);
    mLogs = Power_Logs::bulid(this);
    mRead = Power_DevRead::bulid(this);
    mCtrl = Power_DevCtrl::bulid(this);
    Printer_BarTender::bulid(this);
    mSource = Dev_Source::bulid(this);
    mCfg = TestConfig::bulid()->item;
    mItem = Cfg::bulid()->item;

    mModbus = Rtu_Modbus::bulid(this)->get(2);
    connect(mModbus,&RtuRw::sendNumAndIndexSig, this, &Power_CoreThread::getNumAndIndexSlot);
    connect(mModbus,&RtuRw::sendDelaySig, this, &Power_CoreThread::getDelaySlot);
}

bool Power_CoreThread::initDev()
{
    mLogs->updatePro(tr("即将开始"));
    bool ret  = true;
    if(mItem->modeId == INSERT_BUSBAR){

        QString str = tr("开始测试插接箱串口通讯");           //自动分配地址
        emit TipSig(str); sleep(2);
        str = tr("请将插接箱IN口与测试治具IN口对接，OUT口与测试治具OUT口对接");
        emit TipSig(str); emit ImageSig(3);

        mModbus->autoSetAddress(); emit ImageSig(4);

    }

    ret = mRead->readSn();
    if(ret) mItem->modeId = mDt->devType;

    return ret;
}

void Power_CoreThread::StartErrRange()
{
    bool ret = false; QString str ; int flag = 0;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);
    QString str1 = tr("串口线接入IN接口，Modbus RTU串口命令正常返回，IN接口通讯正常");
    QString str2 = tr("接口检查"); QString str3 = tr("符合要求");

    QString eng1 = tr("The serial cable is connected to the IN interface, and the Modbus RTU serial command returns normally, indicating that the IN interface communication is normal.");
    QString eng2 = tr("Interface inspection"); QString eng3 = tr("Meeting the requirements");

    str = tr("请将串口线接入到始端箱IN口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3);
    while(1)
    {
        ret = mSiRtu->readPduData();
        if(ret) break;

        flag++;
        if(flag >30){
            ret = false; break;
        }
    }
    str = tr("始端箱IN口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
    }else {
        str += tr("失败"); str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);
    mLogs->writeDataEng(eng1, eng3, eng2, ret);

    str = tr("请将IN串口线拔出");           //IN OUT口通讯正常，及网口
    emit TipSig(str); flag = 0;
    while(1)
    {
        ret = mSiRtu->readPduData();
        if(!ret) break;

        flag++;
        if(flag >30){
            ret = false; break;
        }
    }

    str = tr("请将串口线接入到始端箱OUT口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3); flag = 0;
    while(1)
    {
        ret = mSiRtu->readPduData();
        if(ret) break;

        flag++;
        if(flag >30){
            ret = false; break;
        }
    }
    str1 = tr("串口线 接入OUT接口，Modbus RTU串口命令正常返回，OUT接口通讯正常");
    eng1 = tr("The serial cable is connected to the OUT interface, and the Modbus RTU serial command returns normally. The OUT interface communication is normal");
    str = tr("始端箱OUT口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
    }else {
        str += tr("失败"); str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);
    mLogs->writeDataEng(eng1, eng3, eng2, ret);

    str = tr("请将网线接入到始端箱NET口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3); flag = 0;
    while(1)
    {

        ret = mRead->checkNet();
        if(ret) break;

        flag++;
        if(flag >30){
            ret = false; break;
        }
    }
    str1 = tr("ping功能正常，NET接口通讯正常");
    eng1 = tr("Ping function is normal, NET interface communication is normal");
    str = tr("始端箱NET口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
    }else {
        str += tr("失败"); str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);
    mLogs->writeDataEng(eng1, eng3, eng2, ret);

    mRead->readData();
    ret = false;
    int curValue = b->buzzerStatus;
    int expect = mItem->ip.ip_buzzer;
    if(curValue == expect) ret = true;
    str = tr("始端箱蜂鸣器实际值：%1 ， 期待值：%2").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret); ret = false; //mLogs->writeData(str1, str, ret);

    curValue = b->alarmTime;
    mItem->ip.ip_filter = curValue;
    if(curValue >= 5) ret = true;
    str = tr("始端箱过滤次数实际值：%1").arg(curValue);
    if(ret) str += tr("成功");
    else str += tr("失败，实际值小于5");
    mLogs->updatePro(str,ret); ret = false;

    str1 = tr("检查SPD防雷模块接入状态，接入正常");
    eng1 = tr("Check the connection status of the SPD lightning protection module, and ensure that it is connected normally");
    str2 = tr("SPD防雷检查"); eng2 = tr("SPD lightning protection inspection");
    curValue = b->lightning;
    expect = mItem->ip.ip_lightning;
    if(curValue == expect) ret = true;
    str = tr("始端箱防雷模块实际值：%1 ， 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);
    flag = 0;
    if(ret) {
        if(curValue == 1) {//防雷模块
            while(1)
            {
                ret = mRead->readData();
                if(ret){
                    if(b->lpsState == 2) {
                        str = tr("请检查始端箱防雷模块是否正确安装");
                        emit TipSig(str);
                    }else if(b->lpsState == 1) {
                        break;
                    }
                }
                flag++;
                if(flag >40){
                    ret = false; break;
                }
            }
            str = tr("始端箱防雷模块状态：%1").arg(b->lpsState ==1?tr("工作正常"):tr("损坏"));
            eng3 = tr("Busbar feeder box lightning protection module status:%1").arg(b->lpsState ==1?tr("normal operation"):tr("damage"));
            // if(ret) {
            //     str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
            // }else {
            //     str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
            // }
            mLogs->updatePro(str,ret); mLogs->writeData(str1, str, str2, ret);
            mLogs->writeDataEng(eng1, eng3, eng2, ret);

            if(ret) {
                str = tr("请将始端箱防雷模块拔出");
                emit TipSig(str); flag = 0;
                while(1)
                {
                    ret = mRead->readData();
                    if(ret) {
                        if(b->lpsState == 2) break;
                    }

                    flag++;
                    if(flag >40){
                        ret = false; break;
                    }
                }
                str1 = tr("检查SPD防雷模块报警状态，报警与恢复状态正常");
                eng1 = tr("Check the alarm status of the SPD lightning protection module, and ensure that the alarm and recovery status are normal");
                str = tr("始端箱防雷模块状态：%1，（模拟）").arg(b->lpsState ==1?tr("工作正常"):tr("损坏"));
                eng3 = tr("Busbar feeder box lightning protection module status:%1, (simulated)").arg(b->lpsState ==1?tr("normal operation"):tr("damage"));

                mLogs->updatePro(str,ret);
                mLogs->writeData(str1, str, str2, ret);
                mLogs->writeDataEng(eng1, eng3, eng2, ret);

                str = tr("始端箱防雷模块检查");
                if(ret) {
                    str += tr("成功");
                }else {
                    str += tr("失败");
                }
                mLogs->updatePro(str,ret);

                str = tr("请将始端箱防雷模块插入原位");
                emit TipSig(str);
                while(1)
                {
                    ret = mRead->readData();
                    if(ret){
                        if(b->lpsState == 1) {
                            break;
                        }
                    }
                    flag++;
                    if(flag >40){
                        ret = false; break;
                    }
                }
            }
        }
    }

    ret = false; str2 = tr("断路器iOF辅助触点检查");
    eng2 = tr("Circuit breaker iOF auxiliary contact inspection");
    curValue = b->iOF;
    expect = mItem->ip.ip_iOF;
    if(curValue == expect) ret = true;
    str = tr("始端箱iOF辅助触点实际值：%1 ， 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    eng3 = tr("Busbar feeder box iOF auxiliary contact actual value:%1, expected value:%2").arg(curValue?tr("have"):tr("nothing")).arg(expect?tr("have"):tr("nothing"));
    mLogs->updatePro(str,ret); flag = 0;

    str1 = tr("依据产品规格书核对始端箱iOF辅助触点");
    eng1 = tr("Verify the iOF auxiliary contacts of the starting box according to the product specifications");
    mLogs->writeData(str1, str, str2, ret); mLogs->writeDataEng(eng1, eng3, eng2, ret);
    str1.clear();

    if(1) {
        str = tr("请将始端箱的断路器断开");
        emit TipSig(str);
        while(1)
        {
            int a=0,d=0,c=0;
            ret = mRead->readData();
            a = b->data.vol.value[0]; d = b->data.vol.value[1]; c = b->data.vol.value[2];
            if(ret) {
                if(b->data.sw[0] == 2) {
                    if((!a)&&(!d) &&(!c)) {
                        for(int i =0;i<b->loopNum;i++)
                        {
                            QString temp = trans(i);
                            str = tr("%1电压 %2V，").arg(temp).arg(b->data.vol.value[i]/COM_RATE_VOL);
                            str1 += str;
                        }
                        mLogs->updatePro(str1, ret);
                        break;
                    }
                }
            }
            flag++;
            if(flag >40){
                ret = false; break;
            }
        }
        str = tr("始端箱的断路器检测");
        str1 = tr("断开断路器状态显示为分闸");
        str2 = tr("断路器检查"); eng2 = tr("Circuit breaker inspection");
        eng1 = tr("The status of the disconnected circuit breaker is displayed as open");
        if(ret) {
            str += tr("成功"); str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
        }else {
            str += tr("失败"); str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
        }
        mLogs->writeData(str1, str3, str2, ret); mLogs->updatePro(str,ret);
        mLogs->writeDataEng(eng1, eng3, eng2, ret);

        str = tr("请将始端箱的断路器闭合");
        emit TipSig(str);
        while(1)
        {
            ret = mRead->readData();
            if(ret) {
                if(b->data.sw[0] == 1) break; //1：合闸   2：分闸   3：跳闸（选配ISD报警触点）
            }
            flag++;
            if(flag >40){
                ret = false; break;
            }
        }
    }

    ret = false; str2 = tr("断路器ISD报警触点检查");
    eng2 = tr("Inspection of ISD alarm contacts for circuit breakers");
    curValue = b->isd;
    expect = mItem->ip.ip_ISD;
    if(curValue == expect) ret = true;
    str = tr("始端箱ISD报警触点实际值：%1 ， 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    eng3 = tr("Busbar feeder box ISD alarm contact actual value:%1, expected value:%2").arg(curValue?tr("have"):tr("nothing")).arg(expect?tr("have"):tr("nothing"));
    mLogs->updatePro(str,ret); flag = 0;

    str1 = tr("依据产品规格书核对始端箱ISD报警触点"); eng1 = tr("Verify the ISD alarm contacts of the starting box according to the product specifications");
    mLogs->writeData(str1, str, str2, ret); mLogs->writeDataEng(eng1, eng3, eng2, ret);

    // if(ret) {
    //     if(curValue == 1) {
    //         emit ImageSig(5);
    //         str = tr("请使用螺丝刀顶一下始端箱上的黄色按钮");
    //         emit TipSig(str);
    //         while(1)
    //         {
    //             ret = mRead->readData();
    //             if(ret) {
    //                 if(b->data.sw[0] == 3) break; //1：合闸   2：分闸   3：跳闸（选配ISD报警触点）
    //             }
    //             flag++;
    //             if(flag >40){
    //                 ret = false; break;
    //             }
    //         }
    //         str1 = tr("螺丝刀按一下黄色测试按钮，检查断路器状态：跳闸");
    //         eng1 = tr("Press the yellow test button with a screwdriver to check the circuit breaker status: tripped");
    //         str = tr("始端箱ISD报警触点检查");
    //         if(ret) {
    //             str += tr("成功"); str3 = tr("符合要求"); eng3 = tr("Meeting the requirements");
    //         }else {
    //             str += tr("失败"); str3 = tr("不符合要求"); eng3 = tr("Not Satisfiable");
    //         }
    //         mLogs->writeData(str1, str3, str2, ret);
    //         mLogs->updatePro(str,ret); mLogs->writeDataEng(eng1, eng3, eng2, ret);
    //         str = tr("请将始端箱的断路器闭合");
    //         emit TipSig(str); emit ImageSig(4);
    //         while(1)
    //         {
    //             ret = mRead->readData();
    //             if(ret) {
    //                 if(b->data.sw[0] == 1) break; //1：合闸   2：分闸   3：跳闸（选配ISD报警触点）
    //             }
    //             flag++;
    //             if(flag >40){
    //                 ret = false; break;
    //             }
    //         }
    //     }
    // }


    ret = false;
    curValue = b->shuntRelease;
    expect = mItem->ip.ip_shunt;
    if(curValue == expect) ret = true;
    str = tr("始端箱分励脱扣实际值：%1 ， 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret); flag = 0;
    if(ret) {
        if(curValue == 1) {
            str = tr("始端箱分励脱扣测试开始");
            emit TipSig(str);
            for(int i=0;i<2;i++)
            {
                Ctrl_SiRtu::bulid()->setBusbarStartShuntRelease(12);
            }
            while(1)
            {
                ret = mRead->readData();
                if(ret) {
                    if(b->data.sw[0] == 3) break;
                }
                flag++;
                if(flag >40){
                    ret = false; break;
                }
            }
            str = tr("始端箱分励脱扣检查");
            str1 = tr("与参数设置一致；若有分励脱扣，则发送命令断开断路器，状态显示为分闸");
            if(ret) str += tr("成功");
            else str += tr("失败");
            // mLogs->writeData(str1, str, ret);
            mLogs->updatePro(str,ret);
            str = tr("请将始端箱的断路器闭合");
            emit TipSig(str);
            while(1)
            {
                ret = mRead->readData();
                if(ret) {
                    if(b->data.sw[0] == 1) break; //1：合闸   2：分闸   3：跳闸（选配ISD报警触点）
                }
                flag++;
                if(flag >40){
                    ret = false; break;
                }
            }
        }
    }

    ret = false;
    curValue = b->reState;
    expect = mItem->ip.ip_residual;
    if(curValue == expect) ret = true;
    str = tr("始端箱剩余电流检测模块实际值：%1 ， 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);

    ret = false;
    for(int i=0; i<b->loopNum; ++i) {
        ret = curAlarmErr(i);
    }

    ret = VolErrRange();
    if(!ret) {
        ret = mRead->readData();
        str = tr("第二次读取数据"); mLogs->updatePro(str,true);
        VolErrRange();
    }

    ret = CurErrRange();
    if(!ret) {
        ret = mRead->readData();
        str = tr("第二次读取数据"); mLogs->updatePro(str,true);
        CurErrRange();
    }

    ret = false;
    str1 = tr("依据产品规格书核对产品的软件烧录版本"); str2 = tr("软件版本");
    eng1 = tr("Verify the software burning version of the product according to the product specification sheet");
    eng2 = tr("Software version");
    curValue = b->version;
    expect = mItem->ip.version;
    if(curValue == expect) ret = true;
    QString curVer = QString::number(curValue/100)+"."+QString::number(curValue/10%10)+"."+QString::number(curValue%10);
    QString expectVer = QString::number(expect/100)+"."+QString::number(expect/10%10)+"."+QString::number(expect%10);
    str = tr("版本信息实际值：%1 , 期待值：%2").arg(curVer).arg(expectVer);
    eng3 = tr("Version information actual value:%1, expected value:%2").arg(curVer).arg(expectVer);

    mLogs->writeData(str1, str ,str2, ret); mLogs->writeDataEng(eng1, eng3, eng2, ret);
    mLogs->updatePro(str,ret);
}

bool Power_CoreThread::curAlarmErr(int i)
{
    QString str = tr("电流报警阈值 L%1 ").arg(i+1);
    bool ret = mErr->curAlarm(i);
    if(ret) str += tr("正常");
    else str += tr("错误");

    return mLogs->updatePro(str, ret);
}

void Power_CoreThread::InsertErrRange()
{
    bool ret = false;

    emit ImageSig(4);
    ret = mRead->readData();
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);    //比较基本配置信息

    ret = false;
    int curValue = b->buzzerStatus;
    int expect = mItem->si.si_buzzer;
    if(curValue == expect) ret = true;
    QString str = tr("插接箱蜂鸣器实际值：%1 , 期待值：%2").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    mItem->si.si_filter = curValue;
    if(curValue >= 5) ret = true;
    str = tr("插接箱过滤次数实际值：%1").arg(curValue);
    if(ret) str += tr("成功");
    else str += tr("失败，实际值小于5");
    mLogs->updatePro(str,ret); ret = false;

    QString str1 = tr("是否包含此功能与规格书要求一致");
    QString str2 = tr("符合要求"); QString str3 = tr("断路器iOF辅助触点检查");
    QString eng1 = tr("Does it include this function that is consistent with the specifications");
    QString eng2 = tr("Meeting the requirements"); QString eng3 = tr("Circuit breaker iOF auxiliary contact inspection");

    curValue = b->iOF;
    expect = mItem->si.si_iOF;
    if(curValue == expect) ret = true;
    str = tr("插接箱iOF辅助触点实际值：%1 , 期待值：%2").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    eng2 = tr("Busbar tap-off box iOF auxiliary contact actual value:%1, expected value:%2").arg(curValue?tr("have"):tr("nothing")).arg(expect?tr("have"):tr("nothing"));

    mLogs->updatePro(str,ret);
    mLogs->writeData(str1, str ,str3, ret); mLogs->writeDataEng(eng1, eng2, eng3, ret);
    ret = false;

    str1 = tr("单相或三相与规格书要求一致"); str3 = tr("输出插座类型");
    eng1 = tr("Single or three-phase in accordance with the specifications"); eng3 = tr("Output socket type");
    curValue = b->phaseFlag;
    expect = mItem->si.si_phaseflag;
    if(curValue == expect) ret = true;
    str = tr("插接箱输出位类型实际值：%1 , 期待值：%2").arg(curValue?tr("三相"):tr("单相")).arg(expect?tr("三相"):tr("单相"));
    eng2 = tr("Busbar tap-off box output bit type actual value:%1, expected value:%2").arg(curValue?tr("three phase"):tr("single phase")).arg(expect?tr("three phase"):tr("single phase"));

    mLogs->updatePro(str,ret);
    mLogs->writeData(str1, str ,str3, ret); mLogs->writeDataEng(eng1, eng2, eng3, ret);
    ret = false;

    str1 = tr("回路数量与规格书要求一致"); str3 = tr("输出回路数量");
    eng1 = tr("The number of circuits is consistent with the requirements of the specification sheet"); eng3 = tr("Number of output circuits");
    curValue = b->loopNum;
    expect = mItem->si.loopNum;
    if(curValue == expect) ret = true;
    str = tr("插接箱回路数量实际值：%1 , 期待值：%2").arg(curValue).arg(expect);
    eng2 = tr("Actual number of plug-in box circuits:%1, expected value:%2").arg(curValue).arg(expect);

    mLogs->updatePro(str,ret);
    mLogs->writeData(str1, str ,str3, ret); mLogs->writeDataEng(eng1, eng2, eng3, ret);
    ret = false;

    for(int i=0; i< b->loopNum; ++i) {
        ret = curAlarmErr(i);
    }

    ret = VolErrRange();
    if(!ret) {
        ret = mRead->readData();
        str = tr("第二次读取数据"); mLogs->updatePro(str,true);
        VolErrRange();
    }

    ret = CurErrRange();
    if(!ret) {
        ret = mRead->readData();
        str = tr("第二次读取数据"); mLogs->updatePro(str,true);
        CurErrRange();
    }

    ret = false;
    str1 = tr("依据产品规格书核对产品的软件烧录版本"); str3 = tr("软件版本");
    eng1 = tr("Verify the software burning version of the product according to the product specification sheet");
    eng3 = tr("Software version");
    curValue = b->version;
    expect = mItem->si.version;
    if(curValue == expect) ret = true;
    QString curVer = QString::number(curValue/100)+"."+QString::number(curValue/10%10)+"."+QString::number(curValue%10);
    QString expectVer = QString::number(expect/100)+"."+QString::number(expect/10%10)+"."+QString::number(expect%10);
    str = tr("版本信息实际值：%1 , 期待值：%2").arg(curVer).arg(expectVer);
    eng2 = tr("Version information actual value:%1, expected value:%2").arg(curVer).arg(expectVer);

    mLogs->updatePro(str,ret);
    mLogs->writeData(str1, str ,str3, ret); mLogs->writeDataEng(eng1, eng2, eng3, ret);

}

bool Power_CoreThread::VolErrRange()
{
    bool ret = true , res = true; QString str;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);
    for(int i=0; i<mBusData->box[mItem->addr - 1].loopNum; ++i) {
        ret = mErr->volErr(i);
        if(mItem->modeId == START_BUSBAR) {
            str = tr("始端箱电压 L%1，实际值：%2V ").arg(i+1).arg(QString::number((b->data.vol.value[i]/COM_RATE_VOL),'f',1));
        }else {
            str = tr("插接箱电压 L%1，实际值：%2V ").arg(i+1).arg(QString::number((b->data.vol.value[i]/COM_RATE_VOL),'f',1));
        }

        if(ret) str += tr("检测正常");
        else str += tr("超出误差范围");

        if(!ret) res = false;
        mLogs->updatePro(str,ret);
    }

    return res;
}

bool Power_CoreThread::CurErrRange()
{
    bool ret = true , res = true; QString str;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);
    for(int i=0; i<mBusData->box[mItem->addr - 1].loopNum; ++i) {
        ret = true ;
        if(mItem->modeId == START_BUSBAR) {
            str = tr("始端箱电流 L%1，实际值：%2A ").arg(i+1).arg(QString::number((b->data.cur.value[i]/COM_RATE_CUR),'f',3));
        }else {
            str = tr("插接箱电流 L%1，实际值：%2A ").arg(i+1).arg(QString::number((b->data.cur.value[i]/COM_RATE_CUR),'f',3));
        }
        if(b->data.cur.value[i]) ret = false;
        if(ret) {
            str += tr("检测正常");
        }else {
            str += tr("检测失败,有底数");
        }
        if(!ret) res = false;
        mLogs->updatePro(str,ret);
    }

    return res;
}

double calculateAverageWithoutHighestAndLowest(QVector<ushort> &numbers) {
    // 如果数组为空，返回0
    if (numbers.isEmpty()) {
        return 0.0;
    }
    // 对数组进行排序
    std::sort(numbers.begin(), numbers.end());

    // 去掉最高和最低的值
    numbers.remove(numbers.size() - 1, 1); // 移除最高值
    numbers.remove(0, 1);                 // 移除最低值

    // 计算剩余数字的平均值
    double sum = std::accumulate(numbers.begin(), numbers.end(), 0.0);
    double average = sum / numbers.size();

    return average;
}

void Power_CoreThread::EnvErrRange()
{
    bool ret = true; QString str, str2, str3 ;
    sDataValue *unit = &(mBusData->box[mItem->addr - 1].env.tem);
    QString str1 = tr("所有值在平均值的±5℃范围内");
    QString eng1 = tr("All values are within the range of ± 5 ℃ of the average value");
    QString eng2 = tr("Temperature value check"); QString eng3;
    str = tr("温度检测");
    str2 = tr("温度模块检测：");
    QVector<ushort> myNumbers = {0};
    for(int i = 0; i<4; i++)
    {
        myNumbers.append(unit->value[i]);
    }
    double average = calculateAverageWithoutHighestAndLowest(myNumbers);

    QList<bool> pass; pass.clear();
    for(int i = 0; i<4; i++)
    {
        str2 += tr("温度%1 %2℃ ").arg(i+1).arg(QString::number(unit->value[i],'f',1));
        str3 += tr("温度%1 %2℃ ").arg(i+1).arg(QString::number(unit->value[i],'f',1));
        eng3 += tr("Temperature%1 %2℃ ").arg(i+1).arg(QString::number(unit->value[i],'f',1));
        if(ret) ret = mErr->checkErrRange(average, unit->value[i], 5.0);
        pass << ret;
    }

    ret = true;
    mLogs->updatePro(str2, ret);
    int i = 0;
    for(; i<pass.size(); ++i)
    {
        if(pass.at(i) == 0) {
            ret = 0; break;
        }
    }
    if(ret) {
        str += tr("成功");
    }else {
        str += tr("失败，温度%1检测异常，请检查该温度传感器是否正常").arg(i+1);
    }

    str2 = tr("温度值检查");
    mLogs->updatePro(str, ret);
    mLogs->writeData(str1, str3, str2, ret); mLogs->writeDataEng(eng1, eng3, eng2, ret);
}

void Power_CoreThread::BaseErrRange()   //比较基本配置信息
{
    if(mItem->modeId == START_BUSBAR) {
        StartErrRange();
    }else {
        InsertErrRange();
    }
}

bool Power_CoreThread::eleErrRange(int i)
{
    bool ret = false;
    if(0 != mBusData->box[mItem->addr-1].data.ele[i]) {
        ret = false;
    }else {
        ret = true;
    }

    return ret;
}

bool Power_CoreThread::eleErrRange0(int i)
{
    QString str = tr("电能 L%1，实测电能=%2kWh").arg(i+1).arg(mBusData->box[mItem->addr-1].data.ele[i]/COM_RATE_ELE);
    bool ret = false;
    if(0 != mBusData->box[mItem->addr-1].data.ele[i]) {
        str += tr("错误");
        ret = false;
    }else {
        str += tr("正常");
        ret = true;
    }

    return mLogs->updatePro(str, ret);
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
    QString str = tr("请将负载输入端L1、L2、L3断开");  //b1,b2,b3
    emit TipSig(str); emit ImageSig(2); sleep(6);

    bool ret = true , res = true;
    uchar tag = mBusData->box[mItem->addr-1].workMode;
    if(mItem->modeId == START_BUSBAR && mBusData->box[mItem->addr-1].workMode ==0){//&& mItem->ip.addr == SNMP
        mRead->SetInfo(mRead->getConnectModeOid(),"1");//切换成RTU模式
    }
    str = tr("正在清除电能");  //b1,b2,b3
    emit TipSig(str);

    ret = mCtrl->eleClean();
    mRead->readDevData();

    int j = 0;
    for(; j<mBusData->box[mItem->addr-1].loopNum; ++j) {
        ret = eleErrRange(j); if(!ret) res = false;
    }
    if(!res) {
        ret = mCtrl->eleClean();
        mRead->readDevData();
    }
    ret = true ; res = true;
    int i = 0;
    for(; i<mBusData->box[mItem->addr-1].loopNum; ++i) {
        ret = eleErrRange0(i); if(!ret) res = false;
    }
    str = tr("清除电能");
    if(res) str += tr("成功"); else str += tr("失败");
    mLogs->updatePro(str, res);
    if(mItem->modeId == START_BUSBAR)
        mRead->SetInfo(mRead->getConnectModeOid(),QString::number(tag));//切换成SNMP模式

    //恢复出厂设置
    if(ret) {
        if(mItem->modeId == START_BUSBAR){//始端箱
            str = tr("始端箱恢复出厂设置");
            ret = Ctrl_SiRtu::bulid()->setBusbarStartRestore(12);
        }else {
            str = tr("插接箱恢复出厂设置");
            Ctrl_SiRtu::bulid()->setBusbarInsertRestore(12);
        }
        mLogs->updatePro(str, res);
    }

    return res;
}

void Power_CoreThread::clearStartEleSlot()
{
    mCtrl->eleClean();

    return;
}

bool Power_CoreThread::printer()
{
    QString method = "Integration/Busbar-Product/Execute";
    QString ip = "192.168.1.16";
    bool ret = true;
    QString str = tr("标签打印 "); QString str1;
    if(mPro->result != Test_Fail){
        sBarTend it;
        QString mPn = mItem->pn;//订单号+成品代码
        QStringList list = mPn.split("+");
        for(int i = 0; i < list.count(); i++)
        {
            if(i == 0) it.on = list.at(i);
            if(i == 1) it.pn = list.at(i);
        }

        QString mSn = mDev->devType.sn;//模块序列号
        it.sn =  mSn.remove(QRegExp("\\s"));

        int ver = get_share_mem()->box[mItem->addr-1].version;//软件版本号
        it.fw = QString::number(ver/100)+"."+QString::number(ver/10%10)+"."+QString::number(ver%10);
        it.hw = "V1.0";//暂时设置默认值
        if(it.sn.isEmpty() || it.fw.isEmpty()){
            mPro->result = Test_Fail;
            ret  = false;
            if(it.sn.isEmpty()) str += tr(" 读取到序列号SN为空 ");
            if(it.fw.isEmpty()) str += tr(" 读取到软件版本FW为空 ");
        }

        if(ret){
            str1 = Printer_BarTender::bulid(this)->http_post(method, ip, it);
            if(str1 == "Success") {
                ret = true;
            }else {
                str1 = Printer_BarTender::bulid(this)->http_post(method, ip, it);
                if(str1 == "Success") {
                    ret = true;
                }else ret = false;
            }
        }
        if(ret) str += tr("正常"); else str += tr("错误");
    } else str = tr("因测试未通过，标签未打印");
    return mPacket->updatePro(str, ret);
}

void Power_CoreThread::workResult(bool)
{
    mLogs->updatePro(tr("测试结束"));
    bool res = false;
    QString str = tr("测试结果 ");
    if(mPro->result != Test_Fail) {
        res = printer();
        if(res)
            str += tr("通过");
        else
            str += tr("失败");
        // res = true;
        // str += tr("通过");

    }else {
        res = false;
        str += tr("失败");
    }
    mPacket->updatePro(str, res);

    mPro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);
    mPro->itemContent << "设备类型：" + mPro->productType;
    mPro->itemContent << "告警滤波次数：" + QString::number(mBusData->box[mItem->addr-1].alarmTime);

    ePro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);
    ePro->itemContent << "Equipment type:" + mPro->productType;
    ePro->itemContent << "Alarm filtering frequency:" + QString::number(mBusData->box[mItem->addr-1].alarmTime);

    if(mCfg->work_mode == 3 && mPro->result != Test_Fail){
        while(1)
        {
            msleep(100);
            if(mPro->issure)
            {
                break;
            }
        }
    }

    mPro->work_mode = 3;
    mLogs->saveLogs();

    mCfg->work_mode = 2; emit finshSig(res);   
    mPro->step = Test_Over;
}

QString Power_CoreThread::trans(int index)
{
    QString str;
    int value = index % 3 + 1;
    switch(value) {
    case 1: str = "A" + QString::number(index / 3+1);
            break;
    case 2: str = "B" + QString::number(index / 3+1);
            break;
    case 3: str = "C" + QString::number(index / 3+1);
            break;
    }

    return str;
}

bool Power_CoreThread::Vol_ctrlOne()
{
    bool ret = true; int flag = 0;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;
    QString str, str1, str2, str3, str4; QString eng2;
    str4 = tr("回路电压检查");
    str = tr("关闭电源输出端L2");  //b1,b2,b3
    emit TipSig(str); emit ImageSig(0);
    QString eng3; QString eng4 = tr("Circuit voltage check");
    if(loop == 9){
        str2 = tr("断开电源输出端断路器L2，读取B1/B2/B3电压为0V，A1/A2/A3/C1/C2/C3正常");
        eng2 = tr("Disconnect the power output circuit breaker L2, read the voltage of B1/B2/B3 as 0V, A1/A2/A3/C1/C2/C3 is normal");
    }else if(loop == 6){
        str2 = tr("断开电源输出端断路器L2，读取B1/B2电压为0V，A1/A2/C1/C2正常");
        eng2 = tr("Disconnect the power output circuit breaker L2, read B1/B2 voltage as 0V, A1/A2/C1/C2 normal");
    }else if(loop == 3){
        str2 = tr("断开电源输出端断路器L2，检查B电压为0V，A/C电压正常");
        eng2 = tr("Disconnect the power output circuit breaker L2 and check that the voltage of B is 0V, and the voltage of A/C is normal");
    }

    while(1)
    {
        int a=0, b=0, c=0;
        ret = mRead->readData();

        if(loop == 9)
        {
            for(int i =0;i<loop;i++)
            {
                mErr->volErr(i);
            }
            for(int i =0;i<9;i+=3)
            {
                a += Obj->vol.status[i]; //  1--成功 2---失败
                b += Obj->vol.value[1+i];
                c += Obj->vol.status[2+i];
            }
            if((!b)&&(a == 3)&&(c == 3)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    QString temp = trans(i);
                    str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str1 += str; eng3 += str3;
                }
                mLogs->updatePro(str1, ret);
                str = tr("B路电压检测成功 "); mLogs->updatePro(str, ret);
                mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                str1.clear(); break;
            }
        }else if(loop == 6) {
            for(int i =0;i<loop;i++)
            {
                mErr->volErr(i);
            }
            for(int i =0;i<6;i+=3)
            {
                a += Obj->vol.status[i];//  1--成功 2---失败
                c += Obj->vol.status[2+i];
                b += Obj->vol.value[1+i];
            }
            if((!b)&&(a == 2)&&(c == 2)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    QString temp;
                    temp = trans(i);
                    str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str1 += str; eng3 += str3;
                }
                mLogs->updatePro(str1, ret);
                str = tr("B路电压检测成功 ");mLogs->updatePro(str, ret);
                mLogs->writeData(str2,str1,str4,ret);
                mLogs->writeDataEng(eng2,eng3,eng4,ret); str1.clear();break;
            }
        }else if(loop == 3){
            for(int i =0;i<loop;i++)
            {
                mErr->volErr(i);
            }
            a = Obj->vol.status[0];
            b = Obj->vol.value[1];
            c = Obj->vol.status[2];
            if((!b)&&(a == 1)&&(c == 1)) {
                ret = true;
                for(int i =0;i<loop;i++)
                {
                    QString temp = trans(i);
                    str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                    str1 += str; eng3 += str3;
                }
                mLogs->updatePro(str1, ret);
                str = tr("B路电压检测成功 "); mLogs->updatePro(str, ret);
                mLogs->writeData(str2,str1,str4,ret);
                mLogs->writeDataEng(eng2,eng3,eng4,ret); str1.clear();break;}
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            ret = false;
            if(mItem->modeId == START_BUSBAR)
                str = tr("B路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->ip.ip_vol).arg(mItem->ip.ip_volErr);
            else str = tr("B路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->si.si_vol).arg(mItem->si.si_volErr);
            mLogs->updatePro(str, ret);
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit TipSig(str); sleep(3);
            mLogs->writeData(str2,str1,str4,ret);
            mLogs->writeDataEng(eng2,eng3,eng4,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::Vol_ctrlTwo()
{
    bool ret = true; int flag = 0; QString str1,str2,str3,str4;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    str4 = tr("回路电压检查"); QString eng2;
    QString str = tr("打开电源输出端L2，关闭电源输出端L3");//c1,c2,c3
    emit TipSig(str); emit ImageSig(0);
    QString eng3; QString eng4 = tr("Circuit voltage check");

    if(loop == 9){
        str2 = tr("断开电源输出端断路器L3，读取C1/C2/C3电压为0V ，A1/A2/A3/B1/B2/B3正常");
        eng2 = tr("Disconnect circuit breaker L3 at the power output terminal, read the voltage of C1/C2/C3 as 0V, A1/A2/A3/B1/B2/B3 is normal");
    }else if(loop == 6){
        str2 = tr("断开电源输出端断路器L3，读取C1/C2电压为0V，A1/A2/B1/B2/正常");
        eng2 = tr("Disconnect circuit breaker L3 at the power output terminal, read C1/C2 voltage as 0V, A1/A2/B1/B2/normal");
    }else if(loop == 3){
        str2 = tr("断开电源输出端断路器L3，检查C电压为0V，A/B电压正常");
        eng2 = tr("Disconnect the power output circuit breaker L3 and check that the voltage of C is 0V, and the voltage of A/B is normal");
    }

    while(1)
    {
        int a=0, b=0, c=0;
        ret = mRead->readData();
        if(ret) {
            if(loop == 9)
            {
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                for(int i =0;i<9;i+=3)
                {
                    a += Obj->vol.status[i];
                    b += Obj->vol.status[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!c)&&(a == 3)&&(b == 3)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3 += str3;
                    }
                    mLogs->updatePro(str1, ret);
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }else if(loop == 6) {
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.status[i];
                    b += Obj->vol.status[1+i];
                    c += Obj->vol.value[2+i];
                }
                if((!c)&&(a == 2)&&(b == 2)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3 += str3;
                    }
                    mLogs->updatePro(str1, ret);              
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }else if(loop == 3){
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                a = Obj->vol.status[0];
                b = Obj->vol.status[1];
                c = Obj->vol.value[2];
                if((!c)&&(a == 1)&&(b== 1)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3 += str3;
                    }
                    mLogs->updatePro(str1, ret);
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }
        }
        flag++;
        if(flag >50) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3 += str3;
            }
            mLogs->updatePro(str1, ret);
            ret = false;

            if(mItem->modeId == START_BUSBAR)
                str = tr("C路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->ip.ip_vol).arg(mItem->ip.ip_volErr);
            else str = tr("C路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->si.si_vol).arg(mItem->si.si_volErr);
            mLogs->updatePro(str, ret);
            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit TipSig(str); sleep(3);
            mLogs->writeData(str2,str1,str4,ret);
            mLogs->writeDataEng(eng2,eng3,eng4,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::Vol_ctrlThree()
{
    bool ret = true; int flag = 0; QString str1,str2,str3,str4;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    str4 = tr("回路电压检查");
    QString str = tr("打开电源输出端L3");//c1,c2,c3
    emit TipSig(str); emit ImageSig(0); QString eng2;
    QString eng3; QString eng4 = tr("Circuit voltage check");

    while(1)
    {
        int a=0, b=0, c=0;
        ret = mRead->readData();
        if(ret) {
            if(loop == 9)
            {
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                for(int i =0;i<9;i+=3)
                {
                    a += Obj->vol.status[i];
                    b += Obj->vol.status[1+i];
                    c += Obj->vol.status[2+i];
                }
                if((c == 3)&&(a == 3)&&(b == 3)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3+= str3;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    eng2 = tr("Close the circuit breaker and ensure that the voltage of each circuit is normal");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }else if(loop == 6) {
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                for(int i =0;i<6;i+=3)
                {
                    a += Obj->vol.status[i];
                    b += Obj->vol.status[1+i];
                    c += Obj->vol.status[2+i];
                }
                if((c == 2)&&(a == 2)&&(b == 2)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3+= str3;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    eng2 = tr("Close the circuit breaker and ensure that the voltage of each circuit is normal");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }else if(loop == 3){
                for(int i =0;i<loop;i++)
                {
                    mErr->volErr(i);
                }
                a = Obj->vol.status[0];
                b = Obj->vol.status[1];
                c = Obj->vol.status[2];
                if((c == 1)&&(a == 1)&&(b== 1)) {
                    ret = true;
                    for(int i =0;i<loop;i++)
                    {
                        QString temp = trans(i);
                        str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                        str1 += str; eng3+= str3;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    eng2 = tr("Close the circuit breaker and ensure that the voltage of each circuit is normal");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str1,str4,ret); mLogs->writeDataEng(eng2,eng3,eng4,ret);
                    str1.clear(); break;
                }
            }
        }
        flag++;
        if(flag >55) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str3 = tr("%1voltage %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str; eng3+= str3;
            }
            mLogs->updatePro(str1, ret);
            ret = false;
            if(mItem->modeId == START_BUSBAR)
                str = tr("A路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->ip.ip_vol).arg(mItem->ip.ip_volErr);
            else str = tr("A路电压检测失败，超出误差范围，设置的电压 %1V，误差 %2V").arg(mItem->si.si_vol).arg(mItem->si.si_volErr);
            mLogs->updatePro(str, ret);

            str = tr("电压超出误差范围，请到参数设置页面检查产线测试电流和误差是否设置合适");
            emit TipSig(str); sleep(3);
            str2 = tr("闭合断路器，各回路电压正常");
            eng2 = tr("Close the circuit breaker and ensure that the voltage of each circuit is normal");
            mLogs->writeData(str2,str1,str4,ret);
            mLogs->writeDataEng(eng2,eng3,eng4,ret);str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::stepVolTest()
{
    bool ret = true;
    if((mItem->modeId == START_BUSBAR) || (mBusData->box[mItem->addr-1].phaseFlag ==1))
    {
        QString str = tr("请将电源输出端L1、L2、L3打开");
        emit TipSig(str); sleep(4);
        ret = Vol_ctrlOne();
        if(ret) ret = Vol_ctrlTwo();
        if(ret) ret = Vol_ctrlThree();
    }
    emit ImageSig(4);

    return ret;
}

bool Power_CoreThread::stepLoadTest()       //电流测试
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
bool Power_CoreThread::BreakerTest()        //断路器测试
{
    bool ret = true;
    if(mBusData->box[mItem->addr-1].loopNum == 9) {
        ret = mRead->Break_NineLoop();
    }else if(mBusData->box[mItem->addr-1].loopNum == 6) {
        ret = mRead->Break_SixLoop();
    }else if((mItem->modeId != START_BUSBAR)&&(mBusData->box[mItem->addr-1].loopNum == 3)) {
        if(mBusData->box[mItem->addr-1].phaseFlag == 1) {
            ret = mRead->Three_Breaker();
        }else if(mBusData->box[mItem->addr-1].phaseFlag == 0) {    //单相三回路三个输出位
            ret = mRead->Three_Break();
        }
    }

    return ret;
}
void Power_CoreThread::getDelaySlot()
{
    QString str, eng; bool ret = false;
    if(mCurBoxNum == 0){
        str = tr("始端箱未发出命令");
    }else if(mCurBoxNum >= 2 && mCurBoxNum <= 3 ){
        str = tr("设置地址失败");
    }
    emit TipSig(str);
    mLogs->updatePro(str, ret);

    if(mCurBoxNum == 2) {
        str = tr("插接箱 IN口接错"); eng = tr("The IN port of the busbar tap-off box is connected incorrectly");
    }
    if(mCurBoxNum == 3) {
        str = tr("插接箱 OUT口接错"); eng = tr("The OUT port of the busbar tap-off box is connected incorrectly");
    }
    emit TipSig(str);
    mLogs->updatePro(str, ret);
    QString str1 = tr("级联治具IN口与被测插接箱IN口直连，OUT口与OUT口直连，级联通讯测试正常");
    QString str2 = tr("IN/OUT接口检查");
    QString eng1 = tr("The IN port of the cascaded fixture is directly connected to the IN port of the tested plug-in box, and the OUT port is directly connected to the OUT port. The cascaded communication test is normal");
    QString eng2 = tr("IN/OUT interface check");

    mLogs->writeData(str1,str,str2,ret); mLogs->writeDataEng(eng1,eng,eng2,ret);
    mCurBoxNum = 0;
}

void Power_CoreThread::getNumAndIndexSlot(int curnum)
{
    int num = curnum >= 2 ? curnum : 2;
    bool ret = false;
    mCurBoxNum = num;
    QString str = tr("串口测试成功，分配地址正常");
    QString eng1 = tr("The IN port of the cascaded fixture is directly connected to the IN port of the tested plug-in box, and the OUT port is directly connected to the OUT port. The cascaded communication test is normal");
    QString eng2 = tr("IN/OUT interface check");
    if(num > 3){
        ret = true;
        emit TipSig(str);
        mLogs->updatePro(str, ret);
        str = tr("分配地址正常"); QString eng = tr("Address allocation is normal");
        QString str1 = tr("级联治具IN口与被测插接箱IN口直连，OUT口与OUT口直连，级联通讯测试正常");
        QString str2 = tr("IN/OUT接口检查");
        mLogs->writeData(str1,str,str2,ret); mLogs->writeDataEng(eng1,eng,eng2,ret);
    }
}

void Power_CoreThread::workDown()
{
    mPro->step = Test_Start;
    bool ret = true;
    ret = initDev();
    if(ret) ret = mRead->readDev();
    if(mItem->modeId == INSERT_BUSBAR)
    {
        sBoxData *b = &(mBusData->box[mItem->addr - 1]);
        if((b->phaseFlag == mItem->si.si_phaseflag) && (b->loopNum == mItem->si.loopNum))
        {
            ret = true;
        }else {
            QString str = tr("请确认规格书与该工具参数设置的输出位类型、回路数量值是否一致");
            emit TipSig(str); sleep(2); ret = false; mLogs->updatePro(str, ret);
        }
    }

    if(ret){
        int ver = get_share_mem()->box[mItem->addr-1].version;
        mPro->softwareVersion = "V" +QString::number(ver/100)+"."+QString::number(ver/10%10)+"."+QString::number(ver%10);
        ePro->softwareVersion = mPro->softwareVersion;

        BaseErrRange();                                 //检查IN OUT口 网口对比始端箱/插接箱基本信息
        EnvErrRange();                                  //温度模块检测

        if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(),"0");
        else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(0); //设置滤波=0

        if(ret) ret = BreakerTest();                            //断路器测试
        if(ret) ret = stepVolTest();                            //电压测试

      // if(ret) ret = mSource->read();
      // else mPro->result = Test_Fail;
      // if(ret) ret = checkLoadErrRange();

        if(ret) ret = stepLoadTest();               //电流测试
        if(ret) ret = factorySet(); sleep(2);                      //清除电能

        QString str = tr("请将电源输出端L1、L2、L3关闭");
        emit TipSig(str); emit ImageSig(2);

        mCfg->work_mode = 3;
        if(ret) emit JudgSig(); //极性测试弹窗

        if(mItem->modeId == START_BUSBAR)
        {
            int temp = mItem->ip.ip_filter;
            mRead->SetInfo(mRead->getFilterOid(),QString::number(temp));
        }else {
            int temp = mItem->si.si_filter;
            Ctrl_SiRtu::bulid()->setBusbarInsertFilter(temp); //设置滤波
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
