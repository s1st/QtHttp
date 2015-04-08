#include "UPnPHandler.h"
#include "parser.h"
#include <QCoreApplication>
#include <QtNetwork>
#include <qnetworkaccessmanager.h>


int main(int argc, char *argv[])
{   
    QCoreApplication a(argc, argv);
    UPnPHandler man;
    QUrl url;
    QString descriptionUrl, eventSubUrl, controlUrl, serviceType;
    url = "http://localhost:49152/";
    descriptionUrl = "http://localhost:49152/description.xml";
    eventSubUrl = "/upnp/event/cds";
    controlUrl = "/upnp/control/cds";
    serviceType = "urn:schemas-upnp-org:service:ContentDirectory:1";

    man.init(url, descriptionUrl, eventSubUrl, controlUrl, serviceType);

    return a.exec();
}
