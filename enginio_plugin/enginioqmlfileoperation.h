#ifndef ENGINIOQMLFILEOPERATION_H
#define ENGINIOQMLFILEOPERATION_H

#include "enginiofileoperation.h"
#include "enginioqmlclient.h"

#include <QJSValue>

class EnginioQmlFileOperation : public EnginioFileOperation
{
    Q_OBJECT
    Q_DISABLE_COPY(EnginioQmlFileOperation)
    Q_PROPERTY(EnginioQmlClient* client READ getClient WRITE setClient)
    Q_PROPERTY(QString fileId READ fileId)
    Q_PROPERTY(QJSValue object READ object)

public:
    EnginioQmlFileOperation(EnginioQmlClient *client = 0,
                            QObject *parent = 0);

    EnginioQmlClient * getClient() const;
    QJSValue object() const;
};

#endif // ENGINIOQMLFILEOPERATION_H
