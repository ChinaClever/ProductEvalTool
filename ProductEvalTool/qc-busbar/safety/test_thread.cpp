#include "test_thread.h"
#include "config.h"

#define SERIAL_TIMEOUT 1000

Test_Thread::Test_Thread(QObject *parent) : QThread(parent)
{
    mRead = Power_DevRead::bulid(this);
    mItem = TestConfig::bulid()->item;
    mTrans = new Test_TransThread(this);
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
}

Test_Thread::~Test_Thread()
{
    quit();
    terminate();
    wait();
}

void Test_Thread::startThread()
{
    start();
}

void Test_Thread::stopThread()
{
    quit();
    terminate();
}

void Test_Thread::timeoutDone()
{
    if(mPro->oning)
    {
        bool ret = mRead->readDevBus();
        //bool ret = true;
        QString sendStr = "";

        if(!ret) {
            Breaker = false ;
            mTrans->sentStep(mStep , Reset , sendStr);//RESET
            mTrans->sentStep(mStep , Reset , sendStr);//RESET
            mPro->result = Test_Fail; mPro->oning = false;
        }else{
            Breaker = true ;
        }
    }
}

void Test_Thread::run()
{
    while(Breaker)
    {
        timeoutDone();
        msleep(100);
    }
    mItem->progress.errNum += 1;
    mItem->progress.finishNum += 1;
    emit messageSig();
}
