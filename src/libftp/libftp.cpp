// libftp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-08 00:55:42 +0800
// Version: $Id$
// 

#include <assert.h>
#include <sys/stat.h>

#include "utils.h"
#include "libftp.h"

LibFtp::LibFtp(QObject *parent)
    : QObject(parent)
{
    this->qsock = NULL;
    this->qdsock = NULL;
    this->setEncoding("UTF-8");
    this->useTLS = false;

}
LibFtp::~LibFtp()
{
}
int LibFtp::connect(const QString host, unsigned short port)
{
    QByteArray ba;
    QString replyText;

    this->qsock = new QSslSocket();
    this->qsock->connectToHost(host, port);
    if (this->qsock->waitForConnected()) {
        qDebug()<<"ftp ctrl connect ok";
        ba = this->readAll(this->qsock);
        if (ba.isEmpty()) {
            this->errmsg = this->qsock->errorString();
            this->errnum = -1;
	    q_debug()<<"connect error:"<<this->errmsg;
            return -1;
        }
        replyText = ba;
        this->parseWelcome(replyText);
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
        this->errnum = -1;
        this->errmsg = this->qsock->errorString();
        return -1; // Connection::CONN_OTHER;
    }

    return 0;
}
int LibFtp::login(const QString &user, const QString &password) 
{
    qDebug()<<__FUNCTION__<<user<<password;
    QString cmd;
    QString sigLog;
    bool needPassword = true;

    assert(this->qsock->bytesAvailable() == 0);

    // AUTH SSL
    cmd = QString("AUTH TLS\r\n");
    this->qsock->write(cmd.toAscii());
    if (this->qsock->waitForBytesWritten()) {
        QByteArray ball;
        ball = this->readAll(this->qsock);
        qDebug()<<ball;

        QStringList sl = QString(ball).split("\n");
        if (sl.at(sl.count() - 2).split(" ").at(0) == "234") {
            // this->useTLS = 1;
            qDebug()<<"Server support FTP/TLS, using encrypt connection"<<this->qsock->peerName();
            // QList<QSslError> ignoreErrors;
            // ignoreErrors.append( QSslError::HostNameMismatch);
            // ignoreErrors.append( QSslError::SelfSignedCertificate);
            // ignoreErrors.append( QSslError::SelfSignedCertificateInChain);
            // ignoreErrors.append( QSslError::CertificateUntrusted);
            // ignoreErrors.append( QSslError::CertificateRejected);

            // this->qsock->ignoreSslErrors(ignoreErrors);
            this->qsock->ignoreSslErrors();
            this->qsock->setProtocol(QSsl::TlsV1);
            this->qsock->startClientEncryption();
            if (!this->qsock->waitForEncrypted()) {
                qDebug()<<"waitForEncrypted failed"<<this->qsock->errorString()<<this->qsock->isEncrypted();
                qDebug()<<this->qsock->sslErrors();
            }
        } else {
            // == 500 server 
            qDebug()<<"Server maybe can not support FTP/TLS";
        }
    }

    assert(this->qsock->bytesAvailable() == 0);

	if (user.length() == 0) {
        cmd = QString("USER %1\r\n").arg("ftp");
    } else {
        cmd = QString("USER %1\r\n").arg(user);
    }

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;

        // 对有些匿名服务器不需要输入密码的
        // 像ftp.gnu.org
        QStringList sl = QString(ball).split("\n");
        if (sl.at(sl.count() - 2).split(" ").at(0) == "230") {
            needPassword = false;
            // return 0;
        }

        if (needPassword) {
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
                    // return 0;	//suppose succ,must be changed for correct checking
                }
            }
        }
	}

    // get supportedCmds;
    this->getSupportedCmds();
    qDebug()<<this->supportedCmds;

    //    QString cmd;
    cmd = QString("PBSZ 0\r\n");
    this->qsock->write(cmd.toAscii());
    if (this->qsock->waitForBytesWritten()) {
        if (this->qsock->waitForReadyRead()) {
            QByteArray ball = this->qsock->readAll();
            qDebug()<<"PBSZ: "<<ball;
            
            QStringList sl = QString(ball).split("\n");
            if (sl.at(0).split(" ").at(0) == "200") {
                this->useTLS = true;
            } else {
                // some else: 421, 501, 502, 503, 530, 550
                this->useTLS = false;
                qDebug()<<__FUNCTION__<<__LINE__<<"server not support TLS";
            }
        }
    }
    
    if (this->useTLS) {
        cmd = QString("PROT P\r\n");
        this->qsock->write(cmd.toAscii());
        if (this->qsock->waitForBytesWritten()) {
            if (this->qsock->waitForReadyRead()) {
                QByteArray ball = this->qsock->readAll();
                qDebug()<<"PROT: "<<ball;
            }
        }
    }

    // cmd = QString("ADAT\r\n");
    // this->qsock->write(cmd.toAscii());
    // if (this->qsock->waitForBytesWritten()) {
    //     if (this->qsock->waitForReadyRead()) {
    //         QByteArray ball = this->qsock->readAll();
    //         qDebug()<<"ADAT: "<<ball;
    //     }
    // }

    return 0;

	return -1;
}

