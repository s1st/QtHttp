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
    /* bubble upnp */
//    url = "http://192.168.2.101:58645";
//    descriptionUrl = "http://192.168.2.101:58645/dev/8cde490a-cce3-b010-ffff-ffffccccb326/desc.xml";
//    eventSubUrl = "/dev/8cde490a-cce3-b010-ffff-ffffccccb326/svc/upnp-org/ContentDirectory/event";
//    controlUrl = "/dev/8cde490a-cce3-b010-ffff-ffffccccb326/svc/upnp-org/ContentDirectory/action";
    serviceType = "urn:schemas-upnp-org:service:ContentDirectory:1";

    man.init(url, descriptionUrl, eventSubUrl, controlUrl, serviceType);

    return a.exec();
}
