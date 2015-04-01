#include "httpmanager.h"
#include <QHttpPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkConfiguration>
#include <QXmlSimpleReader>
#include <QXmlInputSource>

#include <QXmlStreamWriter>
#include <QThread>
#include <iterator>
//#include <error.h>

HttpManager::HttpManager()
{
}


HttpManager::~HttpManager()
{

}

int HttpManager::init()
{
    QUrl bubbleUrl = QUrl("http://192.168.178.34:58645"); //TODO get from glimpse
    QUrl mediatombUrl = QUrl("http://172.16.172.1:49153/");
    QUrl myUrl = (QUrl("http://192.168.178.24:49152"));
//    QString getPath = "/dev/51ff7eac-45dc-bdff-ffff-ffffabac1331/desc.xml";
//    QString eventPath = "/dev/51ff7eac-45dc-bdff-ffff-ffffabac1331/svc/upnp-org/ContentDirectory/event";
//    QString actionPath = "/dev/51ff7eac-45dc-bdff-ffff-ffffabac1331/svc/upnp-org/ContentDirectory/action";
    QString getPath = "/description.xml";
    QString eventPath = "/upnp/event/cds";
    QString actionPath = "/upnp/control/cds";
    setMyUrl(myUrl);
//    setRemoteUrl(bubbleUrl);
    setRemoteUrl(mediatombUrl);
    QUrl getUrl, aUrl, sUrl, bUrl;
    getUrl = bUrl = sUrl = aUrl = m_remoteUrl;
    getUrl.setPath(getPath);
    setGETUrl(getUrl);
    bUrl.setPath(actionPath);
    sUrl.setPath(eventPath);
    setActionUrl(bUrl);
    setSubscribeUrl(sUrl);
    setFile("/home/simon/code/QtHttp/xmlInfoFromVlc.xml");
    m_answerFromServer = "";

    m_parser = new Parser();
    setNetworkAccessManager(new QNetworkAccessManager());
    startGet();
    connect(m_GETReply, SIGNAL(readyRead()),this, SLOT(GETreadyRead()));
    connect(m_GETReply, SIGNAL(finished()), this, SLOT(GETFinished())); //Note: finished() signal comes after readyRead()
    connect(this, SIGNAL(xmlParsed()), this, SLOT(subscribe()));
    connect(this, SIGNAL(subscribed()), this, SLOT(startAction()));
    connect(this, SIGNAL(readyToParse(QByteArray ba)), m_parser, SLOT(parseAnswer(QByteArray ba)));
    return 0;
}

QNetworkAccessManager * HttpManager::networkAccessManager() const
{
    return m_networkAccessManager;
}

void HttpManager::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    m_networkAccessManager = networkAccessManager;
}

void HttpManager::GETFinished()
{
    qDebug() << "Get finished";
}

void HttpManager::GETreadyRead()
{
    QByteArray content = m_GETReply->readAll();
    m_xmlByteArray->append(content);
    //qDebug() << "Reply:" + *m_xmlByteArray + "EOF";
    /* comes in multiple packets */
    m_parser->parseXML(*m_xmlByteArray);
}

void HttpManager::subscribe()
{
    /* Now subscribe */
    m_subscribeRequest.setUrl(m_subscribeUrl);
    QByteArray val = ("</>");
    val.insert(1, m_myUrl.toString());
    m_subscribeRequest.setRawHeader(QByteArray("CALLBACK"), val); //TODO flexible
    m_subscribeRequest.setRawHeader(QByteArray("NT"), ("upnp:event"));
    m_subscribeReply = m_networkAccessManager->sendCustomRequest(m_subscribeRequest, "SUBSCRIBE"); //TODO answer?
    emit subscribed();
}

