#ifndef POWER_DATAREAD_H
#define POWER_DATAREAD_H

#include "power_devctrl.h"

class Power_DevRead : public Power_Object
{
    Q_OBJECT
public:
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

    bool Load_NineLoop();
    bool Load_SixLoop();
    bool Load_ThreeLoop();

    bool Break_NineLoop();
    bool Break_SixLoop();
    bool Break_ThreeLoop();
protected:
    void run();
    bool readSnmp();

    bool checkSiLine();
    bool checkIpLine();
    bool checkIpVersion();

protected slots:
    void initFunSlot();
signals:
    void StepSig(QString str);
private:
    Dev_Object *mRtu;
    Dev_SiRtu *mSiRtu;
    Dev_IpRtu *mIpRtu;
    Sn_SerialNum *mSn;
    Dev_IpSnmp *mIpSnmp;
    Power_Logs *mLogs;
    sCfgItem *mItem;
};

#endif // POWER_DATAREAD_H
