// remotedirmodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:47:47 +0000
// Version: $Id$
// 

#include <cassert>
#include <QtCore>

#include "utils.h"
#include "globaloption.h"
#include "remotedirmodel.h"
#include "rfsdirnode.h"
#include "connection.h"
#include "dirretriver.h"
#include "ftpdirretriver.h"

// from mimetypeshash.cpp
extern QHash<QString, QString> gMimeHash;

// 类定义
RemoteDirModel::RemoteDirModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    this->rootNode = NULL;

    //keep alive 相关设置
    this->keep_alive = true;
    this->keep_alive_timer = new QTimer();
     this->keep_alive_interval = DEFAULT_KEEP_ALIVE_TIMEOUT;
    this->keep_alive_timer->setInterval(this->keep_alive_interval);
    QObject::connect(this->keep_alive_timer, SIGNAL(timeout()),
                     this, SLOT(slot_keep_alive_time_out()));
}

RemoteDirModel::~RemoteDirModel()
{
    if (this->keep_alive_timer->isActive()) {
        this->keep_alive_timer->stop();
    }
    delete this->keep_alive_timer ;
    
    if (this->dir_retriver->isRunning()) {
        //TODO 怎么能友好的结束,  现在这么做只能让程序不崩溃掉。
        qDebug()<<"remote_dir_retrive_thread is run , how stop ?";
        this->dir_retriver->terminate();
        //this->remote_dir_retrive_thread->wait();  //这个不好用
        delete this->dir_retriver;        
    } else {
        delete this->dir_retriver;
    }
    // if (tree_root != 0) delete tree_root;
    //TODO: 删除model中的现有数据, 已经实现，上一行
}

void RemoteDirModel::setConnection(Connection *conn)
{
    this->conn = conn;
    this->ssh2_sess = this->conn->sess;

    switch (this->conn->protocolType()) {
    case Connection::PROTO_FTP:
        this->dir_retriver = new FTPDirRetriver();
        break;
    case Connection::PROTO_SFTP:
        this->dir_retriver = new SSHDirRetriver();
        break;
    default:
        assert(1 == 2);
        break;
    }
    
    QObject::connect(this->dir_retriver, SIGNAL(remote_dir_node_retrived(NetDirNode *, void *, NetDirNode *)),
                     this, SLOT(slot_remote_dir_node_retrived(NetDirNode*, void *, NetDirNode *)));
    QObject::connect(this->dir_retriver, SIGNAL(enter_remote_dir_retrive_loop()),
                     this, SIGNAL(enter_remote_dir_retrive_loop()));
    QObject::connect(this->dir_retriver, SIGNAL(leave_remote_dir_retrive_loop()),
                     this, SIGNAL(leave_remote_dir_retrive_loop()));
    
    QObject::connect(this->dir_retriver, SIGNAL(execute_command_finished(NetDirNode *, void *, int, int)),
                     this, SLOT(execute_command_finished(NetDirNode *, void *, int, int)));

    // 设置连接对象
    this->dir_retriver->setConnection(this->conn);
    emit operationTriggered(tr("Info SSH2_FXP_INIT"));
}

void vdump_node(NetDirNode *node, int depth)
{
    // verbose dumping
    
    QString prepad; prepad.fill(' ', depth);
    qDebug()<<prepad<<node->fullPath<<node->childNodes.count()<<node->onRow;

    for (int i = 0 ; i < node->childNodes.count(); i++) {
        vdump_node(node->childNodes.value(i), depth + 2);
    }
}

