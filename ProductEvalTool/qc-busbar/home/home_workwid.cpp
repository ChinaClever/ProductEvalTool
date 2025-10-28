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
    initTypeComboBox();
    Cfg::bulid()->readQRcode();
    ui->codeEit->setText(mCfgm->pn);
    ui->stopFag->hide();
    QTimer::singleShot(7*1000,this,SLOT(PingSlot())); //延时初始化
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
    mPacketEng = datapacket_English::bulid();
    mDev = mPacket->getDev();
    mPro = mPacket->getPro();
    eDev = mPacketEng->getDev();
    ePro = mPacketEng->getPro();
    mLogs = Power_Logs::bulid(this);
    mPro->step = Test_End;
    mCfgm->online = false;

    mJudg = new People_judg(this);
    mSafePeo = new People_Safety(this);

    mVolInsul = new Face_Volinsul(ui->stackedWid);
    ui->stackedWid->addWidget(mVolInsul);
    connect(mVolInsul, &Face_Volinsul::StatusSig, this, &Home_WorkWid::StatusSlot);
    connect(mVolInsul, &Face_Volinsul::finshSig, this, &Home_WorkWid::SafeSlot);
    connect(mVolInsul, &Face_Volinsul::overSig, this, &Home_WorkWid::JudgSlots);

    mSafrtyThread = new Test_safety(this);
    connect(mSafrtyThread, SIGNAL(overSig()), this, SLOT(overSlot()));

    mThread = new Test_Thread(this);
    connect(mThread, &Test_Thread::messageSig, this, &Home_WorkWid::SafeStop);

    mPower = new Face_Power(ui->stackedWid2);
    ui->stackedWid2->addWidget(mPower);


    connect(this, &Home_WorkWid::startSig, this, &Home_WorkWid::updateWidSlot);
    connect(Json_Pack::bulid(this), &Json_Pack::httpSig, this, &Home_WorkWid::insertTextslots);

    mPowThread = new Power_CoreThread(this);
        connect(mPowThread,&Power_CoreThread::changeLoadSig,this,&Home_WorkWid::changeLoadSlot);
        connect(this,&Home_WorkWid::changLoadEmitSig,mPowThread,&Power_CoreThread::changLoadEmitSlot);
    // connect(this , SIGNAL(clearStartEleSig()), mPowThread, SLOT(clearStartEleSlot()));
    connect(mPowThread,&Power_CoreThread::finshSig, this, &Home_WorkWid::StatusSlot);
    connect(mPowThread,&Power_CoreThread::JudgSig, this, &Home_WorkWid::JudgSlots);
    // connect(mPowThread, &Power_CoreThread::TipSig , mPower, &Face_Power::TextSlot);
    // connect(mPowThread, &Power_CoreThread::ImageSig , mPower, &Face_Power::ImageSlot);

    mPowDev = Power_DevRead::bulid(this);

    connect(mPowDev, &Power_DevRead::PloarSig , this, &Home_WorkWid::polarSlot);
    // connect(mPowDev, &Power_DevRead::StepSig , mPower, &Face_Power::TextSlot);
    // connect(mPowDev, &Power_DevRead::CurImageSig , mPower, &Face_Power::ImageSlot);

    mDevSn = Sn_DevId::bulid(this);
    // connect(mDevSn, &Sn_DevId::TipImageSig , mPower, &Face_Power::ImageSlot);
    // connect(mDevSn, &Sn_DevId::TipSig , mPower, &Face_Power::TextSlot);

    // updateSig();
    timer = new QTimer(this);
    timer->start(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutDone()));
}

void Home_WorkWid::PingSlot()
{
    QString ip;
    bool ret = false;
    QString str = tr("服务端IP异常");
    for(int k=0; k<2; ++k) {
        if(!ret) {
            mPacket->delay(2);
            ip = mCfgm->Service;
            ret = cm_pingNet(ip);
        }
        if(ret) break;
    }
    if(ret) mCfgm->online = true;
    if(!ret) MsgBox::information(this,str);
}

