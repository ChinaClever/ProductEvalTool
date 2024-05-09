#ifndef TESTDATASAVE_H
#define TESTDATASAVE_H

#include "excel_savethread.h"
#include "comtablewid.h"
#include "testconfig.h"
#include "common.h"
#include "datapacket.h"
#include "dbstates.h"

class TestDataSave : public QObject
{
    Q_OBJECT
public:
    explicit TestDataSave(QObject *parent = nullptr);

    void saveTestData();

protected:
    int bulidHead(sSafetyItem *item);
    int bulidProgressLog(int id, sTestProgress &arg);
    void bulidTestData(QList<QStringList> &list);
    bool saveDb();

private:
    QString bulidFileName(sSafetyItem *item);
    QString bulidPath(sSafetyItem *item);

private:
    Excel_SaveThread *mSaveThread;
    sTestConfigItem *mItem;
    QString mFileName;
    QList<QStringList> mList;
    sProgress *mPro;
    sDevData *mDev;
    sDataPacket *mPacket;
    QList<sStateItem> mLogItems;
};

#endif // TESTDATASAVE_H
