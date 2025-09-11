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
    // initDataWid(header,box->loopNum,"测试数据");
    if(box->loopNum){
        for(int i=0; i<box->loopNum; ++i) {
            QStringList listStr;
            listStr << QString::number(dev->vol.value[i]/COM_RATE_VOL,'f',1);
            listStr << QString::number(dev->cur.value[i]/COM_RATE_CUR,'f',3);
            listStr << QString::number(dev->pow.value[i]/COM_RATE_POW,'f',3);
            setTableRow(i, listStr);
        }
    }
}

void Home_DataWid::timeoutDone()
{
    clearTable();
    if(mItem->modeId == BASIC_TYPE){
        for(int i = 0; i < 3; ++i) { // 显示0 ~ 2共3路
            QStringList listStr;
            listStr << QString::number(mData->source_vol[i] / COM_RATE_CUR, 'f', 1);
            listStr << QString::number(mData->source_cur[i] / COM_RATE_CUR, 'f', 3);
            listStr << QString::number(mData->pow[i] / (COM_RATE_POW), 'f', 3);

            setTableRow(i, listStr); // 写入表格第i行
        }
    }
    else{
        if((mBusData->box[mItem->addr-1].loopNum)&&(mBusData->box[mItem->addr-1].loopNum < 10)) {
            appendItem(&(mBusData->box[mItem->addr-1]));
        }
    }
}

