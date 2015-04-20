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

    /* Media Tomb */
    url = "http://localhost:49152/";
    descriptionUrl = "http://localhost:49152/description.xml";
    eventSubUrl = "/upnp/event/cds";
    controlUrl = "/upnp/control/cds";

    /* minidlna */
//    url = "http://192.168.2.104:8200/";
//    descriptionUrl = "http://192.168.2.104:8200/rootDesc.xml";
//    eventSubUrl = "/evt/ContentDir";
//    controlUrl = "/ctl/ContentDir";

    /* bubble upnp */
//    url = "http://192.168.2.101:58645";
//    descriptionUrl = "http://192.168.2.101:58645/dev/8cde490a-cce3-b010-ffff-ffffccccb326/desc.xml";
//    eventSubUrl = "/dev/8cde490a-cce3-b010-ffff-ffffccccb326/svc/upnp-org/ContentDirectory/event";
//    controlUrl = "/dev/8cde490a-cce3-b010-ffff-ffffccccb326/svc/upnp-org/ContentDirectory/action";

    /* plex media Melanie */
//    url = "http://192.168.2.105:32469";
//    descriptionUrl = "http://192.168.2.105:32469/DeviceDescription.xml";
//    eventSubUrl = "/ContentDirectory/0bdbbd34-455e-f0a3-91a3-5c86541fc65a/event.xml";
//    controlUrl = "/ContentDirectory/0bdbbd34-455e-f0a3-91a3-5c86541fc65a/control.xml";

    /* my plex media */
//    url = "http://172.16.172.1:32469";
//    descriptionUrl = "http://172.16.172.1:32469/DeviceDescription.xml";
//    eventSubUrl = "/ContentDirectory/fe8e1f2a-1505-c82d-80e2-721ad07a0f10/event.xml";
//    controlUrl = "/ContentDirectory/fe8e1f2a-1505-c82d-80e2-721ad07a0f10/control.xml";

    serviceType = "urn:schemas-upnp-org:service:ContentDirectory:1";
    man.init(url, descriptionUrl, eventSubUrl, controlUrl, serviceType);

    return a.exec();
}
