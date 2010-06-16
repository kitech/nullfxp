// ftpdirretriver.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-25 09:50:50 +0000
// Version: $Id$
// 

#include <QtCore>

#ifdef WIN32
#include <winsock2.h>
#endif

#include "utils.h"
#include "globaloption.h"
#include "ftpdirretriver.h"

#include "rfsdirnode.h"
#include "connection.h"
#include "libftp/libftp.h"

///////////////////////////////////
FTPDirRetriver::FTPDirRetriver(QObject *parent)
    : DirRetriver(parent)
{
}

FTPDirRetriver::~FTPDirRetriver()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<<__FILE__;
    // libssh2_sftp_shutdown(this->ssh2_sftp);
    // libssh2_session_disconnect(this->ssh2_sess, "SSH_DISCONNECT_BY_APPLICATION");
    // libssh2_session_free(this->ssh2_sess);
}

// void FTPDirRetriver::set_ssh2_handler(void *ssh2_sess)
// {
//     this->ssh2_sess = (LIBSSH2_SESSION*)ssh2_sess;
//     this->ssh2_sftp = libssh2_sftp_init(this->ssh2_sess);
//     assert(this->ssh2_sftp != 0);
// }

void FTPDirRetriver::setConnection(Connection *conn)
{
    this->conn = conn;
    // this->ssh2_sess = this->conn->sess;
    // this->ssh2_sftp = libssh2_sftp_init(this->ssh2_sess);
    // assert(this->ssh2_sftp != 0);
}

// LIBSSH2_SFTP *FTPDirRetriver::get_ssh2_sftp()
// {
//     return this->ssh2_sftp;
// }

void FTPDirRetriver::run()
{
    emit enter_remote_dir_retrive_loop();
    
    int exec_ret_code = -1;
    command_queue_elem *cmd_elem;
    while (this->command_queue.size() > 0) {
        cmd_elem = this->command_queue.at(0);
        switch(cmd_elem->cmd) {
        case SSH2_FXP_READDIR:
            this->dir_node_process_queue.insert(std::make_pair(cmd_elem->parent_item, 
                                                               cmd_elem->parent_persistent_index));
            exec_ret_code = this->retrive_dir();
            break;
        case SSH2_FXP_MKDIR:
            exec_ret_code = this->mkdir();
            break;
        case SSH2_FXP_RMDIR:
            exec_ret_code = this->rmdir();
            break;
        case SSH2_FXP_REMOVE:
            exec_ret_code = this->rm_file_or_directory_recursively();
            break;
        case SSH2_FXP_RENAME:
            exec_ret_code = this->rename();
            break ;
        case SSH2_FXP_KEEP_ALIVE:
            exec_ret_code = this->keep_alive();
            break;
        case SSH2_FXP_REALPATH:
            exec_ret_code = this->fxp_realpath();
            break;
        default:    
            qDebug()<<tr("unknown command:")<<cmd_elem->cmd;
            break;
        }

        // 通知其他人命令执行完成
        emit execute_command_finished(cmd_elem->parent_item, cmd_elem->parent_persistent_index,
                                      cmd_elem->cmd, exec_ret_code);

        //delet item form queue , stopping infinite cycle
        this->command_queue.erase(this->command_queue.begin());
        delete cmd_elem;
        cmd_elem = 0;
        exec_ret_code = -1;
    }

    emit this->leave_remote_dir_retrive_loop();
}

