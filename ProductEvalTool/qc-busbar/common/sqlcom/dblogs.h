#ifndef DBLOGS_H
#define DBLOGS_H
#include "sqltablemodel.h"

#define DB_Logs "logs"

struct DbLogItem : public DbBasicItem {
    DbLogItem(){};
    QString cn; // 电脑号
    QString QRcode; // 工状条码
    QString batch; // 批次
    QString type; // 用途
    QString sn; // 产品条码
    QString dev;

    QString gnd; // 接地测试结果
    QString ir; // 绝缘测试结果
    QString dcw; // 直流耐压测试结果
    QString acw; // 交流耐压测试结果

    QString passed;  // 结果
    QString content;
};

class DbLogs: public SqlBasic<DbLogItem>
{
public:
    DbLogs();
    static DbLogs *bulid();
    bool insertItem(DbLogItem &item); // 插入
    QString tableName(){return QString("%1").arg(DB_Logs);}

protected:
    bool modifyItem(const DbLogItem& item,const QString& cmd);
    void selectItem(QSqlQuery &query,DbLogItem &item);
    void createTable();
};

#endif // DBLOGS_H