int LibFtp::getSupportedCmds()
{
    QString cmd;
    QString sigLog;
    QString replyText;
    QStringList cmds;    

    assert(this->qsock->bytesAvailable() == 0);

    cmd = QString("HELP\r\n");
    this->qsock->write(cmd.toAscii());
    qDebug()<<cmd;
    
    if (this->qsock->waitForBytesWritten()) {
        QByteArray ball;
        ball = this->readAll(this->qsock);
        qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");

        if (sl.at(sl.count() - 2).startsWith("214")) {
            // 214 Help OK.
            for (int i = 1; i < sl.count() - 1; ++i) {
                QStringList lcmds = sl.at(i).split(" ");
                for (int j = 0; j < lcmds.count(); ++j) {
                    if (!lcmds.at(j).trimmed().isEmpty()) {
                        cmds << lcmds.at(j).trimmed().toUpper();
                    }
                }
            }
        } else {

        }
    }

    // 1c photo, lzzm copy and orig, 6,17, 9:30

    Q_ASSERT(this->supportedCmds.count() == 0);
    this->supportedCmds.clear();
    for (int i = 0 ; i < cmds.count(); ++i) {
        this->supportedCmds.insert(cmds.at(i), true);
    }

    return cmds.count();
}

bool LibFtp::isCmdSupported(QString cmd)
{
    return this->supportedCmds.contains(cmd.toUpper());
}

int LibFtp::logout()
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "221") {
            return 0;
        }
        assert(sl.at(0) == "221");
	}

	return -1;
}

int LibFtp::connectDataChannel()
{
    // this->qdsock = new QTcpSocket();
    this->qdsock = new QSslSocket();
    int qtype1 = qRegisterMetaType<QAbstractSocket::SocketError>("SocketError" );
    int qtype2 = qRegisterMetaType<QAbstractSocket::SocketState>("SocketState" );
    QObject::connect(this->qdsock, SIGNAL(connected()),
                     this, SLOT(onDataSockConnected()));
    QObject::connect(this->qdsock, SIGNAL(disconnected()),
                     this, SLOT(onDataSockDisconnected()));
    QObject::connect(this->qdsock, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(onDataSockError(QAbstractSocket::SocketError)));
    QObject::connect(this->qdsock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                     this, SLOT(onDataSockStateChanged(QAbstractSocket::SocketState)));

    this->qdsock->connectToHost(this->pasvHost, pasvPort);
    if (this->qdsock->waitForConnected()) {
        qDebug()<<"ftp data connect ok";
    } else {
        qDebug()<<this->qdsock->errorString();
        return -1; // Connection::CONN_OTHER;
    }

    QString cmd;

    // cmd = QString("PBSZ 0\r\n");
    // this->qsock->write(cmd.toAscii());
    // if (this->qsock->waitForBytesWritten()) {
    //     if (this->qsock->waitForReadyRead()) {
    //         QByteArray ball = this->qsock->readAll();
    //         qDebug()<<"PBSZ: "<<ball;
    //     }
    // }
    
    // cmd = QString("PROT S\r\n");
    // this->qsock->write(cmd.toAscii());
    // if (this->qsock->waitForBytesWritten()) {
    //     if (this->qsock->waitForReadyRead()) {
    //         QByteArray ball = this->qsock->readAll();
    //         qDebug()<<"PROT: "<<ball;
    //     }
    // }

    qDebug()<<__FUNCTION__<<this->qsock->isEncrypted()<<this->qdsock->isEncrypted();
   // if (this->qsock->isEncrypted()) {
    // this->qdsock->ignoreSslErrors();
    // this->qdsock->startClientEncryption();
    // if (!this->qdsock->waitForEncrypted()) {
    //     qDebug()<<"data sock waitForEncrypted error:"<<this->qdsock->errorString();
    // }
   //  }
 
    return 0;    
}

