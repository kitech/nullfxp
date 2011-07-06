// forwardportworker.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-02 10:30:27 +0000
// Version: $Id$
// 

#ifndef _FORWARDPORTWORKER_H_
#define _FORWARDPORTWORKER_H_

#include <QtCore>
#include <QtNetwork>

#include "libssh2.h"

class ForwardPortWorker : public QThread 
{
    Q_OBJECT;
public:
    ForwardPortWorker(LIBSSH2_LISTENER *lsner, const QString &dest_host, int dest_ports, QObject *parent = 0);
    virtual ~ForwardPortWorker();

    virtual void run();

    void set_listener(LIBSSH2_LISTENER *lsner);
public slots:
    void slot_poll_timeout();
    ///////
    void slot_forward_dest_connected();
    void slot_forward_dest_disconnected();
    void slot_forward_dest_socket_ready_read();
    void slot_forward_dest_socket_error(QAbstractSocket::SocketError socketError);
    
protected:
    bool remove_forward_by_channel(LIBSSH2_CHANNEL *chan);

signals:
    void listen_channel_error(int eno);

private:
    LIBSSH2_LISTENER *mlsner;
    QTimer *msrvtimer;

    // TODO if only two part, use QBiHash is better.
    QVector<QPair<QTcpSocket*, LIBSSH2_CHANNEL*> > mfwds;

    // QTcpSocket *mlsnsock;
    QString dest_host;
    int dest_ports;

    /// 统计
    int mconn_count;
    int mrecv_len; // from network
    int msend_len; // from SSH channel
    QDateTime mctime; // 启动时间
    float mavg_speed;

    int mreconn_times;
    int mrelisten_times;
};

#endif /* _FORWARDPORTWORKER_H_ */
