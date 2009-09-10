// libftp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-08 00:55:42 +0800
// Version: $Id$
// 

#include <assert.h>

#include "libftp.h"

LibFtp::LibFtp(QObject *parent)
    : QObject(parent)
{
}
LibFtp::~LibFtp()
{
}
int LibFtp::connect(const QString host, short port // = 21
            )
{
    QByteArray ba;
    QString replyText;
    this->qsock = new QTcpSocket();
    this->qsock->connectToHost(host, port);
    if (this->qsock->waitForConnected()) {
        qDebug()<<"ftp ctrl connect ok";
        ba = this->readAll(this->qsock);
        replyText = ba;
        // replyText maybe contains welcome text
        QStringList sl = replyText.trimmed().split("\n");
        sl = sl.at(sl.count() - 1).split(" ");
        assert(sl.at(0) == "220");
        this->servBanner.clear();
        for (int i = 1 ; i < sl.count(); i++) {
            if (sl.at(i).startsWith("(") || sl.at(i).startsWith("[")) {
                break;
            }
            this->servBanner += sl.at(i) + " ";
        }
        qDebug()<<"Got server banner:"<<this->servBanner;
    } else {
        qDebug()<<this->qsock->errorString();
        return -1; // Connection::CONN_OTHER;
    }

    return 0;
}
int LibFtp::login(const QString &user, const QString &password) 
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

int LibFtp::logout()
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("QUIT\r\n");

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "221");
        return 0;
	}

	return -1;
}

int LibFtp::connectDataChannel()
{
    this->qdsock = new QTcpSocket();
    this->qdsock->connectToHost(this->pasvHost, pasvPort);
    if (this->qdsock->waitForConnected()) {
        qDebug()<<"ftp data connect ok";
    } else {
        qDebug()<<this->qdsock->errorString();
        return -1; // Connection::CONN_OTHER;
    }

    return 0;    
}

static void _q_fixupDateTime(QDateTime *dateTime)
{
    // Adjust for future tolerance.
    const int futureTolerance = 86400;
    if (dateTime->secsTo(QDateTime::currentDateTime()) < -futureTolerance) {
        QDate d = dateTime->date();
        d.setYMD(d.year() - 1, d.month(), d.day());
        dateTime->setDate(d);
    }
}

static void _q_parseUnixDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
    // Unix style, 7 + 1 entries
    // -rw-r--r--    1 ftp      ftp      17358091 Aug 10  2004 qt-x11-free-3.3.3.tar.gz
    // drwxr-xr-x    3 ftp      ftp          4096 Apr 14  2000 compiled-examples
    // lrwxrwxrwx    1 ftp      ftp             9 Oct 29  2005 qtscape -> qtmozilla
    if (tokens.size() != 8)
        return;

    char first = tokens.at(1).at(0).toLatin1();
    if (first == 'd') {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(false);
    } else if (first == '-') {
        info->setDir(false);
        info->setFile(true);
        info->setSymLink(false);
    } else if (first == 'l') {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(true);
    }

    // Resolve filename
    QString name = tokens.at(7);
    if (info->isSymLink()) {
        int linkPos = name.indexOf(QLatin1String(" ->"));
        if (linkPos != -1)
            name.resize(linkPos);
    }
    info->setName(name);

    // Resolve owner & group
    info->setOwner(tokens.at(3));
    info->setGroup(tokens.at(4));

    // Resolve size
    info->setSize(tokens.at(5).toLongLong());

    QStringList formats;
    formats << QLatin1String("MMM dd  yyyy") << QLatin1String("MMM dd hh:mm") << QLatin1String("MMM  d  yyyy")
            << QLatin1String("MMM  d hh:mm") << QLatin1String("MMM  d yyyy") << QLatin1String("MMM dd yyyy");

    QString dateString = tokens.at(6);
    dateString[0] = dateString[0].toUpper();

    // Resolve the modification date by parsing all possible formats
    QDateTime dateTime;
    int n = 0;
#ifndef QT_NO_DATESTRING
    do {
        dateTime = QLocale::c().toDateTime(dateString, formats.at(n++));
    }  while (n < formats.size() && (!dateTime.isValid()));
