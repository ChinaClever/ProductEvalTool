/*
 *
 *
 *  Created on: 2018年10月1日
 *      Author: Lzy
 */
#include "test_safety.h"
#define SERIAL_TIMEOUT 1000  // 1000MS

int mStep;
bool Breaker;

Test_safety::Test_safety(QObject *parent) : QThread(parent)
{
    mItem = TestConfig::bulid()->item;
    mTrans = new Test_TransThread(this);
    mPacket = sDataPacket::bulid();
    mPacketEng = datapacket_English::bulid();
    mRead = Power_DevRead::bulid(this);
    mPro = mPacket->getPro();
    ePro = mPacketEng->getPro();
    mCfg = Cfg::bulid()->item;
    Breaker = true;

    // timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &Test_safety::timeoutDone);

}

Test_safety::~Test_safety()
{
    quit();
    terminate();
    wait();
}

bool Test_safety::testReady()
{
    bool ret = true;
    sTestDataItem item;
    item.item = tr("测试前准备");

    return ret;
}


void Test_safety::startThread()
{
    mItemId = 1;
    mTestStep = 1;
    mStep = 1;

    start();

}

void Test_safety::stopThread()
{
    quit();
    terminate();
    wait();
}

void Test_safety::conditionExec(bool s)
{
    if(mItem->mode == Test_Pause) {
        do{
            msleep(1);
        } while(mItem->mode == Test_Pause);
    }

    if(mItem->mode == Test_Complete){
        stopThread();
    }

    if(mItem->sn.errStop) {
        if(!s) stopThread();
    }
}

void Test_safety::updateProgress(bool status, QString &str)
{
    sTestProgress *p = &(mItem->progress);
    if(status) {
        p->okNum++;
    } else {
        p->errNum++;
    }
    p->finishNum++;
    p->status = tr("测试项:%1  %2").arg(p->finishNum).arg(str);
    mPacket->updatePro(str, status);
    msleep(100);
}

bool Test_safety::appendResult(sTestDataItem &item)
{
    QString str = tr("失败");
    if(item.status) {
        str = tr("成功");
    }
    item.id = mItemId++;
    mTestStep++;
    item.result = str;
    mItem->dataItem.append(item);
    conditionExec(item.status);
    QString statusStr = item.subItem + " 结果: " + str;

    updateProgress(item.status, statusStr);

    return item.status;
}

void Test_safety::delayItem(sTestDataItem &item, int s)
{
    item.subItem = tr("延时%1ms").arg(s);
    item.expect = "";
    item.measured = "";
    item.status = true;
    appendResult(item);
    msleep(s);
}



bool Test_safety::startTest(sTestDataItem &item,QString & recv , const QString &test , int step , int &stepTotal)
{
    QString sendStr = "";

    mTestStep = Reset;
    item.subItem = tr("%1复位").arg(test);
    item.status = true;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//RESET+回车 连接命令 1
    appendResult(item);

    mTestStep = Reset;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//RESET+回车 连接命令 1
    appendResult(item);

    mTestStep = ConnectReady;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//*IDN?+回车 连接命令 1
    item.subItem = tr("握手");
    item.status = !recv.isEmpty();
    appendResult(item);

    mTestStep = ConnectReady;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//*IDN?+回车 连接命令 1
    item.status = !recv.isEmpty();
    appendResult(item);

    mTestStep = ChoseeFile;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr , step);//FL 1+回车 连接命令 1
    item.subItem = tr("选择第%1个步骤").arg(step);
    item.status = !recv.isEmpty();
    appendResult(item);

    mTestStep = StepTotal;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//ST ?+回车 连接命令 1
    item.subItem = tr("读取步骤总数");
    item.status = !recv.isEmpty();
    appendResult(item);
    if(!recv.isEmpty())stepTotal = recv.toUInt();


//=================================================================
    QString str; mTestStep = TestParm;
    if(mStep == GNDTest) mTestStep = GndParm;
    for(int i = 0; i < stepTotal ; i++)
    {
        recv = mTrans->sentStep(mStep , mTestStep , sendStr , i+1);//ST ?+回车 连接命令 1
        str += recv;
    }
     item.subItem = tr("读取参数");
     item.status = !str.isEmpty();
     appendResult(item);
    if(!str.isEmpty())
    {

         if(mStep == GNDTest) {
             mItem->sn.gndParm = str.split(","); mPro->gndParm = mItem->sn.gndParm;
         } else if(mStep == IRTest){
             mItem->sn.irParm = str.split(","); mPro->irParm = mItem->sn.irParm;
         } else if(mStep == ACWTest) {
             mItem->sn.acwParm = str.split(","); mPro->acwParm = mItem->sn.acwParm;
         }
    }
//=================================================================

    mTestStep = Test;
    recv = mTrans->sentStep(mStep , mTestStep , sendStr);//TEST+回车 连接命令 1
    item.subItem = tr("%1测试开始").arg(test);
    item.status = !recv.isEmpty();
    appendResult(item);


    return item.status;
}

