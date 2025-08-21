#ifndef POWER_DATAREAD_H
#define POWER_DATAREAD_H

#include "power_errrange.h"
#include "safety/test_transthread.h"

class Power_DevRead : public Power_Object
{
    Q_OBJECT
public:
    explicit Power_DevRead(QObject *parent = nullptr);
public:
    static Power_DevRead *bulid(QObject *parent = nullptr);

    bool readSn();
    bool readDev();
    bool readDevBasicType();
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
    bool Load_NineLoop(Test_TransThread *trans_ctrl);
    bool Break_NineLoop();
    bool NineInsertOne_CtrlOne(Test_TransThread *trans_ctrl);
    bool NineInsertOne_CtrlTwo(Test_TransThread *trans_ctrl);
    bool NineInsertOne_CtrlThree(Test_TransThread *trans_ctrl);
    bool Insert_NoneOutput(Test_TransThread *trans_ctrl,int index);
    bool NineInsertTwo_CtrlOne(Test_TransThread *trans_ctrl);
    bool NineInsertTwo_CtrlTwo(Test_TransThread *trans_ctrl);
    bool NineInsertTwo_CtrlThree(Test_TransThread *trans_ctrl);
    bool NineInsertThree_CtrlOne(Test_TransThread *trans_ctrl);
    bool NineInsertThree_CtrlTwo(Test_TransThread *trans_ctrl);
    bool NineInsertThree_CtrlThree(Test_TransThread *trans_ctrl);
    bool NineInsertOne_BreakerOne();
    bool NineInsertOne_BreakerTwo();
    bool NineInsertOne_BreakerThree();

    //六回路二个输出位
    bool Load_SixLoop(Test_TransThread *trans_ctrl);
    bool Break_SixLoop();
    bool SixInsertOne_CtrlOne(Test_TransThread *trans_ctrl);
    bool SixInsertOne_CtrlTwo(Test_TransThread *trans_ctrl);
    bool SixInsertOne_CtrlThree(Test_TransThread *trans_ctrl);
    bool SixInsertTwo_CtrlOne(Test_TransThread *trans_ctrl);
    bool SixInsertTwo_CtrlTwo(Test_TransThread *trans_ctrl);
    bool SixInsertTwo_CtrlThree(Test_TransThread *trans_ctrl);
    bool SixInsertOne_BreakerOne();
    bool SixInsertOne_BreakerTwo();

    //始端箱-三相三回路三个输出位
    bool Load_ThreeLoop(Test_TransThread *trans_ctrl);
    bool Three_CtrlOne(Test_TransThread *trans_ctrl);
    bool Three_CtrlTwo(Test_TransThread *trans_ctrl);
    bool Three_CtrlThree(Test_TransThread *trans_ctrl);
    bool Three_Breaker();

    //单相三回路三个输出位
    bool Three_One(Test_TransThread *trans_ctrl);
    bool Three_Two(Test_TransThread *trans_ctrl);
    bool Three_Three(Test_TransThread *trans_ctrl);
    bool Three_Break();
    bool Three_OneBreaker();
    bool Three_TwoBreaker();
    bool Three_ThreeBreaker();

    Dev_Source *mSource;
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