#endif

    if (n == 2 || n == 4) {
        // Guess the year.
        dateTime.setDate(QDate(QDate::currentDate().year(),
                               dateTime.date().month(),
                               dateTime.date().day()));
        _q_fixupDateTime(&dateTime);
    }
    if (dateTime.isValid())
        info->setLastModified(dateTime);

    // Resolve permissions
    int permissions = 0;
    QString p = tokens.at(2);
    permissions |= (p[0] == QLatin1Char('r') ? QUrlInfo::ReadOwner : 0);
    permissions |= (p[1] == QLatin1Char('w') ? QUrlInfo::WriteOwner : 0);
    permissions |= (p[2] == QLatin1Char('x') ? QUrlInfo::ExeOwner : 0);
    permissions |= (p[3] == QLatin1Char('r') ? QUrlInfo::ReadGroup : 0);
    permissions |= (p[4] == QLatin1Char('w') ? QUrlInfo::WriteGroup : 0);
    permissions |= (p[5] == QLatin1Char('x') ? QUrlInfo::ExeGroup : 0);
    permissions |= (p[6] == QLatin1Char('r') ? QUrlInfo::ReadOther : 0);
    permissions |= (p[7] == QLatin1Char('w') ? QUrlInfo::WriteOther : 0);
    permissions |= (p[8] == QLatin1Char('x') ? QUrlInfo::ExeOther : 0);
    info->setPermissions(permissions);

    bool isOwner = info->owner() == userName;
    info->setReadable((permissions & QUrlInfo::ReadOther) || ((permissions & QUrlInfo::ReadOwner) && isOwner));
    info->setWritable((permissions & QUrlInfo::WriteOther) || ((permissions & QUrlInfo::WriteOwner) && isOwner));
}

static void _q_parseDosDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
    // DOS style, 3 + 1 entries
    // 01-16-02  11:14AM       <DIR>          epsgroup
    // 06-05-03  03:19PM                 1973 readme.txt
    if (tokens.size() != 4)
        return;

    Q_UNUSED(userName);

    QString name = tokens.at(3);
    info->setName(name);
    info->setSymLink(name.toLower().endsWith(QLatin1String(".lnk")));

    if (tokens.at(2) == QLatin1String("<DIR>")) {
        info->setFile(false);
        info->setDir(true);
    } else {
        info->setFile(true);
        info->setDir(false);
        info->setSize(tokens.at(2).toLongLong());
    }

    // Note: We cannot use QFileInfo; permissions are for the server-side
    // machine, and QFileInfo's behavior depends on the local platform.
    int permissions = QUrlInfo::ReadOwner | QUrlInfo::WriteOwner
                      | QUrlInfo::ReadGroup | QUrlInfo::WriteGroup
                      | QUrlInfo::ReadOther | QUrlInfo::WriteOther;
    QString ext;
    int extIndex = name.lastIndexOf(QLatin1Char('.'));
    if (extIndex != -1)
        ext = name.mid(extIndex + 1);
    if (ext == QLatin1String("exe") || ext == QLatin1String("bat") || ext == QLatin1String("com"))
        permissions |= QUrlInfo::ExeOwner | QUrlInfo::ExeGroup | QUrlInfo::ExeOther;
    info->setPermissions(permissions);

    info->setReadable(true);
    info->setWritable(info->isFile());

    QDateTime dateTime;
#ifndef QT_NO_DATESTRING
    dateTime = QLocale::c().toDateTime(tokens.at(1), QLatin1String("MM-dd-yy  hh:mmAP"));
    if (dateTime.date().year() < 1971) {
        dateTime.setDate(QDate(dateTime.date().year() + 100,
                               dateTime.date().month(),
                               dateTime.date().day()));
    }
#endif

    info->setLastModified(dateTime);

}

bool LibFtp::parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info)
{
    if (buffer.isEmpty())
        return false;

    QString bufferStr = QString::fromLatin1(buffer).trimmed();

    // Unix style FTP servers
    QRegExp unixPattern(QLatin1String("^([\\-dl])([a-zA-Z\\-]{9,9})\\s+\\d+\\s+(\\S*)\\s+"
                                      "(\\S*)\\s+(\\d+)\\s+(\\S+\\s+\\S+\\s+\\S+)\\s+(\\S.*)"));
    if (unixPattern.indexIn(bufferStr) == 0) {
        _q_parseUnixDir(unixPattern.capturedTexts(), userName, info);
        return true;
    }

    // DOS style FTP servers
    QRegExp dosPattern(QLatin1String("^(\\d\\d-\\d\\d-\\d\\d\\ \\ \\d\\d:\\d\\d[AP]M)\\s+"
                                     "(<DIR>|\\d+)\\s+(\\S.*)$"));
    if (dosPattern.indexIn(bufferStr) == 0) {
        _q_parseDosDir(dosPattern.capturedTexts(), userName, info);
        return true;
    }

    // Unsupported
    return false;
}


