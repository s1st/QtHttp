#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include "parser.h"

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFile>
#include <QHostInfo>

class UPnPHandler :public QObject
{
    Q_OBJECT

public:
    enum DownloadThreadStatus
    {
        Inactive,
        ConnectingTCP,
        ConnectedTCP,
        AwaitingFirstByte,
        DownloadInProgress,
        FinishedSuccess,
        FinishedError
    };

    UPnPHandler();
    ~UPnPHandler();

    int init(QUrl remoteUrl, QString descriptionUrl, QString eventSubUrl, QString controlUrl, QString serviceType);

    QNetworkAccessManager * networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
    bool parseXML(QByteArray ba);

    QUrl GETUrl() const;
    void setGETUrl(const QUrl &GETUrl);

    QNetworkRequest GETrequest() const;
    void setGETrequest(const QNetworkRequest &GETrequest);

    QFile *file() const;
    void setFile(const QString filename);

    QUrl subscribeUrl() const;
    void setSubscribeUrl(const QUrl &subscribeUrl);

    QUrl actionUrl() const;
    void setActionUrl(const QUrl &actionUrl);

    static const int tcpConnectTimeout = 10000;
    static const int firstByteReceivedTimeout = 10000;
    static const int defaultPort = 80;
    static const int threadnum = 1;

    QUrl remoteUrl() const;
    void setRemoteUrl(const QUrl &url);

    QHostInfo getServer() const;
    void setServer(const QHostInfo &value);

    QString servicetype() const;
    void setServicetype(const QString &servicetype);

    QList<QMap<QString, QString> > foundContent() const;

    QList<QUrl> ownUrls() const;
    void setOwnUrls(const QList<QUrl> &ownUrls);

    QList<QPair<QString, QString> > containerIDs() const;

    bool startTCPConnection();
    int handleContent(QString t);
    int sendRequest();
    int read();

public slots:
    void startGet();
    void GETreadyRead();
    void subscribe();
    void startAction(bool firstShot);
    void setupTCPSocket(const QHostInfo &server);
    void disconnectionHandling();
//    void sendExactRequests();

signals:
    void subscribed(bool firstShot);
    void TCPConnected(bool success);
    void TCPDisconnected();
    void connectTCP();
    void startDownload();
    void readyToParse(int m_objectID);
    void handlingDone();
//    void searchForObjectIDs();
    void exactScan();
    void foundContainer();

private:
    QString m_servicetype;
    DownloadThreadStatus tStatus;
    Parser *m_parser;
    QTcpSocket *m_socket;
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_GETReply;
    QNetworkReply *m_subscribeReply;
    QNetworkReply *m_browseReply;
    QNetworkRequest m_GETrequest;
    QNetworkRequest m_subscribeRequest;
    QNetworkRequest m_browseRequest;
    QList<qint64> bytesReceived;
    QHostInfo server;
    QUrl m_remoteUrl;
    QList<QUrl> m_ownUrls;
    QUrl m_GETUrl;
    QUrl m_subscribeUrl;
    QUrl m_actionUrl;
    QFile *m_file;
    QByteArray *m_xmlByteArray;
    QByteArray m_answerFromServer;
    QList<QMap<QString, QString> > m_foundContent;
    QList<QPair<QString, QString> > m_containerIDs;
    QString m_objectID;
    QByteArray m_soapData;
};

#endif // HTTPMANAGER_H