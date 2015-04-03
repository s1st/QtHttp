#include "httpmanager.h"
#include "parser.h"
#include <QCoreApplication>
#include <QtNetwork>
#include <qnetworkaccessmanager.h>


int main(int argc, char *argv[])
{   
    QCoreApplication a(argc, argv);
    HttpManager man;
    man.init();
//    Parser parser;
//    parser.parseUpnpReply();

    return a.exec();
}
