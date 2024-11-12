#ifndef POWER_DATAREAD_H
#define POWER_DATAREAD_H

#include "power_errrange.h"

class Power_DevRead : public Power_Object
{
    Q_OBJECT
public:
    explicit Power_DevRead(QObject *parent = nullptr);
public:
    static Power_DevRead *bulid(QObject *parent = nullptr);

    bool readSn();
    bool readDev();
    bool readDevBus();
    bool readNet();
    bool checkNet();
    bool readDevData();
    bool readData();
    QString getConnectModeOid();
    QString getFilterOid();
    QString setShuntReleaseCtrlOid();
    QString getBreakCtrlOid();
    bool SetInfo(QString o , QString val);
    QString trans(int index);
    //九回路三个输出位
    bool Load_NineLoop();
    bool Break_NineLoop();
    bool NineInsertOne_CtrlOne();
    bool NineInsertOne_CtrlTwo();
    bool NineInsertOne_CtrlThree();
    bool NineInsertTwo_CtrlOne();
    bool NineInsertTwo_CtrlTwo();
    bool NineInsertTwo_CtrlThree();
    bool NineInsertThree_CtrlOne();
    bool NineInsertThree_CtrlTwo();
    bool NineInsertThree_CtrlThree();
    bool NineInsertOne_BreakerOne();
    bool NineInsertOne_BreakerTwo();
    bool NineInsertOne_BreakerThree();

    //六回路二个输出位
    bool Load_SixLoop();
    bool Break_SixLoop();
    bool SixInsertOne_CtrlOne();
    bool SixInsertOne_CtrlTwo();
    bool SixInsertOne_CtrlThree();
    bool SixInsertTwo_CtrlOne();
    bool SixInsertTwo_CtrlTwo();
    bool SixInsertTwo_CtrlThree();
    bool SixInsertOne_BreakerOne();
    bool SixInsertOne_BreakerTwo();

    //始端箱-三相三回路三个输出位
    bool Load_ThreeLoop();
    bool Three_CtrlOne();
    bool Three_CtrlTwo();
    bool Three_CtrlThree();
    bool Three_Breaker();

    //单相三回路三个输出位
    bool Three_One();
    bool Three_Two();
    bool Three_Three();
    bool Three_Break();
    bool Three_OneBreaker();
    bool Three_TwoBreaker();
    bool Three_ThreeBreaker();
protected:
    bool readSnmp();
protected slots:
    void initFunSlot();
signals:
    void StepSig(QString str);
    void PloarSig(QString str);
    void CurImageSig(int value);
private:
    Dev_Object *mRtu;
    Dev_SiRtu *mSiRtu;
    Dev_IpRtu *mIpRtu;
    Sn_SerialNum *mSn;
    Dev_IpSnmp *mIpSnmp;
    Power_Logs *mLogs;
    sCfgItem *mItem;
    Power_ErrRange *mErr;
    int exValue ;
    int err ;
};

#endif // POWER_DATAREAD_H
