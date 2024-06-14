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
    }else {
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
    bool ret = false; QString str ; int flag = 0;
    sBoxData *b = &(mBusData->box[mItem->addr - 1]);
    QString str1 = tr("串口线接入IN接口，Modbus RTU串口命令正常返回，IN接口通讯正常");
    QString str2 = tr("接口检查"); QString str3 = tr("符合要求");
    str = tr("请将串口线接入到始端箱IN口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3);
    while(1)
    {
        ret = mSiRtu->readPduData();
        if(ret) break;

        flag++;
        if(flag >60){            
            ret = false; break;
        }
    }
    str = tr("始端箱IN口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求");
    }else {
        str += tr("失败"); str3 = tr("不符合要求");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);

    str = tr("请将串口线接入到始端箱OUT口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3);
    while(1)
    {
        ret = mSiRtu->readPduData();
        if(ret) break;

        flag++;
        if(flag >60){
            ret = false; break;
        }
    }
    str1 = tr("串口线 接入OUT接口，Modbus RTU串口命令正常返回，OUT接口通讯正常");
    str = tr("始端箱OUT口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求");
    }else {
        str += tr("失败"); str3 = tr("不符合要求");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);

    str = tr("请将网线接入到始端箱NET口");           //IN OUT口通讯正常，及网口
    emit TipSig(str); sleep(3);
    while(1)
    {
        ret = mRead->readData();
        if(ret) break;

        flag++;
        if(flag >60){
            ret = false; break;
        }
    }
    str1 = tr("ping功能正常，NET接口通讯正常");
    str = tr("始端箱NET口通讯");
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求");
    }else {
        str += tr("失败"); str3 = tr("不符合要求");
    }
    mLogs->updatePro(str, ret); mLogs->writeData(str1, str3, str2, ret);

    ret = false;
    int curValue = b->buzzerStatus;
    int expect = mItem->ip.ip_buzzer;
    if(curValue == expect) ret = true;
    qDebug()<<"curValue == expect"<<curValue << expect<< ret;
    str = tr("始端箱蜂鸣器实际值：%1 ， 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret); ret = false; //mLogs->writeData(str1, str, ret);

    curValue = b->alarmTime;
    expect = mItem->ip.ip_filter;
    if(curValue == expect) ret = true;
    str = tr("始端箱过滤次数实际值：%1 ， 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret); ret = false;

    str1 = tr("检查SPD防雷模块接入状态，接入正常");
    str2 = tr("SPD防雷检查");
    curValue = b->lightning;
    expect = mItem->ip.ip_lightning;
    if(curValue == expect) ret = true;
    str = tr("始端箱防雷模块实际值：%1 ， 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
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
                if(flag >60){
                    ret = false; break;
                }
            }
            str = tr("始端箱防雷模块状态：%1").arg(b->lpsState ==1?tr("工作正常"):tr("损坏"));
            if(ret) {
                str3 = tr("符合要求");
            }else {
                str3 = tr("不符合要求");
            }
            mLogs->updatePro(str,ret); mLogs->writeData(str1, str3, str2, ret);
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
                    if(flag >60){
                        ret = false; break;
                    }
                }
                str1 = tr("检查SPD防雷模块报警状态，报警与恢复状态正常");
                str = tr("始端箱防雷模块状态：%1").arg(b->lpsState ==1?tr("工作正常"):tr("损坏"));
                mLogs->updatePro(str,ret);

                str = tr("始端箱防雷模块检查");
                if(ret) {
                    str += tr("成功"); str3 = tr("符合要求");
                }else {
                    str += tr("失败"); str3 = tr("不符合要求");
                }
                mLogs->updatePro(str,ret); mLogs->writeData(str1, str3, str2, ret);

                str = tr("请将始端箱防雷模块插入原位");
                emit TipSig(str); sleep(2);
            }
        }
    }

    ret = false; str2 = tr("断路器iOF辅助触点检查");
    curValue = b->iOF;
    expect = mItem->ip.ip_iOF;
    if(curValue == expect) ret = true;
    str = tr("始端箱iOF辅助触点实际值：%1 ， 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret); flag = 0;
    if(ret) {
        str3 = tr("符合要求");
    }else {
        str3 = tr("不符合要求");
    }
    str1 = tr("依据产品规格书核对始端箱iOF辅助触点");
    mLogs->writeData(str1, str3, str2, ret);
    if(ret) {
        if(curValue == 1) {
            str = tr("请将始端箱的断路器断开");
            emit TipSig(str);
            while(1)
            {
                ret = mRead->readData();
                if(ret) {
                    if(b->data.sw[0] == 2) break;
                }
                flag++;
                if(flag >60){
                    ret = false; break;
                }
            }
            str = tr("始端箱iOF辅助触点检查");
            str1 = tr("断开断路器状态显示为分闸");
            if(ret) {
                str += tr("成功"); str3 = tr("符合要求");
            }else {
                str += tr("失败"); str3 = tr("不符合要求");
            }
            mLogs->writeData(str1, str3, str2, ret); mLogs->updatePro(str,ret);
            str = tr("请将始端箱的断路器闭合");
            emit TipSig(str); sleep(2);
        }
    }

    ret = false; str2 = tr("断路器ISD报警触点检查");
    curValue = b->isd;
    expect = mItem->ip.ip_ISD;
    if(curValue == expect) ret = true;
    str = tr("始端箱ISD报警触点实际值：%1 ， 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret); flag = 0;
    if(ret) {
        str3 = tr("符合要求");
    }else {
        str3 = tr("不符合要求");
    }
    str1 = tr("依据产品规格书核对始端箱ISD报警触点");
    mLogs->writeData(str1, str3, str2, ret);
    if(ret) {
        if(curValue == 1) {
            str = tr("请使用螺丝刀顶一下始端箱上的红色按钮");
            emit TipSig(str);
            while(1)
            {
                ret = mRead->readData();
                if(ret) {
                    if(b->data.sw[0] == 3) break; //1：合闸   2：分闸   3：跳闸（选配ISD报警触点）
                }
                flag++;
                if(flag >60){
                    ret = false; break;
                }
            }
            str1 = tr("螺丝刀按一下黄色测试按钮，检查断路器状态：跳闸");
            str = tr("始端箱ISD报警触点检查");
            if(ret) {
                str += tr("成功"); str3 = tr("符合要求");
            }else {
                str += tr("失败"); str3 = tr("不符合要求");
            }
            mLogs->writeData(str1, str3, str2, ret);
            mLogs->updatePro(str,ret);
            str = tr("请将始端箱的断路器闭合");
            emit TipSig(str); sleep(2);
        }
    }

    ret = false;
    curValue = b->shuntRelease;
    expect = mItem->ip.ip_shunt;
    if(curValue == expect) ret = true;
    str = tr("始端箱分励脱扣实际值：%1 ， 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);
    if(ret) {
        if(curValue == 1) {
            for(int i=0;i<2;i++)
                mRead->SetInfo(mRead->setShuntReleaseCtrlOid(),"12");
            while(1)
            {
                ret = mRead->readData();
                if(ret) {
                    if(b->data.sw[0] == 3) break;
                }
                flag++;
                if(flag >60){
                    ret = false; break;
                }
            }
            str = tr("始端箱分励脱扣检查");
            str1 = tr("与参数设置一致；若有分励脱扣，则发送命令断开断路器，状态显示为分闸");
            if(ret) str += tr("成功");
            else str += tr("失败");
            // mLogs->writeData(str1, str, ret);
            mLogs->updatePro(str,ret);
        }
    }

    ret = false;
    curValue = b->reState;
    expect = mItem->ip.ip_residual;
    if(curValue == expect) ret = true;
    str = tr("始端箱剩余电流检测模块实际值：%1 ， 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    mLogs->updatePro(str,ret);

    ret = false;
    for(int i=0; i<mBusData->box[mItem->addr - 1].loopNum; ++i) {
        ret = curAlarmErr(i);
    }
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
    QString str = tr("开始测试插接箱串口通讯");           //自动分配地址
    emit TipSig(str); sleep(2);

    str = tr("请将插接箱IN口与测试治具IN口对接，OUT口与测试治具OUT口对接");
    emit TipSig(str);

    mModbus->autoSetAddress();

    sBoxData *b = &(mBusData->box[mItem->addr - 1]);    //比较基本配置信息

    int curValue = b->buzzerStatus;
    int expect = mItem->si.si_buzzer;
    if(curValue == expect) ret = true;
    str = tr("插接箱蜂鸣器实际值：%1 , 期待值：%2！").arg(curValue?tr("关闭"):tr("开启")).arg(expect?tr("关闭"):tr("开启"));
    mLogs->updatePro(str,ret);ret = false;

    curValue = b->alarmTime;
    expect = mItem->si.si_filter;
    if(curValue == expect) ret = true;
    str = tr("插接箱过滤次数实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    mLogs->updatePro(str,ret); ret = false;

    QString str1 = tr("是否包含此功能与规格书要求一致");
    QString str2 = tr("符合要求"); QString str3 = tr("断路器iOF辅助触点检查");
    curValue = b->iOF;
    expect = mItem->si.si_iOF;
    if(curValue == expect) ret = true;
    str = tr("插接箱iOF辅助触点实际值：%1 , 期待值：%2！").arg(curValue?tr("有"):tr("无")).arg(expect?tr("有"):tr("无"));
    if(ret) {
        str2 = tr("符合要求");
    }else {
        str2 = tr("不符合要求");
    }
    mLogs->writeData(str1, str2 ,str3, ret);
    mLogs->updatePro(str,ret);ret = false;

    str1 = tr("单相或三相与规格书要求一致"); str3 = tr("输出插座类型");
    curValue = b->phaseFlag;
    expect = mItem->si.si_phaseflag;
    if(curValue == expect) ret = true;
    str = tr("插接箱输出位类型实际值：%1 , 期待值：%2！").arg(curValue?tr("三相"):tr("单相")).arg(expect?tr("三相"):tr("单相"));
    if(ret) {
        str2 = tr("符合要求");
    }else {
        str2 = tr("不符合要求");
    }
    mLogs->writeData(str1, str2 ,str3, ret);
    mLogs->updatePro(str,ret);ret = false;

    str1 = tr("回路数量与规格书要求一致"); str3 = tr("输出回路数量");
    curValue = b->loopNum;
    expect = mItem->si.loopNum;
    if(curValue == expect) ret = true;
    str = tr("插接箱回路数量实际值：%1 , 期待值：%2！").arg(curValue).arg(expect);
    if(ret) {
        str2 = tr("符合要求");
    }else {
        str2 = tr("不符合要求");
    }
    mLogs->writeData(str1, str2 ,str3, ret);
    mLogs->updatePro(str,ret);ret = false;

    ret = false;
    for(int i=0; i<mBusData->box[mItem->addr - 1].loopNum; ++i) {
        ret = curAlarmErr(i);
    }
}

void Power_CoreThread::EnvErrRange()
{
    bool ret = true; QString str, str2, str3 ;
    sDataValue *unit = &(mBusData->box[mItem->addr - 1].env.tem);
    qDebug()<<"EnvErrRange"<<mItem->addr;
    QString str1 = tr("所有值在平均值的±5℃范围内");
    str = tr("温度检测");
    str2 = tr("温度模块检测：");
    ushort tem = 0;
    for(int i = 0; i<4; i++)
    {
        tem += unit->value[i];
    }

    ushort ave = tem / 4.0;
    QList<bool> pass; pass.clear();
    for(int i = 0; i<4; i++)
    {
        str2 += tr("温度%1 %2℃ ").arg(i+1).arg(unit->value[i]);
        if(ret) ret = mErr->checkErrRange(ave, unit->value[i], 5.0);
        pass << ret;
    }
    ret = true;
    mLogs->updatePro(str2, ret);
    for(int i=0; i<pass.size(); ++i)
    {
        if(pass.at(i) == 0) {
            ret = 0; break;
        }
    }
    if(ret) {
        str += tr("成功"); str3 = tr("符合要求");
    }else {
        str += tr("失败"); str3 = tr("不符合要求");
    }
    str2 = tr("温度值检查");
    mLogs->writeData(str1, str3, str2, ret);
    mLogs->updatePro(str, ret);
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
    }else {
        res = false;
        str += tr("失败");
    }
    mPacket->updatePro(str, res);
    mPro->loopNum = QString::number(mBusData->box[mItem->addr-1].loopNum);

    mPro->itemContent << "模块序列号：" + mPro->moduleSN;
    mPro->itemContent << "设备类型：" + mPro->productType;
    mPro->itemContent << "告警滤波次数：" + QString::number(mBusData->box[mItem->addr-1].alarmTime);

    if(mCfg->work_mode == 3){
        while(1)
        {
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
    QString str, str1, str2, str3, str4;
    str3 = tr("符合要求"); str4 = tr("回路电压检查");
    str = tr("关闭电源输出端L2");  //b1,b2,b3
    emit TipSig(str); emit ImageSig(0);

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
                    str1 += str;
                }
                mLogs->updatePro(str1, ret);
                str2 = tr("断开电源输出端断路器L2，读取B1/B2/B3电压为0V，A1/A2/A3/C1/C2/C3正常");
                str = tr("B路电压检测成功 "); mLogs->updatePro(str, ret);
                mLogs->writeData(str2,str3,str4,ret);
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
                    str1 += str;
                }
                mLogs->updatePro(str1, ret);
                str2 = tr("断开电源输出端断路器L2，读取B1/B2电压为0V，A1/A2/C1/C2正常");
                str = tr("B路电压检测成功 ");mLogs->updatePro(str, ret);
                str += str1; mLogs->writeData(str2,str3,str4,ret); str1.clear();break;}
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
                    str1 += str;
                }
                mLogs->updatePro(str1, ret);
                str2 = tr("断开电源输出端断路器L2，检查B电压为0V，A/C电压正常");
                str = tr("B路电压检测成功 "); mLogs->updatePro(str, ret);
                str += str1; mLogs->writeData(str2,str3,str4,ret); str1.clear();break;}
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str3 = tr("不符合要求");
            str = tr("B路电压检测失败"); mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::Vol_ctrlTwo()
{
    bool ret = true; int flag = 0; QString str1,str2,str3,str4;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    str3 = tr("符合要求"); str4 = tr("回路电压检查");
    QString str = tr("打开电源输出端L2，关闭电源输出端L3");//c1,c2,c3
    emit TipSig(str); emit ImageSig(0);
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("断开电源输出端断路器L3，读取C1/C2/C3电压为0V ，A1/A2/A3/B1/B2/B3正常");
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("断开电源输出端断路器L3，读取C1/C2电压为0V，A1/A2/B1/B2/正常");
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("断开电源输出端断路器L3，检查C电压为0V，A/B电压正常");
                    str = tr("C路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
                }
            }
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str3 = tr("不符合要求");
            str = tr("C路电压检测失败");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::Vol_ctrlThree()
{
    bool ret = true; int flag = 0; QString str1,str2,str3,str4;
    sObjectData *Obj = &(mBusData->box[mItem->addr - 1].data);
    uchar loop = mBusData->box[mItem->addr-1].loopNum;

    str3 = tr("符合要求"); str4 = tr("回路电压检查");
    QString str = tr("打开电源输出端L3");//c1,c2,c3
    emit TipSig(str); emit ImageSig(0);
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
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
                        str1 += str;
                    }
                    mLogs->updatePro(str1, ret);
                    str2 = tr("闭合断路器，各回路电压正常");
                    str = tr("A路电压检测成功 ");mLogs->updatePro(str, ret);
                    mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
                }
            }
        }
        flag++;
        if(flag >40) {
            for(int i =0;i<loop;i++)
            {
                QString temp = trans(i);
                str = tr("%1电压 %2V，").arg(temp).arg(Obj->vol.value[i]/COM_RATE_VOL);
                str1 += str;
            }
            mLogs->updatePro(str1, ret);
            ret = false; str3 = tr("不符合要求");
            str = tr("A路电压检测失败");mLogs->updatePro(str, ret);
            str += str1; mLogs->writeData(str2,str3,str4,ret); str1.clear(); break;
        }
    }

    return ret;
}

