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
                                                               cmd_elem->parent_model_internal_pointer));
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
        emit execute_command_finished(cmd_elem->parent_item, cmd_elem->parent_model_internal_pointer,
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
        attr->permissions |= S_IFREG;
    } else if (ui.isSymLink()) {
        attr->permissions |= S_IFLNK;
    } else if (ui.isDir()) {
        attr->permissions |= S_IFDIR;
    } else {
        qDebug()<<"unknown file type:"<<ui.name();
    }

    if (perm & QUrlInfo::ReadOwner) {
        attr->permissions |= S_IRUSR;
    }
    if (perm & QUrlInfo::WriteOwner) {
        attr->permissions |= S_IWUSR;
    }
    if (perm & QUrlInfo::ExeOwner) {
        attr->permissions |= S_IXUSR;
    }

    if (perm & QUrlInfo::ReadGroup) {
        attr->permissions |= S_IRGRP;
    }
    if (perm & QUrlInfo::WriteGroup) {
        attr->permissions |= S_IWGRP;
    }
    if (perm & QFile::ExeGroup) {
        attr->permissions |= S_IXGRP;
    }

    if (perm & QUrlInfo::ReadOther) {
        attr->permissions |= S_IROTH;
    }
    if (perm & QUrlInfo::WriteOther) {
        attr->permissions |= S_IWOTH;
    }
    if (perm & QUrlInfo::ExeOther) {
        attr->permissions |= S_IXOTH;
    }

    // TODO how got uid and gid. ftp not given it?
    return 0;
}

static QVector<directory_tree_item *> dirListToTreeNode(QVector<QUrlInfo> &dirList, directory_tree_item *pnode)
{
    QVector<directory_tree_item *> nodes;
    directory_tree_item *node;
    LIBSSH2_SFTP_ATTRIBUTES attr;
    
    for (int i = 0 ; i < dirList.count(); i++) {
        QUrlInfo ui = dirList.at(i);
        
        node = new directory_tree_item();
        QUrlInfo2LIBSSH2_SFTP_ATTRIBUTES(ui, &node->attrib);
        node->prev_retr_flag = 0;
        node->retrived = 0;
        node->delete_flag = 0;
        node->row_number = i;
        node->strip_path = pnode->strip_path + "/" + ui.name();
        node->file_name = ui.name();
        node->parent_item = pnode;

        nodes.append(node);
    }
    return nodes;
}

int FTPDirRetriver::retrive_dir()
{
    int exec_ret = -1;
    
    directory_tree_item *parent_item, *new_item;
    void *parent_model_internal_pointer;

    QByteArray ba;
    QString tmp;
    QVector<directory_tree_item*> deltaItems;
    QVector<QMap<char, QString> > fileinfos;
    char file_name[PATH_MAX+1];
    int fxp_ls_ret = 0;
 
    while (this->dir_node_process_queue.size() > 0) {
        std::map<directory_tree_item*,void * >::iterator mit;
        mit = this->dir_node_process_queue.begin();

        parent_item = mit->first;
        parent_model_internal_pointer = mit->second;
       
        fileinfos.clear();
        //状态初始化
        for (int i = 0; i < parent_item->childCount(); i++) {
            parent_item->childAt(i)->delete_flag = 1;
        }

        // passive
        this->conn->ftp->passive();
        this->conn->ftp->connectDataChannel();
        
        // list 
        this->conn->ftp->lista(parent_item->strip_path+"/");
        QVector<QUrlInfo> dirList = this->conn->ftp->getDirList();
        for (int i = dirList.count() - 1 ; i >= 0 ; i--) {
            QUrlInfo ui = dirList.at(i);
            qDebug()<<ui.name()<<ui.lastModified()<<ui.permissions()<<ui.size()<<ui.isSymLink();
            if (ui.name() == "." || ui.name() == "..") {
                dirList.remove(i); // 去掉本目录及上级目录这个特殊目录
            }
        }
        deltaItems = dirListToTreeNode(dirList, parent_item);
        //将多出的记录插入到树中
        for (int i = 0 ; i < deltaItems.count(); i ++) {
            deltaItems.at(i)->row_number = parent_item->childCount();
            parent_item->child_items.insert(std::make_pair(parent_item->childCount(), deltaItems.at(i)));
        }

        deltaItems.clear();
        parent_item->prev_retr_flag = parent_item->retrived;
        parent_item->retrived = POP_NEWEST;

        //         //////
        this->dir_node_process_queue.erase(parent_item);
        emit this->remote_dir_node_retrived(parent_item, parent_model_internal_pointer);

        // if (ssh2_sftp_handle != 0) //TODO 应该在循环上面检测到为0就continue才对啊。
        //     libssh2_sftp_closedir(ssh2_sftp_handle);
    }

    return exec_ret;
}

