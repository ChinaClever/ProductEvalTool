/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "power_devctrl.h"

Power_DevCtrl::Power_DevCtrl(QObject *parent) : Power_Object(parent)
{

}

Power_DevCtrl *Power_DevCtrl::bulid(QObject *parent)
{
    static Power_DevCtrl* sington = nullptr;
    if(sington == nullptr)
        sington = new Power_DevCtrl(parent);
    return sington;
}

void Power_DevCtrl::initFunSlot()
{
    mRtu = nullptr;
    mSiRtu = Ctrl_SiRtu::bulid(this);
    mIpRtu = Ctrl_IpRtu::bulid(this);
}

bool Power_DevCtrl::setCurTh(int i)
{
    bool ret = initDev();
    if(ret) {
        ret = mRtu->setCurTh(i);
    }
    return ret;
}

bool Power_DevCtrl::setVolTh(int i)
{
    bool ret = initDev();
    if(ret) {
        ret = mRtu->setVolTh(i);
    }
    return ret;
}


bool Power_DevCtrl::eleClean()
{
    bool ret = initDev();
    mRtu->eleClean();

    return ret;
}

bool Power_DevCtrl::factorySet()
{
    bool ret = initDev();
    if(ret) {
        ret = mRtu->factorySet();
    }
    return ret;
}

bool Power_DevCtrl::initDev()
{
    bool ret = true;
    mRtu = mSiRtu;

    return ret;
}
