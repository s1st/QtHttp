#include "UPnPHandler.h"
#include <QHttpPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkConfiguration>
#include <QCoreApplication>
#include <QNetworkInterface>

#include <QXmlStreamWriter>
#include <QThread>
#include <iterator>

UPnPHandler::UPnPHandler()
{
}


UPnPHandler::~UPnPHandler()
{

}

int UPnPHandler::init(QUrl remoteUrl, QString descriptionUrl, QString eventSubUrl, QString controlUrl, QString serviceType)
{
    QList<QUrl> ownUrls;
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
        {
            QString s = QString("http://%1").arg(address.toString());
            ownUrls.append(QUrl(s));
        }
    }

    setRemoteUrl(remoteUrl);
    setOwnUrls(ownUrls); //TODO check if valid for all urls

    QUrl subscribeUrl, browseUrl;
    QUrl descUrl(descriptionUrl);
    browseUrl = subscribeUrl = m_remoteUrl;
    browseUrl.setPath(controlUrl);
    subscribeUrl.setPath(eventSubUrl);

    setGETUrl(descUrl);
    setActionUrl(browseUrl);
    setSubscribeUrl(subscribeUrl);
    setServicetype(serviceType);
    setFile(":/xml/soapData.xml");
    /* If the file is already open, do nothing */
    if(!m_file->isOpen())
    {
        if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Opening XML File Problem";
            exit(-1);
        }
    }
    m_soapData = m_file->readAll();
    m_file->close();
    m_answerFromServer = "";
    m_objectID = "0";
    m_globalCounter = 0;

    m_parser = new Parser();
    m_parser->setSearchTerm("container"); //will be changed later, after all levels have been gone trough
    setNetworkAccessManager(new QNetworkAccessManager());
    startGet();
    connect(m_GETReply, SIGNAL(readyRead()),this, SLOT(GETreadyRead()));
    connect(m_parser, SIGNAL(xmlParsed()), this, SLOT(subscribe()));
    return 0;
}

QNetworkAccessManager * UPnPHandler::networkAccessManager() const
{
    return m_networkAccessManager;
}

void UPnPHandler::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    m_networkAccessManager = networkAccessManager;
}

void UPnPHandler::GETreadyRead()
{
    if(m_GETReply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error occured while get-request:" , m_GETReply->errorString();
        return;
    }
    QByteArray content = m_GETReply->readAll();
    m_xmlByteArray->append(content);
    /* comes in multiple packets */
     m_parser->parseXML(*m_xmlByteArray);
}

void UPnPHandler::subscribe()
{
    /* Now subscribe */
    m_subscribeRequest.setUrl(m_subscribeUrl);
    QByteArray valTemplate = ("</>");
    QByteArray val = valTemplate;
    /* trying all localhost addresses */
    foreach(QUrl u, m_ownUrls)
    {
        u.setPort(49153); //That is the vlc port, using it since it works in vlc
        val = valTemplate;
        val.insert(1, u.toString());
        m_subscribeRequest.setRawHeader(QByteArray("CALLBACK"), val);
        m_subscribeRequest.setRawHeader(QByteArray("NT"), ("upnp:event"));
        m_subscribeReply = m_networkAccessManager->sendCustomRequest(m_subscribeRequest, "SUBSCRIBE");
    }
    startAction(true);
}

//TODO timer for glimpse
int UPnPHandler::sendRequest()
{
    //we can only download, if this thread sucessfully established the
    //TCP connection (all threads will receive the signal)
    if(m_socket->state() != QAbstractSocket::ConnectedState)
    {
        tStatus = FinishedError;
        return(-1);
    }
    //different values for the object-IDs need to be inserted. starting with 0;
    //Parsing the answer and then again send request to the corresponding object-IDs -> new loop or slots TODO
    QByteArray b;
    b.append(m_objectID);
    QByteArray data = m_soapData;
    data.replace("##wildcard##", QByteArray(b));
    int dataLength = data.length();
    QString url = m_remoteUrl.toString();
    url.remove("http://");
    QString header = QString("POST %1 HTTP/1.1\r\n"
                              "HOST: %2\r\n"
                              "CONTENT-LENGTH: %3\r\n"
                              "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
                              "SOAPACTION: \"%4#Browse\"\r\n"
                              "USER-AGENT: Linux/3.13.0-24-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.17\r\n\r\n").arg(m_actionUrl.path())
                                                                                                                            .arg(url)
                                                                                                                            .arg(dataLength)
                                                                                                                            .arg(m_servicetype);
    header.append(data);
    int bytesWritten = 0;

    /* Send the SOAP request -> HTTP POST header and xml data */
    while(bytesWritten < header.length())
    {
        bytesWritten += m_socket->write(header.mid(bytesWritten).toLatin1());

        //on error
        if(bytesWritten < 0)
        {
            m_socket->close();
            tStatus = FinishedError;
            return(-1);
        }
    }
    bytesWritten = 0;
    tStatus = AwaitingFirstByte;
    int ret = 0;

    //do a blocking read for the first bytes or time-out if nothing is arriving
    if (m_socket->waitForReadyRead(firstByteReceivedTimeout))
    {
        tStatus = DownloadInProgress;
        ret = read();
    }
    else
    {
        qDebug() << m_socket->errorString();
        tStatus = FinishedError;
        m_socket->close();
        return(-1);
    }
    return ret ;
}

