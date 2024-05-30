#ifndef PEOPLE_JUDG_H
#define PEOPLE_JUDG_H

#include <QDialog>
#include "config.h"
#include "json_pack.h"

namespace Ui {
class People_judg;
}

class People_judg : public QDialog
{
    Q_OBJECT

public:
    explicit People_judg(QWidget *parent = nullptr);
    ~People_judg();
    void writeData(const QString &str1,const QString &str2,bool pass);
    void initData();
private slots:
    void on_sureButton_clicked();

private:
    Ui::People_judg *ui;
    sDataPacket *mPacket;
    sProgress *mPro;
    sCfgItem *mItem;
};

#endif // PEOPLE_JUDG_H
