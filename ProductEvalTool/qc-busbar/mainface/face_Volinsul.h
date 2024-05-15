#ifndef FACE_VOLINSUL_H
#define FACE_VOLINSUL_H

#include <QWidget>
#include "test_transthread.h"
#include "json_pack.h"

namespace Ui {
class Face_Volinsul;
}

class Face_Volinsul : public QWidget
{
    Q_OBJECT

public:
    explicit Face_Volinsul(QWidget *parent = nullptr);
    ~Face_Volinsul();

signals:
    void StatusSig(bool ret);
public slots:
    void startSlot();
    void progressSlot();
    void resultSlot();

private:
    Ui::Face_Volinsul *ui;
    sTestConfigItem *mItem;
    sProgress *mPro;
    sDataPacket *mPacket;
    QTimer *timer;
    TestDataSave *mDataSave;
};

#endif // VOL_INSUL_H
