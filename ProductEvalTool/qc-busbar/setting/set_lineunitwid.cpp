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
    ui->label->setHidden(show);
    ui->lineVolMinSpin->setHidden(show);
    ui->label_3->setHidden(show);
    ui->lineVolMaxSpin->setHidden(show);
    ui->label_7->setHidden(show);
    ui->hzMinSpin->setHidden(show);
    ui->label_4->setHidden(show);
    ui->hzMaxSpin->setHidden(show);
    ui->label_16->setHidden(show);
    ui->totalpowMinSpin->setHidden(show);
    ui->label_15->setHidden(show);
    ui->totalpowMaxSpin->setHidden(show);
    ui->label_18->setHidden(show);/////////////////////////////////////////////////////////////////////////////
    ui->zerocurMinSpin->setHidden(show);
    ui->label_17->setHidden(show);
    ui->zerocurMaxSpin->setHidden(show);/////////////////////////////////////////////////////////////////////////////
    ui->label_2->setHidden(show);
    ui->reMaxSpin->setHidden(show);
    ui->label_17->hide();
    ui->zerocurMinSpin->hide();
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
    ui->powMinSpin->setValue(obj->pow.min);
    ui->powMaxSpin->setValue(obj->pow.max);

    ui->totalpowMinSpin->setValue(obj->totalpow.min);
    ui->totalpowMaxSpin->setValue(obj->totalpow.max);
    ui->lineVolMinSpin->setValue(obj->linevol.min);
    ui->lineVolMaxSpin->setValue(obj->linevol.max);
    ui->hzMinSpin->setValue(obj->hz.min);
    ui->hzMaxSpin->setValue(obj->hz.max);
    ui->zerocurMinSpin->setValue(obj->zerocur.min);
    ui->zerocurMaxSpin->setValue(obj->zerocur.max);

    ui->reMaxSpin->setValue(obj->recur.max);

    obj->vol.rate = transformRate(ui->volMaxSpin->decimals());
    obj->cur.rate = transformRate(ui->curMaxSpin->decimals());
    obj->tem.rate = transformRate(ui->temMaxSpin->decimals());
    obj->pow.rate = transformRate(ui->powMaxSpin->decimals());
    obj->totalpow.rate = transformRate(ui->totalpowMaxSpin->decimals());
    obj->linevol.rate = transformRate(ui->lineVolMaxSpin->decimals());
    obj->hz.rate = transformRate(ui->hzMaxSpin->decimals());
    obj->zerocur.rate = transformRate(ui->zerocurMaxSpin->decimals());
    obj->recur.rate = transformRate(ui->reMaxSpin->decimals());

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
    obj->pow.min = ui->powMinSpin->value();
    obj->pow.max = ui->powMaxSpin->value();

    obj->totalpow.min = ui->totalpowMinSpin->value();
    obj->totalpow.max = ui->totalpowMaxSpin->value();
    obj->linevol.min = ui->lineVolMinSpin->value();
    obj->linevol.max = ui->lineVolMaxSpin->value();
    obj->hz.min = ui->hzMinSpin->value();
    obj->hz.max = ui->hzMaxSpin->value();
    obj->zerocur.min = ui->zerocurMinSpin->value();
    obj->zerocur.max = ui->zerocurMaxSpin->value();

    obj->recur.max = ui->reMaxSpin->value();


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