int  FTPDirRetriver::mkdir()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    command_queue_elem *cmd_item = this->command_queue.at(0);
    
    QString abs_path = cmd_item->parent_item->strip_path + QString("/") + cmd_item->params;
    
    qDebug()<<"abs path :"<<abs_path;
    
    // exec_ret = libssh2_sftp_mkdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data(), 0777);
    this->conn->ftp->mkdir(GlobalOption::instance()->remote_codec->fromUnicode(abs_path));

    this->add_node(cmd_item->parent_item, cmd_item->parent_model_internal_pointer);
    
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
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" + cmd_item->params;

    qDebug()<<"abs path :"<<abs_path;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rm system directory , this is danger.";
    } else {
        // exec_ret = libssh2_sftp_rmdir(ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path).data());
        exec_ret = this->conn->ftp->rmdir(GlobalOption::instance()->remote_codec->fromUnicode(abs_path));
    }
    //cmd_item->parent_item->retrived = 2;   //让上层视图更新这个结点
    this->add_node(cmd_item->parent_item, cmd_item->parent_model_internal_pointer);
    
    return exec_ret;
}

int  FTPDirRetriver::rm_file_or_directory_recursively()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int exec_ret = -1;
    QStringList  sys_dirs;
    sys_dirs<<"/usr"<<"/bin"<<"/sbin"<<"/lib"<<"/etc"<<"/dev"<<"/proc"
            <<"/mnt"<<"/sys"<<"/var";
    
    directory_tree_item *child_item = 0;
    directory_tree_item *parent_item = 0;
    command_queue_elem *cmd_item = this->command_queue.at(0);
    parent_item = cmd_item->parent_item;
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params;
    qDebug()<<"abs path :"<<abs_path;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rm system directory recusively, this is danger.";
    } else {
        //找到这个要删除的结点并删除
        for (unsigned int i = 0 ; i < parent_item->child_items.size() ; i ++) {
            child_item = parent_item->child_items[i];
            if (child_item->file_name.compare(cmd_item->params) == 0) {
                qDebug()<<"found will remove file:"<<child_item->strip_path;
                this->rm_file_or_directory_recursively_ex(child_item->strip_path);
                break;
            }
        }
    }

    this->add_node(cmd_item->parent_item, cmd_item->parent_model_internal_pointer);
    
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

    // exec_ret = this->fxp_do_ls_dir(parent_path + ("/"), fileinfos);
    
    // int file_count = fileinfos.size();
    // //qDebug()<<" rm ex :" << file_count;
    
    // for (int i = file_count -1 ; i >= 0 ; i --) {
    //     //qDebug()<<" lsed file:"<< fileinfos.at(i)['N'] ;
    //     if (fileinfos.at(i)['T'].at(0) == 'd') {
    //         if (fileinfos.at(i)['N'].compare(".") == 0 
    //             || fileinfos.at(i)['N'].compare("..") == 0)
    //         {
    //             //qDebug()<<"do ... ls shown . and .. ???";
    //             continue;
    //         } else {
    //             this->rm_file_or_directory_recursively_ex( parent_path + ("/") + fileinfos.at(i)['N']);
    //         }
    //     } else if (fileinfos.at(i)['T'].at(0) == 'l' || fileinfos.at(i)['T'].at(0) == '-') {
    //         abs_path = parent_path + "/" + fileinfos.at(i)['N'] ;//+ "/" + child_item->file_name ;
    //         //strcpy( remote_path , abs_path.toAscii().data() );
    //         //qDebug()<<QString(tr("Removing %1")).arg( remote_path );
    //         exec_ret = libssh2_sftp_unlink( ssh2_sftp, GlobalOption::instance()->remote_codec->fromUnicode(abs_path));
    //     } else {
    //         qDebug()<<" unknow file type ,don't know how to remove it";
    //     }
    // }
    
    // //删除这个目录
    // abs_path = GlobalOption::instance()->remote_codec->fromUnicode(parent_path); //+ "/" + parent_item->file_name;
    // //qDebug()<<"rmdir: "<<abs_path;
    // exec_ret = libssh2_sftp_rmdir(ssh2_sftp, abs_path.toAscii().data());
    // if (exec_ret != 0) //可能这是一个文件，不是目录，那么使用删除文件的指令
    // {
    //     exec_ret = libssh2_sftp_unlink(ssh2_sftp, abs_path.toAscii().data());
    //     if (exec_ret != 0) {
    //         qDebug()<<"Can't remove file or directory ("<< libssh2_sftp_last_error(ssh2_sftp) <<"): "<<abs_path;
    //     }
    // }
    
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
    
    QString abs_path = cmd_item->parent_item->strip_path + "/" +  cmd_item->params.mid(0, sep_pos);
    QString abs_path_rename_to = cmd_item->parent_item->strip_path + "/" + cmd_item->params.mid(sep_pos+1, -1);
    
    qDebug()<<"abs  path :"<<abs_path
            <<"abs path rename to:"<< abs_path_rename_to;
    
    if (sys_dirs.contains(abs_path)) {
        qDebug()<<"rename system directory , this is danger.";
    } else {
        exec_ret = this->conn->ftp->rename(GlobalOption::instance()->remote_codec->fromUnicode(abs_path),
                                           GlobalOption::instance()->remote_codec->fromUnicode(abs_path_rename_to));
    }

    this->add_node(cmd_item->parent_item, cmd_item->parent_model_internal_pointer);
    
    return exec_ret;
}