int LibFtp::closeDataChannel()
{
    if (this->qdsock != NULL) {
        this->qdsock->close();
        delete this->qdsock;
        this->qdsock = NULL;
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


bool parseMLSTLine(const QByteArray &buffer, QUrlInfo *info)
{
    QList<QByteArray> fields = buffer.split(';');

    QHash<QByteArray, QByteArray> fieldHash;

    for (int i = 0 ; i < fields.count(); ++i) {
        QList<QByteArray> kv = fields.at(i).split('=');
        if (kv.count() == 2) {
            fieldHash[kv.at(0)] = kv.at(1);
        } else {
            fieldHash[QString::number(i).toAscii()] = kv.at(0);
        }
    }

    Q_ASSERT(info != NULL);
    if (fieldHash.value("type") == "file") {
        info->setFile(true);
        info->setDir(false);
        info->setSize(fieldHash.value("size").toLongLong());
    } else {
        info->setFile(false);
        info->setDir(true);
    }

// type=file;size=85701976;modify=20100411162529;UNIX.mode=0644;UNIX.uid=1001;UNIX.gid=100;unique=801g120871; //Desktop Pictures.tar.gz

    info->setName(fields.at(fields.count() - 1));
    info->setReadable(true);
    info->setWritable(info->isFile());

    QDateTime dateTime;
    dateTime = QDateTime::fromString(fieldHash.value("modify"), "yyyyMMddHHmmss");
    dateTime.addMSecs(fieldHash.value("modify").right(6).toLongLong());

    if (dateTime.date().year() < 1971) {
        dateTime.setDate(QDate(dateTime.date().year() + 100,
                               dateTime.date().month(),
                               dateTime.date().day()));
    }
    info->setLastModified(dateTime);

    int perm = 0;
    int umode = fieldHash.value("UNIX.mode").at(0) << 3
        | fieldHash.value("UNIX.mode").at(1) << 2
        | fieldHash.value("UNIX.mode").at(2) << 1
        | fieldHash.value("UNIX.mode").at(3);

    if (umode & S_IRUSR) {
        perm |= QUrlInfo::ReadOwner;
    }
    if (umode & S_IWUSR) {
        perm |= QUrlInfo::WriteOwner;
    }
    if (umode & S_IXUSR) {
        perm |= QUrlInfo::ExeOwner;
    }

    if (umode & S_IRGRP) {
        perm |= QUrlInfo::ReadGroup;
    }
    if (umode & S_IWGRP) {
        perm |= QFile::WriteGroup;
    }
    if (umode & S_IXGRP) {
        perm |= QUrlInfo::ExeGroup;
    }

    if (umode & S_IROTH) {
        perm |= QUrlInfo::ReadOther;
    }
    if (umode & S_IWOTH) {
        perm |= QUrlInfo::WriteOther;
    }
    if (umode & S_IXOTH) {
        perm |= QUrlInfo::ExeOther;
    }
    info->setPermissions(perm);


    return true;
}

int LibFtp::list(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);
    // sigLog = this->readAll(this->qsock);
    // q_debug()<<sigLog;

    // TODO when path has space, list match 0
    // there is a new cmd MLST and MLSD, try it.
    this->dirList.clear();
	cmd = QString("LIST -a -l %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");
        sl = sl.at(sl.count() - 2).split(" ");
        assert(sl.at(0) == "150");

        if (this->useTLS) {
            qDebug()<<"Before data socket startClientEncryption";
            this->qdsock->ignoreSslErrors();
            this->qdsock->startClientEncryption();
            if (!this->qdsock->waitForEncrypted()) {
                qDebug()<<this->qdsock->errorString();
            }
            qDebug()<<"end data socket startClientEncryption";
        }

        // ball = this->readAll(this->qdsock);
        // qDebug()<<ball;
        this->qdsock->waitForReadyRead();
        qDebug()<<__FUNCTION__<<this->qdsock->canReadLine();
        while (this->qdsock->canReadLine()) {
            QUrlInfo i;
            QByteArray line = this->qdsock->readLine();
            qDebug("QFtpDTP read (list): '%s'", line.constData());

            if (this->parseDir(line, QLatin1String(""), &i)) {
                // emit listInfo(i);
                // 转换文件名编码
                i.setName(this->codec->toUnicode(i.name().toAscii()));
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
        replyText = ball;
        sl = replyText.split("\n");
        sl = sl.at(sl.count() - 2).split(" ");
        if (sl.at(0) == "226") {
            return 0;
        }
        assert(sl.at(0) == "226");
	}

	return -1;        
    return 0;
}

int LibFtp::lista(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);
    // sigLog = this->readAll(this->qsock);
    // q_debug()<<sigLog;

    this->dirList.clear();
	cmd = QString("LIST -a %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");
        sl = sl.at(sl.count() - 2).split(" ");
        assert(sl.at(0) == "150");

        if (this->useTLS) {
            // call this after any data retr, such as LIST*/RETR/STOR...
            qDebug()<<"Before data socket startClientEncryption";
            this->qdsock->ignoreSslErrors();
            this->qdsock->startClientEncryption();
            if (!this->qdsock->waitForEncrypted()) {
                qDebug()<<this->qdsock->errorString();
            }
            qDebug()<<"end data socket startClientEncryption";
        }

        // ball = this->readAll(this->qdsock);
        // qDebug()<<ball;
        this->qdsock->waitForReadyRead();
        qDebug()<<__FUNCTION__<<this->qdsock->canReadLine();
        while (this->qdsock->canReadLine()) {
            QUrlInfo i;
            QByteArray line = this->qdsock->readLine();
            qDebug("LibFtpDTP read (list): '%s'", line.constData());

            if (this->parseDir(line, QLatin1String(""), &i)) {
                // emit listInfo(i);
                // 转换文件名编码
                i.setName(this->codec->toUnicode(i.name().toAscii()));
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
        if (!ball.isEmpty()) {
            replyText = ball;
            sl = replyText.split("\n");
            if (sl.count() > 0) {
                sl = sl.at(sl.count() - 2).split(" ");
                if (sl.at(0) == "226") {
                    return 0;
                }
                assert(sl.at(0) == "226");
            }
        }
	}

	return -1;
}

int LibFtp::mlst(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

    if (!this->supportedCmds.contains("MLST")) {
        this->passive();
        this->connectDataChannel();
        return this->lista(path);
    }

    // TODO when path has space, list match 0
    this->dirList.clear();
    cmd = QString("MLST %1\r\n").arg(path);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");
        sl = sl.at(sl.count() - 2).split(" ");
        assert(sl.at(0) == "250");
        //  type=file;size=85701976;modify=20100411162529;UNIX.mode=0644;UNIX.uid=1001;UNIX.gid=100;unique=801g120871; //Desktop Pictures.tar.gz
        QList<QByteArray> inforows = ball.split('\n');
        qDebug()<<__FUNCTION__<<inforows.count();
        for (int i = 0; i < inforows.count(); ++i) {
            qDebug()<<"line: "<<i<<inforows.at(i).trimmed();
        }
        for (int i = 1 ; i < inforows.count() - 2; ++i) {
            QUrlInfo info;
            parseMLSTLine(inforows.at(i), &info);
            this->dirList.append(info);
            qDebug()<<"line: "<<inforows.at(i);
        }
        qDebug()<<__FUNCTION__<<__LINE__<<this->dirList.count();
        if (sl.at(0) == "250") {
            return 0;
        } else {
            qDebug()<<__FUNCTION__<<"response error:";
        }

        // qDebug()<<"Before data socket startClientEncryption";
        // this->qdsock->ignoreSslErrors();
        // this->qdsock->startClientEncryption();
        // if (!this->qdsock->waitForEncrypted()) {
        //     qDebug()<<this->qdsock->errorString();
        // }
        // qDebug()<<"end data socket startClientEncryption";

        // // ball = this->readAll(this->qdsock);
        // // qDebug()<<ball;
        // this->qdsock->waitForReadyRead();
        // qDebug()<<__FUNCTION__<<this->qdsock->canReadLine();
        // while (this->qdsock->canReadLine()) {
        //     QUrlInfo i;
        //     QByteArray line = this->qdsock->readLine();
        //     qDebug("QFtpDTP read (list): '%s'", line.constData());

        //     if (this->parseDir(line, QLatin1String(""), &i)) {
        //         // emit listInfo(i);
        //         // 转换文件名编码
        //         i.setName(this->codec->toUnicode(i.name().toAscii()));
        //         this->dirList.append(i);
        //     } else {
        //         // some FTP servers don't return a 550 if the file or directory
        //         // does not exist, but rather write a text to the data socket
        //         // -- try to catch these cases
        //         if (line.endsWith("No such file or directory\r\n"))
        //             // err = QString::fromLatin1(line);
        //             qDebug()<<line;
        //     }
        // }
        // this->qdsock->close();
        // delete this->qdsock;
        // this->qdsock = NULL;

		// ball = this->readAll(this->qsock);
		// qDebug()<<ball;
        // replyText = ball;
        // sl = replyText.split("\n");
        // sl = sl.at(sl.count() - 2).split(" ");
        // if (sl.at(0) == "226") {
        //     return 0;
        // }
        // assert(sl.at(0) == "226");
	}

	return -1;        
    return 0;
}

int LibFtp::passive()
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);
    // sigLog = this->readAll(this->qsock);
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
            qDebug("LibFtp: bad 227 response -- address and port information missing");
            // this error should be reported
            // assert(1 == 2);
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

