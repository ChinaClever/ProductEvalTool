#include "face_Volinsul.h"
#include "ui_face_Volinsul.h"

Face_Volinsul::Face_Volinsul(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Face_Volinsul)
{
    ui->setupUi(this);
    mItem = TestConfig::bulid()->item;
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
    mDataSave = new TestDataSave(this);
    initWid();
}

Face_Volinsul::~Face_Volinsul()
{
    delete ui;
}

void Face_Volinsul::initWid()
{
    timer = new QTimer(this);
    timer->start(100);
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

void Face_Volinsul::resultSlot()
{
    bool p = true;
    if(mItem->progress.errNum)  p = false;

    mItem->progress.allNum = mItem->progress.finishNum;
    int ok = (mItem->progress.okNum * 100.0) / mItem->progress.allNum;
    QString str1 = tr("测试项目数:%1  失败项目数：%2  项目测试通过率：%3%").arg(mItem->progress.allNum).arg(mItem->progress.errNum).arg(ok);
    mPacket->updatePro(str1);

    bool res = false;
    QString str = tr("测试结果 ");
    if(!p)
    {
        if(mItem->work_mode == 0) ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        else if(mItem->work_mode == 1) ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );

        res = false; str += tr("失败");
        mPro->uploadPassResult = 0;
    } else {
        res = true; str += tr("通过");
        mPro->uploadPassResult = 1;
    }
    mPacket->updatePro(str, res);
    mDataSave->saveTestData();
    if(mPro->online) {
        mPacket->delay(2);
        Json_Pack::bulid()->http_post("busbarreport/add",mPro->Service);
    }
    if(mPro->result == Test_Fail) res = false;
    else {res = true;}
    emit StatusSig(res); mPro->step = Test_Over;
}

void Face_Volinsul::progressSlot()
{
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