void RemoteDirModel::set_user_home_path(QString user_home_path)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->user_home_path = user_home_path;

    qDebug()<<"I know remote home path: "<<this->user_home_path;

    //Todo: rewrite simple and clearly code
    // test case :
    // 1.      /home/users/l/li/liuguangzhao
    // 2.      /root
    // 3.      /home/gzl
    // 4.      /
    // 5.      /home/gzleo/     note the last /
    // 6.      /home/users//l/li//liuguangzhao     note the double //
    
    Q_ASSERT(user_home_path.left(1) == "/"); // must begin with /

    //{创建初始化目录树
    QModelIndex pIndex;
    QVariant nv;
    NetDirNode *currNode;
    NetDirNode *tempNode = new NetDirNode();
    QString fullPath, fileName;
    
    QStringList pathParts = user_home_path.split('/');
    QString lastStripPath;
    for (int i = 0 ; i < pathParts.count(); ++i) {
        if (i == 0) {
            pIndex = QModelIndex();
            fullPath = "";
            fileName = "/";
            lastStripPath = "";
        } else {
            if (pathParts.at(i).length() == 0) {
                // should be the case: "/", special for ftp
                q_debug()<<"detect empty path part, omit.";
                break;
            }
            fullPath = lastStripPath + "/" + pathParts.at(i);
            fileName = pathParts.at(i);
            lastStripPath += "/" + fileName;
        }

        qDebug()<<"Distance to begin: strip path:"<<fullPath<<"dir name:"
                <<fileName<<tempNode<<tempNode->childNodes.count()<<pIndex;

        //assign
        tempNode->fullPath = fullPath;
        tempNode->_fileName = fileName;
        tempNode->onRow = 0;    //指的是此结点在父结点中的第几个结点，在这里预置的只能为0
        // fixed only / can not list dir automatically when init
        if (user_home_path.length() == 1 && user_home_path == "/") {
            tempNode->retrFlag = POP_NO_NEED_NO_DATA;
        } else {
            // temp_tree_item->retrFlag = (i == pathParts.count() - 1) ? 0:1;   // 半满结点
            tempNode->retrFlag = (i == pathParts.count() - 1) ? POP_NO_NEED_NO_DATA : POP_NO_NEED_WITH_DATA;
        }
        tempNode->prevFlag = 0xFF;
        tempNode->attrib.permissions = 16877;    //默认目录属性: drwxr-xr-x

        // dump_tree_node_item(tempNode);

        this->insertRows(0, 1, pIndex);
        QModelIndex currIndex = this->index(0, 0, pIndex);
        nv = qVariantFromValue((void*)tempNode);
        this->setData(currIndex, nv);
        pIndex = currIndex;

        currNode = static_cast<NetDirNode*>(pIndex.internalPointer());
        // dump_tree_node_item(currNode);
    }

    // vdump_node(this->rootNode, 1);
    // currNode->dumpTreeRecursive();
    delete tempNode; tempNode = NULL;    
    // 启动keep alive 功能。
    if (this->keep_alive ) {
        // this->keep_alive_timer->start();
    }
}

QModelIndex RemoteDirModel::index(int row, int column, const QModelIndex &parent) const
{
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<<row<<column<<parent;

    NetDirNode *parent_item;
    NetDirNode *child_item = 0;
    QModelIndex idx;

    if (parent.row() == -1 && parent.column() == 0) {
        q_debug()<<parent.isValid();
        // Q_ASSERT(1 == 2);        
    }
    // Q_ASSERT(row == 0);
    Q_ASSERT(column < 4);

    if (!parent.isValid()) {
        Q_ASSERT(row == 0);
        parent_item = 0;
        child_item = this->rootNode;
        idx = this->createIndex(row, column, this->rootNode);
        // qDebug()<<__FUNCTION__<<__LINE__<<"row :"<<row<<" column:" <<column<<parent<<idx
        //         <<child_item->fileName()<<child_item
        //         <<"0";
        Q_ASSERT(!(idx.row() == -1 && idx.column() == 0));
        return idx;
    } else {
        parent_item = static_cast<NetDirNode*>(parent.internalPointer());
        // dump_tree_node_item(parent_item);
        Q_ASSERT(row < parent_item->childNodes.count());
        child_item = parent_item->childNodes.value(row);
        idx = createIndex(row, column, child_item);
        // qDebug()<<__FUNCTION__<<__LINE__<<"row :"<<row<<" column:" <<column<<parent<<idx
        //         <<child_item->fileName()
        //         <<parent_item->fileName()
        //         <<child_item;
        Q_ASSERT(!(idx.row() == -1 && idx.column() == 0));
        return idx;
    }

    qDebug()<<__FUNCTION__<<__LINE__<<"invalid QModelIndex returnted";
    return QModelIndex();
}

