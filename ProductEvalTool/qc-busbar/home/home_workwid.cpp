/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "home_workwid.h"
#include "ui_home_workwid.h"



Home_WorkWid::Home_WorkWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Home_WorkWid)
{
    ui->setupUi(this);
    mId = 1;
    initWid();
    initLayout();
}

Home_WorkWid::~Home_WorkWid()
{
    delete ui;
}

void Home_WorkWid::initWid()
{
    mCfgm = Cfg::bulid()->item;
    mItem = TestConfig::bulid()->item;
    mPacket = sDataPacket::bulid();
    mDev = mPacket->getDev();
    mPro = mPacket->getPro();
    mLogs = Power_Logs::bulid(this);
    mPro->step = Test_End;

    mVolInsul = new Face_Volinsul(ui->stackedWid);
    ui->stackedWid->addWidget(mVolInsul);

    mPower = new Face_Power(ui->stackedWid2);
    ui->stackedWid2->addWidget(mPower);

    mDataSave = new TestDataSave(this);
    mSafrtyThread = new Test_safety(this);
    connect(mSafrtyThread, SIGNAL(overSig()), this, SLOT(overSlot()));
    connect(mVolInsul, &Face_Volinsul::StatusSig, this, &Home_WorkWid::StatusSlot);

    connect(this, &Home_WorkWid::startSig, this, &Home_WorkWid::updateWidSlot);
    connect(Json_Pack::bulid(this), &Json_Pack::httpSig, this, &Home_WorkWid::insertTextslots);

    mPowThread = new Power_CoreThread(this);
    connect(this , SIGNAL(clearStartEleSig()), mPowThread, SLOT(clearStartEleSlot()));
    connect(mPowThread,&Power_CoreThread::finshSig, this, &Home_WorkWid::StatusSlot);
    connect(mPowThread, &Power_CoreThread::TipSig , mPower, &Face_Power::TextSlot);

    mPowDev = Power_DevRead::bulid(this);
    connect(mPowDev, &Power_DevRead::StepSig , mPower, &Face_Power::TextSlot);


    mModbus = Ad_Modbus::bulid(this);
    connect(mModbus,&Ad_Modbus::sendNumAndIndexSig, mPowThread, &Power_CoreThread::getNumAndIndexSlot);
    connect(mModbus,&Ad_Modbus::sendDelaySig, mPowThread, &Power_CoreThread::getDelaySlot);

    timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutDone()));
}



void Home_WorkWid::overSlot()
{
    QString str = tr("请去掉被测试产品线缆。");
    MsgBox::information(this,str);
    mItem->mode = Test_Over;
    mVolInsul->resultSlot();
    mDataSave->saveTestData();
}

void Home_WorkWid::overTest()
{
    bool ret = MsgBox::information(this, tr("是否停止测试?"));
    if(ret){
        overSlot();
    }
}

void Home_WorkWid::startTest()
{
    int ret = true;
    if(ret == QDialog::Accepted ) {
        QString str = tr("\n将测试仪高压线（红色）接产品输入/输出L、N\n线，黑色线接产品输入PE线，另一线缆（红色）接产品机壳。\n");
        ret = MsgBox::question(this,str);
        if(ret) {
            QString str = tr("请确认");
            ret = MsgBox::question(this,str);
            if(ret) {
                mItem->mode = Test_Start;
                mSafrtyThread->startThread();
                mVolInsul->startSlot();
            }
        }
    }
}

void Home_WorkWid:: updateWidSlot(int id)
{
    switch (id) {
    case Test_Start: startTest(); break;
    case Test_Over: overTest(); break;
    }
}

void Home_WorkWid::initLayout()
{
    QPalette pl = ui->textEdit->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(255,0,0,0)));
    ui->textEdit->setPalette(pl);
    QGridLayout *gridLayout = new QGridLayout(this->parentWidget());
    gridLayout->setContentsMargins(0, 7, 0, 0);
    gridLayout->addWidget(this);

}

void Home_WorkWid::setTextColor()
{
    QColor color("black");
    bool pass = mPro->pass.first();
    if(!pass) color = QColor("red");
    ui->textEdit->moveCursor(QTextCursor::Start);

    QTextCharFormat fmt;//文本字符格式
    fmt.setForeground(color);// 前景色(即字体色)设为color色
    QTextCursor cursor = ui->textEdit->textCursor();//获取文本光标
    cursor.mergeCharFormat(fmt);//光标后的文字就用该格式显示
    ui->textEdit->mergeCurrentCharFormat(fmt);//textEdit使用当前的字符格式
}

void Home_WorkWid::insertText()
{
    if(mPro->status.size()) {
        setTextColor();
        QString str = QString::number(mId++) + "、"+ mPro->status.first() + "\n";
        ui->textEdit->insertPlainText(str);
        mPro->status.removeFirst();
        mPro->pass.removeFirst();
    }
}