void Home_WorkWid::overSlot()
{
    mItem->mode = Test_Over;
    mVolInsul->resultSlot();

}
void Home_WorkWid::polarSlot(QString str)
{
    bool ret = MsgBox::question(this, str);

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
        mItem->mode = Test_Start;
        mSafrtyThread->startThread();
        if(mCfgm->modeId == 2) mThread->startThread();
        mVolInsul->startSlot();

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

    mPower->ClearText();

    if(mItem->work_mode == 2) ui->safeSnEit->clear();

    disconnect(mDevSn, &Sn_DevId::TipImageSig , mPower, &Face_Power::ImageSlot);
    disconnect(mDevSn, &Sn_DevId::TipSig , mPower, &Face_Power::TextSlot);
    disconnect(mPowDev, &Power_DevRead::StepSig , mPower, &Face_Power::TextSlot);
    disconnect(mPowDev, &Power_DevRead::CurImageSig , mPower, &Face_Power::ImageSlot);
    disconnect(mPowThread, &Power_CoreThread::TipSig , mPower, &Face_Power::TextSlot);
    disconnect(mPowThread, &Power_CoreThread::ImageSig , mPower, &Face_Power::ImageSlot);

    str = QTime::currentTime().toString("hh:mm:ss");
    ui->endLab->setText(str);
    mPro->step = Test_End;
}

void Home_WorkWid::updateSig()
{
    connect(mDevSn, &Sn_DevId::TipImageSig , mPower, &Face_Power::ImageSlot);
    connect(mDevSn, &Sn_DevId::TipSig , mPower, &Face_Power::TextSlot);
    connect(mPowDev, &Power_DevRead::StepSig , mPower, &Face_Power::TextSlot);
    connect(mPowDev, &Power_DevRead::CurImageSig , mPower, &Face_Power::ImageSlot);
    connect(mPowThread, &Power_CoreThread::TipSig , mPower, &Face_Power::TextSlot);
    connect(mPowThread, &Power_CoreThread::ImageSig , mPower, &Face_Power::ImageSlot);
}

void Home_WorkWid::updateWid()
{
    QString mPn = ui->codeEit->text();//订单号+成品代码
    QStringList list = mPn.split("+");
    for(int i = 0; i < list.count(); i++)
    {
        if(i == 0) mPro->order_id = list.at(i);
        if(i == 1) mPro->product_sn = list.at(i);
    }
    ePro->product_sn = mPro->product_sn;
    ePro->order_id = mPro->order_id;
    mCfgm->user = mPro->order_id;

    mCfgm->pn = ui->codeEit->text();//订单号+成品代码
    if(mCfgm->modeId != 2){
        if(mPro->product_sn.mid(3,1) != 'B' && mCfgm->moduleType == 0) mPro->type = 1;//判断是智能型还是基本型
        else mPro->type = 0;
    }else {mPro->type = 2;}

    if(mPro->type == 1 && (mPro->work_mode == 0 || mPro->work_mode == 1)) //智能型，耐压绝缘、接地测试---解析序列号
    {
        QString mSn = ui->safeSnEit->text();//订单号+序列号
        QStringList mList = mSn.split("+");
        for(int i = 0; i < mList.count(); i++)
        {
            if(i == 2) mPro->moduleSN = mList.at(i);
        }
        ePro->moduleSN = mPro->moduleSN;
    }else if(mPro->type == 1 && mPro->work_mode == 2){
        QString str = mDev->devType.sn;
        mPro->moduleSN = str.remove(QRegExp("\\s"));
        ePro->moduleSN = mPro->moduleSN;
    }

    QString str = mDev->devType.dev_type;
    mPro->productType = str;
    ePro->productType = str;

    mPro->Service = mCfgm->Service;
    mPro->stopFlag = 1;
    mPro->online = mCfgm->online;

    switch (ui->comBox->currentIndex()) {
    case 0: {
        mPro->dev_name = tr("始端箱"); ePro->dev_name = tr("Busbar feeder box");
        break;
    }
    case 1: {
        mPro->dev_name = tr("插接箱"); ePro->dev_name = tr("Busbar tap-off box");
        break;
    }
    case 2: {
        mPro->dev_name = tr("母线槽"); ePro->dev_name = tr("Busway");
        break;
    }
    case 3: {
        mPro->dev_name = tr("基本型"); ePro->dev_name = tr("Basic type");
        break;
    }
    default:
        break;
    }

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
    ui->textEdit->clear();
    //if(mPro->type == 0 && mCfgm->modeId != 2){
        mDev->devType.sn.clear(); mCfgm->moduleSn.clear();
    //}

    Cfg::bulid()->writeQRcode();//成品sn===成品代码+订单号

    bool ret = false;
    if(!coms->ser1){MsgBox::critical(this, tr("请先打Acw/Ir串口")); return ret;}
    ret = coms->ser1->isOpened();
    if(!ret){MsgBox::critical(this, tr("请先打Acw/Ir串口")); return ret;}

    return ret;
}

