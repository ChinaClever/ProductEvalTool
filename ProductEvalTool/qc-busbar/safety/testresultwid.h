#ifndef TESTRESULTWID_H
#define TESTRESULTWID_H

#include "testdatatablewid.h"

namespace Ui {
class TestResultWid;
}



class TestResultWid : public QWidget
{
    Q_OBJECT

public:
    explicit TestResultWid(QWidget *parent = 0);
    ~TestResultWid();   

signals:
    void startSig(int);

public slots:
    void startSlot();    
    void progressSlot();
    void resultSlot();

protected:
    bool initSerialPort();
    bool initSerialGNDPort();
    void updateSerialWid();
    void updateSerialGNDWid();
    bool checkSerial();
    bool checkSerialGND();

private slots:
    void on_comBtn_clicked();
    void on_startBtn_clicked();
    void on_comBtnGND_clicked();
private:
    Ui::TestResultWid *ui;

    QTimer *timer;
    sTestConfigItem *mItem;
    SerialPortDlg *mSerialDlg;
    SerialPortDlg *mSerialGNDDlg;
    sProgress *mPro;
    sDataPacket *mPacket;
};

#endif // TESTRESULTWID_H
