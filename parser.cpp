#include "parser.h"

#include <QFile>
#include <QDebug>
#include <QStringList>
#include <QTextDocumentFragment>
#include <QXmlStreamReader>
#include <QMap>

Parser::Parser(QObject *parent) : QObject(parent)
{

}

Parser::~Parser()
{

}

void Parser::parseUpnpReply()
{
    //TODO delete -> flexible
    QFile *answer = new QFile("/home/simon/code/QtHttp/massiveAnswer");
    if (!answer->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Loading File Problem";
        exit(-1);
    }
    QByteArray ba = answer->readAll();
    QString s(ba);
    QTextDocumentFragment html;
    QTextDocumentFragment frag = html.fromHtml(s);
    QString s2 = frag.toPlainText();
    QByteArray realXML;
    realXML.append(frag.toPlainText());
    qDebug() << realXML;
    //parseXML(realXML); //TODO debug
    QList<QMap<QString, QString> > tableOfContents = parseXMLtoMaps(realXML);
    for(int i = 0; i < tableOfContents.length(); i++)
    {
        qDebug() << "*** The Following contents are available in this element: ***";
        QString all = "";
        foreach(QString s, tableOfContents[i].keys())
        {
               all.append(" " + s);
        }
        qDebug() << all;
        qDebug() << tableOfContents[i].value("title");
    }
}

QHash<QString, QString> Parser::results() const
{
    return m_results;
}

void Parser::setResults(const QHash<QString, QString> &results)
{
    m_results = results;
}


bool Parser::parseXML(QByteArray ba)
{
    QXmlStreamReader * xmlReader = new QXmlStreamReader(ba);

    QHash<QString, QString> values;
    QString name;
    QString text;
    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError()) {
            // Read next element
            QXmlStreamReader::TokenType token = xmlReader->readNext();
            //If token is just StartDocument - go to next
            if(token == QXmlStreamReader::StartDocument) {
                continue;
            }
            //If token is StartElement - read it
            if(token == QXmlStreamReader::StartElement) {
                if(xmlReader->name() == "root") {
                    continue;
                }
                name = xmlReader->name().toString();
            }
//            if(token ==)
            if(token == QXmlStreamReader::Characters && !name.isEmpty()) {
                text = xmlReader->text().toString();
                values.insert(name, text); //TODO excerpt
                name.clear();
                text.clear();
            }
    }
    if(xmlReader->hasError()){
            qDebug() << "xmlFile.xml Parse Error";
            values.clear();
            return false;
    }
    emit xmlParsed();
    m_results = values;
    return true;
}

QList<QMap<QString, QString> > Parser::parseXMLtoMaps(QByteArray ba)
{
    QXmlStreamReader * xmlReader = new QXmlStreamReader(ba);

    QList<QMap<QString, QString> > contents;
    QString name;
    QString text;
    QString key;
    QString value;
    QMap<QString, QString> element;
    int foundElementFlag = 0;
    //Parse the XML until we reach end of it
    while(!xmlReader->atEnd() && !xmlReader->hasError())
    {
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        name = xmlReader->name().toString();
        text = xmlReader->text().toString();
        if(token == QXmlStreamReader::StartDocument)
        {
           continue;
        }
        if(token == QXmlStreamReader::StartElement)
        {
            if(name == "item")
            {
                foundElementFlag = 1;
            }
            if(foundElementFlag == 1)
            {
                key = name;
            }
        }
        if(token == QXmlStreamReader::Characters)
        {
            if(foundElementFlag == 1)
            {
                value = text;
                if(!key.isEmpty())
                {
                    element.insert(key, value);
                }
            }
        }
        if(token == QXmlStreamReader::EndElement)
        {
            if(name == "item")
            {
                contents.append(element);
                element.clear();
                foundElementFlag = 0;
            }
        }
    }
    if(xmlReader->hasError()){
            qDebug() << "xmlFile.xml Parse Error";
            contents.clear();
            return contents;
    }
    return contents;
}

void Parser::parseAnswer(QByteArray ba)
{
    qDebug() << "parsing answer ...";
    qDebug() << "before ...";
    qDebug() << ba;
    int end = ba.indexOf("s:Envelope"); //TODO needed?
    ba.remove(0, end-1);
    qDebug() << "after ...";
    qDebug() << ba;
    //Stripping the header away
//    m_answerFromServer.split();
    parseXML(ba);
    QString res = m_results.value("Result");
//    QString res = m_results.value("Result");
    qDebug() << res;
    QByteArray resBA = "";
    resBA.append(res);
    //unfortunately no xml, so -> parsing?! edit: XML with weird html-tags
    qDebug() << "what now";
}
