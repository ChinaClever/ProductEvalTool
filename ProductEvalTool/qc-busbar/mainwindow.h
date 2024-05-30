#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "navbarwid.h"
#include "logmainwid.h"
#include "setup_mainwid.h"
#include "home_mainwid.h"
#include "setting.h"
// #include "set_mainwid.h"
// #include "test_setting.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void initWid();

protected slots:
    void initFunSlot();
    void navBarSlot(int);

private:
    Ui::MainWindow *ui;

    LogMainWid *mLog;
    NavBarWid *mNavBarWid;
    Setup_MainWid *mSetupWid;
    Home_MainWid *mHomeWid;
    // Set_MainWid *mSet;
    // Test_setting *mTestSet;
    Setting *mSetting;
};
#endif // MAINWINDOW_H