QString LibFtp::getServerWelcome()
{
    return this->welcomeText;
}

int LibFtp::pwd(QString &path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "257") {
            return 0;
        }
        assert(sl.at(0) == "257");
	}

	return -1;    
}
int LibFtp::mkdir(const QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "257") {
            return 0;
        }
        assert(sl.at(0) == "257"); // 550 perm denied
	}

	return -1;
}
int LibFtp::rmdir(const QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "250") {
            return 0;
        } else {
            qDebug()<<"cmd faild: "<<replyText.trimmed();
            assert(sl.at(0) == "250"); // 550 no such file
            // 550 Can't remove directory: Directory not empty
        }
	}

	return -1;
}

int LibFtp::chdir(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("CWD %1\r\n").arg(path);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");
        sl = sl.at(sl.count() - 2).split(" ");
        if (sl.at(0) == "250") {
            return 0;
        }
        // assert(sl.at(0) == "250"); // 550 no such file
        // 550 Failed to change directory.
	}

	return -1;
}

// 在调用这个方法前，调用端需要正确设置本地当前目录，服务器当前目录
// 否则会出现找不到目录的情况
int LibFtp::put(const QString fileName)
{
	QString cmd, encodeCMD;
    QString sigLog;
    QString replyText;

	cmd = QString("STOR %1\r\n").arg(fileName);
    encodeCMD = this->codec->fromUnicode(cmd);
	this->qsock->write(encodeCMD.toAscii());
	qDebug()<<cmd<<encodeCMD;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
        ball = this->readAll(this->qsock);
        qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        this->setError(150, replyText);
        // assert(sl.at(0) == "150");
        // 550 no such file 
        // 553 Disk full - please upload later
        // 550 Permission denied.
        if (sl.at(0) == "150") {
            if (this->useTLS) {
                // call this after any data retr, such as LIST*/RETR/STOR...
                qDebug()<<"Before data socket startClientEncryption";
                this->qdsock->ignoreSslErrors();
                this->qdsock->startClientEncryption();
                if (!this->qdsock->waitForEncrypted()) {
                    qDebug()<<this->qdsock->errorString();
                }
                qDebug()<<"end data socket startClientEncryption";
            }
            return 0;
        } else {
            
        }
	}
    
    return -1;
}

