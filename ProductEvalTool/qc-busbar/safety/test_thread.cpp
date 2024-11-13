#include "test_thread.h"
#include "test_safety.h"
#include "config.h"

#define SERIAL_TIMEOUT 1000

Test_Thread::Test_Thread(QObject *parent) : QThread(parent)
{
    mRead = Power_DevRead::bulid(this);
    mTrans = new Test_TransThread(this);
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();

//     timer = new QTimer(this);
//     connect(timer, &QTimer::timeout, this, &Test_Thread::timeoutDone);
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
    // timer->stop();
    qDebug()<<"mPro->oning"<<mPro->oning;
    if(mPro->oning)
    {
        bool ret = mRead->readDevBus();
        QString sendStr = "";
        qDebug()<<"ret"<<ret;
        if(!ret) {
            Breaker = false ;
            mTrans->sentStep(mStep , Reset , sendStr);//RESET
            mPro->result = Test_Fail; mPro->step = Test_Over;
        }else{
            Breaker = true ;
        }
    }
    qDebug()<<"mStep"<<mStep<<Breaker;
}

void Test_Thread::run()
{
    while(Breaker)
    {
        timeoutDone();
        msleep(200);
    }

    // emit messageSig();
}
