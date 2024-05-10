#ifndef TEST_SAFETY_H
#define TEST_SAFETY_H

#include "test_transthread.h"

class Test_safety : public QThread
{
    Q_OBJECT
public:
    explicit Test_safety(QObject *parent = nullptr);
    ~Test_safety();

    void startThread();

signals:
    void overSig();

protected:
    void run();
    void stopThread();
    //void updateData(int i);
    void conditionExec(bool s);
    void updateProgress(bool status, QString &str);
    bool appendResult(sTestDataItem &item);

    void delayItem(sTestDataItem &item, int s);
    bool testReady();
    bool testGND(QString &recv);
    bool testIR(QString &recv);
    bool testACW(QString &recv);
    bool startTest(sTestDataItem &item,QString & recv , const QString &test , int step , int &stepTotal);

private:
    int mItemId;
    int mTestStep;
    int mStep;
    sTestConfigItem  *mItem;
    Test_TransThread *mTrans;
    sProgress *mPro;
    sDataPacket *mPacket;
};

#endif // TEST_SAFETY_H
