#ifndef PEOPLE_SAFETY_H
#define PEOPLE_SAFETY_H

#include <QDialog>
#include "config.h"
#include "json_pack.h"

namespace Ui {
class People_Safety;
}

class People_Safety : public QDialog
{
    Q_OBJECT

public:
    explicit People_Safety(QWidget *parent = nullptr);
    ~People_Safety();

private slots:
    void on_sureButton_clicked();
protected:
    void writeData(const QString &str1,const QString &str2,const QString &str3,bool pass);
    void writeData_Eng(const QString &str1,const QString &str2,const QString &str3,bool pass);
    void initData();

private:
    Ui::People_Safety *ui;
    sDataPacket *mPacket;
    sProgress *mPro;
    sProgress *ePro;
    datapacket_English *mPacketEng;
    sCfgItem *mItem;
};

#endif // PEOPLE_SAFETY_H
