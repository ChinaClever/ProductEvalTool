/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_network.h"

Power_NetWork::Power_NetWork(QObject *parent) : Power_Object(parent)
{

}

Power_NetWork *Power_NetWork::bulid(QObject *parent)
{
    static Power_NetWork* sington = nullptr;
    if(sington == nullptr)
        sington = new Power_NetWork(parent);
    return sington;
}

void Power_NetWork::initFunSlot()
{    
    mLogs = Power_Logs::bulid(this);
    mUdp = new UdpRecvSocket(this);
    mUdp->initSocket(10010);
    this->start();
}

void Power_NetWork::workDown()
{
    UdpBaseData *res = mUdp->getData();
    if(res) {
        QStringList list = QString(res->datagram).split(";");
        if(list.size() ==2) {
            QString str = list.first();
            if(str.contains(("Mac")))
                emit sendMACSig(str);
            bool pass = list.last().toInt();
            mLogs->updatePro(str, pass, 0);
        }
        delete res;
    } else {
        msleep(1);
    }
}

void Power_NetWork::run()
{
    isRun = true;
    while (isRun) {
        workDown();
    }
}
