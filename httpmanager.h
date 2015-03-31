#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFile>
#include <QHostInfo>

class HttpManager :public QObject
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

    HttpManager();
    ~HttpManager();

    int init();

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

    QUrl myUrl() const;
    void setMyUrl(const QUrl &myUrl);

public slots:
    void startGet();
    void GETFinished();
    void GETreadyRead();
    void subscribe();
    void browse();
    void sendRequest();
    void startAction();
    void startThreads(const QHostInfo &server);
    void read();
    void disconnectionHandling();
    void startTCPConnection();
    void TCPConnectionTracking(bool success);
    void downloadStartedTracking(bool success);
//    void stopDownload();
    void parseAnswer();
    void tidyUp();

signals:
    void xmlParsed();
    void subscribed();
    void firstByteReceived(bool success);
    void TCPConnected(bool success);
    void TCPDisconnected();
    void connectTCP();
    void startDownload();
    void readyToParse();

private:
    QList <QThread *> threads;
    QList <HttpManager *> workers;
    DownloadThreadStatus tStatus;
    //the socket from which to read/write, to be created after moved to thread!
    //that's why we need a pointer here
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
    QUrl m_myUrl;
    QUrl m_GETUrl;
    QUrl m_subscribeUrl;
    QUrl m_actionUrl;
    QFile *m_file;
    QByteArray *m_xmlByteArray;
    QByteArray m_answerFromServer;
    QHash<QString, QString> m_results;
};

#endif // HTTPMANAGER_H
