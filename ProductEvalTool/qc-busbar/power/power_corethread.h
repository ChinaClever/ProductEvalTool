#ifndef POWER_CORETHREAD_H
#define POWER_CORETHREAD_H

#include "power_devread.h"
#include "json_pack.h"
#include "testconfig.h"


class Power_CoreThread : public Power_Object
{
    Q_OBJECT
public:
    explicit Power_CoreThread(QObject *parent = nullptr);

protected slots:
    void initFunSlot();

signals:
    void noLoadSig();
    void StepSig(QString str);
    void finshSig(bool ret);

private slots:
    void noloadHomeSlot(int ret);

public slots:
    void clearStartEleSlot();
    void getNumAndIndexSlot(int curnum);
    void getDelaySlot();
protected:
    void run();
    void workDown();

    bool initDev();
    bool hubPort();
    bool volErrRange(int i);
    bool curErrRange(int i);
    bool powErrRange(int i);
    bool pfErrRange(int i);
    bool eleErrRange(int i);
    bool eleErrRange0(int i);
    bool envErrRange();

    bool volAlarmErr(int i);
    bool curAlarmErr(int i);
    bool powAlarmErr(int i);
    bool lineVolAlarmErr(int i);
    bool checkAlarmErr();
    bool envAlarmErr();
    bool hzAlarmErr();
    bool totalPowAlarmErr();
    bool zeroLineCurAlarmErr();
    bool residualAlarmErr();

    bool volAlarmWrite(int i);
    bool curAlarmWrite(int i);
    bool writeAlarmTh();

    bool factorySet();
    void workResult(bool);

    bool checkVersion();
    bool checkEnvVersion();
    bool checkVolErrRange();
    bool checkLoadErrRange();
    bool volLoadErrRange(int i);
    bool curLoadErrRange(int i);
    bool powLoadErrRange(int i);

    bool stepVolTest();
    bool stepLoadTest();
    bool stepBreakerTest(int sum);
    QString changeMode(int index);
    void autoSetAddress();

private:
    Power_Logs *mLogs;
    Power_DevRead *mRead;
    Power_DevCtrl *mCtrl;
    QString mMacStr;
    int mRet;
    int mCurBoxNum;
    sTestConfigItem  *mCfg;
    Ad_Modbus *mModbus;
};

#endif // POWER_CORETHREAD_H