QModelIndex RemoteDirModel::index(const QString &path, int column) const
{
    if (path.isEmpty()) {
        qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        return this->createIndex(0, 0, this->rootNode);
        // return QModelIndex();
    } 

    QString absolutePath = QDir(path).absolutePath();
#ifdef Q_OS_WIN
    //absolutePath = absolutePath.toLower();
    // On Windows, "filename......." and "filename" are equivalent
    if (absolutePath.endsWith(QLatin1Char('.'))) {
        int i;
        for (i = absolutePath.count() - 1; i >= 0; --i) {
            if (absolutePath.at(i) != QLatin1Char('.'))
                break;
        }
        absolutePath = absolutePath.left(i+1);
    }
#endif

    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
//     if ((pathElements.isEmpty() /*|| !QFileInfo(path).exists()*/)   // the path is "/"
// #ifndef Q_OS_WIN
//         && path != QLatin1String("/")
// #endif
//         )
//     {
//         return QModelIndex();
//     }
   
    
    pathElements.prepend("/");

    // if pathElements.count() == 1, then the path is only /
    return this->find_node_item_by_path_elements(this->rootNode, pathElements, pathElements.count() == 1 ? 0 : 1);
    // return this->find_node_item_by_path_elements(this->tree_root->child_items[0], pathElements, 1);
    
    return QModelIndex();
}
QModelIndex RemoteDirModel::find_node_item_by_path_elements(NetDirNode *parent_node_item,
                                                            QStringList &path_elements, int level) const
{
    if (level < 0 ) return this->createIndex(0, 0, this->rootNode);
    QString aim_child_node_item_name = path_elements.at(level);
    Q_ASSERT(parent_node_item);
    //qDebug()<<__LINE__<< parent_node_item->_fileName<<level<<path_elements;
    for (int i = 0 ; i < parent_node_item->childNodes.count(); i ++ ) {
        NetDirNode *child_item = parent_node_item->childNodes.value(i);
        //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__<< child_item->_fileName;
        if (child_item->_fileName.compare(aim_child_node_item_name) == 0) {
            if (level == path_elements.count()-1) {
                //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
                QModelIndex mi = this->createIndex(i, 0, child_item);
                //return child_item;
                return mi ;
            } else {
                //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
                QModelIndex mi =this->find_node_item_by_path_elements(child_item, path_elements,level + 1);
                return mi ; 
            }
        }
    }
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    return this->createIndex(0, 0, parent_node_item);
    //return parent_node_item ;
}

QModelIndex RemoteDirModel::parent(const QModelIndex &child) const
{
    // qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    if (!child.isValid()) {
        //qDebug()<<" ! child.isValid()";
        return QModelIndex();
    }

    NetDirNode *child_item = static_cast<NetDirNode *>(child.internalPointer());
    NetDirNode *parent_item = child_item->pNode;

    if (parent_item == 0) {
        // currIndex is the rootNode index
    } else {
        QModelIndex idx = createIndex(parent_item->onRow, 0, child_item->pNode);
        if (idx.row() == -1 && idx.column() == 0) {
            dump_tree_node_item(child_item->pNode);
        }
        Q_ASSERT(!(idx.row() == -1 && idx.column() == 0));
        return idx;
    }

    return QModelIndex();
}

QVariant RemoteDirModel::data(const QModelIndex &index, int role) const
{
    QVariant ret_var;
    QString unicode_name;

    Q_ASSERT(index.isValid());
    NetDirNode *item = static_cast<NetDirNode*>(index.internalPointer());

    if (role == Qt::DecorationRole && index.column()==0) {
        if (this->isDir(index)) {
            return qApp->style()->standardIcon(QStyle::SP_DirIcon);
        } else if (this->isSymLink(index)) {
            return QIcon(":/icons/emblem-symbolic-link.png");
            // return qApp->style()->standardIcon(QStyle::SP_DirLinkIcon);
        } else {
            // qDebug()<<this->mime->fromFileName("test.png")<<this->mime->fromFileName(item->fullPath);
            int lastSplashPos = item->fullPath.lastIndexOf(QChar('/'));
            int lastDotPos = item->fullPath.lastIndexOf(QChar('.'));
            QString suffix;
            if (lastDotPos > lastSplashPos) {
                suffix = item->fullPath.right(item->fullPath.length() - lastDotPos - 1);
            }
            if (suffix.isEmpty()) {
                return qApp->style()->standardIcon(QStyle::SP_FileIcon);
            } else {
                if (suffix.right(1) == "~" || suffix.right(1) == "#") {
                    suffix = suffix.left(suffix.length() - 1);
                }
                suffix = suffix.toLower();
                // qDebug()<<suffix<<gMimeHash.value(suffix);
                QIcon icon = QIcon(qApp->applicationDirPath() + "/icons/mimetypes/" + gMimeHash.value(suffix) + ".png");
                if (icon.actualSize(QSize(32,32)).isEmpty() || gMimeHash.value(suffix).isEmpty()) {
                    icon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
                }
                return icon;
            }
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    // this->dump_tree_node_item(item);

    switch (index.column()) {
    case 0:
        ret_var = item->fileName();
        // this->dump_tree_node_item(item);
        break;
    case 2:
        ret_var = item->fileMode();
        break;
    case 1:
        ret_var = item->strFileSize();
        break;
    case 3:
        ret_var = item->fileMDate();
        break;
    default:
        return QVariant();
    }
    // qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<<__FILE__<<index<<role<<ret_var;
    return ret_var ;
}

Qt::ItemFlags RemoteDirModel::flags(const QModelIndex &index) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if (index.column() == 0) {
        return flags | Qt::ItemIsEditable;
    } else {
        return flags;
    }
}
QVariant RemoteDirModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug() <<"section:" << section ;

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return QVariant(tr("Name"));
        case 1:
            return QVariant(tr("Size"));
        case 2:
            return QVariant(tr("Type"));
        case 3:
            return QVariant(tr("Date"));
        default:
            return QVariant(tr("what are you want?"));
        }
    }

    return QVariant();

}

int RemoteDirModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    return 4 ;
}

int RemoteDirModel::rowCount(const QModelIndex &parent) const
{
    // q_debug()<<parent;
    int row_count = 0;

    if (!parent.isValid()) {
        row_count = 1;
    } else {
        NetDirNode *parent_item = static_cast<NetDirNode*>(parent.internalPointer());
        Q_ASSERT(parent_item != NULL);

        // q_debug()<<parent<<"opening "<<parent_item->fullPath<<parent_item->_fileName;
        if (parent_item->_fileName == ".wicd") {
            // this->dump_tree_node_item(parent_item);
            Q_ASSERT(1 == 2);
        }

        // if (parent_item->retrFlag == 0
        //     // || parent_item->retrFlag == 1
        //     || parent_item->retrFlag == 2 ) //为了lazy模式的需要，才做一个假数据。
        if (parent_item->retrFlag == POP_NO_NEED_NO_DATA
            || parent_item->retrFlag == POP_WITH_NEED_WANT_UPDATE)
        {
            //或者是不是要在这里取子结点的行数，即去远程找数据呢。
            // row_count =1 ;
            qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__
                    <<parent_item->fullPath<<parent_item->_fileName;
            QPersistentModelIndex *persisIndex = new QPersistentModelIndex(parent);
            this->dir_retriver->add_node(parent_item, persisIndex);
            emit operationTriggered(QString(tr("Sending SSH2_FXP_READDIR %1")).arg(parent_item->filePath()));
        } else {
            row_count = parent_item->childNodes.count();
        }
        // dump_tree_node_item(parent_item);
        row_count = parent_item->childNodes.count();
        // row_count =1 ;
    }
    // q_debug()<<parent<< "row_count="<<row_count;
    return row_count;
}

bool RemoteDirModel::hasChildren(const QModelIndex &parent) const
{

    if (parent.isValid()) {
        NetDirNode *curr_item = static_cast<NetDirNode*>(parent.internalPointer());
        if (curr_item != NULL) {
            // q_debug()<<parent<<this->isDir(parent)<<"opening "<<curr_item->fullPath<<curr_item->_fileName;
            // bool hc = this->isDir(parent) || this->isSymLinkToDir(parent);
            bool hc = curr_item->isDir() || curr_item->isSymLinkToDir();
            return hc;
        } else {
            q_debug()<<parent<<"parent pointer null";
            assert(0);
        }
        return true;
    } else {
        return true;
    }
}

bool RemoteDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role != Qt::EditRole) return false;
    // q_debug()<<value.type();

    if (value.type() == QVariant::String) {
        if (value.toString().length() <= 0) return false;
        Q_ASSERT(index.column() == 0);

        NetDirNode *parent_item = static_cast<NetDirNode*>(index.parent().internalPointer());
        NetDirNode *dti =  static_cast<NetDirNode*>(index.internalPointer());
        int cmd = SSH2_FXP_RENAME;
        QString data = dti->_fileName + "!" + value.toString();

        if (dti->_fileName == value.toString()) return false;

        dti->fullPath = parent_item->filePath() + "/" + value.toString();
        dti->_fileName = value.toString();//这时是不能修改这个值,否则上句命令执行的时候找不到原文件名
        
        QPersistentModelIndex *persisIndex = new QPersistentModelIndex(index.parent());
        this->dir_retriver->slot_execute_command(parent_item, persisIndex, cmd, data);
        emit operationTriggered(QString(tr("Sending SSH2_FXP_RENAME %1")).arg(parent_item->filePath()));
    } else {
        NetDirNode *oldNode = static_cast<NetDirNode*>(index.internalPointer());
        NetDirNode *newNode = (NetDirNode*)qVariantValue<void*>(value);

        Q_ASSERT(oldNode != NULL);
        Q_ASSERT(newNode != NULL);
        int oldOnRow = oldNode->onRow;
        oldNode->copyFrom(newNode);
        oldNode->onRow = oldOnRow;

        QModelIndex endIndex = this->createIndex(index.row(), 3, oldNode->pNode);
        emit this->dataChanged(index, endIndex);
    }

    return true;
}

