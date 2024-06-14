#include "people_judg.h"
#include "ui_people_judg.h"

People_judg::People_judg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::People_judg)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("人工确认"));
    groupBox_background_icon(this);
    initData();

}

People_judg::~People_judg()
{
    delete ui;
}

void People_judg::initData()
{
    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();

    // ui->supEdit1->setText(mItem->supCheck1);
    // ui->supEdit2->setText(mItem->supCheck2);
    // if(!(ui->supEdit1->text().isEmpty()))
    //     ui->supBox1->setChecked(true);
    // if(!(ui->supEdit2->text().isEmpty()))
    //     ui->supBox2->setChecked(true);
}
void People_judg::writeData(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    mPro->stepRequest << str1; mPro->itemData << str2;
    mPro->test_function << str3;
    mPro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if(pass) {
        mPro->stepResult << "1";
    } else {
        mPro->stepResult << "0";
    }

}
void People_judg::on_sureButton_clicked()
{
    QString str2 = tr("符合要求");
    QString str3 = tr("不符合要求");

    QString str = tr("指示灯检查");
    QString str1 = ui->lcdBox->text();
    bool ret = ui->lcdBox->isChecked();
    if(ret) writeData(str1,str2,str,ret);
    else  writeData(str1,str3,str,ret);

    str = tr("指示灯检查");
    ret = ui->keyBox->isChecked();
    str1 = ui->keyBox->text();
    if(ret) writeData(str1,str2,str,ret);
    else writeData(str1,str3,str,ret);

    str = tr("蜂鸣器开关检查");
    ret = ui->alarmBox->isChecked();
    str1 = ui->alarmBox->text();
    if(ret) writeData(str1,str2,str,ret);
    else writeData(str1,str3,str,ret);

    str = tr("蜂鸣器开关检查");
    ret = ui->beepBox->isChecked();
    str1 = ui->beepBox->text();
    if(ret) writeData(str1,str2,str,ret);
    else writeData(str1,str3,str,ret);

    str = tr("极性检查");
    ret = ui->testBox->isChecked();
    str1 = ui->testBox->text();
    if(ret) writeData(str1,str2,str,ret);
    else writeData(str1,str3,str,ret);
    // Cfg::bulid()->writeJudgItem();

    mPro->issure = 1;
    this->close();
}

