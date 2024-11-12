#include "face_Volinsul.h"
#include "ui_face_Volinsul.h"
#include "printer_bartender.h"

Face_Volinsul::Face_Volinsul(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Face_Volinsul)
{
    ui->setupUi(this);
    mItem = TestConfig::bulid()->item;
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
    mDataSave = new TestDataSave(this);
    mCfg = Cfg::bulid()->item;
    Printer_BarTender::bulid(this);
    mSn = Sn_SerialNum::bulid(this);
    mDev = mPacket->getDev();

    initWid();
}

Face_Volinsul::~Face_Volinsul()
{
    delete ui;
}

void Face_Volinsul::initWid()
{
    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(progressSlot()));
}

void Face_Volinsul::startSlot()
{
    QString name = mItem->sn.name;
    mPro->productType = mItem->sn.name;
    QString sn = mItem->sn.sn;
    mItem->progress.errNum = mItem->progress.finishNum = mItem->progress.okNum = 0;
    if(mItem->work_mode == 0) {
        ui->progressBar->setValue(0);
        ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:green;}" );
    }else if(mItem->work_mode == 1) {
        ui->progressBar2->setValue(0);
        ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:green;}" );
    }

}

bool Face_Volinsul::printer()
{
    QString method = "Integration/Busbar-Product/Execute";
    QString ip = "192.168.1.16";
    bool ret = true;
    QString str = tr("标签打印 "); QString str1;
        if(mPro->result != Test_Fail){
        sBarTend it;
        QString mPn = mCfg->pn;//订单号+成品代码
        QStringList list = mPn.split("+");
        for(int i = 0; i < list.count(); i++)
        {
            if(i == 0) it.on = list.at(i);
            if(i == 1) it.pn = list.at(i);
        }

        QString mSn = mDev->devType.sn;//模块序列号
        it.sn =  mSn.remove(QRegExp("\\s"));
        it.fw = "V0.0";
        it.hw = "V1.0";
        // if(it.sn.isEmpty()){
        //     mPro->result = Test_Fail;
        //     ret  = false;
        //     if(it.sn.isEmpty()) str += tr(" 读取到序列号SN为空 ");
        // }
        if(ret){
            str1 = Printer_BarTender::bulid(this)->http_post(method, ip, it);
            if(str1 == "Success") {
                ret = true;
            }else {
                str1 = Printer_BarTender::bulid(this)->http_post(method, ip, it);
                if(str1 == "Success") {
                    ret = true;
                }else ret = false;
            }
        }
        if(ret) str += tr("正常"); else str += tr("错误");
    } else str = tr("因测试未通过，标签未打印");
    return mPacket->updatePro(str, ret);
}

void Face_Volinsul::resultSlot()
{
    bool p = true;
    if(mItem->progress.errNum)  p = false;

    mItem->progress.allNum = mItem->progress.finishNum;
    int ok = (mItem->progress.okNum * 100.0) / mItem->progress.allNum;
    QString str1 = tr("测试项目数:%1  失败项目数：%2  项目测试通过率：%3%").arg(mItem->progress.allNum).arg(mItem->progress.errNum).arg(ok);
    mPacket->updatePro(str1);

    if(mCfg->modeId == 2 || mPro->type == 0)
    {
        mSn->createSn();//设置序列号
        QString str = mDev->devType.sn;
        mPro->moduleSN = str.remove(QRegExp("\\s"));

    }

    if(mCfg->modeId == 2) emit finshSig();
    if(mCfg->modeId == 2){
        while(1)
        {
            mPacket->delay(1);
            if(mPro->issure)
            {
                break;
            }
        }
    }

    bool res = false;

    QString str = tr("测试结果 ");
    if(!p)
    {
        if(mItem->work_mode == 0) ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        else if(mItem->work_mode == 1) ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );

        res = false; str += tr("失败");
        mPro->uploadPassResult = 0;
    } else {
        if(mPro->work_mode == 1 && (mCfg->modeId == 2 || mPro->type == 0))//母线槽与基本型始端箱和插接箱只需安规测试，接地测试成功打印标签
        {
            res = printer();
            if(res) {
                str += tr("通过");mPro->uploadPassResult = 1;
            }else {
                str += tr("失败");mPro->uploadPassResult = 0;
            }
            if(mCfg->cnt.num > 0) {
                mCfg->cnt.num -= 1;
            }
        }else {
            res = true; str += tr("成功");
            mPro->uploadPassResult = 1;
        }
        // res = true; str += tr("成功");
        // mPro->uploadPassResult = 1;
    }
    mPacket->updatePro(str, res);

    mDataSave->saveTestData();
    if(mPro->online) {
        mPacket->delayMs(20);
        Json_Pack::bulid()->SendJson_Safe();
    }

    if(mPro->result == Test_Fail) res = false;
    else {res = true;}
    emit StatusSig(res); mPro->step = Test_Over;
}

void Face_Volinsul::progressSlot()
{
//    overSig();
    sTestProgress *arg = &(mItem->progress);

    int progress = (arg->finishNum * 100.0) / arg->allNum;
    if(mItem->work_mode == 0) ui->progressBar->setValue(progress);
    else if(mItem->work_mode == 1) ui->progressBar2->setValue(progress);

    if(arg->errNum)
    {
        if(mItem->work_mode == 0) ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        else if(mItem->work_mode == 1) ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
    }


}
