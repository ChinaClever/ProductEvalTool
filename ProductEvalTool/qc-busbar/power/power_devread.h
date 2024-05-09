#ifndef POWER_DATAREAD_H
#define POWER_DATAREAD_H

#include "power_devctrl.h"

class Power_DevRead : public Power_Object
{
    Q_OBJECT
    explicit Power_DevRead(QObject *parent = nullptr);
public:
    static Power_DevRead *bulid(QObject *parent = nullptr);

    bool readSn();
    bool readDev();
    bool readHub();
    bool readNet();
    bool checkNet();
    bool readDevData();
    bool readData();
    QString getConnectModeOid();
    QString getFilterOid();
    bool SetInfo(QString o , QString val);

protected:
    void run();
    bool readSnmp();

    bool checkSiLine();
    bool checkIpLine();
    bool checkIpVersion();

protected slots:
    void initFunSlot();

private:
    Dev_Object *mRtu;
    Dev_SiRtu *mSiRtu;
    Dev_IpRtu *mIpRtu;
    Sn_SerialNum *mSn;
    Dev_IpSnmp *mIpSnmp;
    Dev_Source *mSource;
    Power_Logs *mLogs;
    sCfgItem *mItem;
};

#endif // POWER_DATAREAD_H
