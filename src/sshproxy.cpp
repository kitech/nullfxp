// sshproxy.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-19 17:25:48 +0000
// Version: $Id$
// 

#include "simplelog.h"
#include "basestorage.h"

#include "connector.h"
#include "sshconnection.h"

#include "ui_sshproxy.h"
#include "sshproxy.h"



SSHProxy::SSHProxy(QWidget *parent)
    :QDialog(parent)
    ,uiw(new Ui::SSHProxy())
{
    this->uiw->setupUi(this);

    QObject::connect(this->uiw->pushButton_2, SIGNAL(clicked()), this, SLOT(slot_start()));

    this->timer = new QTimer();
    this->timer->setInterval(50);
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(slot_check_chan_timeout()));
    // timer->start();

}

SSHProxy::~SSHProxy()
{
}

void SSHProxy::slot_start()
{
    qLogx()<<"";
    unsigned short lsn_port;
    QString sess_name;

    sess_name = this->uiw->lineEdit_2->text();

    QMap<QString, QString> host;
    host = BaseStorage::instance()->getHost(sess_name);

    // this->connect_status_dailog = new ConnectingStatusDialog(username, hostname, port, this, Qt::Dialog);
    // QObject::connect(this->connect_status_dailog, SIGNAL(cancel_connect()),
    //                  this, SLOT(slot_cancel_connect()));

    Connector *connector = new Connector();
    connector->setHostInfo(host);
    QObject::connect(connector, SIGNAL(connect_finished(int, Connection *)),
                     this, SLOT(slot_connect_remote_host_finished (int, Connection *)));

#if  defined(NS_HAS_CXX0X) 
    //    #warning "modern c++0x mode"
    //  this->mfwdstate.insert(fsess_name, {connector, 0, 0, 0});
#else
    //    #warning "basic c++ mode"
    // ForwardState st(connector, 0, 0); 
    // this->mfwdstate.insert(fsess_name, st);
#endif
    // QObject::connect(this->connector, SIGNAL(connect_state_changed(QString)),
    //                  this->connect_status_dailog, SLOT(slot_connect_state_changed(QString)));

    // carefully set ui status
    // this->slot_set_ui_state(S_STARTING);

    connector->start();
    this->connector = connector;
}

