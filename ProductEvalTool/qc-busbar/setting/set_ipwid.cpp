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
    ui->curTypeBox->setCurrentIndex(dt->ip_curtype);
    ui->buzzerBox->setCurrentIndex(dt->ip_buzzer);
    ui->filterspinBox->setValue(dt->ip_filter);
    ui->modeBox->setCurrentIndex(dt->ip_mode);
    ui->numBox->setCurrentIndex(dt->ip_num-1);
    ui->ISDBox->setCurrentIndex(dt->ip_ISD);
    ui->iOFBox->setCurrentIndex(dt->ip_iOF);
    ui->shuntBox->setCurrentIndex(dt->ip_shunt);
    ui->residualBox->setCurrentIndex(dt->ip_residual);
    ui->lightningBox->setCurrentIndex(dt->ip_lightning);
    int ver = dt->version;
    QString str = QString::number(ver/100)+"."+QString::number(ver/10%10)+"."+QString::number(ver%10);
    ui->verlineEdit->setText(str);
}


void Set_IpWid::updateType()
{
    sIpCfg *dt = &(mItem->ip); //设备类型
    dt->ip_curtype = ui->curTypeBox->currentIndex();
    dt->ip_buzzer = ui->buzzerBox->currentIndex();
    dt->ip_filter = ui->filterspinBox->value();
    dt->ip_mode = ui->modeBox->currentIndex();
    dt->ip_num = ui->numBox->currentIndex()+1;
    dt->ip_ISD = ui->ISDBox->currentIndex();
    dt->ip_iOF = ui->iOFBox->currentIndex();
    dt->ip_shunt = ui->shuntBox->currentIndex();
    dt->ip_residual = ui->residualBox->currentIndex();
    dt->ip_lightning = ui->lightningBox->currentIndex();
    dt->version = ui->verlineEdit->text().remove(".").toUInt();
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
