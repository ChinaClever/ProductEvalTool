#ifndef FACE_POWER_H
#define FACE_POWER_H

#include <QWidget>
#include "home_looptabwid.h"
#include "home_loadwid.h"
#include "home_datawid.h"

namespace Ui {
class Face_Power;
}

class Face_Power : public QWidget
{
    Q_OBJECT

public:
    explicit Face_Power(QWidget *parent = nullptr);
    ~Face_Power();

protected:
    void initWid();

private:
    Ui::Face_Power *ui;
    Home_LoopTabWid *mLineTabWid;
    Home_Loadwid *mLoadWid;
    Home_DataWid *mDataWid;
};

#endif // TEST_POWER_H
