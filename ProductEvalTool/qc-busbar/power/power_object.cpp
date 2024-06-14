/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_object.h"

Power_Object::Power_Object(QObject *parent) : QThread(parent)
{
    isRun = false;
    mPacket = sDataPacket::bulid();
    mPacketEng = datapacket_English::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();
    ePro = mPacketEng->getPro();
    mDev = mPacket->getDev();
    mSour = mPacket->getDev(0);
    mDt = &(mDev->devType);
    mBusData = get_share_mem();
    QTimer::singleShot(500,this,SLOT(initFunSlot()));
}

Power_Object::~Power_Object()
{
    isRun = false;
    wait();
}
