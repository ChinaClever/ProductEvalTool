#ifndef FACE_VOLINSUL_H
#define FACE_VOLINSUL_H

#include <QWidget>
#include "test_transthread.h"
#include "json_pack.h"
#include "sn_serialnum.h"

namespace Ui {
class Face_Volinsul;
}

class Face_Volinsul : public QWidget
{
    Q_OBJECT

public:
    explicit Face_Volinsul(QWidget *parent = nullptr);
    ~Face_Volinsul();

    void initWid();
    // void progressSlot();
    void startSlot();
    void resultSlot();
    bool printer();
signals:
    void StatusSig(bool ret);
    void finshSig();
    void overSig();
public slots:
    // void startSlot();
    void progressSlot();
    // void resultSlot();


private:
    Ui::Face_Volinsul *ui;
    sTestConfigItem *mItem;
    sProgress *mPro;
    sDataPacket *mPacket;
    QTimer *timer;
    TestDataSave *mDataSave;
    sCfgItem *mCfg;
    Sn_SerialNum *mSn;
    sDevData *mDev;
};

#endif // VOL_INSUL_H
