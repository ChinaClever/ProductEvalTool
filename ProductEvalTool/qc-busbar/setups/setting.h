#ifndef SETTING_H
#define SETTING_H

#include "config.h"
#include <QWidget>
#include "comtablewid.h"

namespace Ui {
class Setting;
}

class Setting : public QWidget
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();

signals:
    void errSig();

protected:
    void initType();

    void updateType();
    void updateData();

    bool inputCheck();
    bool dataSave();

public slots:
    void initFunSlot();
    void enabledSlot(bool en);
    void saveFunSlot();
    void saveErrSlot();
private slots:
    void on_setBtn_clicked();

    void on_userEdit_textChanged(const QString &arg1);

private:
    Ui::Setting *ui;
    sCfgItem *mItem;
    int mCnt;
};

#endif // SETTING_H
