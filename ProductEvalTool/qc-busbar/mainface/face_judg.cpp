#include "face_judg.h"
#include "ui_face_judg.h"

Face_judg::Face_judg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Face_judg)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("人工确认"));
    groupBox_background_icon(this);

    initData();
}

Face_judg::~Face_judg()
{
    delete ui;
}
void Face_judg::initData()
{
    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();

    ui->supBox1->setText(mItem->supCheck1);
    ui->supBox2->setText(mItem->supCheck2);
    if(!mItem->supCheck1.isEmpty())
        ui->supBox1->setChecked(true);
    if(!mItem->supCheck2.isEmpty())
        ui->supBox2->setChecked(true);
}
void Face_judg::writeData(const QString &str1,const QString &str2,bool pass)
{
    mPro->stepRequest << str1; mPro->itemData << str2;
    mPro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if(pass) {
        mPro->stepResult << "1";
    } else {
        mPro->stepResult << "0";
    }

}
void Face_judg::on_sureButton_clicked()
{
    QString str2 = tr("经人工确认：结果成功");
    QString str3 = tr("经人工确认：结果失败");

    QString str1 = ui->lcdBox->text();
    bool ret = ui->lcdBox->isChecked();

    if(ret) writeData(str1,str2,ret);
    else  writeData(str1,str3,ret);

    ret = ui->keyBox->isChecked();
    str1 = ui->keyBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    ret = ui->alarmBox->isChecked();
    str1 = ui->alarmBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    ret = ui->testBox->isChecked();
    str1 = ui->testBox->text();
    if(ret) writeData(str1,str2,ret);
    else writeData(str1,str3,ret);

    str1 = ui->supBox1->text();
    if(!str1.isEmpty()) {
        ret = ui->supBox1->isChecked();
        if(ret) writeData(str1,str2,ret);
        else writeData(str1,str3,ret);
    }

    str1 = ui->supBox2->text();
    if(!str1.isEmpty()) {
        ret = ui->supBox2->isChecked();
        if(ret) writeData(str1,str2,ret);
        else writeData(str1,str3,ret);
    }

    this->close();

}