void Home_WorkWid::insertTextslots(QString str , bool ret)
{
    mPro->pass << ret;
    mPro->status << str;

    mPro->itemName<< str;
    mPro->uploadPass<< ret;

}

QString Home_WorkWid::getTime()
{
    QTime t(0,0,0,0);
    t = t.addSecs(mPro->startTime.secsTo(QTime::currentTime()));
    return  tr("%1").arg(t.toString("mm:ss"));
}

void Home_WorkWid::updateTime()
{
    QString str = getTime();
    QString style = "background-color:yellow; color:rgb(0, 0, 0);";
    style += "font:100 34pt \"微软雅黑\";";

    ui->timeLab->setText(str);
    ui->timeLab->setStyleSheet(style);

    str = mPro->startTime.toString("hh:mm:ss");
    ui->startLab->setText(str);
    ui->endLab->setText("---");
}

void Home_WorkWid::updateResult()
{
    QString style;
    QString str = tr("---");
    if (Test_Fail == mPro->result) {
        str = tr("失败");
        style = "background-color:red; color:rgb(255, 255, 255);";
    } else {
        str = tr("成功");
        style = "background-color:green; color:rgb(255, 255, 255);";
    }
    style += "font:100 34pt \"微软雅黑\";";

    ui->timeLab->setText(str);
    ui->timeLab->setStyleSheet(style);

    str = QTime::currentTime().toString("hh:mm:ss");
    ui->endLab->setText(str);
    mPro->step = Test_End;
}

void Home_WorkWid::updateWid()
{
    QString str = mDev->devType.sn;
    mPro->moduleSN = str;
    mItem->sn.sn = ui->codeEit->text();
    mPro->product_sn = ui->codeEit->text();
    str = mDev->devType.dev_type;
    mPro->productType = str;
    int ver = get_share_mem()->box[mCfgm->addr-1].version;
    mPro->softwareVersion = QString::number(ver/100)+"."+QString::number(ver/10%10)+"."+QString::number(ver%10);

    if(mPro->step < Test_Over) {
        updateTime();
    } else if(mPro->step < Test_End) {
        updateResult();
    }
}

void Home_WorkWid::timeoutDone()
{
    if(mPro->step) {
        insertText();
        updateWid();
    }
}

bool Home_WorkWid::initSerialVol()
{
    QString str;  mId = 1; mFirst = 1;
    sSerial *coms = &(mCfgm->coms);
    Cfg::bulid()->setAddr(mCfgm->addr);
    ui->textEdit->clear();
    bool ret = false;
    if(mCfgm->modeId == TEMPER_BUSBAR) {
        ret = true;
    } else{
        ret = coms->ser1->isOpened();
        if(!ret){MsgBox::critical(this, tr("请先打绝缘/耐压串口")); return ret;}
    }

        return ret;
}

bool Home_WorkWid::initSerialGND()
{
    QString str;  mId = 1; mFirst = 1;
    sSerial *coms = &(mCfgm->coms);
    Cfg::bulid()->setAddr(mCfgm->addr);
    ui->textEdit->clear();
    bool ret = false;
    if(mCfgm->modeId == TEMPER_BUSBAR){
        ret = true;
    } else{
        ret = coms->ser2->isOpened();
        if(!ret){MsgBox::critical(this, tr("请先打开GND串口")); return ret;}
    }

    return ret;
}

bool Home_WorkWid::initSerial()
{
    QString str;  mId = 1; mFirst = 1;
    sSerial *coms = &(mCfgm->coms);
    Cfg::bulid()->setAddr(mCfgm->addr);
    ui->textEdit->clear();
    bool ret = false;
    if(mCfgm->modeId == TEMPER_BUSBAR){
        ret = true;
    }else{
        ret = coms->ser3->isOpened();
        if(!ret){MsgBox::critical(this, tr("请先打开级联串口")); return ret;}
    }

    return ret;
}

void Home_WorkWid::AcwStatus(bool ret)
{
    if(ret) ui->acwLab->setStyleSheet("background-color:green; color:rgb(255, 255, 255);");
    else ui->acwLab->setStyleSheet("background-color:red; color:rgb(255, 255, 255);");
}

void Home_WorkWid::GndStatus(bool ret)
{
    if(ret) ui->gndLab->setStyleSheet("background-color:green; color:rgb(0, 0, 0);");
    else ui->gndLab->setStyleSheet("background-color:red; color:rgb(0, 0, 0);");
}

void Home_WorkWid::VolStatus(bool ret)
{
    if(ret) ui->volLab->setStyleSheet("background-color:green; color:rgb(0, 0, 0);");
    else  ui->volLab->setStyleSheet("background-color:red; color:rgb(0, 0, 0);");
}

