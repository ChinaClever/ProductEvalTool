#include "face_power.h"
#include "ui_face_power.h"

Face_Power::Face_Power(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Face_Power)
{
    ui->setupUi(this);
    initWid();
}

Face_Power::~Face_Power()
{
    delete ui;
}

void Face_Power::initWid()
{
    mDataWid = new Home_DataWid(ui->tabWidget);
    ui->tabWidget->addTab(mDataWid, "测试数据");

    mLineTabWid = new Home_LoopTabWid(ui->tabWidget);
    ui->tabWidget->addTab(mLineTabWid, "回路数据");

    mLoadWid = new Home_Loadwid(ui->tabWidget);
    ui->tabWidget->addTab(mLoadWid, "负载数据");

    ui->tabWidget->setCurrentIndex(0);
}
