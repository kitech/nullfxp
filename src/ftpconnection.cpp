// ftpconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:35:47 +0800
// Version: $Id$
// 

#include "utils.h"

#include "ftpconnection.h"

FTPConnection::FTPConnection(QObject *parent)
    : Connection(parent)
{
}
FTPConnection::~FTPConnection()
{
    if (this->qsock != NULL) {
        delete this->qsock;
        this->qsock = NULL;
    }
    if (this->qdsock != NULL) {
        delete this->qdsock;
        this->qdsock = NULL;
    }
}

int FTPConnection::connect()
{
    this->qsock = new QTcpSocket();
    this->qsock->connectToHost(this->hostName, this->port);
    if (this->qsock->waitForConnected()) {
        q_debug()<<"ftp ctrl connect ok";
    } else {
        q_debug()<<this->qsock->errorString();
        return Connection::CONN_OTHER;
    }
    this->homePath = QString("/");

    // login
    int iret = this->login(this->userName, this->password);
    if (iret != 0) {
        return Connection::CONN_AUTH_ERROR;
    }
    
    return 0;
}
int FTPConnection::disconnect()
{
    return 0;
}
int FTPConnection::reconnect()
{
    return 0;
}
bool FTPConnection::isConnected()
{
    return Connection::isConnected();
}
bool FTPConnection::isRealConnected()
{
    return Connection::isRealConnected();
}

int FTPConnection::alivePing()
{
    return 0;
}

/////////// private
int FTPConnection::login(const QString &user, const QString &password) 
{
	qDebug()<<__FUNCTION__<<user<<password;
	QString cmd;
    QString sigLog;

    sigLog = this->readAll(this->qsock);
    // q_debug()<<sigLog;

	if (user.length() == 0) cmd = QString("USER %1\r\n").arg("ftp");
	else   cmd = QString("USER %1\r\n").arg(user);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;

		if (password.length() == 0)  cmd = QString("PASS %1\r\n").arg("ftp");
		else  cmd = QString("PASS %1\r\n").arg(password);

		this->qsock->write(cmd.toAscii());
		qDebug()<<cmd;
		if (this->qsock->waitForBytesWritten()) {
			ball = this->readAll(this->qsock);
			qDebug()<<ball ;		
			if (ball.length() > 0 ) {
				sigLog = QString(ball).split("\r\n").at(0);
				if (!sigLog.startsWith("230"))	//login error for password or ip limit 
					return -1 ;
				return 0;	//suppose succ,must be changed for correct checking
			}
		}
	}

	return -1;
}

QByteArray FTPConnection::readAll(QTcpSocket *sock)
{
	QByteArray ball;
	ball = this->readAllByEndSymbol(sock);
	return ball ;
}
QByteArray FTPConnection::readAllByEndSymbol(QTcpSocket *sock)
{
	qDebug()<<__FUNCTION__<<__LINE__;

	QByteArray ball ;
	QString sall ;

	long bavable ;
	//qDebug()<<__FUNCTION__<<__LINE__;
	bavable = sock->readBufferSize();
	//qDebug()<<__FUNCTION__<<__LINE__;
	sock->setReadBufferSize(1);
	// qDebug()<<__FUNCTION__<<__LINE__;
	while (sock->isOpen() && sock->waitForReadyRead(3000) )
	{		
		// qDebug()<<__FUNCTION__<<__LINE__;
		int rlen ;
		char buff[8] = {0} ;
		bool cmdend = false ;
		
		while (true) {
			//read n			
			rlen = sock->read(buff, 1);
			if (rlen != 1) break;
			sall += QString(buff);
			// qDebug()<<sall;
						
			if (sall.endsWith("\r\n")) {
				//qDebug()<<"\\r\\n found"<<sall.length() <<(sall.at(sall.trimmed().lastIndexOf("\r\n")+5));
				//qDebug() << sall.at(3);
				int lastindex = sall.trimmed().lastIndexOf("\r\n") ;
				//qDebug()<< sall.mid(lastindex + 2 ,3).toInt() ;
				if (sall.at(3) == QChar(' ')
					|| (sall.trimmed().lastIndexOf("\r\n") >=0 && sall.at(sall.trimmed().lastIndexOf("\r\n")+5) == QChar(' ')
					&& sall.mid(lastindex + 2, 3).toInt() > 0 &&  sall.mid(lastindex + 2, 3).toInt() < 1000))
				{
					qDebug()<<"cmd tail found :"<<(sall.at(sall.trimmed().lastIndexOf("\r\n")+5))<<sall;
					cmdend = true ;
					break ;	//cmd return end , we assume it right for all case .
				}
			}
		}
		if (cmdend == true) break;
		//qDebug()<<" cmdend == false :"<< sock->errorString() ;
		// this->msleep(10);
        // sleep(1);
	}
	
	//qDebug()<<sall.right(4)<<sock->errorString();
	sock->setReadBufferSize(bavable);
	ball = sall.toAscii();

	return ball;
}
