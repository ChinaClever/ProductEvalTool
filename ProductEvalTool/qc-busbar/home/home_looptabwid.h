#ifndef HOME_LOOPTABWID_H
#define HOME_LOOPTABWID_H

#include "comtablewid.h"
#include "datapacket.h"
#include "config.h"

class Home_LoopTabWid : public ComTableWid
{
    Q_OBJECT
public:
    explicit Home_LoopTabWid(QWidget *parent = nullptr);

signals:

protected:
    void initWid();
    void appendItem(sBoxData *box);

protected slots:
    void timeoutDone();

private:
    sObjData *mData;
    sDevData *mDev;
    sCfgItem *mItem;
    sBusData *mBusData;
    QStringList header;
};

#endif // HOME_LINETABWID_H
