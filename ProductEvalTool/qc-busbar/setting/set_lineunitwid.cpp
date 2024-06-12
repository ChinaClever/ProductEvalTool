/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "set_lineunitwid.h"
#include "ui_set_lineunitwid.h"

Set_LineUnitWid::Set_LineUnitWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Set_LineUnitWid)
{
    ui->setupUi(this);
    QGridLayout *gridLayout = new QGridLayout(parent);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(this);
    mIndex = 0;
}

Set_LineUnitWid::~Set_LineUnitWid()
{
    delete ui;
}

void Set_LineUnitWid::showAndHideWid(bool show)
{  
    ui->label_16->setHidden(show);
    ui->totalpowMinSpin->setHidden(show);
    ui->label_15->setHidden(show);
    ui->totalpowMaxSpin->setHidden(show);  
}

void Set_LineUnitWid::init(sObjCfg *obj , int index)
{
    mIndex = index;
    mDev = obj;
    ui->curMinSpin->setValue(obj->cur.min);
    ui->curMaxSpin->setValue(obj->cur.max);
    ui->volMinSpin->setValue(obj->vol.min);
    ui->volMaxSpin->setValue(obj->vol.max);
    ui->temMinSpin->setValue(obj->tem.min);
    ui->temMaxSpin->setValue(obj->tem.max);

    ui->totalpowMinSpin->setValue(obj->totalpow.min);
    ui->totalpowMaxSpin->setValue(obj->totalpow.max);   

    obj->vol.rate = transformRate(ui->volMaxSpin->decimals());
    obj->cur.rate = transformRate(ui->curMaxSpin->decimals());
    obj->tem.rate = transformRate(ui->temMaxSpin->decimals());
    obj->totalpow.rate = transformRate(ui->totalpowMaxSpin->decimals());

    if(index == START_BUSBAR){
        showAndHideWid(false);
    }else if(index == INSERT_BUSBAR){
        showAndHideWid(true);
    }
}

void Set_LineUnitWid::updateData()
{
    sObjCfg *obj = mDev;
    obj->cur.min = ui->curMinSpin->value();
    obj->cur.max = ui->curMaxSpin->value();
    obj->vol.min = ui->volMinSpin->value();
    obj->vol.max = ui->volMaxSpin->value();
    obj->tem.min = ui->temMinSpin->value();
    obj->tem.max = ui->temMaxSpin->value();

    obj->totalpow.min = ui->totalpowMinSpin->value();
    obj->totalpow.max = ui->totalpowMaxSpin->value();
}

int Set_LineUnitWid::transformRate(int index)
{
    int rate = 1;
    switch(index){
    case 0: rate = 1;break;
    case 1: rate = 10;break;
    case 2: rate = 100;break;
    case 3: rate = 1000;break;
    default: rate = 1;break;
    }
    return rate;
}
