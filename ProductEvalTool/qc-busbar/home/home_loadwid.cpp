#include "home_loadwid.h"

Home_Loadwid::Home_Loadwid(QWidget *parent) : ComTableWid(parent)
{
    initWid();
    mDev = sDataPacket::bulid()->getDev();
    mItem = Cfg::bulid()->item;
    mData = &(mDev->line);
}

void Home_Loadwid::initWid()
{
    QString title = tr("负载数据");
    QStringList header;
    header << tr("电流") << tr("电压") << tr("有功功率");
    initTableWid(header, 3, title);
}


void Home_Loadwid::appendItem(sDevData *box)
{
    for(int i=0; i<3; ++i) {
        QStringList listStr;

        listStr << QString::number(box->line.cur.value[i]/COM_RATE_CUR,'f',3);
        listStr << QString::number(box->line.vol.value[i]/COM_RATE_VOL,'f',1);
        listStr << QString::number(box->line.pow[i]/COM_RATE_POW,'f',3);

        setTableRow(i, listStr);
    }
}

void Home_Loadwid::timeoutDone()
{
    clearTable();

    appendItem(mDev);

}
