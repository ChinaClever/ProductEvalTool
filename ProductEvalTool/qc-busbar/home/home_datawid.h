#ifndef HOME_DATAWID_H
#define HOME_DATAWID_H
#include "comtablewid.h"
#include "datapacket.h"
#include "config.h"
#include "testconfig.h"
#include "safety/test_transthread.h"
class Home_DataWid : public ComTableWid
{
    Q_OBJECT
public:
    explicit Home_DataWid(QWidget *parent = nullptr);

signals:

protected:
    void initWid();
    void appendItem(sBoxData *box);

public slots:
    void timeoutDone();

private:
    sObjData *mData;
    sDevData *mDev;
    sCfgItem *mItem;
    sBusData *mBusData;
    QStringList header;
    sTestConfigItem  *mCfgm;
};

#endif // HOME_DATAWID_H
