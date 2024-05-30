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

    ui->supEdit1->setText(mItem->supCheck1);
    ui->supEdit2->setText(mItem->supCheck2);
    if(!(ui->supEdit1->text().isEmpty()))
        ui->supBox1->setChecked(true);
    if(!(ui->supEdit2->text().isEmpty()))
        ui->supBox2->setChecked(true);
}
void People_judg::writeData(const QString &str1,const QString &str2,bool pass)
{
    mPro->stepRequest << str1; mPro->itemData << str2;
    mPro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if(pass) {
        mPro->stepResult << "1";
    } else {
        mPro->stepResult << "0";
    }

}
void People_judg::on_sureButton_clicked()
{
    QString str2 = tr("经人工确认：通过，屏幕各界面显示正常");
    QString str3 = tr("经人工确认：不通过，屏幕各界面显示异常");

    QString str1 = ui->lcdBox->text();
    bool ret = ui->lcdBox->isChecked();
    if(ret) writeData(str1,str2,ret);
    else  writeData(str1,str3,ret);

    str2 = tr("经人工确认：通过，按键功能正常");
    str3 = tr("经人工确认：不通过，按键功能异常");
    ret = ui->keyBox->isChecked();
    str1 = ui->keyBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    str2 = tr("经人工确认：通过，预警、告警正常");
    str3 = tr("经人工确认：不通过，预警、告警异常");
    ret = ui->alarmBox->isChecked();
    str1 = ui->alarmBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    str2 = tr("经人工确认：通过，极性测试成功");
    str3 = tr("经人工确认：不通过，极性测试失败");
    ret = ui->testBox->isChecked();
    str1 = ui->testBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    str2 = tr("经人工确认：通过");
    str3 = tr("经人工确认：不通过");
    mItem->supCheck1.clear();
    str1 = ui->supEdit1->text();
    if(!str1.isEmpty()) {
        mItem->supCheck1 = str1;
        ret = ui->supBox1->isChecked();
        if(ret) writeData(str1,str2,ret);
        else writeData(str1,str3,ret);
    }

    mItem->supCheck2.clear();
    str1 = ui->supEdit2->text();
    if(!str1.isEmpty()) {
        mItem->supCheck2 = str1;
        ret = ui->supBox2->isChecked();
        if(ret) writeData(str1,str2,ret);
        else writeData(str1,str3,ret);
    }   
    Cfg::bulid()->writeJudgItem();
    mPro->issure = 1;
    this->close();
}

