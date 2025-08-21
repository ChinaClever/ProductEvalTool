#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
    mItem = Cfg::bulid()->item;

    QTimer::singleShot(10,this,SLOT(initFunSlot()));
}

Setting::~Setting()
{
    delete ui;
}

void Setting::initFunSlot()
{
    ui->groupBox2->setEnabled(false);
    ui->groupBox3->setEnabled(false);
    ui->groupBox4->setEnabled(false);
    ui->label_17->setEnabled(false);
    ui->moduleBox->setEnabled(false);

    ui->userEdit->setText(mItem->user);
    ui->cntSpin->setValue(mItem->cnt.num);//剩余数量
    ui->orderBox->setValue(mItem->cnt.all);//订单数量
    ui->moduleBox->setCurrentIndex(mItem->moduleType);
    initType(); mCnt = 0;
}

void Setting::initType()
{
    sIpCfg *dt = &(mItem->ip); //始端箱
    // ui->buzzerBox->setCurrentIndex(dt->ip_buzzer);
    ui->ISDBox->setCurrentIndex(dt->ip_ISD);
    ui->iOFBox->setCurrentIndex(dt->ip_iOF);
    ui->breakBox->setCurrentIndex(dt->ip_break);
    ui->shuntBox->setCurrentIndex(dt->ip_shunt);
    ui->lightningBox->setCurrentIndex(dt->ip_lightning);
    ui->residualBox->setCurrentIndex(dt->ip_residual);
    ui->ip_volSpin->setValue(dt->ip_vol);
    ui->ip_curSpin->setValue(dt->ip_cur);
    ui->ip_volErrSpin->setValue(dt->ip_volErr);
    ui->ip_curErrSpin->setValue(dt->ip_curErr);
    dt->ip_powErr = 1;
    ui->ip_curMaxSpin->setValue(dt->ip_curMax);
    int ver = dt->version;
    QString str = QString::number(ver/100)+"."+QString::number(ver/10%10)+"."+QString::number(ver%10);
    ui->ip_versionEdit->setText(str);

    dt->rate = transformRate(ui->ip_curMaxSpin->decimals());
    sSiCfg *dv = &(mItem->si); //插接箱
    ui->iOFBox_2->setCurrentIndex(dv->si_iOF);
    ui->outputBox->setCurrentIndex(dv->si_phaseflag);
    uchar loop = 0;
    if(dv->loopNum == 3)
        loop = 0;
    else if (dv->loopNum == 6)
        loop = 1;
    else if (dv->loopNum == 9)
        loop = 2;

    ui->loopNumBox->setCurrentIndex(loop);
    ui->si_volSpin->setValue(dv->si_vol);
    ui->si_curSpin->setValue(dv->si_cur);
    ui->si_volErrSpin->setValue(dv->si_volErr );
    ui->si_curErrSpin->setValue(dv->si_curErr );
    ui->si_curMaxSpin->setValue(dv->si_curMax);
    dv->rate = transformRate(ui->si_curMaxSpin->decimals());
    int sion = dv->version;
    QString str1 = QString::number(sion/100)+"."+QString::number(sion/10%10)+"."+QString::number(sion%10);
    ui->si_versionEdit->setText(str1);

    ui->itemBox->setCurrentIndex(dv->itemType);
}

void Setting::updateType()
{
    mItem->user = ui->userEdit->text();
    mItem->cnt.num = ui->cntSpin->value();
    mItem->cnt.all = ui->orderBox->value();
    mItem->moduleType = ui->moduleBox->currentIndex();


    sIpCfg *dt = &(mItem->ip); //设备类型
    dt->ip_buzzer = 0;
    dt->ip_ISD = ui->ISDBox->currentIndex();
    dt->ip_iOF = ui->iOFBox->currentIndex();
    dt->ip_break = ui->breakBox->currentIndex();
    dt->ip_shunt = ui->shuntBox->currentIndex();
    dt->ip_lightning = ui->lightningBox->currentIndex();
    dt->ip_residual = ui->residualBox->currentIndex();

    dt->ip_vol = ui->ip_volSpin->value();
    dt->ip_cur = ui->ip_curSpin->value();
    dt->ip_volErr = ui->ip_volErrSpin->value();
    dt->ip_curErr = ui->ip_curErrSpin->value();
    dt->ip_powErr = 1;
    dt->ip_curMin = 0;
    dt->ip_curMax = ui->ip_curMaxSpin->value();
    dt->version = ui->ip_versionEdit->text().remove(".").toUInt();

    sSiCfg *dv = &(mItem->si);
    dv->si_buzzer = 0;
    dv->si_iOF = ui->iOFBox_2->currentIndex();
    dv->si_phaseflag = ui->outputBox->currentIndex();
    uchar loop = ui->loopNumBox->currentIndex();
    if(loop == 0)
        dv->loopNum = 3;
    else if (loop == 1)
        dv->loopNum = 6;
    else if (loop == 2)
        dv->loopNum = 9;
    dv->si_vol = ui->si_volSpin->value();
    dv->si_cur = ui->si_curSpin->value();
    dv->si_volErr = ui->si_volErrSpin->value();
    dv->si_curErr = ui->si_curErrSpin->value();
    dv->si_curMin = 0;
    dv->si_curMax = ui->si_curMaxSpin->value();
    dv->version = ui->si_versionEdit->text().remove(".").toUInt();
    // qDebug()<<"---temp---"<<dv->si_vol<<dv->si_cur<<dv->si_volErr<<dv->si_curErr;

    dv->itemType = ui->itemBox->currentIndex();
}

bool Setting::dataSave()
{
    bool ret = true;
    if(ret) {
        updateType();
    }

    return ret;
}

void Setting::saveErrSlot()
{
    mCnt = 1;
    enabledSlot(true);
    ui->setBtn->setText(tr("保 存"));
}

void Setting::enabledSlot(bool en)
{
    ui->groupBox2->setEnabled(en);
    ui->groupBox3->setEnabled(en);
    ui->groupBox4->setEnabled(en);
    ui->label_17->setEnabled(en);
    ui->moduleBox->setEnabled(en);

    if(!en) {
        en = dataSave();
        if(en) {
            Cfg::bulid()->writeCfgDev();
            Cfg::bulid()->writeCnt();
        } else {
            saveErrSlot();
        }
    }
}

void Setting::on_setBtn_clicked()
{
    QString str = tr("修 改");
    bool en = ++mCnt % 2;
    if(en) str = tr("保 存");
    ui->setBtn->setText(str);
    ui->groupBox2->setEnabled(en);
    ui->groupBox3->setEnabled(en);
    ui->groupBox4->setEnabled(en);
    if(!en) {
        mItem->user = ui->userEdit->text();
        mItem->cnt.num = ui->cntSpin->value();
        sCount *cnt = &(mItem->cnt);
        cnt->all = cnt->ok = cnt->err = 0;
        Cfg::bulid()->writeCnt();
        Cfg::bulid()->initCnt();
    }

    QTimer::singleShot(50,this,SLOT(saveFunSlot()));
}

void Setting::saveFunSlot()
{
    bool en = mCnt % 2;
    enabledSlot(en);
    if(!en) {
        Cfg::bulid()->writeCnt();
        Cfg::bulid()->writeCfgDev();
    }
}

void Setting::on_userEdit_textChanged(const QString &arg1)
{
    ui->userEdit->setClearButtonEnabled(1);
}

int Setting::transformRate(int index)
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