// SSHConnection *gconn = NULL;
void SSHProxy::slot_connect_remote_host_finished (int eno, Connection *conn)
{

    qLogx()<<eno<<conn;
    
    int iret = -1;
    LIBSSH2_LISTENER *lsner = NULL;
    int bound_port = 0;
    int src_port;
    QString dest_hostname;
    int dest_ports;
    QString fsess_name;
    QMap<QString, QString> fwd;
    Connector *aconnector = static_cast<Connector*>(sender());
    Connection *aconn = conn;
    // check connection status
    int lsn_port = 0;

    sender()->deleteLater();

#if defined(NS_HAS_CXX0X)
    // auto it = this->mfwdstate.begin();
#else
    // QHash<QString, ForwardState>::iterator it;
#endif
    // for (it = this->mfwdstate.begin(); it != this->mfwdstate.end(); it++) {
    //     if (it.value().connector == aconnector) {
    //         fsess_name = it.key();
    //         it.value().conn = conn;
    //         break;
    //     }
    // }

    lsn_port = this->uiw->lineEdit->text().toInt();
    QTcpServer *serv = new QTcpServer();
    QObject::connect(serv, SIGNAL(newConnection()), SLOT(slot_newconnection()));
    bool bok = serv->listen(QHostAddress::Any, lsn_port);
    qLogx()<<bok;

    // gconn = (SSHConnection*)conn;
    this->conn = conn;

    //Q_ASSERT(!fsess_name.isEmpty());
    // this->mfwdstate[fsess_name].connector = NULL;
    // fsess_name = this->uiw->lineEdit_2->text();
    // fwd = BaseStorage::instance()->getForwarder(fsess_name);

    // if (fwd["ref_sess_name"] == "") {
    //     qLogx()<<"maybe session have not saved.";
    // }

    // src_port = this->uiw->lineEdit->text().toInt();
    // lsner = libssh2_channel_forward_listen_ex(conn->sess, NULL, 1234, &bound_port, 10);
    // lsner = libssh2_channel_forward_listen_ex(conn->sess, NULL, src_port, &bound_port, 10);
    // qLogx()<<lsner<<libssh2_session_last_errno(conn->sess);
    // if (lsner == NULL) {
    //     int eno = libssh2_session_last_errno(conn->sess);
    //     if (eno == LIBSSH2_ERROR_REQUEST_DENIED) {
    //         // 这种情况有可能是前一个forward_listen没有正确退出，导致服务器端的监听仍旧在运行，监听端口没有关闭
    //         // 这只是一种可能而已
    //         // 另还有一种情况，确实是服务器不支持这种操作
    //         // 目前还没有办法区分这两种情况。
    //         // 这种情况怎么能解决呢？
    //         // 一种方法，尝试执行远程命令，查看是否在有相应的监听端口,不过这种方法对windows无效
    //         // 
    //         qLogx()<<"Remote server denied forward port request.";
    //         const char *netstat = "netstat -ant";
    //         SSHConnection *sconn = (SSHConnection *)conn;
    //         QString result = sconn->get_server_env_vars(netstat);
    //         // qLogx()<<netstat<<" --> "<< result;
    //         bool listen_exist = false;
    //         QStringList state_lines = result.split("\n");
    //         QStringList state_fields;
    //         for (int i = 0; i < state_lines.count(); i ++) {
    //             state_fields = state_lines.at(i).split(" ", QString::SkipEmptyParts);
    //             // qLogx()<<state_fields;
    //             if (state_fields[0] == "tcp") {
    //                 if (state_fields[3].endsWith(QString(":%1").arg(src_port))
    //                     && state_fields[5] == "LISTEN") {
    //                     listen_exist = true;
    //                     break;
    //                 }
    //             }
    //         }
    //         if (listen_exist) {
    //             qLogx()<<"listen socket already exist, but connot control it, try drop this connection.";
    //             if (this->mfwdstate[fsess_name].want_reconn) {
    //                 // should run full restart
    //             } else {
    //                 // only run paritial restart 
    //             }
    //         } else {
    //             qLogx()<<"Maybe server can not support this forward port feature.";
    //         }
    //     } else {
    //         qLogx()<<"Unknown ssh channel error:"<<eno;
    //     }

    //     conn->disconnect();
    //     delete conn;
    //     this->mfwdstate.remove(fsess_name);

    //     this->slot_set_ui_state(S_START_READY);
    // } else {
    //     // Q_ASSERT(1234 == bound_port);
    //     Q_ASSERT(src_port == bound_port);
    //     this->mfwdstate[fsess_name].lsner = lsner;

    //     dest_hostname = this->uiw->lineEdit_4->text();
    //     dest_ports = this->uiw->lineEdit_5->text().toInt();

    //     ForwardPortWorker *worker = new ForwardPortWorker(lsner, dest_hostname, dest_ports);
    //     this->mfwdstate[fsess_name].worker = worker;
    //     QObject::connect(worker, SIGNAL(finished()), this, SLOT(slot_forward_worker_finished()));
    //     QObject::connect(worker, SIGNAL(listen_channel_error(int)),
    //                      this, SLOT(slot_listen_channel_error(int)));
    //     worker->start();

    //     this->slot_set_ui_state(S_STOP_READY);
    // }
}

