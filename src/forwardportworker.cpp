// forwardportworker.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-02 10:30:44 +0000
// Version: $Id$
// 

#include "libssh2/src/libssh2_priv.h"
// #include "libssh2.h"

#include "simplelog.h"

#include "forwardportworker.h"

/*
  When run ls -lR:

  Corrupted MAC on input.
  Disconnecting: Packet corrupt

  
  问题还不少，处理前几个连接之后，不能接收新的连接了。
 */
ForwardPortWorker::ForwardPortWorker(LIBSSH2_LISTENER *lsner,  const QString &dest_host, int dest_port, QObject *parent)
    : QThread(parent)
    , mlsner(lsner)
    , dest_host(dest_host)
    , dest_port(dest_port)
{
}

ForwardPortWorker::~ForwardPortWorker()
{

}

void ForwardPortWorker::run()
{
    int rn = 0;
    long timeout = 1;
    LIBSSH2_CHANNEL *chan = NULL;

    Q_ASSERT(this->mlsner != NULL);

    libssh2_session_set_blocking(this->mlsner->session, 0);
    QTimer *ter = new QTimer();
    QObject::connect(ter, SIGNAL(timeout()), this, SLOT(slot_poll_timeout()));
    ter->start(30);

    // while(true) {
    //     chan = libssh2_channel_forward_accept(this->mlsner);
    //     if (chan == NULL) {
    //         // qLogx()<<"accept null"<<libssh2_session_last_errno(this->mlsner->session);
    //         if (libssh2_session_last_errno(this->mlsner->session) == LIBSSH2_ERROR_EAGAIN) {
    //             this->msleep(20);
    //         }
    //     } else {
    //         qLogx()<<"new forward channel connected in.";
    //         QTcpSocket *sock = new QTcpSocket();
    //         QObject::connect(sock, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
    //         QObject::connect(sock, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
    //         QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     
            
    //         this->mfwds.append(QPair<QTcpSocket*, LIBSSH2_CHANNEL*>(sock, chan));

    //         sock->connectToHost("localhost", 22);
            
    //         // 
    //         libssh2_channel_set_blocking(chan, 0);
    //         QTcpSocket *qchan = new QTcpSocket();
    //         qchan->setSocketDescriptor(chan->session->socket_fd);
    //         QObject::connect(qchan, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
    //         QObject::connect(qchan, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
    //         QObject::connect(qchan, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     
    //         QObject::connect(qchan, SIGNAL(error(QAbstractSocket::SocketError)),
    //                          this, SLOT(slot_forward_dest_socket_error(QAbstractSocket::SocketError)));
    //     }
    // }

    this->exec();
}

void ForwardPortWorker::set_listener(LIBSSH2_LISTENER *lsner)
{
    int rn = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    QTcpSocket *sock = NULL;

    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> > tfwds = this->mfwds;
    this->mfwds.clear();
    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;
    for (it = tfwds.begin(); it != tfwds.end(); it++) {
        sock = it->first;
        chan = it->second;

        //////
        rn = libssh2_channel_close(chan);
        rn = libssh2_channel_free(chan);
        chan = NULL;

        ////////
        QObject::disconnect(sock);
        // QObject::disconnect(sock, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
        // QObject::disconnect(sock, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
        // QObject::disconnect(sock, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     
        // QObject::disconnect(sock, SIGNAL(error(QAbstractSocket::SocketError)), 
        //                     this, SLOT(slot_forward_dest_socket_error(QAbstractSocket::SocketError)));     

        sock->close();
        delete sock;
        sock = NULL;
    }

    this->mlsner = lsner;
    
}