bool Power_CoreThread::stepVolTest()
{
    bool ret = true;
    if(mPro->stopFlag == 0) {   //失败即停止测试
        if((mItem->modeId == START_BUSBAR) || (mBusData->box[mItem->addr-1].phaseFlag ==1))
        {
            if(ret) ret = Vol_ctrlOne();
            if(ret) ret = Vol_ctrlTwo();
            if(ret) ret = Vol_ctrlThree();
        }
    }else {
        if((mItem->modeId == START_BUSBAR) || (mBusData->box[mItem->addr-1].phaseFlag ==1))
        {
            ret = Vol_ctrlOne();
            ret = Vol_ctrlTwo();
            ret = Vol_ctrlThree();
        }
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
        str = tr("始端箱未发出命令");
    }else if(mCurBoxNum >= 2 && mCurBoxNum <= 3 ){
        str = tr("设置地址失败");
    }
    emit TipSig(str);
    mLogs->updatePro(str, ret);

    if(mCurBoxNum == 2) str = tr("插接箱 IN口接错");
    if(mCurBoxNum == 3) str = tr("插接箱 OUT口接错");
    emit TipSig(str);
    mLogs->updatePro(str, ret);
    QString str1 = tr("级联治具IN口与被测插接箱IN口直连，OUT口与OUT口直连，级联通讯测试正常");
    QString str2 = tr("IN/OUT接口检查");
    mLogs->writeData(str1,str,str2,ret);
    mCurBoxNum = 0;
}

