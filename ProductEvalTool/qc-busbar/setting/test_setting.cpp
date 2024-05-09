#include "test_setting.h"
#include "ui_test_setting.h"

Test_setting::Test_setting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Test_setting)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
}

Test_setting::~Test_setting()
{
    delete ui;
}
