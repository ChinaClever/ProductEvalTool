/*
 *
 *
 *  Created on: 2018年10月1日
 *      Author: Lzy
 */
#include "testresultwid.h"
#include "ui_testresultwid.h"

TestResultWid::TestResultWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestResultWid)
{
    ui->setupUi(this);
    groupBox_background_icon(this);
    mItem = TestConfig::bulid()->item;
    initSerialPort();
    initSerialGNDPort();
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();

    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()),this, SLOT(progressSlot()));

}

TestResultWid::~TestResultWid()
{
    delete ui;
}

void TestResultWid::startSlot()
{
    QString name = mItem->sn.name;
    mPro->productType = mItem->sn.name;
    QString sn = mItem->sn.sn;
    mItem->progress.errNum = mItem->progress.finishNum = mItem->progress.okNum = 0;

    QString str = tr("产品:%1").arg(name);
    ui->titleTab->setText(str);
    str = tr("条码：%1").arg(sn);
    ui->snLab->setText(str);
    mPro->moduleSN = mItem->sn.sn;
    ui->itemNumLab->clear();
    ui->statusLab->clear();
    ui->progressBar->setValue(0);
    //ui->progressBar->setBgColor(Qt::green);
    ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:green;}" );

    timer->start(200);
    ui->startBtn->setText(tr("停止测试"));
}

void TestResultWid::resultSlot()
{
    bool p = true;
    if(mItem->progress.errNum)  p = false;
    timer->stop();

    mItem->progress.allNum = mItem->progress.finishNum;
    progressSlot();

    ui->startBtn->setText(tr("立即测试"));
    if(!p)
    {
        ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        ui->statusLab->setText(tr("测试失败!!!"));
        mPro->uploadPassResult = 0;
    }
    else{
        ui->statusLab->setText(tr("测试成功!!!"));
        mPro->uploadPassResult = 1;
    }
    // mPacket->delay(2);
    // Json_Pack::bulid()->http_post("safety/add","192.168.1.12");
}

void TestResultWid::progressSlot()
{
    sTestProgress *arg = &(mItem->progress);

    int progress = (arg->finishNum * 100.0) / arg->allNum;
    //ui->progressWater->setValue(progress);
    ui->progressBar->setValue(progress);
    ui->statusLab->setText(arg->status);

    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::black);
    int ok = (arg->okNum * 100.0) / arg->allNum;
    if(arg->errNum)
    {
        pe.setColor(QPalette::WindowText,Qt::red);//ui->progressWater->setBgColor(Qt::red);
        //ui->progressBar->setBgColor(Qt::red);
        ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
    }
    QString str = tr("测试项目数:%1  失败项目数：%2  项目测试通过率：%3%").arg(arg->allNum).arg(arg->errNum).arg(ok);
    ui->itemNumLab->setText(str);
    ui->itemNumLab->setPalette(pe);
}



void TestResultWid::on_startBtn_clicked()
{
    bool ret = checkSerial();
    bool retGND = checkSerialGND();
    if(1) {
        int mode = Test_Complete;
        if(mItem->mode != Test_Begin) {
            mode = Test_Begin;
        }

        emit startSig(mode);
    }
}

bool TestResultWid::checkSerial()
{
    SerialPort *serial = mSerialDlg->getSerialPort();
    bool ret = serial->isOpened();
    if(!ret)
    {
       ret = MsgBox::critical(this, tr("请先打开串口!!"));
        if(ret) {
            on_comBtn_clicked();
        }
    }
    return ret;
}

bool TestResultWid::checkSerialGND()
{
    SerialPort *serial = mSerialGNDDlg->getSerialPort();
    bool ret = serial->isOpened();
    if(!ret)
    {
        ret = MsgBox::critical(this, tr("请先打开GND串口!!"));
        if(ret) {
            on_comBtnGND_clicked();
        }
    }
    return ret;
}

void TestResultWid::on_comBtn_clicked()
{
    if(mItem->mode != Test_Begin) {
        mSerialDlg->exec();
        updateSerialWid();
    } else {
        MsgBox::critical(this, tr("请先停止测试！！！"));
    }
}


/**
 * @brief 初始化串口
 * @return
 */
bool TestResultWid::initSerialPort()
{
    bool ret = false;

    mSerialDlg = new SerialPortDlg(this);
    SerialPort *serial = mSerialDlg->getSerialPort();
    mItem->serial = serial;
    QString com = TestConfig::bulid()->getSerialName();
    if(!com.isEmpty())
    {
        ret = serial->isContains(com);
        if(ret) {
            ret = serial->open(com);
            updateSerialWid();
        }
    }

    return ret;
}

/**
 * @brief 初始化串口
 * @return
 */
bool TestResultWid::initSerialGNDPort()
{
    bool ret = false;
    mSerialGNDDlg = new SerialPortDlg(this);
    SerialPort *serialGND = mSerialGNDDlg->getSerialPort();
    mItem->serialGND = serialGND;
    QString com = TestConfig::bulid()->getSerialName("GND");
    if(!com.isEmpty())
    {
        ret = serialGND->isContains(com);
        if(ret) {
            ret = serialGND->open(com);
            updateSerialGNDWid();
        }
    }


    return ret;
}


/**
 * @brief 更新串口窗口
 */
void TestResultWid::updateSerialWid()
{
    QPalette pe;
    SerialPort *serial = mSerialDlg->getSerialPort();
    QString str = serial->getSerialName();

    if(serial->isOpened()) {
        TestConfig::bulid()->setSerialName(str);
        str += tr(" 已打开");
        pe.setColor(QPalette::WindowText,Qt::black);
    } else {
        str += tr(" 串口未打开");
        pe.setColor(QPalette::WindowText,Qt::red);
    }

    ui->serialLab->setText(str);
    ui->serialLab->setPalette(pe);
}

void TestResultWid::updateSerialGNDWid()
{
    QPalette pe;
    SerialPort *serialGND = mSerialGNDDlg->getSerialPort();
    QString str = serialGND->getSerialName();

    if(serialGND->isOpened()) {
        TestConfig::bulid()->setSerialName(str , "GND");
        str += tr(" 已打开");
        pe.setColor(QPalette::WindowText,Qt::black);
    } else {
        str += tr(" 串口未打开");
        pe.setColor(QPalette::WindowText,Qt::red);
    }

    ui->serialLabGND->setText(str);
    ui->serialLabGND->setPalette(pe);
}

void TestResultWid::on_comBtnGND_clicked()
{
    if(mItem->mode != Test_Begin) {
        mSerialGNDDlg->exec();
        updateSerialGNDWid();
    } else {
        MsgBox::critical(this, tr("请先停止测试！！！"));
    }
}
