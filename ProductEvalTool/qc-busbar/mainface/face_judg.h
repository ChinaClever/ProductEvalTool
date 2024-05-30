#ifndef FACE_JUDG_H
#define FACE_JUDG_H

#include <QWidget>
#include "config.h"
#include "json_pack.h"

namespace Ui {
class Face_judg;
}

class Face_judg : public QWidget
{
    Q_OBJECT

public:
    explicit Face_judg(QWidget *parent = nullptr);
    ~Face_judg();
    void writeData(const QString &str1,const QString &str2,bool pass);
    void initData();
private slots:
    void on_sureButton_clicked();
private:
    Ui::Face_judg *ui;
    sDataPacket *mPacket;
    sProgress *mPro;
    sCfgItem *mItem;
};

#endif // FACE_JUDG_H