bool RemoteDirModel::insertRows(int row, int count, const QModelIndex &parent)
{
    // q_debug()<<""<<row<<count<<parent;
    NetDirNode *pnode = NULL;
    NetDirNode *node = NULL;

    Q_ASSERT(count >= 1);

    emit this->beginInsertRows(parent, row, row + count - 1);
    if (!parent.isValid()) {
        Q_ASSERT(row == 0);
        Q_ASSERT(count == 1);
        node = new NetDirNode();
        node->onRow = 0;
        this->rootNode = node;
    } else {
        pnode = static_cast<NetDirNode*>(parent.internalPointer());
        // dump_tree_node_item(pnode);
        Q_ASSERT(row >= 0 && row <= pnode->childNodes.count());
        // Q_ASSERT(pnode->childNodes.count() == 0);
        if (row < pnode->childNodes.count()) {
            for (int i = count - 1; i >= 0; --i) {
                node = pnode->childNodes.value(i + row);
                node->onRow = i + row + count - 1;
                Q_ASSERT(node->onRow >= 0);
                pnode->childNodes[i + row] = 0;
                pnode->childNodes.insert(i + row + count - 1, node);
            }
        }
        for (int i = row; i < row + count; ++i) {
            node = new NetDirNode();
            node->pNode = pnode;
            node->onRow = i;
            Q_ASSERT(node->onRow >= 0);
            pnode->childNodes.insert(i, node);
        }
        // dump_tree_node_item(pnode);
    }
    emit this->endInsertRows();
    return true;
}
bool RemoteDirModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<" row = "<< row << " count = "<< count;
    if (parent.isValid()) {
        qDebug() << parent;
    } else {
        qDebug()<< parent.isValid();
    }
    NetDirNode *parent_item = static_cast<NetDirNode*>(parent.internalPointer());
    qDebug()<<__FUNCTION__<<__LINE__<<parent_item;
    NetDirNode *delete_item = 0;
    NetDirNode *temp_item  = 0;
    
    this->beginRemoveRows(parent, row, row + count - 1);
    // this->dump_tree_node_item(parent_item);
    
    for (int i = row + count -1 ; i >= row ; i --) {
        delete_item = parent_item->childNodes.value(i);
        if (i != parent_item->childNodes.count() - 1) {
            for (int j = i + 1 ; j < parent_item->childNodes.count(); j ++) {
                temp_item = parent_item->childNodes.value(j);
                temp_item->onRow = j - 1;
                parent_item->childNodes.insert(j-1, temp_item);
            }
            int c_size = parent_item->childNodes.count();
            parent_item->childNodes.remove(c_size - 1);
        } else {
            int c_size = parent_item->childNodes.count();
            parent_item->childNodes.remove(c_size - 1);
        }
        delete delete_item; delete_item = 0;
    }

    this->endRemoveRows();
    q_debug()<<"";
    return true;
}
bool RemoteDirModel::rowMoveTo(const QModelIndex &from, const QModelIndex &to)
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    return true;
}

bool RemoteDirModel::insertColumns(int column, int count, const QModelIndex &parent /*= QModelIndex() */)
{
    Q_UNUSED(column);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    return true ;
}
bool RemoteDirModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    Q_UNUSED(roles);
    return true;
}

void RemoteDirModel::dump_tree_node_item(NetDirNode *node_item) const
{
    assert(node_item != 0);
    qDebug()<<"====================>>>>"<<node_item;
    qDebug()<<"dir="<<QString(node_item->fullPath);
    qDebug()<<"name="<<QString(node_item->_fileName);
    qDebug()<<"Type="<<QString(node_item->fileType() );
    qDebug()<<"Size="<<QString(node_item->strFileSize());
    qDebug()<<"Date="<<QString(node_item->fileMDate());
    qDebug()<<"RetrFlag="<<node_item->retrFlag;
    qDebug()<<"prev_retr_flag="<<node_item->prevFlag;
    qDebug()<<"ChildCount="<<node_item->childNodes.count()<<node_item->childNodes.keys().count()
            <<node_item->childNodes.keys();
    QStringList values;
    for (int i = 0 ; i < node_item->childNodes.count(); ++i) {
        values << node_item->childAt(i)->_fileName;
        values << (node_item->childAt(i)->deleted ? "d" : "r");
    }
    qDebug()<<values;
    qDebug()<< "DeleteFlag="<<node_item->deleted;
    qDebug()<<"onRow="<<node_item->onRow;
    qDebug()<<"parent="<<node_item->pNode;
    qDebug()<<"<<<<====================";
}

