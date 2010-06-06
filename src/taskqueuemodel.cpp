// taskqueuemodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-25 10:35:29 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtSql>
#include <QSqlDriver>

#include "taskqueuemodel.h"


// this is the problem
// Every :memory: database is distinct from every other. So, opening two database connections each with the filename ":memory:" will create two independent in-memory databases.
// 这个是Qt中的使用已有连接的例子，
/*
  #include "qtdir/src/sql/drivers/psql/qsql_psql.cpp"

  PGconn *con = PQconnectdb("host=server user=bart password=simpson dbname=springfield");
  QPSQLDriver *drv =  new QPSQLDriver(con);
  QSqlDatabase db = QSqlDatabase::addDatabase(drv); // becomes the new default connection
  QSqlQuery query;
  query.exec("SELECT NAME, ID FROM STAFF");
 */

// 将Qt的Sql模块的功能与原生的sqlite等数据函数一些调用很难整合起来了。
// 只使用Qt的吧，方便一点。

// QSqlDatabase need a QCoreApplication on win7 x64, see #0000250
// TaskQueueModel *TaskQueueModel::inst = 
//     new TaskQueueModel(0, QSqlDatabase::addDatabase("QSQLITE", "idtq"));
TaskQueueModel *TaskQueueModel::inst = NULL;

TaskQueueModel::TaskQueueModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db), inited(false)
{
    QSqlDatabase taskQueueDb = QSqlDatabase::database("idtq", false);
    // taskQueueDb.setDatabaseName("tobememory");
    taskQueueDb.setDatabaseName(":memory:");
    if (! taskQueueDb.open()) {
        qDebug()<<"open sqlite error";
    } else {
        qDebug()<<"open sqlite okkkkkkkkkkkkk";
    }
}

TaskQueueModel::~TaskQueueModel()
{
    this->finalize();
}

TaskQueueModel *TaskQueueModel::instance()
{
  if (TaskQueueModel::inst == NULL) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "idtq");
    TaskQueueModel *m = new TaskQueueModel(0, db);
    TaskQueueModel::inst = m;
  }
    TaskQueueModel *q = TaskQueueModel::inst;
    if (!q->inited) {
        q->init();
        q->inited = true;
        // QSqlDriver *drv = new QSQLiteDriver(this->pDB);
    }
    return q;
}

void TaskQueueModel::init()
{
    int rv = 0;
    // rv = sqlite3_open_v2("tobememory", &this->pDB, 
    //                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,
    //                      NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    //     sqlite3_close(this->pDB);
    //     this->pDB = NULL;
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // 创建表，
    char * createTableSql = 
        " CREATE TABLE IF NOT EXISTS task_queue ("
        "  id INTEGER PRIMARY KEY,"
        "  file_name VARCHAR(128),"
        "  dest_path VARCHAR(256),"
        "  file_size UNSIGNED BIG INT,"
        "  got_size  UNSIGNED BIG INT,"
        "  got_percent INT2,"
        "  eclapsed_time VARCHAR(10),"
        "  left_time VARCHAR(10),"
        "  speed VARCHAR(10),"
        "  status VARCHAR(10),"
        "  start_time VARCHAR(16),"
        "  finish_time VARCHAR(16)"
        "); DELETE FROM task_queue WHERE 1;";

    // rv = sqlite3_exec(this->pDB, createTableSql, NULL, NULL, NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);
    this->database().exec(createTableSql);
}

void TaskQueueModel::finalize()
{
    // int rv = sqlite3_close(this->pDB);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);
}

QVariant TaskQueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    QVariant dv;    
    if (orientation == Qt::Vertical) {
        dv = section + 1;
    } else {
        switch (section) {
        case 0:
            dv = tr("Id");
            break;
        case 1:
            dv = tr("Filename");
            break;
        case 2:
            dv = tr("Destination");
            break;
        case 3:
            dv = tr("File Size");
            break;
        case 4:
            dv = tr("Transferred");
            break;
        case 5:
            dv = tr("Progress %");
            break;
        case 6:
            dv = tr("Elapsed Time");
            break;
        case 7:
            dv = tr("Time Left");
            break;
        case 8:
            dv = tr("Speed");
            break;
        case 9:
            dv = tr("Status");
            break;
        case 10:
            dv = tr("Start Time");
            break;
        case 11:
            dv = tr("Finish Time");
            break;
        default:
            dv = tr("coming...");
            break;
        };
    }

    return dv;
}

