#ifndef TEST_SETTING_H
#define TEST_SETTING_H

#include <QWidget>
#include "backcolourcom.h"

namespace Ui {
class Test_setting;
}

class Test_setting : public QWidget
{
    Q_OBJECT

public:
    explicit Test_setting(QWidget *parent = nullptr);
    ~Test_setting();

private:
    Ui::Test_setting *ui;
};

#endif // TEST_SETTING_H