void ForwardPortWorker::slot_poll_timeout()
{
    // qLogx()<<"";

    int rn = 0, eno = 0;
    ssize_t rlen = 0, wlen = 0;
    char rbuf[1000] = {0};
    LIBSSH2_CHANNEL *chan = NULL;
    QTcpSocket *sock = NULL;

    chan = libssh2_channel_forward_accept(this->mlsner);
    if (chan == NULL) {
        // qLogx()<<"accept null"<<libssh2_session_last_errno(this->mlsner->session);
        eno = libssh2_session_last_errno(this->mlsner->session);
        if (eno == LIBSSH2_ERROR_EAGAIN) {
            // this->msleep(20);
        } else if (eno == LIBSSH2_ERROR_CHANNEL_UNKNOWN) {
            qLogx()<<"Try restarting listen channel..."<<eno;
            // 
            emit listen_channel_error(eno);
        } else {
            qLogx()<<"accept null"<<libssh2_session_last_errno(this->mlsner->session);
        }
    } else {
        qLogx()<<"new forward channel connected in.";
        sock = new QTcpSocket();
        QObject::connect(sock, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
        QObject::connect(sock, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
        QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     
        QObject::connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), 
                         this, SLOT(slot_forward_dest_socket_error(QAbstractSocket::SocketError)));     
            
        this->mfwds.append(QPair<QTcpSocket*, LIBSSH2_CHANNEL*>(sock, chan));

        // sock->connectToHost("localhost", 22);
        if (!this->dest_host.isEmpty()) {
            qLogx()<<"Connect to dest host:"<<this->dest_host<<this->dest_port;
            sock->connectToHost(this->dest_host, this->dest_port);
        } else {
            // for test only
            sock->connectToHost("localhost", 22);
        }
        
        // 
        libssh2_channel_set_blocking(chan, 0);
    }
    
    int inv_cnt = 0;
    LIBSSH2_CHANNEL *inv_chans[100] = {0};
    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::const_iterator it;
    for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
        sock = it->first;
        chan = it->second;

        // read 
        rlen = libssh2_channel_read(chan, rbuf, sizeof(rbuf));
        if (rlen > 0)  {
            if (sock != NULL) {
                wlen = sock->write(rbuf, rlen);
                qLogx()<<"ssh -> sock, wlen:"<<wlen<<rlen;                
            } else {
            }
        } else {
            rn = libssh2_session_last_errno(this->mlsner->session);
            if (rn == LIBSSH2_ERROR_EAGAIN) {
                rn = libssh2_channel_eof(chan);
                if (rn == 1) {
                    qLogx()<<"Channel already closed by peer."<<chan;
                    inv_chans[inv_cnt++] = chan;
                }
            } else {
                qLogx()<<"Unknwon channel read error"<<rlen<<libssh2_session_last_errno(this->mlsner->session);
            }
        }
    }

    for (int i = 0; i < inv_cnt; i++) {
        this->remove_forward_by_channel(inv_chans[i]);
    }
}

bool ForwardPortWorker::remove_forward_by_channel(LIBSSH2_CHANNEL *pchan)
{
    int rn = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    QTcpSocket *sock = NULL;

    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;
    for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
        sock = it->first;
        chan = it->second;

        if (chan == pchan) {
            break;
        }
    }

    if (it == this->mfwds.end()) {
        qLogx()<<"Can not find for:"<<sender();
    } else {
        qLogx()<<"remove connection :"<<sender()<<chan<<libssh2_channel_get_exit_status(chan);
        this->mfwds.erase(it);

        rn = libssh2_channel_close(chan);
        rn = libssh2_channel_free(chan);

        QObject::disconnect(sock);
        // QObject::disconnect(sock, SIGNAL(connected()), this, SLOT(slot_forward_dest_connected()));
        // QObject::disconnect(sock, SIGNAL(disconnected()), this, SLOT(slot_forward_dest_disconnected()));
        // QObject::disconnect(sock, SIGNAL(readyRead()), this, SLOT(slot_forward_dest_socket_ready_read()));     

        sock->close();
        delete sock;
    }

    return true;
}

/////////////
void ForwardPortWorker::slot_forward_dest_connected()
{
    qLogx()<<"";
}

void ForwardPortWorker::slot_forward_dest_disconnected()
{
    qLogx()<<"";

    int rn = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    QTcpSocket *sock = NULL;

    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;
    for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
        sock = it->first;
        chan = it->second;

        if (sock == sender()) {
            break;
        }
    }

    if (it == this->mfwds.end()) {
        qLogx()<<"Can not find for:"<<sender();
    } else {
        qLogx()<<"remove connection :"<<sender()<<chan<<libssh2_channel_get_exit_status(chan);
        this->mfwds.erase(it);

        rn = libssh2_channel_close(chan);
        rn = libssh2_channel_free(chan);
        
        sender()->deleteLater();
    }
}

void ForwardPortWorker::slot_forward_dest_socket_ready_read()
{
    qLogx()<<"";
    ssize_t wlen = 0;
    LIBSSH2_CHANNEL *chan = NULL;
    char rbuf[1000] = {0};
    QTcpSocket *sock = static_cast<QTcpSocket*>(sender());
    
    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> >::iterator it;

    for (it = this->mfwds.begin(); it != this->mfwds.end(); it++) {
        if (it->first == sock) {
            chan = it->second;
            break;
        }
    }
    if (chan == NULL) {
        Q_ASSERT(chan != NULL);
    }

    ////////
    QByteArray ba = sock->readAll();
    wlen = libssh2_channel_write(chan, ba.data(), ba.length());
    qLogx()<<"sock -> ssh, wlen:"<<wlen<<ba.length();
}

void ForwardPortWorker::slot_forward_dest_socket_error(QAbstractSocket::SocketError socketError)
{
    qLogx()<<socketError;
}


