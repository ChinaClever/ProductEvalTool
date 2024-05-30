#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
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
    mItem = Cfg::bulid()->item;
    initType(); mCnt = 0;
}

void Setting::initType()
{
    sIpCfg *dt = &(mItem->ip); //始端箱
    ui->buzzerBox->setCurrentIndex(dt->ip_buzzer);
    ui->filterspinBox->setValue(dt->ip_filter);

    ui->ISDBox->setCurrentIndex(dt->ip_ISD);
    ui->iOFBox->setCurrentIndex(dt->ip_iOF);
    ui->shuntBox->setCurrentIndex(dt->ip_shunt);
    ui->lightningBox->setCurrentIndex(dt->ip_lightning);

    sSiCfg *dv = &(mItem->si); //插接箱
    ui->buzzerBox_2->setCurrentIndex(dv->si_buzzer);
    ui->filterspinBox_2->setValue(dv->si_filter);
    ui->iOFBox_2->setCurrentIndex(dv->si_iOF);
}

void Setting::updateType()
{
    sIpCfg *dt = &(mItem->ip); //设备类型
    dt->ip_buzzer = ui->buzzerBox->currentIndex();
    dt->ip_filter = ui->filterspinBox->value();
    dt->ip_ISD = ui->ISDBox->currentIndex();
    dt->ip_iOF = ui->iOFBox->currentIndex();
    dt->ip_shunt = ui->shuntBox->currentIndex();
    dt->ip_lightning = ui->lightningBox->currentIndex();

    sSiCfg *dv = &(mItem->si);

    dv->si_buzzer = ui->buzzerBox_2->currentIndex();
    dv->si_filter = ui->filterspinBox_2->value();
    dv->si_iOF = ui->iOFBox_2->currentIndex();
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
    if(mItem->modeId != START_BUSBAR) return;
    ui->groupBox2->setEnabled(en);
    ui->groupBox3->setEnabled(en);
    if(!en) {
        en = dataSave();
        if(en) {
            Cfg::bulid()->writeCfgDev();
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

    QTimer::singleShot(50,this,SLOT(saveFunSlot()));
}

void Setting::saveFunSlot()
{
    bool en = mCnt % 2;
    enabledSlot(en);
    if(!en) Cfg::bulid()->writeCfgDev();
}
