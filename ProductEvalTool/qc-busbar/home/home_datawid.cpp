#include "home_datawid.h"

Home_DataWid::Home_DataWid(QWidget *parent) : ComTableWid(parent)
{
    initWid();
    mDev = sDataPacket::bulid()->getDev();
    mItem = Cfg::bulid()->item;
    mCfgm = TestConfig::bulid()->item;
    mData = &(mDev->line);
}

void Home_DataWid::initWid()
{
    header << tr("电压")<< tr("电流")<< tr("功率");
    initDataWid(header, 9, "测试数据");
    mBusData = get_share_mem();
}

void Home_DataWid::appendItem(sBoxData *box)
{
    sObjectData *dev = &(box->data);
    initDataWid(header,box->loopNum,"测试数据");
    QStringList listStr;QStringList listStr1;

    if(mCfgm->work_mode == 2) {
        for(int i=0; i<box->loopNum; ++i) {
            listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);
            listStr << " " << " ";
        }
        setTableRow(0, listStr);
    }else if(mCfgm->work_mode == 3) {
        for(int i=0; i<box->loopNum; ++i) {
            listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);
            listStr << QString::number(dev->cur.value[i]/COM_RATE_CUR,'f',3);
            listStr1 << QString::number(dev->pow.value[i]/COM_RATE_CUR,'f',3);
        }
        setTableRow(0, listStr);
    }
}

void Home_DataWid::timeoutDone()
{
    clearTable();

    if(mBusData->box[mItem->addr-1].loopNum) {
        appendItem(&(mBusData->box[mItem->addr-1]));
    }
}