static int QUrlInfo2LIBSSH2_SFTP_ATTRIBUTES(QUrlInfo &ui, LIBSSH2_SFTP_ATTRIBUTES *attr)
{
    assert(attr != NULL);
    int perm = ui.permissions();

    memset(attr, 0, sizeof(*attr));
    attr->filesize = ui.size();
    attr->atime = ui.lastRead().toTime_t();
    attr->mtime = ui.lastModified().toTime_t();

    if (ui.isFile() && !ui.isSymLink()) {
        attr->permissions |= LIBSSH2_SFTP_S_IFREG;
    } else if (ui.isSymLink()) {
        attr->permissions |= LIBSSH2_SFTP_S_IFLNK;
    } else if (ui.isDir()) {
        attr->permissions |= LIBSSH2_SFTP_S_IFDIR;
    } else {
        qDebug()<<"unknown file type:"<<ui.name();
    }

    if (perm & QUrlInfo::ReadOwner) {
        attr->permissions |= LIBSSH2_SFTP_S_IRUSR;
    }
    if (perm & QUrlInfo::WriteOwner) {
        attr->permissions |= LIBSSH2_SFTP_S_IWUSR;
    }
    if (perm & QUrlInfo::ExeOwner) {
        attr->permissions |= LIBSSH2_SFTP_S_IXUSR;
    }

    if (perm & QUrlInfo::ReadGroup) {
        attr->permissions |= LIBSSH2_SFTP_S_IRGRP;
    }
    if (perm & QUrlInfo::WriteGroup) {
        attr->permissions |= LIBSSH2_SFTP_S_IWGRP;
    }
    if (perm & QFile::ExeGroup) {
        attr->permissions |= LIBSSH2_SFTP_S_IXGRP;
    }

    if (perm & QUrlInfo::ReadOther) {
        attr->permissions |= LIBSSH2_SFTP_S_IROTH;
    }
    if (perm & QUrlInfo::WriteOther) {
        attr->permissions |= LIBSSH2_SFTP_S_IWOTH;
    }
    if (perm & QUrlInfo::ExeOther) {
        attr->permissions |= LIBSSH2_SFTP_S_IXOTH;
    }

    // TODO how got uid and gid. ftp not given it?
    return 0;
}

static QVector<NetDirNode *> dirListToTreeNode(QVector<QUrlInfo> &dirList, NetDirNode *pnode)
{
    QVector<NetDirNode *> nodes;
    NetDirNode *node;
    // LIBSSH2_SFTP_ATTRIBUTES attr;
    
    for (int i = 0 ; i < dirList.count(); i++) {
        QUrlInfo ui = dirList.at(i);
        
        node = new NetDirNode();
        QUrlInfo2LIBSSH2_SFTP_ATTRIBUTES(ui, &node->attrib);
        node->prevFlag = 0;
        node->retrFlag = (node->isDir() || node->isSymLink()) 
                    ? POP_NO_NEED_NO_DATA : POP_NEWEST;
        node->deleted = 0;
        node->onRow = i;
        node->fullPath = pnode->fullPath + "/" + ui.name();
        node->_fileName = ui.name();
        node->pNode = pnode;

        nodes.append(node);
    }
    return nodes;
}

int FTPDirRetriver::retrive_dir()
{
    int exec_ret = -1;
    
    NetDirNode *parent_item;
    void *parent_persistent_index;

    QByteArray ba;
    QString tmp;
    QVector<NetDirNode*> deltaItems;
    QVector<QMap<char, QString> > fileinfos;
    // char file_name[PATH_MAX+1] = {0};
    // int fxp_ls_ret = 0;
 
    while (this->dir_node_process_queue.size() > 0) {
        std::map<NetDirNode*,void * >::iterator mit;
        mit = this->dir_node_process_queue.begin();

        parent_item = mit->first;
        parent_persistent_index = mit->second;
       
        fileinfos.clear();

        // passive
        this->conn->ftp->passive();
        this->conn->ftp->connectDataChannel();
        
        // list 
        this->conn->ftp->lista(parent_item->fullPath + "/");
        QVector<QUrlInfo> dirList = this->conn->ftp->getDirList();
        for (int i = dirList.count() - 1 ; i >= 0 ; i--) {
            QUrlInfo ui = dirList.at(i);
            qDebug()<<ui.name()<<ui.lastModified()<<ui.permissions()<<ui.size()<<ui.isSymLink();
            if (ui.name() == "." || ui.name() == "..") {
                dirList.remove(i); // omit . and .. special director
            }
        }
        deltaItems = dirListToTreeNode(dirList, parent_item);
        // copy to temp node, for pass as params
        NetDirNode *newNodes = new NetDirNode();
        for (int i = 0 ; i < deltaItems.count(); i ++) {
            newNodes->childNodes.insert(i, deltaItems.at(i));
        }

        deltaItems.clear();
        // parent_item->prevFlag = parent_item->retrFlag;
        // parent_item->retrFlag = POP_NEWEST;

        //         //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item, parent_persistent_index, newNodes);
        exec_ret = 0; // for return the properly value
    }

    return exec_ret;
}