int LibFtp::putNoWaitResponse(const QString fileName)
{
	QString cmd;
    QString sigLog;
    QString replyText;

	cmd = QString("STOR %1\r\n").arg(fileName);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
        return 0;
	}
    
    return -1;
}

int LibFtp::get(const QString fileName)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("RETR %1\r\n").arg(fileName);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
        ball = this->readAll(this->qsock);
        qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split("\n");
        sl = sl.at(sl.count()-2).split(" "); // fix pure-ftpd multi line response error.
        if (sl.at(0) == "150") {
            return 0;
        }
        assert(sl.at(0) == "150"); // 550 no such file
	}
    
    return -1;    
}
int LibFtp::getNoWaitResponse(const QString fileName)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("RETR %1\r\n").arg(fileName);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
        return 0;
	}
    
    return -1;    
}

int LibFtp::remove(const QString path)
{
	QString cmd, encodeCMD;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("DELE %1\r\n").arg(path);
    encodeCMD = this->codec->fromUnicode(cmd);
	this->qsock->write(encodeCMD.toAscii());
	qDebug()<<cmd<<encodeCMD;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        // assert(sl.at(0) == "250"); // 500 no such file
        // 550 Prohibited file name
        if (sl.at(0) == "250") {
            return 0;
        } else {
            qDebug()<<"cmd faild: "<<replyText.trimmed();
        }
	}

	return -1;
}

