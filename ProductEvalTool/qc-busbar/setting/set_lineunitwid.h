#ifndef HOME_THUNITWID_H
#define HOME_THUNITWID_H

#include <QWidget>
#include "common.h"
#include "config.h"

namespace Ui {
class Set_LineUnitWid;
}

class Set_LineUnitWid : public QWidget
{
    Q_OBJECT

public:
    explicit Set_LineUnitWid(QWidget *parent = nullptr);
    ~Set_LineUnitWid();

    void updateData();
    void init(sObjCfg *obj, int index=0);

protected:
    void showAndHideWid(bool show=false);
    int transformRate(int index);

private:
    Ui::Set_LineUnitWid *ui;
    sObjCfg *mDev;
    int mIndex;
};

#endif // HOME_THUNITWID_H