// TODO a lot of memory leak here
// should not memory leak now.
void RemoteDirModel::slot_remote_dir_node_retrived(NetDirNode *parent_item,
                                                   void *parent_persistent_index, 
                                                   NetDirNode *newNodes
                                                   )
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__
            <<parent_item->fullPath<<parent_item->_fileName<<newNodes;

    // QModelIndex pindex;
    // pindex = homeIndex;

    // this->insertRows(0, 1, pindex);
    // NetDirNode *node = new NetDirNode();
    // node->fullPath = "/home/group/team/kitsoft/abcd";
    // node->_fileName = "abcd";

    // QModelIndex cindex = this->index(0, 0, pindex);
    // QVariant vv = qVariantFromValue((void*)node);
    // this->setData(cindex, vv);

    QPersistentModelIndex *persisIndex = (QPersistentModelIndex*)parent_persistent_index;
    QModelIndex currIndex = this->index(persisIndex->row(), persisIndex->column(), persisIndex->parent());
    QString currPath = parent_item->filePath();
    int row, col = 0;
    int changeCount = 0;
   
    emit operationTriggered(QString(tr("Info subdirectories of %1")).arg(parent_item->filePath()));
    Q_ASSERT(newNodes != NULL);
    for (int i = 0; i < newNodes->childNodes.count(); ++i) {
        NetDirNode *node = newNodes->childAt(i);
        emit operationTriggered(QString(tr("< %1 %2 %3 %4")).arg(node->fileMode()).arg(node->fileSize())
                                .arg(node->fileMDate()).arg(node->fileName()));
    }

    // calc delta items, some should deleted and some should add
    // this method maybe has perfomance problem, but run properly.
    // first, search should deleted items, and delete it
    bool found = false;
    int i, j;
    QModelIndex tmpIndex;
    q_debug()<<this->rowCount(currIndex);
    for (i = this->rowCount(currIndex) - 1; i >= 0; --i) {
        found = false;
        tmpIndex = this->index(i, 0, currIndex);
        NetDirNode *node = static_cast<NetDirNode*>(tmpIndex.internalPointer());
        for (j = newNodes->childNodes.count() - 1; j >= 0; --j) {
            NetDirNode *nnode = newNodes->childAt(j);
            if (nnode->_fileName == node->_fileName) {
                found = true;
                break;
            }
        }

        // the i'th item is not in newNodes, found should remove row
        if (found == false) {
            qDebug()<<"find should delete item: "<<i
                    << node->_fileName;

            this->removeRows(i, 1, currIndex);
        } else {
            // update the old node
            NetDirNode *nnode = newNodes->childAt(j);
            QVariant vv = qVariantFromValue((void*)nnode);
            this->setData(tmpIndex, vv);
        }
    }

    // second, search should added items, and add it
    for (i = 0; i < newNodes->childNodes.count(); ++i) {
        found = false;
        NetDirNode *nnode = newNodes->childAt(i);
        NetDirNode *node = NULL;

        int row = this->rowCount(currIndex);
        for (j = 0; j < row ; ++j) {
            tmpIndex = this->index(j, 0, currIndex);
            node = static_cast<NetDirNode*>(tmpIndex.internalPointer());
            if (node->_fileName == nnode->_fileName) {
                found = true;
                break;
            }
        }

        if (found == true) {
            // omit
        } else {
            this->insertRows(row, 1, currIndex);
            nnode->onRow = row;
            QModelIndex newIndex = this->index(row, 0, currIndex);
            QVariant vv = qVariantFromValue((void*)nnode);
            this->setData(newIndex, vv);
        }
    }

    delete persisIndex; persisIndex = NULL;
    delete newNodes; newNodes = NULL;

    // qDebug()<<parent_item->childNodes.keys();
    // for (int i = 0; i < parent_item->childNodes.count() ; ++i) {
    //     NetDirNode *node = parent_item->childNodes.value(i);
    //     qDebug()<<"deleted:"<<node->deleted
    //             <<node->_fileName<<node->onRow;
    // }

    // return;
    // q_debug()<<"befor delete, child count:"<<parent_item->childNodes.count();
    // for (int i = parent_item->childNodes.count() - 1 ; i >= 0; i --) {
    //     if (parent_item->childNodes.value(i)->deleted == true) {
    //         row = i;
    //         qDebug()<<"find should delete item: "<<i
    //                 <<"row num:"<<row;

    //         this->removeRows(row, 1, currIndex);
    //         changeCount ++;
    //     }
    // }

    parent_item->prevFlag = parent_item->retrFlag;
    parent_item->retrFlag = POP_NEWEST;

    emit directoryLoaded(currPath);

    // qDebug()<<parent_item->childNodes.keys();
    // for (int i = 0; i < parent_item->childNodes.count() ; ++i) {
    //     NetDirNode *node = parent_item->childNodes.value(i);
    //     qDebug()<<"deleted:"<<node->deleted
    //             <<node->_fileName<<node->onRow;
    // }

    // clean memory
    // emit layoutChanged(); // this cause crash!!!!!
}