int LibFtp::rename(const QString src, const QString dest)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("RNFR %1\r\n").arg(src);

	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        if (sl.at(0) != "350") {
            assert(sl.at(0) == "350"); // 350 350 File or directory exists, ready for destination name
            return -1;
        }

        cmd = QString("RNTO %1\r\n").arg(dest);
        this->qsock->write(cmd.toAscii());
        qDebug()<<cmd;
	
        if (this->qsock->waitForBytesWritten()) {
            QByteArray ball;
            //read response
            ball = this->readAll(this->qsock);
            qDebug()<<ball;
            replyText = ball;
            QStringList sl = replyText.split(" ");
            if (sl.at(0) == "250") {
                return 0;
            }
            assert(sl.at(0) == "250"); // 500 no such file
        }
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

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "200") {
            return 0;
        }
        assert(sl.at(0) == "200");
	}

	return -1;
}

int LibFtp::noop()
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "200") {
            return 0;
        }
        assert(sl.at(0) == "200");
	}

	return -1;
}

int LibFtp::system(QString &type)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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
        if (sl.at(0) == "215") {
            type = sl.at(1);
            qDebug()<<"ftp system type: "<<type;
            return 0;    
        }
        assert(sl.at(0) == "215");
	}

	return -1;
}
int LibFtp::stat(QString path)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

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

int LibFtp::port(const QString hostip, const unsigned short port) // fxp
{
    QString hn = hostip;
    hn = hn.replace(".", ",");
    quint8 p1 = port >> 8 & 0x00FF;
    quint8 p2 = port & 0x00FF;

	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("PORT %1,%2,%3\r\n").arg(hn).arg(p1).arg(p2);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        // assert(sl.at(0) == "200"); // maybe 500 Illegal PORT command
        // pure-ftpd: 501 Sorry, but I won't connect to ports < 1024
        if (sl.at(0) == "200") {            
            return 0;
        }
	}


    return -1;
}
int LibFtp::portNoWaitResponse(const QString hostip, const unsigned short port)
{
    QString hn = hostip;
    hn = hn.replace(".", ",");
    quint8 p1 = port >> 8 & 0x00FF;
    quint8 p2 = port & 0x00FF;

	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("PORT %1,%2,%3\r\n").arg(hn).arg(p1).arg(p2);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
        return 0;
	}

    return -1;
}
int LibFtp::size(QString path, quint64 &siz)
{
	QString cmd;
    QString sigLog;
    QString replyText;

    assert(this->qsock->bytesAvailable() == 0);

	cmd = QString("SIZE %1\r\n").arg(path);
	this->qsock->write(cmd.toAscii());
	qDebug()<<cmd;
	
	if (this->qsock->waitForBytesWritten()) {
		QByteArray ball;
		//read response
		ball = this->readAll(this->qsock);
		qDebug()<<ball;
        replyText = ball;
        QStringList sl = replyText.split(" ");
        if (sl.at(0) == "213") {
            siz = sl.at(1).trimmed().toULongLong();
            return 0;
        }
        assert(sl.at(0) == "213");
	}

	return -1;
}

QTcpSocket *LibFtp::getDataSocket()
{
    return this->qdsock;
}

unsigned short LibFtp::pasvPeer(QString &hostip) // get passive peer ip
{
    hostip = this->pasvHost;
    return this->pasvPort;
}

