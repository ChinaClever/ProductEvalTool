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
    mPacketEng = datapacket_English::bulid();
    ePro = mPacketEng->getPro();
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

void People_judg::writeData_Eng(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    ePro->stepRequest << str1; ePro->itemData << str2;
    ePro->test_function << str3;
    ePro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if(pass) {
        ePro->stepResult << "1";
    } else {
        ePro->stepResult << "0";
    }

}

void People_judg::on_sureButton_clicked()
{
    QString str2 = tr("符合要求"); QString eng2 = tr("Meet a requirement");
    QString str3 = tr("不符合要求"); QString eng3 = tr("Not Satisfiable");

    QString str = tr("指示灯检查"); QString eng = tr("Indicator light inspection");
    QString str1 = ui->lcdBox->text();
    QString eng1 = tr("Generate an alarm, the indicator light turns red and flashes");
    bool ret = ui->lcdBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    ret = ui->keyBox->isChecked();
    str1 = ui->keyBox->text();
    eng1 = tr("Release the alarm, the indicator light will turn green and flash");
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    str = tr("蜂鸣器开关检查"); eng = tr("Buzzer switch inspection");
    ret = ui->alarmBox->isChecked();
    str1 = ui->alarmBox->text();
    eng1 = tr("Generate an alarm, the buzzer sounds loud and clear");
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    str = tr("蜂鸣器开关检查");
    ret = ui->beepBox->isChecked();
    str1 = ui->beepBox->text();
    eng1 = tr("Release the alarm and stop the buzzer sound");
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    str = tr("极性检查"); eng = tr("Polarity check");
    ret = ui->testBox->isChecked();
    str1 = ui->testBox->text();
    eng1 = tr("Connect the output end to the load and check that the polarity test module indicator lights 1 and 2 are on");
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }
    // Cfg::bulid()->writeJudgItem();

    mPro->issure = 1;
    this->close();
}