// bool has_conn = false;
// QTcpSocket *gsock = NULL;
void SSHProxy::slot_newconnection()
{
    qLogx()<<sender();
    QTcpServer *serv = static_cast<QTcpServer*>(sender());
    QTcpSocket *sock = serv->nextPendingConnection();

    // if (has_conn) {
    //     sock->close();
    //     delete sock;
    //     return;
    // }

    this->mconns.insert(sock, NULL);

    QObject::connect(sock, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
    QObject::connect(sock, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
    QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     
    QObject::connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), 
                     this, SLOT(slot_forward_dest_socket_error(QAbstractSocket::SocketError)));     
    
    // gsock = sock;
    // has_conn = true;
}

/////////////
void SSHProxy::slot_forward_dest_connected()
{
    qLogx()<<"";
}

void SSHProxy::slot_forward_dest_disconnected()
{
    qLogx()<<"";

    int rn = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    QTcpSocket *sock = NULL;

    sock = static_cast<QTcpSocket*>(sender());
    if (this->mconns.contains(sock)) {
        chan = this->mconns[sock];

        this->mconns.remove(sock);
        
        if (chan != NULL) {
            libssh2_channel_close(chan);
            libssh2_channel_free(chan);
            chan = NULL;
        }

        sock->deleteLater();
    } else {
        sock->deleteLater();
    }

    // QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;
    // for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
    //     sock = it->first;
    //     chan = it->second;

    //     if (sock == sender()) {
    //         break;
    //     }
    // }

    // if (it == this->mfwds.end()) {
    //     qLogx()<<"Can not find for:"<<sender();
    // } else {
    //     qLogx()<<"remove connection :"<<sender()<<chan<<libssh2_channel_get_exit_status(chan);
    //     this->mfwds.erase(it);

    //     rn = libssh2_channel_close(chan);
    //     rn = libssh2_channel_free(chan);
        
    //     sender()->deleteLater();
    // }
}

