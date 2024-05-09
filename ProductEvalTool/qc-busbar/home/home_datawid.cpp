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
    header << tr("回路")<< tr("A1")<< tr("A2")<< tr("A3")<< tr("B1")<< tr("B2")<< tr("B3")<< tr("C1")<< tr("C2")<< tr("C3");
    initDataWid(header, 9, "测试数据");
    mBusData = get_share_mem();
}

void Home_DataWid::appendItem(sBoxData *box)
{
    sObjectData *dev = &(box->data);
    initDataWid(header,2,"测试数据");
    QStringList listStr;QStringList listStr1;

    if(mCfgm->work_mode == 2) {
         listStr << "电压";
        for(int i=0; i<box->loopNum; ++i)
            listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);

        setTableRow(0, listStr);
    }else if(mCfgm->work_mode == 3) {
        listStr << "电流"; listStr1 << "功率";
        for(int i=0; i<box->loopNum; ++i) {
            listStr << QString::number(dev->cur.value[i]/COM_RATE_CUR,'f',3);
            listStr1 << QString::number(dev->pow.value[i]/COM_RATE_CUR,'f',3);
        }
        setTableRow(0, listStr); setTableRow(1, listStr1);
    }else if(mCfgm->work_mode == 4) {
        listStr << "电压";
        for(int i=0; i<box->loopNum; ++i)
            listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);

        setTableRow(0, listStr);
    }
}

void Home_DataWid::timeoutDone()
{
    clearTable();

    mBusData->box[mItem->addr-1].loopNum = 6;
    if(mBusData->box[mItem->addr-1].loopNum) {
        appendItem(&(mBusData->box[mItem->addr-1]));
    }
}