Qt::DropActions RemoteDirModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList RemoteDirModel::mimeTypes() const
{
    QStringList mtypes;
    mtypes<<"text/uri-list"<<"application/task-package";
    //mtypes<<"text/plain";
    mtypes<<QAbstractItemModel::mimeTypes();
    //qDebug()<<mtypes ;
    //mtypes = QAbstractItemModel::mimeTypes();
    //qDebug()<<mtypes;
    //return QAbstractItemModel::mimeTypes();
    return mtypes;

}
QMimeData *RemoteDirModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *md = new QMimeData();
    QByteArray encodedData;

    //读取出来indexes中的目录路径信息，封装成特定mine 类型的数据即可以了。
    QString _fileName = "file:///hahahha";
    QString file_type ;

    //现在还没有支持多选择，那么我们就只处理一个即可,不用考虑那么多
    NetDirNode *selected_item = static_cast<NetDirNode*>(indexes.at(0).internalPointer());

    assert(selected_item != NULL);

    _fileName = selected_item->fullPath;
    file_type = selected_item->fileType();

    //格式说明：sftp://_fileName||file_type
    _fileName = QString("sftp://" + _fileName+"||"+file_type ).toAscii();
    encodedData = _fileName.toAscii();
    //QList<QUrl> urls ;
    //application/x-qabstractitemmodeldatalist
    md->setData("text/uri-list", encodedData);
    //urls.append(QUrl(_fileName));

    //md->setUrls(urls);

    return md;
}
bool RemoteDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column, const QModelIndex &parent)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<data->urls()<<parent<<sender();
    emit this->sig_drop_mime_data(data, action, row, column, parent);
    bool ret = true;
    // 	qDebug() <<"signals emited";
    return true;
    return ret;
}

void RemoteDirModel::slot_remote_dir_node_clicked(const QModelIndex &index)
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QPersistentModelIndex *persisIndex = new QPersistentModelIndex(index);
    NetDirNode *clicked_item = 0;
    clicked_item = static_cast<NetDirNode*>(index.internalPointer());

    if (index.isValid() == false) return;
    //this->dump_tree_node_item(clicked_item);

    if (clicked_item->retrFlag == POP_NO_NEED_WITH_DATA) { // 半满状态结点
        this->dir_retriver->add_node(clicked_item, persisIndex);
        emit operationTriggered(QString(tr("Sending SSH2_FXP_READDIR %1")).arg(clicked_item->filePath()));
    } else {
        //no op needed
    }
}

// TODO split this into several method, upper can not call this method.
// maybe execute_rename, execute_mkdir, execute_rmdir and so on.
void RemoteDirModel::slot_execute_command(NetDirNode *parent_item, 
                                           void *parent_persistent_index, int cmd, QString params)
{
    QString msg = QString(tr("Sending %3 %2 %1")).arg(params);
    if (parent_item == NULL) {
        msg.replace("%2", "");
    } else {
        msg.replace("%2", parent_item->filePath());
    }
    switch (cmd) {
    case SSH2_FXP_RENAME:
        msg.replace("%3", "SSH2_FXP_RENAME");
        break;
    case SSH2_FXP_MKDIR:
        msg.replace("%3", "SSH2_FXP_MKDIR");
        break;
    case SSH2_FXP_RMDIR:
        msg.replace("%3", "SSH2_FXP_RMDIR");
        break;
    case SSH2_FXP_REALPATH:
        msg.replace("%3", "SSH2_FXP_REALPATH");
        break;
    default:
        msg.replace("%3", "unknown operation SSH2_FXP_???");
        break;
    };
    emit operationTriggered(msg);
    this->dir_retriver->slot_execute_command(parent_item, parent_persistent_index, cmd, params);
}