void TaskQueueModel::slot_set_transfer_percent(int modelId, int percent, int total_transfered, int speed)
{
    int rowCount;
    rowCount = this->rowCount();
    for (int row = rowCount - 1; row >= 0; row --) {
        if (this->data(this->index(row, 0)).toInt() == modelId) {
            this->setData(this->index(row, 5), percent);
            this->setData(this->index(row, 4), total_transfered);
            this->setData(this->index(row, 8), speed);
            break;
        }
    }
    this->submitAll();    
}
void TaskQueueModel::slot_transfer_thread_finished(int modelId)
{
    int rowCount;
    rowCount = this->rowCount();
    for (int row = rowCount - 1; row >= 0; row --) {
        if (this->data(this->index(row, 0)).toInt() == modelId) {
            this->setData(this->index(row, 11), QDateTime::currentDateTime().toString());
            this->setData(this->index(0, 9), tr("Finished"));
            this->setData(this->index(0, 7), "00:00:00");
            break;
        }
    }
    this->submitAll();    
}
int TaskQueueModel::slot_new_file_transfer_started(QString new_file_name, QString dest_path)
{
    // char *preSql = "INSERT INTO task_queue (id, file_name, dest_path, start_time) VALUES (NULL, '%s', '%s', '%s');";
    // sqlite3_stmt *pStmt = NULL;
    // char sqlBuff[1024] = {0};

    // sprintf(sqlBuff, preSql, QFileInfo(new_file_name).baseName().toAscii().data(),
    //         new_file_name.toAscii().data(),
    //         QDateTime::currentDateTime().toString().toAscii().data());

    // qDebug()<<"SQL:"<<sqlBuff;
    // int rv = sqlite3_exec(this->pDB, sqlBuff, 0, 0, 0);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);
    // qDebug()<<" rowid: "<<sqlite3_last_insert_rowid(this->pDB);
    // emit this->insertRow(sqlite3_last_insert_rowid(this->pDB)-1);

    this->insertRows(0, 1);
    this->setData(this->index(0, 1), QFileInfo(new_file_name).fileName());
    QString dest_file_path = dest_path + "/" + QFileInfo(new_file_name).fileName();
    this->setData(this->index(0, 2), dest_file_path); // TODO need dest path, not src path
    this->setData(this->index(0, 10), QDateTime::currentDateTime().toString());
    this->setData(this->index(0, 9), tr("Running"));
    this->submit();

    int rowCount = this->rowCount();
    int lastId = this->data(this->index(rowCount-1, 0)).toInt();
    return lastId;

    // int rv = sqlite3_exec(this->pDB, "BEGIN TRANSACTION;", 0, 0, 0);
    // rv = sqlite3_prepare_v2(this->pDB, preSql, -1, &pStmt, NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);
    
    // rv = sqlite3_bind_text(pStmt, 1, "abcd", //QFileInfo(new_file_name).baseName().toAscii().data(),
    //                        4, // QFileInfo(new_file_name).baseName().length(),
    //                        NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // rv = sqlite3_bind_text(pStmt, 2, "abcde", // new_file_name.toAscii().data(),
    //                        5,// new_file_name.length(), 
    //                        NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // rv = sqlite3_bind_text(pStmt, 3, "else", //QDateTime::currentDateTime().toString().toAscii().data(),
    //                        4, // QDateTime::currentDateTime().toString().length(), 
    //                        NULL);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // rv = sqlite3_step(pStmt);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // preSql = (char*)sqlite3_sql(pStmt);
    // qDebug()<<"Stmt SQL: "<<preSql<<" rowid: "<<sqlite3_last_insert_rowid(this->pDB);

    // {
    //     rv = sqlite3_reset(pStmt);
    //     if (rv != SQLITE_OK) {
    //         qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    //     }
    //     Q_ASSERT(rv == SQLITE_OK);
    // }

    // rv = sqlite3_finalize(pStmt);
    // if (rv != SQLITE_OK) {
    //     qDebug()<<"Sqlite error:"<<rv<<":"<<sqlite3_errmsg(this->pDB);
    // }
    // Q_ASSERT(rv == SQLITE_OK);

    // rv = sqlite3_exec(this->pDB, "COMMIT TRANSACTION;", 0, 0, 0);
}

void TaskQueueModel::slot_transfer_got_file_size(int modelId, int size)
{
    int rowCount;
    rowCount = this->rowCount();
    for (int row = rowCount - 1; row >= 0; row --) {
        if (this->data(this->index(row, 0)).toInt() == modelId) {
            this->setData(this->index(row, 3), size);
            break;
        }
    }
    this->submitAll();
}

void TaskQueueModel::slot_transfer_time_update(int modelId, QString eclapsed_time, QString left_time)
{
    int rowCount;
    rowCount = this->rowCount();
    for (int row = rowCount - 1; row >= 0; row --) {
        if (this->data(this->index(row, 0)).toInt() == modelId) {
            this->setData(this->index(row, 6), eclapsed_time);
            this->setData(this->index(row, 7), left_time);
            break;
        }
    }
    this->submitAll();
}