int  FTPDirRetriver::mkdir()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    command_queue_elem *cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->fullPath + QString("/") + cmd_item->params;
    
    qDebug()<<"abs path :"<<abs_path;
    
    // exec_ret = libssh2_sftp_mkdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data(), 0777);
    this->conn->ftp->mkdir(GlobalOption::instance()->remote_codec->fromUnicode(abs_path));

    this->add_node(cmd_item->parent_item, cmd_item->parent_persistent_index);
    
    return exec_ret;
}

int  FTPDirRetriver::rmdir()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem *cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->fullPath + "/" + cmd_item->params;

    qDebug()<<"abs path :"<<abs_path;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rm system directory , this is danger.";
    } else {
        // exec_ret = libssh2_sftp_rmdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data());
        exec_ret = this->conn->ftp->rmdir(GlobalOption::instance()->remote_codec->fromUnicode(abs_path));
    }
    //cmd_item->parent_item->retrived = 2;   //让上层视图更新这个结点
    this->add_node(cmd_item->parent_item, cmd_item->parent_persistent_index);
    
    return exec_ret;
}

int  FTPDirRetriver::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var"<<"/media"<<"/lost+found"<<"/tmp"<<"/opt"
            <<"/lib32"<<"/lib64"<<"/home";
    
    NetDirNode *child_item = 0;
    NetDirNode *parent_item = 0;
    command_queue_elem *cmd_item = this->command_queue.at(0);
    parent_item = cmd_item->parent_item;
    
    QString abs_path = cmd_item->parent_item->fullPath + "/" +  cmd_item->params;
    qDebug()<<"abs path :"<<abs_path;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rm system directory recusively, this is danger.";
    } else {
        //找到这个要删除的结点并删除
        for (int i = 0 ; i < parent_item->childNodes.count(); i ++) {
            child_item = parent_item->childNodes.value(i);
            if (child_item->_fileName.compare(cmd_item->params) == 0) {
                qDebug()<<"found will remove file:"<<child_item->fullPath;
                this->rm_file_or_directory_recursively_ex(child_item->fullPath);
                break;
            }
        }
    }

    this->add_node(cmd_item->parent_item, cmd_item->parent_persistent_index);
    
    return exec_ret;
}

