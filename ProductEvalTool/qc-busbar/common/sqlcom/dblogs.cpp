#include "dblogs.h"

DbLogs::DbLogs()
{
    createTable();
    tableTile = tr("状态日志");
    headList  << tr("成品二维码") << tr("设备类型") << tr("测试项目") << tr("设备序列号") <<tr("结果") ;
}

DbLogs *DbLogs::bulid()
{
    static DbLogs* sington = nullptr;
    if(sington == nullptr)
        sington = new DbLogs();
    return sington;
}

void DbLogs::createTable()
{
    QString cmd =
            "create table if not exists %1("
            "id             INTEGER primary key autoincrement not null,"
            "date           VCHAR,"
            "time           VCHAR,"
            "QRcode         VCHAR,"
            "dev            VCHAR,"
            "content        VCHAR,"
            "sn             VCHAR,"
            "result         VCHAR);";

    QSqlQuery query(mDb);
    if(!query.exec(cmd.arg(tableName())))
    {
        throwError(query.lastError());
    }
}

bool DbLogs::insertItem(DbLogItem &item)
{
    bool ret = false;
    QString cmd = "insert into %1 (id, date, time, QRcode, dev, content, sn, result) "
                  "values(:id,:date,:time,:QRcode,:dev,:content,:sn,:result)";
    ret = modifyItem(item,cmd.arg(tableName()));
    if(ret)
        emit itemChanged(item.id,Insert);

    return ret;
}


bool DbLogs::modifyItem(const DbLogItem &item, const QString &cmd)
{
    bool ret = false;
    QSqlQuery query(mDb);
    query.prepare(cmd);

    query.bindValue(":date",item.date);
    query.bindValue(":time",item.time);
    query.bindValue(":QRcode",item.QRcode);
    query.bindValue(":dev",item.dev);
    query.bindValue(":content",item.content);
    query.bindValue(":sn",item.sn);
    query.bindValue(":result",item.passed);
    ret = query.exec();
    if(!ret)
        throwError(query.lastError());
    return ret;
}

void DbLogs::selectItem(QSqlQuery &query, DbLogItem &item)
{
    item.id = query.value("id").toInt();
    item.date = query.value("date").toString();
    item.time = query.value("time").toString();
    item.QRcode = query.value("QRcode").toString();
    item.sn = query.value("sn").toString();
    item.passed = query.value("passed").toString();
}
