#ifndef POWER_LOGS_H
#define POWER_LOGS_H
#include "dbstates.h"
#include "power_object.h"

class Power_Logs : public Power_Object
{
    Q_OBJECT
    explicit Power_Logs(QObject *parent = nullptr);
public:
    static Power_Logs *bulid(QObject *parent = nullptr);

    void saveLogs();
    bool updatePro(const QString &str, bool pass=true, int sec=1);
    void writeData(const QString &str, const QString &str2, bool pass=true);
protected:
    bool writeLog();
    void writeLogs();
    bool appendLogItem(const QString &str, bool pass);

private:
    QList<sStateItem> mLogItems;
};

#endif // TEST_LOGS_H