void HttpManager::browse()
{

    QByteArray deviceFullName = "\"urn:schemas-upnp-org:service:ContentDirectory:1";
    QByteArray key = "SOAPACTION";
    QByteArray action = "#Browse\""; // quotes needed!
    deviceFullName.append(action);
    m_browseRequest.setUrl(m_actionUrl);
    m_browseRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"utf-8\"");
    m_browseRequest.setRawHeader(key, deviceFullName);
    m_browseRequest.setRawHeader(QByteArray("USER-AGENT"), QByteArray("Linux/3.13.0-24-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.17"));
    QNetworkConfiguration conf = m_networkAccessManager->configuration();
    m_browseRequest.setRawHeader("Accept-Language", QByteArray());
    //    m_browseReply = m_networkAccessManager->post(m_browseRequest, xmlRaw);
}

void HttpManager::sendRequest()
{
    //we can only download, if this thread sucessfully established the
    //TCP connection (all threads will receive the signal)
    if(m_socket->state() != QAbstractSocket::ConnectedState)
    {
        tStatus = FinishedError;
        //emit firstByteReceived(false);
        //TODO error
        return;
    }

    if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Load XML File Problem";
        exit(-1);
    }
    QByteArray xmlRaw = m_file->readAll();
    QString data(xmlRaw);
    int dataLength = data.length();
    QString url = m_remoteUrl.toString();
    url.remove("http://");

    QString header = QString("POST %1 HTTP/1.1\r\n"
                              "HOST: %2\r\n"
                              "CONTENT-LENGTH: %3\r\n" //TODO get it automatic
                              "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
                              "SOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n"
                              "USER-AGENT: Linux/3.13.0-24-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.17\r\n\r\n").arg(m_actionUrl.path()).arg(url).arg(dataLength);
    int bytesWritten = 0;

    //send the SOAP request -> HTTP POST and then the xml
    while(bytesWritten < header.length())
    {
        bytesWritten += m_socket->write(header.mid(bytesWritten).toLatin1());

        //on error
        if(bytesWritten < 0)
        {
            m_socket->close();
            tStatus = FinishedError;
            emit firstByteReceived(false);
            return;
        }
    }
    bytesWritten = 0;
    while(bytesWritten < data.length())
    {
        bytesWritten += m_socket->write(data.toLatin1());

        //on error
        if(bytesWritten < 0)
        {
            m_socket->close();
            tStatus = FinishedError;
            emit firstByteReceived(false);
            return;
        }
    }
    tStatus = AwaitingFirstByte;

    //do a blocking read for the first bytes or time-out if nothing is arriving
    if (m_socket->waitForReadyRead(firstByteReceivedTimeout))
    {
        tStatus = DownloadInProgress;
        //connect readyRead of the socket with read() for further reads
        connect(m_socket, &QTcpSocket::readyRead, this, &HttpManager::read);
        //call read
        read();
        emit firstByteReceived(true);
    }
    else
    {
        tStatus = FinishedError;
        QString s = m_socket->errorString();
        m_socket->close();
        emit firstByteReceived(false);
    }
}

void HttpManager::startAction()
{
    //QString h = m_actionUrl.host();
    QHostInfo::lookupHost(m_actionUrl.host(), this, SLOT(startThreads(QHostInfo)));
//    startThreads();
}

void HttpManager::startThreads(const QHostInfo &server)
{
    //check if the name resolution was actually successful
    QString s = server.errorString();
    if (server.error() != QHostInfo::NoError)
    {
        //emit error("Name resolution failed");
        return;
    }
    setServer(server);
    //this signal tells a worker thread to initiate the TCP 3-way handshake
    connect(this, SIGNAL(connectTCP()), this, SLOT(startTCPConnection()));

    //this signal is for the this thread of execution to track the TCP connection state
    //of the worker threads
    connect(this, SIGNAL(TCPConnected(bool)), this, SLOT(TCPConnectionTracking(bool)));

    //this signal tells the threads to start the actual download
    connect(this, SIGNAL(startDownload()), this, SLOT(sendRequest()));

    connect(this, SIGNAL(firstByteReceived(bool)), this, SLOT(downloadStartedTracking(bool)));

    connect(this, SIGNAL(TCPDisconnected()), this, SLOT(tidyUp())); //TODO errorhandling like winter

    emit connectTCP();
}