bool Home_WorkWid::initSerialGND()
{
    QString str;  mId = 1; mFirst = 1;
    sSerial *coms = &(mCfgm->coms);
    ui->textEdit->clear();

    if(mPro->type == 0 && mCfgm->modeId != 2){
        Cfg::bulid()->readQRcode();
        mDev->devType.sn = mCfgm->moduleSn;
    }
    Cfg::bulid()->writeQRcode();
    bool ret = false;
    if(mCfgm->modeId == 2){//母线槽
        ret = true;
    } else{
        if(!coms->ser2){MsgBox::critical(this, tr("请先打开Gnd串口")); return ret;}
        ret = coms->ser2->isOpened();
        if(!ret){MsgBox::critical(this, tr("请先打开Gnd串口")); return ret;}
    }

    return ret;
}

bool Home_WorkWid::initSerial()
{
    mId = 1; mFirst = 1;
    sSerial *coms = &(mCfgm->coms);
    ui->textEdit->clear();
    Cfg::bulid()->writeQRcode();
    bool ret = false;
    if(mCfgm->modeId == 2){//母线槽
        ret = true;
    }else{
        ret = coms->ser3->isOpened();
        if(!ret){MsgBox::critical(this, tr("请先打开Dev串口")); return ret;}
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

void Home_WorkWid::FuncStatus(bool ret)
{
    if(ret) ui->funcLab->setStyleSheet("background-color:green; color:rgb(0, 0, 0);");
    else  ui->funcLab->setStyleSheet("background-color:red; color:rgb(0, 0, 0);");
}



void Home_WorkWid::StatusSlot(bool ret)
{
    switch (mItem->work_mode) {
    case 0: {AcwStatus(ret); break;}
    case 1: {GndStatus(ret); break;}
    case 2: {FuncStatus(ret);break;}
    default:
        break;
    }
}

void Home_WorkWid::ItemStatus()
{
    switch (mItem->work_mode) {
    case 0: {mPro->test_step = "安规测试"; mPro->test_item = ui->vol_insulBtn->text();
             ePro->test_step = "Safety testing"; ePro->test_item = "Voltage withstand/insulation test";
             ui->acwLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");          

             break;
    }
    case 1: {mPro->test_step = "安规测试"; mPro->test_item = ui->groundBtn->text();
             ePro->test_step = "Safety testing"; ePro->test_item = "Grounding test";
             ui->gndLab->setStyleSheet("background-color:yellow; color:rgb(0, 0, 0);");

             break;
    }
    case 2: {mPro->test_step = "功能测试"; ePro->test_step = "Functional testing";
             ui->funcLab->setStyleSheet("background-color:yellow;color:rgb(0, 0, 0);");
             break;
    }
    default:
        break;
    }
}

bool Home_WorkWid::checkUesr()
{
    bool ret = true;
    if(mCfgm->user.isEmpty()) {
        MsgBox::critical(this, tr("请先填写工单号！"));
        return false;
    }
    if(mCfgm->cnt.num < 1) {
        MsgBox::critical(this, tr("请先填写订单剩余数量！"));
        return false;
    }

    return ret;
}

void Home_WorkWid::on_vol_insulBtn_clicked()//耐压--绝缘
{
    ui->stackedWid->show();
    mItem->work_mode = 0;
    mPro->work_mode = 0;
    if(mPro->step == Test_End){
        bool ret = initSerialVol();
        // if(ret) ret = checkUesr();
        if(ret) {
            mPacket->init(); mPacketEng->init();
            Breaker = true;
            int mode = Test_Over;
            if(mItem->mode != Test_Start) {
                mode = Test_Start;
            }
            emit startSig(mode); ItemStatus();
        }
    }else {
        mPro->oning = false; Breaker = false;
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
    mPro->work_mode = 1;
    if(mPro->step == Test_End){
        bool ret = initSerialGND();
        // if(ret) ret = checkUesr();
        if(ret) {
            mPacket->init(); mPacketEng->init();

            int mode = Test_Over;
            if(mItem->mode != Test_Start) {
                mode = Test_Start;
            }

            emit startSig(mode); ItemStatus();
        }
    }else {
        overTest();
        GndStatus(false);
        mPro->result = Test_Fail;
        updateResult();
    }
}

void Home_WorkWid::on_funcBtn_clicked()
{
    ui->stackedWid->hide();
    mItem->work_mode = 2;
    mPro->work_mode = 2;
    if(mPro->step == Test_End) {
        updateSig();
        bool ret = initSerial();
        // if(ret) ret = checkUesr();
        if(ret) {
            mPacket->init(); mPacketEng->init();
            ItemStatus();
            mPowThread->start();
        }
    }else {
        bool ret = MsgBox::question(this, tr("确定需要提前结束？"));
        if(ret) {
            FuncStatus(!ret); mPro->result = Test_Fail;
            updateResult();
        }
    }
}

void Home_WorkWid::JudgSlots()
{
    mJudg->exec();
}

void Home_WorkWid::SafeSlot()
{
    mSafePeo->exec();
}

void Home_WorkWid::SafeStop()
{
    mSafrtyThread->stopThread();
    overSlot();
}

void Home_WorkWid::on_codeEit_textChanged(const QString &arg1)
{
    ui->codeEit->setClearButtonEnabled(1);
    mCfgm->pn = ui->codeEit->text();

    QString mPn = ui->codeEit->text();//订单号+成品代码
    QStringList list = mPn.split("+");
    for(int i = 0; i < list.count(); i++)
    {
        if(i == 0) mPro->order_id = list.at(i);
        if(i == 1) mPro->product_sn = list.at(i);
    }
    ePro->product_sn = mPro->product_sn;
    ePro->order_id = mPro->order_id;
    mCfgm->user = mPro->order_id;

    mCfgm->pn = ui->codeEit->text();//订单号+成品代码
    Cfg::bulid()->writeQRcode();
}

void Home_WorkWid::on_comBox_currentIndexChanged(int index)
{
    mCfgm->modeId = index;
    initTypeComboBox();

    switch (index) {
    case START_BUSBAR:{
        mCfgm->addr = 1;
        break;
    }
    case INSERT_BUSBAR:{
        mCfgm->addr = 2;
        break;
    }
    default:
        break;
    }
    Cfg::bulid()->writeCfgDev();
}

void Home_WorkWid::initTypeComboBox()
{
    int index = mCfgm->modeId;
    ui->comBox->setCurrentIndex(index);
}

void Home_WorkWid::on_snprintBtn_clicked()
{
    if(mPro->step == Test_End){
        if(mCfgm->modeId == 2 || mPro->type == 0)//母线槽/基本型
        {
            QString method; int port = 0; QString ip;
            if(mCfgm->modeId == 2) {method = "Integration/Busbar-Busway/Execute"; port = 81; ip = "127.0.0.1";}
            else {method = "Integration/Busbar-Product/Execute"; port = 80; ip = "127.0.0.1";}
            if((mItem->work_mode == 1 && mPro->type == 0) || mCfgm->modeId == 2) mVolInsul->printer(ip, method, port);
        }else if(mPro->type == 1){
            if(mItem->work_mode == 2) mPowThread->printer();
        }
    }
}


void Home_WorkWid::on_safeSnEit_textChanged(const QString &arg1)
{
    ui->safeSnEit->setClearButtonEnabled(1);
}

void Home_WorkWid::changeLoadSlot(int id)
{
     bool ret = MsgBox::information(this,tr("请确认切换输出口%1").arg(id+1));


     if(ret)emit changLoadEmitSig(1);
     else emit changLoadEmitSig(0);
}
