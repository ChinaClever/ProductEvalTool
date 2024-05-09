#include "test_transthread.h"
Test_TransThread::Test_TransThread(QObject *parent) : QObject(parent)
{
     QTimer::singleShot(300,this,SLOT(initFunSLot()));
}


void Test_TransThread::initFunSLot()
{
    mSerial = Cfg::bulid()->item->coms.ser1;
    mSerialGND  = Cfg::bulid()->item->coms.ser2;
}

void Delay_MSec(unsigned int msec)
{
    QTime _Timer = QTime::currentTime().addMSecs(msec);

    while( QTime::currentTime() < _Timer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString Test_TransThread::sentStep(int step , int i , QString & command , int extra)
{
    QByteArray recv;
    if(step == 1)
        command = sentResisCommand(i , extra);//GND
    else
        command = sentResisCommand(i , extra);//IR//ACW

    for(int count = 0 ; count < 3 ; count ++)
    {
        if(recv.isEmpty()){
            if(step == 1)
                mSerialGND->transmit(command.toLatin1(),recv,10);
            else{
                if(mSerial)
                    mSerial->transmit(command.toLatin1(),recv,10);
            }
        }
        if(!recv.isEmpty())
            break;
        Delay_MSec(1000);
    }

    return recv;
}

QString Test_TransThread::sentResisCommand(int command , int extra)
{
    QString str;
    switch(command)
    {
        case ConnectReady://1
            str = "*IDN?\r\n";
            break;
        case Reset://2
            str = "RESET\r\n";
            break;
        case ChoseeFile://3
            str = QString("FL %1\r\n").arg(extra);
            break;
        case StepTotal://4
            str = "ST?\r\n";
            break;
        case Test://5
            str = "TEST\r\n";
            break;
        case ReadData://6
            str = QString("RD %1?\r\n").arg(extra);
            break;
    }
    return str ;
}
