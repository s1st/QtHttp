#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <qnetworkaccessmanager.h>

class HttpManager :public QObject
{
    Q_OBJECT

public:

    HttpManager();
    ~HttpManager();

    int init();

    QNetworkReply *reply() const;
    void setReply(QNetworkReply *reply);

    QNetworkAccessManager * networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);

private:
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_reply;
};

#endif // HTTPMANAGER_H
