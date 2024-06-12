/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "set_siwid.h"
#include "ui_set_siwid.h"

Set_SiWid::Set_SiWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Set_SiWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    mUnitWid = new Set_LineUnitWid(ui->groupBox);
    QTimer::singleShot(13,this,SLOT(initFunSlot()));
}

Set_SiWid::~Set_SiWid()
{
    delete ui;
}


void Set_SiWid::initFunSlot()
{
    this->setEnabled(false);
    mItem = Cfg::bulid()->item;
    mUnitWid->init(&(mItem->si_cfg) , 1);
    initType();
}

void Set_SiWid::initType()
{
    sSiCfg *dt = &(mItem->si); //设备类型

    ui->filterspinBox->setValue(dt->si_filter);
    ui->iOFBox->setCurrentIndex(dt->si_iOF);
    ui->phaseBox->setCurrentIndex(dt->si_phaseflag);
    uchar loop = 0;
    if(dt->loopNum == 3)
        loop = 0;
    else if (dt->loopNum == 6)
        loop = 1;
    else if (dt->loopNum == 9)
        loop = 2;

    ui->loopNumBox->setCurrentIndex(loop);
}

void Set_SiWid::updateType()
{
    sSiCfg *dt = &(mItem->si);
    dt->si_buzzer = 0;
    dt->si_filter = ui->filterspinBox->value();
    dt->si_iOF = ui->iOFBox->currentIndex();
    dt->si_phaseflag = ui->phaseBox->currentIndex();
    uchar loop = ui->loopNumBox->currentIndex();
    if(loop == 0)
        dt->loopNum = 3;
    else if (loop == 1)
        dt->loopNum = 6;
    else if (loop == 2)
        dt->loopNum = 9;
}

bool Set_SiWid::inputCheck()
{
    return true;
}

bool Set_SiWid::dataSave()
{
    bool ret = inputCheck();
    if(ret) {
        updateType();
        mUnitWid->updateData();
    }

    return ret;
}

void Set_SiWid::enabledSlot(bool en)
{
    if(mItem->modeId != INSERT_BUSBAR) return;

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