int LibFtp::swallowResponse()
{
    QByteArray ba;

    ba = this->readAll(this->qsock);
    qDebug()<<ba;
    QStringList sl = QString(ba).split('\n');
    sl = sl.at(sl.count() - 1).split(" ");
    
    return sl.at(0).toInt();
    return 0;
}

int LibFtp::waitForCtrlResponse()
{
    assert(this->qsock != NULL);
    bool bret = this->qsock->waitForReadyRead(-1);
    qDebug()<<__FILE__<<__LINE__<<bret;
    if (bret) {
        QByteArray ba = this->qsock->readAll();
        qDebug()<<ba;
        return 0;
    }
    return -1;
}

QString LibFtp::errorString()
{
    if (this->errmsg.isEmpty()) {
        return this->qsock->errorString();
    }
    return this->errmsg;
}

/// private
void LibFtp::setError(int okno, QString msg)
{
    if (msg.isEmpty()) {
        return;
    }
    QStringList sl = msg.split(" ");
    if (okno == sl.at(0).toInt()) {
        this->errnum = 0;
        this->errmsg = "Sucess.";
    } else {
        switch (sl.at(0).toInt()) {
        case 0:
            qDebug()<<__FILE__<<__LINE__<<"No response.";
            this->errnum = 600;
            this->errmsg = "No response.";
            break;
        default:
            this->errnum = sl.at(0).toInt();
            this->errmsg = msg.trimmed();
            break;
        };
    }
}
int LibFtp::setEncoding(QString encoding)
{
    this->encoding = encoding;
    QTextCodec *c = QTextCodec::codecForName(encoding.toAscii());
    if (c == NULL) {
        qDebug()<<"Seting Unsupported encoding:"<<encoding;
    } else {
        this->codec = c;
    }
    return 0;
}

bool LibFtp::isSupportTLS()
{
    return this->useTLS;
}

QByteArray LibFtp::readAll(QTcpSocket *sock)
{
	QByteArray ball;
	ball = this->readAllByEndSymbol(sock);
	return ball ;
}
QByteArray LibFtp::readAllByEndSymbol(QTcpSocket *sock)
{
	qDebug()<<__FUNCTION__<<__LINE__;

	QByteArray ball;
	QString sall;

	long bavable;
    int maxRetry = 3;
    int retryTimes = 0;
	//qDebug()<<__FUNCTION__<<__LINE__;
	bavable = sock->readBufferSize();
	//qDebug()<<__FUNCTION__<<__LINE__;
	sock->setReadBufferSize(1);
	// qDebug()<<__FUNCTION__<<__LINE__;

 lableRetryRead:
	while (sock->isOpen() && sock->waitForReadyRead(6000)) {
		// qDebug()<<__FUNCTION__<<__LINE__;
		int rlen;
		char buff[8] = {0};
		bool cmdend = false;
		
		while (true) {
			//read n			
			rlen = sock->read(buff, 1);
			if (rlen != 1) break;
			sall += QString(buff);
            // qDebug()<<sall;
						
			if (sall.endsWith("\r\n")) {
				//qDebug()<<"\\r\\n found"<<sall.length() <<(sall.at(sall.trimmed().lastIndexOf("\r\n")+5));
				// qDebug() << sall.at(3);
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
	
    if (sall.length() == 0) {
        if (retryTimes > maxRetry) {

        } else {
            retryTimes ++;
            goto lableRetryRead;
        }
    }
	//qDebug()<<sall.right(4)<<sock->errorString();
	sock->setReadBufferSize(bavable);
	ball = sall.toAscii();

	return ball;
}

void LibFtp::parseWelcome(QString text)
{
    this->welcomeText = text;
}

void LibFtp::onDataSockConnected()
{
    qDebug()<<__FUNCTION__<<__LINE__;
}

void LibFtp::onDataSockDisconnected()
{
    qDebug()<<__FUNCTION__<<__LINE__;
}
void LibFtp::onDataSockError(QAbstractSocket::SocketError socketError)
{
    qDebug()<<__FUNCTION__<<__LINE__<<socketError;
    
}
void LibFtp::onDataSockStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug()<<__FUNCTION__<<__LINE__<<socketState;
}