void UPnPHandler::startAction(bool firstShot)
{
    if(firstShot == true)
    {
        if(setupTCPSocketAndSend() > 0)
        {
            startAction(false);
        }
    }else{
        while(m_globalCounter < m_containerIDs.length())
        {
            qDebug() << "Scanning " + m_containerIDs[m_globalCounter].first;
            m_objectID = m_containerIDs[m_globalCounter].second;
            m_globalCounter ++;
            setupTCPSocketAndSend();
            startAction(false);
            int i = m_containerIDs.length();
            qDebug() << i; //TODO
        }
    }
}

int UPnPHandler::setupTCPSocketAndSend()
{
    int foundObjs = 1;
    if(!startTCPConnection())
    {
        qDebug() << "No TCP connection established";
        return -1;
    }
    foundObjs = sendRequest();
    return foundObjs;
}

int UPnPHandler::read()
{
    bytesReceived << m_socket->bytesAvailable();
    QByteArray ba = m_socket->readAll();
    m_answerFromServer.append(ba);
    m_parser->setRawData(m_answerFromServer);
    QList<QMap<QString, QString> > l;
    int ret = 0;
    if(m_answerFromServer.contains(QByteArray("200 OK")))
    {
        l = m_parser->parseUpnpReply();
    }else
    {
        qDebug() << "Error from Server:\n" + m_answerFromServer;
        exit(-1);
    }
    if(l.length() != 0)
    {
        m_foundContent = l;
        ret = handleContent(m_parser->searchTerm());
    }else{
        qDebug() << "Now Searching for Items";
        ret = 0;
        /*No more container were found, so now the search term changes to 'item' */
        m_parser->setSearchTerm("item");
    }
    m_answerFromServer.clear();
    return ret;
}

void UPnPHandler::disconnectionHandling()
{
    // handling premature TCP disconnects
    if(tStatus == DownloadInProgress)
    {
        tStatus = FinishedSuccess;
        emit TCPDisconnected();
    }
}

bool UPnPHandler::startTCPConnection()
{
    m_socket = new QTcpSocket();
    m_socket->connectToHost(m_actionUrl.host(), m_actionUrl.port());
    tStatus = ConnectingTCP;

    //wait for some seconds for a successful connection
    if (m_socket->waitForConnected(tcpConnectTimeout))
    {
        //for the successfully connected sockets, we should track the disconnection
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnectionHandling()));
        tStatus = ConnectedTCP;
        return true;
    }
    else
    {
        //something went wrong
        tStatus = FinishedError;
        m_socket->close();
        return false;
    }
}

/**
 * @brief UPnPHandler::handleContent
 * @param t the string which was searched for in the parser before
 * @return number of elements found
 */

int UPnPHandler::handleContent(QString t)
{
    int ret = 0;
    if(t == "container")
    {
        QList<QPair<QString, QString> > objectIDsandPurpose;
        for(int i = 0; i < m_foundContent.length(); i++) {
            QString id = m_foundContent[i].value("id");
            QString title = m_foundContent[i].value("title");
            QPair<QString, QString> m;
            m.first = title;
            m.second = id;
            objectIDsandPurpose.append(m);
        }
        m_containerIDs = objectIDsandPurpose;
        ret = m_containerIDs.length();
    }else if (t == "item") {
        //TODO
    }
    return ret;
}

QList<QPair<QString, QString> > UPnPHandler::containerIDs() const
{
    return m_containerIDs;
}

QList<QUrl> UPnPHandler::ownUrls() const
{
    return m_ownUrls;
}

void UPnPHandler::setOwnUrls(const QList<QUrl> &ownUrls)
{
    m_ownUrls = ownUrls;
}

QList<QMap<QString, QString> > UPnPHandler::foundContent() const
{
    return m_foundContent;
}

QString UPnPHandler::servicetype() const
{
    return m_servicetype;
}

void UPnPHandler::setServicetype(const QString &servicetype)
{
    m_servicetype = servicetype;
}

void UPnPHandler::startGet()
{
    m_GETrequest.setUrl(m_GETUrl);
    m_GETrequest.setRawHeader(QByteArray("USER-AGENT"), QByteArray("Linux/3.13.0-24-generic, UPnP/1.0, Portable SDK for UPnP devices/1.6.17"));

    qDebug() << "Sending get request ...";
    m_xmlByteArray = new QByteArray();
    m_GETReply = m_networkAccessManager->get(m_GETrequest);
}

QHostInfo UPnPHandler::getServer() const
{
    return m_server;
}

void UPnPHandler::setServer(const QHostInfo &value)
{
    m_server = value;
}

QUrl UPnPHandler::remoteUrl() const
{
    return m_remoteUrl;
}

void UPnPHandler::setRemoteUrl(const QUrl &url)
{
    m_remoteUrl = url;
}

QUrl UPnPHandler::actionUrl() const
{
    return m_actionUrl;
}

void UPnPHandler::setActionUrl(const QUrl &browseUrl)
{
    m_actionUrl = browseUrl;
}

QUrl UPnPHandler::subscribeUrl() const
{
    return m_subscribeUrl;
}

void UPnPHandler::setSubscribeUrl(const QUrl &subscribeUrl)
{
    m_subscribeUrl = subscribeUrl;
}

QFile *UPnPHandler::file() const
{
    return m_file;
}

void UPnPHandler::setFile(const QString filename)
{
    m_file = new QFile(filename);
}

QNetworkRequest UPnPHandler::GETrequest() const
{
    return m_GETrequest;
}

void UPnPHandler::setGETrequest(const QNetworkRequest &GETrequest)
{
    m_GETrequest = GETrequest;
}

QUrl UPnPHandler::GETUrl() const
{
    return m_GETUrl;
}

void UPnPHandler::setGETUrl(const QUrl &GETUrl)
{
    m_GETUrl = GETUrl;
}
