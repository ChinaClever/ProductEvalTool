#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting)
{
    ui->setupUi(this);
    groupBox_background_icon(this);

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
    ui->userEdit->setText(mItem->user);
    ui->cntSpin->setValue(mItem->cnt.num);

    initType(); mCnt = 0;
}

void Setting::initType()
{
    sIpCfg *dt = &(mItem->ip); //始端箱
    // ui->buzzerBox->setCurrentIndex(dt->ip_buzzer);
    ui->filterspinBox->setValue(dt->ip_filter);
    ui->ISDBox->setCurrentIndex(dt->ip_ISD);
    ui->iOFBox->setCurrentIndex(dt->ip_iOF);
    ui->shuntBox->setCurrentIndex(dt->ip_shunt);
    ui->lightningBox->setCurrentIndex(dt->ip_lightning);
    ui->residualBox->setCurrentIndex(dt->ip_residual);

    ui->ip_volErrSpin->setValue(dt->ip_volErr / 10.0);
    ui->ip_curErrSpin->setValue(dt->ip_curErr / 10.0);
    ui->ip_powErrSpin->setValue(dt->ip_powErr / 10.0);

    sSiCfg *dv = &(mItem->si); //插接箱
    ui->filterspinBox_2->setValue(dv->si_filter);
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
    ui->si_volErrSpin->setValue(dv->si_volErr / 10.0);
    ui->si_curErrSpin->setValue(dv->si_curErr / 10.0);
}

void Setting::updateType()
{
    mItem->user = ui->userEdit->text();
    mItem->cnt.num = ui->cntSpin->value();

    sIpCfg *dt = &(mItem->ip); //设备类型
    dt->ip_buzzer = 0;
    dt->ip_filter = ui->filterspinBox->value();
    dt->ip_ISD = ui->ISDBox->currentIndex();
    dt->ip_iOF = ui->iOFBox->currentIndex();
    dt->ip_shunt = ui->shuntBox->currentIndex();
    dt->ip_lightning = ui->lightningBox->currentIndex();
    dt->ip_residual = ui->residualBox->currentIndex();

    dt->ip_vol = ui->ip_volSpin->value();
    dt->ip_cur = ui->ip_curSpin->value();
    dt->ip_pow = ui->ip_powErrSpin->value();
    dt->ip_volErr = ui->ip_volErrSpin->value() * 10;
    dt->ip_curErr = ui->ip_curErrSpin->value() * 10;
    dt->ip_powErr = ui->ip_powErrSpin->value() * 10;

    sSiCfg *dv = &(mItem->si);
    dv->si_filter = ui->filterspinBox_2->value();
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
    dv->si_volErr = ui->si_volErrSpin->value() * 10;
    dv->si_curErr = ui->si_curErrSpin->value() * 10;
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