int LibFtp::list(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    sigLog = this->readAll(this->qsock);
    // q_debug()<<sigLog;

    this->dirList.clear();
	cmd = QString("LIST %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;

        // ball = this->readAll(this->qdsock);
        // qDebug()<<ball;
        this->qdsock->waitForReadyRead();
        qDebug()<<this->qdsock->canReadLine();
        while (this->qdsock->canReadLine()) {
            QUrlInfo i;
            QByteArray line = this->qdsock->readLine();
            qDebug("QFtpDTP read (list): '%s'", line.constData());

            if (this->parseDir(line, QLatin1String(""), &i)) {
                // emit listInfo(i);
                this->dirList.append(i);
            } else {
                // some FTP servers don't return a 550 if the file or directory
                // does not exist, but rather write a text to the data socket
                // -- try to catch these cases
                if (line.endsWith("No such file or directory\r\n"))
                    // err = QString::fromLatin1(line);
                    qDebug()<<line;
            }
        }
        this->qdsock->close();
        delete this->qdsock;
        this->qdsock = NULL;

		ball = this->readAll(this->qsock);
		qDebug()<<ball;
	}

	return -1;        
    return 0;
}

int LibFtp::passive()
{
	QString cmd;
    QString sigLog;
    QString replyText;

    sigLog = this->readAll(this->qsock);
    // q_debug()<<sigLog;

	cmd = QString("PASV\r\n");

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QRegExp addrPortPattern(QLatin1String("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)"));
        if (addrPortPattern.indexIn(replyText) == -1) {
            qDebug("QFtp: bad 227 response -- address and port information missing");
            // this error should be reported
            assert(1 == 2);
        } else {
            QStringList lst = addrPortPattern.capturedTexts();
            QString host = lst[1] + QLatin1Char('.') + lst[2] + QLatin1Char('.') + lst[3] + QLatin1Char('.') + lst[4];
            quint16 port = (lst[5].toUInt() << 8) + lst[6].toUInt();
            this->pasvHost = host;
            this->pasvPort = port;
            qDebug()<<"PASV port:"<<host<<":"<<port;
            return 0;
        }
	}

	return -1;    
    return 0;
}
QVector<QUrlInfo> LibFtp::getDirList()
{
    return this->dirList;
}
QString LibFtp::getServerBanner()
{
    return this->servBanner;
}

int LibFtp::pwd(QString &path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("PWD\r\n");

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        path = sl.at(1).split("\"").at(1);
        qDebug()<<"the pwd is :"<<path;
        assert(sl.at(0) == "257");
        return 0;
	}

	return -1;    
}
int LibFtp::mkdir(const QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("MKD %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "257"); // 550 perm denied
        return 0;
	}

	return -1;
}
int LibFtp::rmdir(const QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("RMD %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "250"); // 550 no such file
        return 0;
	}

	return -1;
}

int LibFtp::chdir(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("CWD %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "250"); // 550 no such file
        return 0;
	}

	return -1;
}

int LibFtp::remove(const QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("DELE %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "250"); // 500 no such file
        return 0;
	}

	return -1;
}

int LibFtp::type(int type)
{
	QString cmd;
    QString sigLog;
    QString replyText;
    QString tname;
    
    switch (type) {
    case TYPE_ASCII:
        tname = "A";
        break;
    case TYPE_EBCID:
    case TYPE_BIN:
    case TYPE_IMAGE:
    case TYPE_LOCAL_BYTE:
    default:
        tname = "I";
        break;
    }
	cmd = QString("TYPE %1\r\n").arg(tname);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "200");
        return 0;
	}

	return -1;
}

int LibFtp::noop()
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("NOOP\r\n");

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "200");
        return 0;
	}

	return -1;
}

int LibFtp::system(QString &type)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("SYST\r\n");

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        assert(sl.at(0) == "215");
        type = sl.at(1);
        qDebug()<<"ftp system type: "<<type;
        return 0;
	}

	return -1;
}
int LibFtp::stat(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    this->dirList.clear();
	cmd = QString("STAT %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;

        QStringList sl = replyText.split("\n");
        // this->qsock->waitForReadyRead();
        // qDebug()<<this->qsock->canReadLine();
        // while (this->qsock->canReadLine()) { // why can not readLine the second line?
        for (int i = 0; i < sl.count(); i++) {
            QUrlInfo ui;
            // QByteArray line = this->qsock->readLine();
            QByteArray line = sl.at(i).toAscii();
            qDebug("QFtpDTP read (list): '%s'", line.constData());
            if (line.startsWith("211")) {
                // not data line
                continue;
            }
            if (line.startsWith("450")) {
                return -1;
            }
            if (this->parseDir(line, QLatin1String(""), &ui)) {
                // emit listInfo(i);
                this->dirList.append(ui);
            } else {
                // some FTP servers don't return a 550 if the file or directory
                // does not exist, but rather write a text to the data socket
                // -- try to catch these cases
                if (line.endsWith("No such file or directory\r\n"))
                    // err = QString::fromLatin1(line);
                    qDebug()<<line;
            }
        }
        return 0;
	}

	return -1;        
}

/// private
QByteArray LibFtp::readAll(QTcpSocket *sock)
{
	QByteArray ball;
	ball = this->readAllByEndSymbol(sock);
	return ball ;
}
QByteArray LibFtp::readAllByEndSymbol(QTcpSocket *sock)
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
