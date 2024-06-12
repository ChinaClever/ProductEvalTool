/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "set_ipwid.h"
#include "ui_set_ipwid.h"

Set_IpWid::Set_IpWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Set_IpWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    mUnitWid = new Set_LineUnitWid(ui->groupBox);
    QTimer::singleShot(10,this,SLOT(initFunSlot()));
}

Set_IpWid::~Set_IpWid()
{
    delete ui;
}

void Set_IpWid::initFunSlot()
{
    this->setEnabled(false);
    mItem = Cfg::bulid()->item;
    mUnitWid->init(&(mItem->ip_cfg) , 0);
    initType();
}

void Set_IpWid::initType()
{
    sIpCfg *dt = &(mItem->ip); //设备类型

    ui->filterspinBox->setValue(dt->ip_filter);

    ui->ISDBox->setCurrentIndex(dt->ip_ISD);
    ui->iOFBox->setCurrentIndex(dt->ip_iOF);
    ui->shuntBox->setCurrentIndex(dt->ip_shunt);
    ui->residualBox->setCurrentIndex(dt->ip_residual);
    ui->lightningBox->setCurrentIndex(dt->ip_lightning);
}


void Set_IpWid::updateType()
{
    sIpCfg *dt = &(mItem->ip); //设备类型
    dt->ip_buzzer = 0;
    dt->ip_filter = ui->filterspinBox->value();
    dt->ip_ISD = ui->ISDBox->currentIndex();
    dt->ip_iOF = ui->iOFBox->currentIndex();
    dt->ip_shunt = ui->shuntBox->currentIndex();
    dt->ip_residual = ui->residualBox->currentIndex();
    dt->ip_lightning = ui->lightningBox->currentIndex();
}

bool Set_IpWid::inputCheck()
{


    return true;
}

bool Set_IpWid::dataSave()
{
    bool ret = inputCheck();
    if(ret) {
        updateType();
        mUnitWid->updateData();
    }

    return ret;
}

void Set_IpWid::enabledSlot(bool en)
{
    if(mItem->modeId != START_BUSBAR) return;
    this->setEnabled(en);
    if(!en) {
        en = dataSave();
        if(en) {
            Cfg::bulid()->writeCfgDev();
        } else {
            emit errSig();
        }
    }
}