int FTPDirRetriver::keep_alive()
{
    int exec_ret;
    char full_path [PATH_MAX+1] = {0};
    char strip_path [PATH_MAX+1] = {0};

    //TODO 在网络失去连接的时候如何向上层类通知，并进行重新连接
    assert(this->conn);
    exec_ret = this->conn->ftp->noop();

    return exec_ret;
}

int FTPDirRetriver::fxp_do_ls_dir(QString path, QVector<QMap<char, QString> > &fileinfos)
{
    
    return 0;
}

int FTPDirRetriver::fxp_do_ls_dir(QString path, QVector<QPair<QString, LIBSSH2_SFTP_ATTRIBUTES*> > & fileinfos)
{
    return 0;
}

int FTPDirRetriver::fxp_realpath()
{
    int ret;
    return ret;
}

void FTPDirRetriver::add_node(directory_tree_item *parent_item, void *parent_model_internal_pointer)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	
    parent_item->prev_retr_flag = parent_item->retrived;
    parent_item->retrived = 8;
    command_queue_elem *cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
    cmd_elem->cmd = SSH2_FXP_READDIR;
    this->command_queue.push_back(cmd_elem);
	
    if (!this->isRunning()) {
        this->start();
    }
}

void FTPDirRetriver::slot_execute_command(directory_tree_item *parent_item,
                                                  void *parent_model_internal_pointer, int cmd, QString params)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    command_queue_elem *cmd_elem = new command_queue_elem();
    cmd_elem->parent_item = parent_item;
    cmd_elem->parent_model_internal_pointer = parent_model_internal_pointer;
    cmd_elem->cmd = cmd;
    cmd_elem->params = params;
    
    this->command_queue.push_back(cmd_elem);
    
    if (!this->isRunning()) {
        this->start();
    }
}