void HttpManager::read()
{
    bytesReceived << m_socket->bytesAvailable();
    QByteArray ba = m_socket->readAll();   //we don't need the actual data but need to free space in the
    m_answerFromServer.append(ba);
    emit readyToParse(m_answerFromServer);
}

void HttpManager::disconnectionHandling()
{
    // handling premature TCP disconnects
    if(tStatus == DownloadInProgress)
    {
        tStatus = FinishedSuccess;
        emit TCPDisconnected();
    }
}

void HttpManager::startTCPConnection()
{
    //shouldn't happen, check anyway
    if (server.addresses().isEmpty() || (!m_actionUrl.isValid()))
    {
        //invoke the connection tracking code
        tStatus = FinishedError;
        return;
    }
    m_socket = new QTcpSocket();

    //so let's connect now (if no port as part of the URL use 80 as default)
    //TODO my port!
    int p = m_actionUrl.port();
    m_socket->connectToHost(server.addresses().first(), p);

    tStatus = ConnectingTCP;

    //wait for up to 5 seconds for a successful connection
    if (m_socket->waitForConnected(tcpConnectTimeout))
    {
        //for the successfully connected sockets, we should track the disconnection
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnectionHandling()));
        tStatus = ConnectedTCP;
        emit TCPConnected(true);
    }
    else
    {
        //something went wrong
        QString s = m_socket->errorString();
        tStatus = FinishedError;
        emit TCPConnected(false);
        m_socket->close();
    }
}

void HttpManager::TCPConnectionTracking(bool success)
{
    if (success)
    {
        emit startDownload();
    }
}

void HttpManager::downloadStartedTracking(bool success)
{
    if(success)
    {
        qDebug() << "SUCCESS download started";
    }
//TODO
}

void HttpManager::tidyUp()
{
    m_file->close();
}

QUrl HttpManager::myUrl() const
{
    return m_myUrl;
}

void HttpManager::setMyUrl(const QUrl &myUrl)
{
    m_myUrl = myUrl;
}

void HttpManager::startGet()
{
    m_GETrequest.setUrl(m_GETUrl);
    m_GETrequest.setRawHeader(QByteArray("USER-AGENT"), QByteArray("Linux/3.13.0-24-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.17"));

    qDebug() << "Sending get request ...";
    m_xmlByteArray = new QByteArray();
    m_GETReply = m_networkAccessManager->get(m_GETrequest);
}

QHostInfo HttpManager::getServer() const
{
    return server;
}

void HttpManager::setServer(const QHostInfo &value)
{
    server = value;
}

QUrl HttpManager::remoteUrl() const
{
    return m_remoteUrl;
}

void HttpManager::setRemoteUrl(const QUrl &url)
{
    m_remoteUrl = url;
}


QUrl HttpManager::actionUrl() const
{
    return m_actionUrl;
}

void HttpManager::setActionUrl(const QUrl &browseUrl)
{
    m_actionUrl = browseUrl;
}


QUrl HttpManager::subscribeUrl() const
{
    return m_subscribeUrl;
}

void HttpManager::setSubscribeUrl(const QUrl &subscribeUrl)
{
    m_subscribeUrl = subscribeUrl;
}


QFile *HttpManager::file() const
{
    return m_file;
}

void HttpManager::setFile(const QString filename)
{
    m_file = new QFile(filename);
}

QNetworkRequest HttpManager::GETrequest() const
{
    return m_GETrequest;
}

void HttpManager::setGETrequest(const QNetworkRequest &GETrequest)
{
    m_GETrequest = GETrequest;
}

QUrl HttpManager::GETUrl() const
{
    return m_GETUrl;
}

void HttpManager::setGETUrl(const QUrl &GETUrl)
{
    m_GETUrl = GETUrl;
}




