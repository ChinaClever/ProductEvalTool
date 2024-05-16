#ifndef HOME_WORKWID_H
#define HOME_WORKWID_H

#include <QWidget>
#include "config.h"
#include "test_safety.h"
#include "mainface/face_volinsul.h"
#include "mainface/face_power.h"
#include "testwebsocket.h"
#include "power_corethread.h"
#include <QLabel>
#include <QPainter>

class VerticalLabel : public QLabel
{
public:
    VerticalLabel(const QString& text, QWidget* parent = nullptr)
        : QLabel(text, parent)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font;
        font.setPointSize(12);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        font.setWordSpacing(0);
        painter.setFont(font);

        painter.translate(width(), 0);
        painter.rotate(90);

        QRect rect(0, 0, height(), width());
        painter.drawText(rect, Qt::AlignCenter, text());
    }
};

namespace Ui {
class Home_WorkWid;
}

class Home_WorkWid : public QWidget
{
    Q_OBJECT

public:
    explicit Home_WorkWid(QWidget *parent = nullptr);
    ~Home_WorkWid();

signals:
    void startSig(int);
    void clearStartEleSig();
    void testBarSig(int);
protected slots:
    void timeoutDone();
    void overSlot();
    void updateWidSlot(int id);

    void StatusSlot(bool ret);
protected:
    void initLayout();
    void insertText();
    void setTextColor();
    void AcwStatus(bool ret);
    void GndStatus(bool ret);
    void VolStatus(bool ret);
    void LoadStatus(bool ret);
    void BreakerStatus(bool ret);
    QString getTime();
    void updateWid();
    void updateTime();
    void updateResult();

    bool initSerial();
    bool initSerialVol();
    bool initSerialGND();
    void initWid();
    void startTest();
    void overTest();
    void ItemStatus();
signals:
    void noloadHomeSig(int ret);

private slots:
    void insertTextslots(QString str,bool res);
    void on_vol_insulBtn_clicked();
    void on_groundBtn_clicked();
    void on_volBtn_clicked();
    void on_loadBtn_clicked();
    void on_breakerBtn_clicked();
    void on_clearBtn_clicked();
    void on_codeEit_textChanged(const QString &arg1);

private:
    Ui::Home_WorkWid *ui;

    uint mId, mFirst;
    QTimer *timer;
    sDevData * mDev;
    sProgress *mPro;
    sCfgItem *mCfgm;
    sDataPacket *mPacket;

    Face_Power *mPower;
    Test_safety *mSafrtyThread;
    Face_Volinsul *mVolInsul;
    Home_LoopTabWid *mLineTabWid;

    sTestConfigItem  *mItem;

    Power_CoreThread *mPowThread;
    Power_Logs *mLogs;
    Home_DataWid *mData;
    RtuRw *mModbus;
    Power_DevRead *mPowDev;
};

#endif // HOME_WORKWID_H
