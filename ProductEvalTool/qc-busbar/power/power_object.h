#ifndef POWER_OBJECT_H
#define POWER_OBJECT_H

#include "ctrl_sirtu.h"
#include "json_pack.h"

class Power_Object : public QThread
{
    Q_OBJECT
public:
    explicit Power_Object(QObject *parent = nullptr);
    ~Power_Object();

protected slots:
    virtual void initFunSlot(){}

protected:
    bool isRun;
    sDevType *mDt;
    sDevData *mDev;
    sDevData *mSour;
    sCfgItem *mItem;
    sProgress *mPro;
    sProgress *ePro;
    sDataPacket *mPacket;
    datapacket_English *mPacketEng;
    sBusData *mBusData;
};

#endif // TEST_OBJECT_H
