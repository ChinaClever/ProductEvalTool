#ifndef TEST_TRANSTHREAD_H
#define TEST_TRANSTHREAD_H
#include <iomanip>
#include <sstream>
#include "testdatasave.h"
// #include <QString>
// #include <QDebug>

enum TestResisEnum {
    ConnectReady = 1,//1
    Reset = 2,
    ChoseeFile = 3,
    StepTotal = 4,
    Test = 5,
    ReadData = 6,
    TestParm = 7,
    GndParm = 8
};

class Test_TransThread : public QObject
{
    Q_OBJECT
public:
    explicit Test_TransThread(QObject *parent = nullptr);

    QString sentStep(int step, int i , QString & command , int extra = 1);
    QString sentResisCommand(int command, int extra);
    bool readDevBus();
    void sendCtrlGnd(int command);
    bool recvPolarity();
    QByteArray sendCmd(int command);
    QString transStr(int command);

    ushort calccrc (ushort crc, uchar crcbuf);
    ushort rtu_crc(const uchar *buf, int len);

signals:

protected slots:
    void initFunSLot();

private:
    SerialPort *mSerial;
    SerialPort *mSerialCtrl;
    SerialPort *mSerialPolar;

    sCfgItem *mItem;
    sDataPacket *mPacket;
};

#endif // TEST_TRANSTHREAD_H