void Power_CoreThread::getNumAndIndexSlot(int curnum)
{
    int num = curnum >= 2 ? curnum : 2;
    bool ret = false;
    mCurBoxNum = num;
    QString str = tr("串口测试成功，分配地址正常");
    if(num > 3){
        ret = true;
        emit TipSig(str);
        mLogs->updatePro(str, ret);
        str = tr("符合要求");
        QString str1 = tr("级联治具IN口与被测插接箱IN口直连，OUT口与OUT口直连，级联通讯测试正常");
        QString str2 = tr("IN/OUT接口检查");
        mLogs->writeData(str1,str,str2,ret);
    }
}

void Power_CoreThread::workDown()
{
    mPro->step = Test_Start;
    bool ret = false; sBoxData *b = &(mBusData->box[mItem->addr - 1]);
    loopNum = mBusData->box[mItem->addr-1].loopNum;
    ret = initDev();

    if(mItem->modeId == INSERT_BUSBAR)
    {
        if(ret) ret = mRead->readDev();
        if((b->phaseFlag == mItem->si.si_phaseflag) && (b->loopNum == mItem->si.loopNum))
        {
            ret = true;
        }else {
            QString str = tr("请确认规格书参数与工具参数设置的数据是否一致");
            emit TipSig(str); sleep(2); ret = false;
        }
    }
    if(ret) {
        if(mCfg->work_mode == 2) {
            BaseErrRange();                                 //检查IN OUT口 网口对比始端箱/插接箱基本信息
            EnvErrRange();                                  //温度模块检测

            if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(),"0");
            else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(0); //设置滤波=0

            if(ret) ret = stepVolTest();                    //电压测试
        }else if(mCfg->work_mode == 3) {
          // if(ret) ret = mSource->read();
          // else mPro->result = Test_Fail;
          // if(ret) ret = checkLoadErrRange();

            if(mItem->modeId == START_BUSBAR) mRead->SetInfo(mRead->getFilterOid(),"0");
            else Ctrl_SiRtu::bulid()->setBusbarInsertFilter(0); //设置滤波=0

            if(ret) ret = stepLoadTest();                    //电流+断路器测试
            if(ret) ret = factorySet();                      //清除电能
            if(ret) emit JudgSig();                                //极性测试弹窗
        }
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
