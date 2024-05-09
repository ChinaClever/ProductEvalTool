/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "set_mainwid.h"
#include "ui_set_mainwid.h"

Set_MainWid::Set_MainWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Set_MainWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    initWid();
}

Set_MainWid::~Set_MainWid()
{
    delete ui;
}


void Set_MainWid::initWid()
{
    ui->tabWidget->tabBar()->hide();
    mWorkWid = new Set_WorkWid(ui->workWid);
    connect(mWorkWid, SIGNAL(typeSig(int)), ui->tabWidget, SLOT(setCurrentIndex(int)));

    mIpWid = new Set_IpWid(ui->tabWidget);
    ui->tabWidget->addTab(mIpWid, tr("始端箱参数设置"));
    connect(mIpWid, SIGNAL(errSig()), mWorkWid, SLOT(errSlot()));
    connect(mWorkWid, SIGNAL(enabledSig(bool)), mIpWid, SLOT(enabledSlot(bool)));

    mSiWid = new Set_SiWid(ui->tabWidget);
    ui->tabWidget->addTab(mSiWid, tr("插接箱参数设置"));
    connect(mSiWid, SIGNAL(errSig()), mWorkWid, SLOT(errSlot()));
    connect(mWorkWid, SIGNAL(enabledSig(bool)), mSiWid, SLOT(enabledSlot(bool)));

    mtemper = new Set_temper(ui->tabWidget);
    ui->tabWidget->addTab(mtemper, tr("温度模块阈值设置"));
    connect(mtemper, SIGNAL(errSig()), mWorkWid, SLOT(errSlot()));
    connect(mWorkWid, SIGNAL(enabledSig(bool)), mtemper, SLOT(enabledSlot(bool)));
}
