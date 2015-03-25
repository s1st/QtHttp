#include "httpmanager.h"
#include <QNetworkRequest>

HttpManager::HttpManager()
{

}

HttpManager::~HttpManager()
{

}

int HttpManager::init()
{
    setNetworkAccessManager(new QNetworkAccessManager());
//    QObject::connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
//            this, SLOT(replyFinished(QNetworkReply*)));

    //QNetworkReply r = manager->get(QNetworkRequest(QUrl("http://qt-project.org")));

    QNetworkRequest request;
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
    QList<QByteArray> headers = request.rawHeaderList();
    QNetworkReply *reply = m_networkAccessManager->get(request);
//    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
//            this, SLOT(slotError(QNetworkReply::NetworkError)));
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
//            this, SLOT(slotSslErrors(QList<QSslError>)));
    return 0;
}

QNetworkReply *HttpManager::reply() const
{
    return m_reply;
}

void HttpManager::setReply(QNetworkReply *reply)
{
    m_reply = reply;
}

QNetworkAccessManager * HttpManager::networkAccessManager() const
{
    return m_networkAccessManager;
}

void HttpManager::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    m_networkAccessManager = networkAccessManager;
}