// LIBSSH2_CHANNEL *gchan = NULL;
void SSHProxy::slot_forward_dest_socket_ready_read()
{
    qLogx()<<"";
    ssize_t wlen = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    // char rbuf[1000] = {0};
    QTcpSocket *sock = static_cast<QTcpSocket*>(sender());
    
    // QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;

    // for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
    //     if (it->first == sock) {
    //         chan = it->second;
    //         break;
    //     }
    // }
    // if (chan == NULL) {
    //     qLogx()<<"Can not find correspode sock:"<<sock;
    //     Q_ASSERT(chan != NULL);
    // }

    // ////////
    QByteArray ba = sock->readAll();
    qLogx()<<ba;
    QHttpRequestHeader reqhdr(ba);
    if (reqhdr.method() == "GET" || reqhdr.method() == "POST"
        || reqhdr.method() == "CONNECT") {
        QString upath = reqhdr.path();
        QString real_upath = upath;
        QString hdr_host;
        QString hdr_port;
        if (!real_upath.contains("://")) {
            hdr_host = reqhdr.value("Host");
            if (hdr_host.contains(":")) {
                hdr_port = hdr_host.split(":").at(1);
            } else {

            }
        }
        qLogx()<<"reqpath:"<<upath;
        QUrl pu = QUrl(upath);
        // gchan = chan = libssh2_channel_direct_tcpip(gconn->sess, "news.163.com", 80);
        // gchan = 
        unsigned short dest_port = pu.scheme() == "http" ? 80 :
            (pu.scheme() == "https" ? 443 : 8000);
        chan = libssh2_channel_direct_tcpip(this->conn->sess, pu.host().toAscii().data(), pu.port(dest_port));
        qLogx()<<"create dest channel:"<<pu.host()<<dest_port<<chan;
        if (chan == NULL) {
            chan = libssh2_channel_direct_tcpip(this->conn->sess, pu.host().toAscii().data(), pu.port(dest_port));
            qLogx()<<"create dest channel:"<<pu.host()<<dest_port<<chan
                   <<libssh2_session_last_errno(this->conn->sess);
        }
        if (chan == NULL) {
            sock->deleteLater();
            return;
        } else {
            libssh2_channel_set_blocking(chan, 0);
        
            Q_ASSERT(this->mconns.contains(sock));
            Q_ASSERT(this->mconns.value(sock) == NULL);
            this->mconns[sock] = chan;

            // QTimer *timer = new QTimer();
            // timer->setInterval(50);
            // QObject::connect(timer, SIGNAL(timeout()), this, SLOT(slot_check_chan_timeout()));
            this->timer->start();
        }
    } else {
        
        Q_ASSERT(this->mconns.contains(sock));
        Q_ASSERT(this->mconns.value(sock) != NULL);
        chan = this->mconns.value(sock);

        
    }
    // if ((ba.startsWith("GET ") || ba.startsWith("POST "))
    //     && (ba.contains("HTTP/1.") || ba.contains("HTTPS/1."))) {
    //     int spos, epos, slashpos;
    //     spos = ba.indexOf(" ") + 1;
    //     epos = ba.indexOf(" ", spos + 1) - 1;
    //     slashpos = ba.indexOf("/", epos + 1);
    //     QString upath = QString(ba.mid(epos+1, slashpos - epos + 1)) + QString("://qtchina.net")
    //         + QString(ba.mid(spos, epos - spos));
    //     QUrl pu = QUrl(QString(upath));
    //     qLogx()<<upath;
    //     gchan = chan = libssh2_channel_direct_tcpip(gconn->sess, "news.163.com", 80);
    //     qLogx()<<chan;
        
    // }

    wlen = libssh2_channel_write(chan, ba.data(), ba.length());
    qLogx()<<"src -> dest:"<<wlen;

    // wlen = libssh2_channel_write(chan, ba.data(), ba.length());
    // // qLogx()<<"sock -> ssh, wlen:"<<wlen<<ba.length();

    // if (wlen < 0) {
    //     if (wlen == LIBSSH2_ERROR_BAD_USE) {
    //         // should close this channel.
    //         qLogx()<<"Channel write error 123:"<<wlen;            
    //     } else if (wlen == LIBSSH2_ERROR_EAGAIN) {
    //         int max_retry = 100;
    //         do {
    //             wlen = libssh2_channel_write(chan, ba.data(), ba.length());
    //         } while (wlen == LIBSSH2_ERROR_EAGAIN && max_retry -- >= 0);
    //     } else {
    //         qLogx()<<"Channel write error:"<<wlen;
    //     }
    // } else if (wlen == 0) {
    // } else {
    //     fprintf(stdout, "R");
    //     fflush(stdout);
    // }
}

void SSHProxy::slot_forward_dest_socket_error(QAbstractSocket::SocketError socketError)
{
    qLogx()<<socketError;
}

void SSHProxy::slot_check_chan_timeout()
{
    // qLogx()<<"";
    char rbuf[1000] = {0};
    int rlen = 0;
    QTcpSocket *sock = NULL;
    LIBSSH2_CHANNEL *chan = NULL;

    QHash<QTcpSocket*, LIBSSH2_CHANNEL*>::iterator it;
    for (it = this->mconns.begin(); it != this->mconns.end(); it ++) {
        sock = it.key();
        chan = it.value();
        
        if (chan != NULL) {
            rlen = libssh2_channel_read(chan, rbuf, sizeof(rbuf)-1);
            qLogx()<<""<<rlen<<rbuf;
            if (rlen > 0) {
                rlen = sock->write(rbuf, rlen);
                qLogx()<<"dest -> src:"<<rlen;
            }
        }
    }

    // if (gchan != NULL) {
    //     rlen = libssh2_channel_read(gchan, rbuf, sizeof(rbuf)-1);
    //     qLogx()<<""<<rlen<<rbuf;
    //     if (rlen > 0) {
    //         rlen = gsock->write(rbuf, rlen);
    //         qLogx()<<"dest -> src:"<<rlen;
    //     }
    // }
}

