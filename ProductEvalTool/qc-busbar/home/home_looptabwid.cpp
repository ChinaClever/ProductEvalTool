/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "home_looptabwid.h"

Home_LoopTabWid::Home_LoopTabWid(QWidget *parent) : ComTableWid(parent)
{
    initWid();
    mDev = sDataPacket::bulid()->getDev();
    mItem = Cfg::bulid()->item;
    mData = &(mDev->line);
}

void Home_LoopTabWid::initWid()
{
    header << tr("电流") << tr("电压") << tr("有功功率")  << tr("功率因数") << tr("电能") << tr("温度");
    initTableWid(header, 9, "回路数据列表");
    mBusData = get_share_mem();
}

void Home_LoopTabWid::appendItem(sBoxData *box)
{
    sObjectData *dev = &(box->data);
    for(int i=0; i<box->loopNum; ++i) {
        QStringList listStr;

        listStr << QString::number(dev->cur.value[i]/COM_RATE_CUR,'f',3);
        listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);
        listStr << QString::number(dev->pow.value[i]/COM_RATE_POW,'f',3);
        listStr << QString::number(dev->pf[i]/COM_RATE_PF,'f',2);
        listStr << QString::number(dev->ele[i]/COM_RATE_ELE,'f',2);
        if(i<4) listStr << QString::number(box->env.tem.value[i]/COM_RATE_TEM,'f',1);

        setTableRow(i, listStr);
        if(i<4) setItemColor(i, 5, box->env.tem.status[i]);

        if(dev->cur.value[i] == 0) setItemColor(i, 0, 3);
        if(dev->vol.value[i] == 0) setItemColor(i, 1, 3);
        if(dev->pow.value[i] == 0) setItemColor(i, 2, 3);
    }
}

void Home_LoopTabWid::timeoutDone()
{
    clearTable();
    initTableWid(header, mBusData->box[mItem->addr-1].loopNum, "回路数据列表");
    // mBusData->box[mItem->addr-1].loopNum = 6;
    if(mBusData->box[mItem->addr-1].loopNum) {
        appendItem(&(mBusData->box[mItem->addr-1]));
    }
}
