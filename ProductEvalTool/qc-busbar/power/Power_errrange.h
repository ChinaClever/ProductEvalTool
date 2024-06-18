#ifndef POWER_ERRRANGE_H
#define POWER_ERRRANGE_H
#include "power_logs.h"

class Power_ErrRange : public Power_Object
{
    Q_OBJECT
    explicit Power_ErrRange(QObject *parent = nullptr);
public:
    static Power_ErrRange *bulid(QObject *parent = nullptr);

    bool volErr(int id);
    bool curErr(int id);

    bool curAlarm(int id);
    QString changeMode(int index);
    bool checkErrRange(int exValue, int value, int err);
protected:
    QString changeBaudType(int index);
    QString changeCurType(int index);

protected slots:
    void initFunSlot();

private:
    sDevData *mSourceDev;
    Power_Logs *mLogs;
};

#endif // TEST_ERRRANGE_H