void Home_WorkWid::LoadStatus(bool ret)
{
    if(ret) ui->loadLab->setStyleSheet("background-color:green; color:rgb(0, 0, 0);");
    else ui->loadLab->setStyleSheet("background-color:red; color:rgb(0, 0, 0);");
}

void Home_WorkWid::BreakerStatus(bool ret)
{
    if(ret) ui->breakLab->setStyleSheet("background-color:green; color:rgb(0, 0, 0);");
    else ui->breakLab->setStyleSheet("background-color:red; color:rgb(0, 0, 0);");
}

void Home_WorkWid::StatusSlot(bool ret)
{
    switch (mItem->work_mode) {
    case 0: AcwStatus(ret); break;
    case 1: GndStatus(ret); break;
    case 2: VolStatus(ret); break;
    case 3: LoadStatus(ret); break;
    case 4: BreakerStatus(ret); break;
    default:
        break;
    }
}

void Home_WorkWid::ItemStatus()
{
    switch (mItem->work_mode) {
    case 0: {mPro->test_step = "安规测试"; mPro->test_item = ui->vol_insulBtn->text();
            ui->acwLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");
            break;
    }
    case 1: {mPro->test_step = "安规测试"; mPro->test_item = ui->groundBtn->text();
            ui->gndLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");
            break;
    }
    case 2: {mPro->test_step = "电力测试"; mPro->test_item = ui->volBtn->text();
            ui->volLab->setStyleSheet("background-color:yellow;color:rgb(0, 0, 0);");
            break;
    }
    case 3: {mPro->test_step = "电力测试";  mPro->test_item = ui->loadBtn->text();
            ui->loadLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");
            break;
    }
    case 4: {mPro->test_step = "电力测试"; mPro->test_item = ui->breakerBtn->text();
            ui->breakLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");
            break;
    }
    default:
        break;
    }
}

void Home_WorkWid::on_vol_insulBtn_clicked()//耐压/绝缘
{
    ui->stackedWid->show();
    mItem->work_mode = 0;
    if(mPro->step == Test_End){
        bool ret = initSerialVol();
        if(ret) {
            mPacket->init(); ItemStatus();
            int mode = Test_Over;
            if(mItem->mode != Test_Start) {
                mode = Test_Start;
            }
            emit startSig(mode);
        }
    }else {
        overTest();
        AcwStatus(false);
        mPro->result = Test_Fail;
        updateResult();
    }
}

void Home_WorkWid::on_groundBtn_clicked()//接地
{
    ui->stackedWid->show();
    mItem->work_mode = 1;
    if(mPro->step == Test_End){
        bool ret = initSerialGND();
        if(ret) {
            mPacket->init(); ItemStatus();
            int mode = Test_Over;
            if(mItem->mode != Test_Start) {
                mode = Test_Start;
            }
            emit startSig(mode);
        }
    }else {
        overTest();
        GndStatus(false);
        mPro->result = Test_Fail;
        updateResult();
    }
}

void Home_WorkWid::on_volBtn_clicked()
{
    ui->stackedWid->hide();
    mItem->work_mode = 2;
    if(mPro->step == Test_End) {
        bool ret = initSerial();
        if(ret) {
            mPacket->init();
            ItemStatus();
            mPowThread->start();
        }
    }else {
        bool ret = MsgBox::question(this, tr("确定需要提前结束？"));
        if(ret) {
            VolStatus(!ret); mPro->result = Test_Fail;
            updateResult();
        }
    }
}

void Home_WorkWid::on_loadBtn_clicked()
{
    ui->stackedWid->hide();
    mItem->work_mode = 3; 
    if(mPro->step == Test_End) {
        bool ret = initSerial();
        if(ret) {
            mPacket->init();
            ItemStatus();
            mPowThread->start();
        }
    }else{
        bool ret = MsgBox::question(this, tr("确定需要提前结束？"));
        if(ret) {
            LoadStatus(!ret); mPro->result = Test_Fail;
            updateResult();
        }
    }
}

void Home_WorkWid::on_breakerBtn_clicked()
{
    ui->stackedWid->hide();
    mItem->work_mode = 4;
    if(mPro->step == Test_End) {
        bool ret = initSerial();
        if(ret) {
            mPacket->init();
            ItemStatus();
            mPowThread->start();
        }
    }else {
        bool ret = MsgBox::question(this, tr("确定需要提前结束？"));
        if(ret) {
            BreakerStatus(!ret); mPro->result = Test_Fail;
            updateResult();
        }
    }
}

void Home_WorkWid::on_clearBtn_clicked()
{
    clearStartEleSig();
}

void Home_WorkWid::on_codeEit_textChanged(const QString &arg1)
{
    ui->codeEit->setClearButtonEnabled(1);
}

