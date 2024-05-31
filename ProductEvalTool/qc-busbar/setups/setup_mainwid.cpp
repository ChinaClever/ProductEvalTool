/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "setup_mainwid.h"
#include "ui_setup_mainwid.h"
#include "versiondlg.h"
#include "macaddr.h"

Setup_MainWid::Setup_MainWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup_MainWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    mItem = Cfg::bulid()->item;
    initSerial();
    QTimer::singleShot(rand()%13,this,SLOT(initFunSlot()));
}

Setup_MainWid::~Setup_MainWid()
{
    delete ui;
}

void Setup_MainWid::initFunSlot()
{
    initPcNum();
    initAddr();
    initLogCount();
    if(ui->addrEdit->text().isEmpty()) {
        ui->addrEdit->setText("192.168.1.15");
        mItem->Service = ui->addrEdit->text();
    }
    mUserWid = new UserMainWid(ui->stackedWid);
    ui->stackedWid->addWidget(mUserWid);

    QDate buildDate = QLocale(QLocale::English ).toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    ui->label_date->setText(buildDate.toString("yyyy-MM-dd"));
    QTimer::singleShot(5*1000,this,SLOT(checkPcNumSlot()));
}

void Setup_MainWid::checkPcNum()
{
    int num = mItem->pcNum;
    if(num < 1) {
        if(!usr_land_jur())
            MsgBox::warning(this, tr("请联系研发部设定电脑号！\n 服务设置 -> 设置功能 \n 需要管理员权限!"));
        else
            MsgBox::warning(this, tr("请自行设定电脑号！\n 服务设置 -> 设置功能 \n 需要管理员权限!"));
    }
}

void Setup_MainWid::checkAddr()
{
    QString str = ui->addrEdit->text();
    if(str.isEmpty() || !str.contains("192.168."))
        MsgBox::warning(this, tr("请设置正确格式的服务端IP!"));
}

void Setup_MainWid::checkPcNumSlot()
{
    checkPcNum(); checkAddr();
    QTimer::singleShot(20*1000,this,SLOT(checkPcNumSlot()));
}

void Setup_MainWid::initSerial()
{
    mComWid = new SerialStatusWid(ui->comWid);
    mItem->coms.ser1 = mComWid->initSerialPort(tr("Acw/Ir 串口"));

    mSourceWid = new SerialStatusWid(ui->sourceWid);
    mItem->coms.ser2 = mSourceWid->initSerialPort(tr("Gnd 串口"));

    mControlWid = new SerialStatusWid(ui->controlWid);
    mItem->coms.ser3 = mControlWid->initSerialPort(tr("Dev 串口"));
}

void Setup_MainWid::initLogCount()
{
    Cfg *con = Cfg::bulid();
    int value = con->read("log_count", 10, "Sys").toInt();

    sCfgItem *item = con->item;
    item->logCount = value * 10000;
    ui->logCountSpin->setValue(value);
}


void Setup_MainWid::writeLogCount()
{
    int arg1 = ui->logCountSpin->value();
    mItem->logCount = arg1;
    Cfg::bulid()->write("log_count", arg1, "Sys");
}

void Setup_MainWid::initAddr()
{
    Cfg *con = Cfg::bulid();
    QString value = con->read("service_addr", "", "Sys").toString();

    mItem->Service = value;
    ui->addrEdit->setText(value);
}

void Setup_MainWid::writeAddr()
{
    QString arg1 = ui->addrEdit->text();
    mItem->Service = arg1;
    Cfg::bulid()->write("service_addr", arg1, "Sys");
}

void Setup_MainWid::initPcNum()
{
    Cfg *con = Cfg::bulid();
    int value = con->read("pc_num", 0, "Sys").toInt();

    mItem->pcNum = value;
    ui->pcNumSpin->setValue(value);
}

void Setup_MainWid::writePcNum()
{
    int arg1 = ui->pcNumSpin->value();
    mItem->pcNum = arg1;
    Cfg::bulid()->write("pc_num", arg1, "Sys");
}

void Setup_MainWid::on_pcBtn_clicked()
{
    static int flg = 0;
    QString str = tr("修改");

    bool ret = usr_land_jur();
    if(!ret) {
        MsgBox::critical(this, tr("你无权进行此操作"));
        return;
    }

    if(flg++ %2) {
        ret = false;
        writePcNum();
        writeAddr();
        writeLogCount();
        Cfg::bulid()->writeCnt();
    } else {
        str = tr("保存");
    }

    ui->pcBtn->setText(str);
    ui->logCountSpin->setEnabled(ret);
    ui->addrEdit->setEnabled(ret);
    ui->pcNumSpin->setEnabled(ret);

    if(!ret) {
        bool res = true;
        QString str1 = tr("该服务端IP异常");
        QString ip = ui->addrEdit->text();
        for(int k=0; k<2; ++k) {
            res = cm_pingNet(ip);
            if(res) break;
        }
        if(res) mItem->online = true;
        if(!res) MsgBox::information(this,str1);
    }
}

void Setup_MainWid::on_verBtn_clicked()
{
    VersionDlg dlg(this);
    dlg.exec();
}