void RemoteDirModel::execute_command_finished(NetDirNode *parent_item, void *parent_persistent_index,
                                              int cmd, int status)
{
    QPersistentModelIndex *persisIndex = (QPersistentModelIndex*)parent_persistent_index;
    // q_debug()<<cmd<<status<<persisIndex<<sender();
    switch (cmd) {
    case SSH2_FXP_REALPATH:
        if (status == 0) {
            parent_item->linkToDir = true;
            // we own persisIndex now, so use data changed on exact index
            QModelIndex beginIndex = this->index(persisIndex->row(), 0, persisIndex->parent());
            QModelIndex endIndex = this->index(persisIndex->row(), 3, persisIndex->parent());
            emit dataChanged(beginIndex, endIndex);

            // TODO how goto the task?
            // if (parent_item->retrFlag == POP_NO_NEED_WITH_DATA) { // 半满状态结点
            if (parent_item->retrFlag == POP_NO_NEED_NO_DATA) {
                this->dir_retriver->add_node(parent_item, parent_persistent_index);
            }
        } else {
            parent_item->prevFlag = parent_item->retrFlag;
            parent_item->retrFlag = POP_NEWEST;
            QModelIndex beginIndex = this->index(persisIndex->row(), 0, persisIndex->parent());
            QModelIndex endIndex = this->index(persisIndex->row(), 3, persisIndex->parent());
            emit dataChanged(beginIndex, endIndex);
        }
        break;
    case SSH2_FXP_REMOVE:
    case SSH2_FXP_RMDIR:
    case SSH2_FXP_MKDIR:
    case SSH2_FXP_READDIR:
    case SSH2_FXP_RENAME:
    case SSH2_FXP_INIT:
    case SSH2_FXP_KEEP_ALIVE:
        break;
    default:
        // persisIndex will be destructed at slot_remote_dir_node_retrived
        // i do not care other command result
        q_debug()<<"Unknown/Uncared command:"<<cmd<<status<<persisIndex<<sender();
        if (persisIndex != NULL) {
            // delete persisIndex;
            // persisIndex = 0;
        }
        break;
    }

    QString msg = QString(tr("Received %2: %1")).arg(status == 0 ? "OK" : "Failed");
    switch (cmd) {
    case SSH2_FXP_READDIR:
        // TODO always return error, even read sucessfully.
        msg = msg.arg("SSH2_FXP_READDIR");
        break;
    case SSH2_FXP_RENAME:
        msg = msg.arg("SSH2_FXP_RENAME");
        break;
    case SSH2_FXP_MKDIR:
        msg = msg.arg("SSH2_FXP_MKDIR");
        break;
    case SSH2_FXP_RMDIR:
        msg = msg.arg("SSH2_FXP_RMDIR");
        break;
    case SSH2_FXP_REALPATH:
        msg = msg.arg("SSH2_FXP_REALPATH");
        break;
    case SSH2_FXP_REMOVE:
        msg = msg.arg("SSH2_FXP_REMOVE");
        break;
    case SSH2_FXP_KEEP_ALIVE:
        // TODO always return error, even exec sucessfully.
        msg = msg.arg("SSH2_FXP_KEEP_ALIVE");
        break;
    case SSH2_FXP_INIT:
        msg = msg.arg("SSH2_FXP_INIT");
        break;
    default:
        msg = msg.arg(QString("SSH2_FXP_%1(%2)").arg(cmd).arg(status));
        break;
    };
    emit operationTriggered(msg);
}

//TODO
void RemoteDirModel::set_keep_alive(bool keep_alive,int time_out)
{
    //this->keep_alive_interval = time_out ;
    //this->keep_alive = keep_alive ;
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug()<<keep_alive<<time_out;
    
    if (time_out != this->keep_alive_interval) {
        this->keep_alive_interval = time_out;
        this->keep_alive_timer->setInterval(this->keep_alive_interval);
    }
    //assert(1==2);
    if (keep_alive != this->keep_alive) {
        if (keep_alive == false) {
            this->keep_alive_timer->stop();
        } else {
            this->keep_alive_timer->start();
        }
        this->keep_alive = keep_alive;
    }
}

void RemoteDirModel::slot_keep_alive_time_out()
{
    //qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->dir_retriver->slot_execute_command(0, 0, SSH2_FXP_KEEP_ALIVE, "");
    
}
QString RemoteDirModel::filePath(const QModelIndex &index) const
{
    NetDirNode *node_item = 0;
    node_item =  static_cast<NetDirNode*>(index.internalPointer());
    assert(node_item != 0);

    return node_item->filePath();
}
QString RemoteDirModel::fileName(const QModelIndex &index) const
{
    NetDirNode *node_item = 0;
    node_item =  static_cast<NetDirNode*>(index.internalPointer());
    assert(node_item != 0);

    return node_item->fileName();
}

bool RemoteDirModel::isDir(const QModelIndex &index) const
{
    NetDirNode *node_item = static_cast<NetDirNode*>(index.internalPointer());
    // assert(node_item != 0);
    if (node_item == 0) {
        return true;
    } else {
        return node_item->isDir();
    }
}

bool RemoteDirModel::isSymLink(const QModelIndex &index) const
{
    NetDirNode *node_item = static_cast<NetDirNode*>(index.internalPointer());
    // assert(node_item != 0);
    if (node_item == 0) {
        return true; // is this right?
    } else {
        return node_item->isSymLink();    
    }
}

bool RemoteDirModel::isSymLinkToDir(const QModelIndex &index) const
{
    NetDirNode *node_item = static_cast<NetDirNode*>(index.internalPointer());
    // assert(node_item != 0);
    if (node_item == 0) {
        return false;
    } else {
        return node_item->isSymLinkToDir();    
    }
}