//TODO 现在删除隐藏文件或者目录还有问题：即以  .  字符开头的项
int FTPDirRetriver::rm_file_or_directory_recursively_ex(QString parent_path)  // <==> rm -rf
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    LibFtp *ftp;
    QString abs_path;
    // QVector<QMap<char, QString> > fileinfos;
    QVector<QUrlInfo> fileList;

    //再次从服务器列出目录，然后处理
    //int lflag = 0 ;    
    //     lflag = LS_LONG_VIEW;
    //     lflag |= LS_SHOW_ALL ;
    
    ftp = this->conn->ftp;
    exec_ret = ftp->chdir(parent_path);
    if (exec_ret == 0) {
        // dir
        // q_debug()<<"rm directory_recursively not impled";
        exec_ret = ftp->passive();
        assert(exec_ret == 0);
        exec_ret = ftp->connectDataChannel();
        assert(exec_ret == 0);
        exec_ret = ftp->lista(parent_path);
        assert(exec_ret == 0);
        exec_ret = ftp->closeDataChannel();
        fileList = ftp->getDirList();
        
        for (int i = fileList.count() - 1; i >= 0; --i) {
            QUrlInfo ui = fileList.at(i);
            if (ui.name() == "." || ui.name() == "..") {
                continue;
            }
            if (ui.isSymLink()) {
                q_debug()<<"How handle the link?";
            } else if (ui.isDir()) {
                exec_ret = this->rm_file_or_directory_recursively_ex( parent_path + ("/") + ui.name());
                assert(exec_ret == 0);
            } else {
                abs_path = parent_path + "/" + ui.name();
                exec_ret = ftp->remove(abs_path);
                if (exec_ret != 0) {
                    q_debug()<<"ftp remove file error:"<<abs_path;
                }                
            }
        }

        // 删除当前目录
        exec_ret = ftp->rmdir(parent_path);
        if (exec_ret != 0) {
            q_debug()<<"ftp rmdir error:"<<parent_path;
        }
    } else {
        // 删除当前文件
        exec_ret = ftp->remove(parent_path);
        if (exec_ret != 0) {
            q_debug()<<"ftp remove file error:"<<parent_path;
        }
    }

    return exec_ret;
}

// linux 路径名中不能出现的字符： ! 
int  FTPDirRetriver::rename()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    command_queue_elem *cmd_item = this->command_queue.at(0);
    
    size_t sep_pos = cmd_item->params.indexOf('!');
    
    QString abs_path = cmd_item->parent_item->fullPath + "/" +  cmd_item->params.mid(0, sep_pos);
    QString abs_path_rename_to = cmd_item->parent_item->fullPath + "/" + cmd_item->params.mid(sep_pos+1, -1);
    
    qDebug()<<"abs  path :"<<abs_path
            <<"abs path rename to:"<< abs_path_rename_to;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rename system directory , this is danger.";
    } else {
        exec_ret = this->conn->ftp->rename(GlobalOption::instance()->remote_codec->fromUnicode(abs_path),
                                           GlobalOption::instance()->remote_codec->fromUnicode(abs_path_rename_to));
    }

    this->add_node(cmd_item->parent_item, cmd_item->parent_persistent_index);
    
    return exec_ret;
}

int FTPDirRetriver::keep_alive()
{
    int exec_ret;
    // char full_path[PATH_MAX + 1] = {0};
    // char strip_path[PATH_MAX + 1] = {0};

    //TODO 在网络失去连接的时候如何向上层类通知，并进行重新连接
    assert(this->conn);
    exec_ret = this->conn->ftp->noop();

    return exec_ret;
}

int FTPDirRetriver::fxp_do_ls_dir(QString path, QVector<QMap<char, QString> > &fileinfos)
{
    
    return 0;
}

int FTPDirRetriver::fxp_do_ls_dir(QString path, QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > &fileinfos)
{
    return 0;
}

int FTPDirRetriver::fxp_realpath()
{
    int ret;
    return ret;
}

void FTPDirRetriver::add_node(NetDirNode *parent_item, void *parent_persistent_index)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	
    parent_item->prevFlag = parent_item->retrFlag;
    parent_item->retrFlag = POP_UPDATING;
    command_queue_elem *cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_persistent_index = parent_persistent_index;
    cmd_elem->cmd = SSH2_FXP_READDIR;
    this->command_queue.push_back(cmd_elem);
	
    if (!this->isRunning()) {
        this->start();
    }
}

void FTPDirRetriver::slot_execute_command(NetDirNode *parent_item,
                                                  void *parent_persistent_index, int cmd, QString params)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    command_queue_elem *cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_persistent_index = parent_persistent_index;
    cmd_elem->cmd = cmd;
    cmd_elem->params = params;
    
    this->command_queue.push_back(cmd_elem);
    
    if (!this->isRunning()) {
        this->start();
    }
}