bool Test_safety::testGND( QString & recv)//acw
{
    bool ret = false;
    sTestDataItem item;
    QString sendStr = "";
    mStep = GNDTest;
    int stepTotal = 0;
    item.item = tr("接地测试");
    ret = startTest(item,recv , tr("接地") , GNDFile , stepTotal);
    delayItem(item, 6*1000);

    for(int i = 0; i < stepTotal ; i++)
    {
        mTestStep = ReadData;
        recv = mTrans->sentStep(mStep , mTestStep , sendStr , i+1);//RD?+回车 连接命令 1
        item.subItem = tr("第%1次接地测试读取测试结果").arg(i+1);

        if(!recv.isEmpty()){
            if(!recv.contains("PASS")){
                mTestStep = Reset;
                mTrans->sentStep(mStep , mTestStep , sendStr);//RESET+回车 连接命令 1
                mTestStep = Reset;
                mTrans->sentStep(mStep , mTestStep , sendStr);//RESET+回车 连接命令 1
                item.status = false;
            }else{
                item.status = true;
            }
            QString ansStr = recv.split(",").at(4);
            mItem->sn.gnd += (i == (stepTotal - 1))
                    ?recv.split(",").at(4):recv.split(",").at(4)+"/";
            item.measured = ansStr;
        }else{item.measured = tr("读取测试结果失败");item.status = false;}
        item.expect = tr("大于20MΩ");
        mPro->gnd = mItem->sn.gnd;
        appendResult(item);       
    }
    QString str = tr("接地测试结果：%1 mΩ").arg(mPro->gnd);
    mPacket->updatePro(str, true); mPro->itemData << str;
    mPro->safe_result << item.status; ePro->safe_result << item.status;
    QString str1 = tr("Grounding test results:%1 mΩ").arg(mPro->gnd);
    ePro->itemData << str1; mItem->sn.gnd.clear();

    return ret;
}

bool Test_safety::testIR(QString & recv)
{
    bool ret = false;
    sTestDataItem item;
    QString sendStr = "";
    mStep = IRTest;
    int stepTotal = 0;
    item.item = tr("绝缘测试");
    ret = startTest(item, recv , tr("绝缘") , IRFile , stepTotal);
    delayItem(item, 7*1000);

    for(int i = 0; i < stepTotal ; i++)
    {
        if(!Breaker)
        {
            break;
        }
        mTestStep = ReadData;
        recv = mTrans->sentStep(mStep , mTestStep , sendStr , i+1);//*IDN?+回车 连接命令 1
        item.subItem = tr("第%1次绝缘测试读取测试结果").arg(i+1);

        if(!recv.isEmpty()){
            if(!recv.contains("PASS")){
                item.status = false;
            }else{
                item.status = true;
            }
            QString ansStr = recv.split(",").at(4);
            mItem->sn.ir += (i == (stepTotal - 1))
                    ?ansStr:ansStr+"/";
            item.measured = ansStr;
        }else{item.measured = tr("读取测试结果失败");item.status = false;}
        item.expect = tr("大于500MΩ");
        mPro->ir = mItem->sn.ir;
        appendResult(item);

    }
    QString str = tr("绝缘测试结果：%1 MΩ").arg(mPro->ir);
    mPacket->updatePro(str, true); mPro->itemData << str;
    mPro->safe_result << item.status; ePro->safe_result << item.status;
    QString str1 = tr("Insulation test results:%1 MΩ").arg(mPro->ir);
    ePro->itemData << str1; mItem->sn.ir.clear();


    return ret;
}

bool Test_safety::testACW(QString & recv)
{
    bool ret = false;
    sTestDataItem item;
    QString sendStr = "";
    mStep = ACWTest;
    int stepTotal = 0;
    item.item = tr("交流耐压测试");
    ret = startTest(item, recv , tr("交流耐压") , ACWFile , stepTotal);
    delayItem(item, 26*1000);//25

    for(int i = 0; i < stepTotal ; i++)
    {
        if(!Breaker)
        {
            break;
        }

        mTestStep = ReadData;
        recv = mTrans->sentStep(mStep , mTestStep , sendStr , i+1);//*IDN?+回车 连接命令 1
        item.subItem = tr("第%1次交流耐压测试读取测试结果").arg(i+1);
        if(!recv.isEmpty()){
            if(!recv.contains("PASS")){
               item.status = false;
            }else{
                item.status = true;
            }
            QString ansStr = recv.split(",").at(4);
            // mItem->sn.acw += (i == (stepTotal - 1))
            //         ?ansStr:ansStr+"/";
            mItem->sn.acw += tr(" %1、").arg(i+1) + ansStr;
            item.measured = ansStr;
        }else{item.measured = tr("读取测试结果失败");item.status = false;}
        item.expect = tr("小于10mA");
        mPro->acw = mItem->sn.acw;
        appendResult(item);       
    }
    QString str = tr("交流耐压测试结果：%1 mA").arg(mPro->acw);
    mPacket->updatePro(str, true); mPro->itemData << str;
    mPro->safe_result << item.status; ePro->safe_result << item.status;
    QString str1 = tr("AC withstand voltage test results:%1 mA").arg(mPro->acw);
    ePro->itemData << str1; mItem->sn.acw.clear();

    return ret;
}

void Test_safety::run()
{
    mPro->oning = true;
    testReady();

    if(!mItem->work_mode) {
        mItem->progress.allNum = 22;
        QString recv = "";
        testACW(recv); testIR(recv);    //先耐压再绝缘
        mPro->oning = false;
        emit overSig();
    } else {
        mItem->progress.allNum = 9;
        QString recv = "";
        testGND(recv);  
        emit overSig();
    }
}
