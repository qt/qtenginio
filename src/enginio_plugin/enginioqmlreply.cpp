#include "enginioqmlreply.h"
#include "enginioqmlclient_p.h"
#include <Enginio/private/enginioreply_p.h>
#include <QtQml/qjsvalue.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

class EnginioQmlReplyPrivate : public EnginioReplyPrivate
{
    EnginioQmlReply *q;
    QMetaObject::Connection _orphanConnection;
    mutable QJSValue _value;

public:
    EnginioQmlReplyPrivate(EnginioQmlClientPrivate *client, QNetworkReply *reply, EnginioQmlReply *q_ptr)
        : EnginioReplyPrivate(client, reply)
        , q(q_ptr)
    {
        Q_ASSERT(client);
    }

    void emitFinished()
    {
        // Enginio doesn't need this reply anymore, so now we need to figure out how to
        // delete it. There are two cases:
        //  - finished and error signals are not connected => nobody really cares about
        //    this reply we can safely delete it.
        //  - at least one of finished and error signals is connected => we assume that
        //    the connection is done from QML, so we transfer reply ownership to it. C++
        //    developers needs to take care of such situation and addapt.

        static QMetaMethod clientFinishedSignal = QMetaMethod::fromSignal(&EnginioClient::finished);
        static QMetaMethod clientErrorSignal = QMetaMethod::fromSignal(&EnginioClient::error);
        static QMetaMethod replyFinishedSignal = QMetaMethod::fromSignal(&EnginioReply::finished);

        bool isReplyFinishedConnected = q->isSignalConnected(replyFinishedSignal);
        bool isClientFinishedConnected = _client->isSignalConnected(clientFinishedSignal);
        bool isClientErrorConnected = _client->isSignalConnected(clientErrorSignal);

        if (Q_LIKELY(isClientFinishedConnected
                  || isReplyFinishedConnected
                  || isClientErrorConnected)) {
            // something is connected and we can transfer the owership.
            q->setParent(0);
            QQmlEngine::setObjectOwnership(q, QQmlEngine::JavaScriptOwnership);
            if (isReplyFinishedConnected)
                emit q->finished(q);
        } else
            q->deleteLater();
    }

    QJSValue data() const
    {
        if (!_value.isObject())
            _value = static_cast<EnginioQmlClientPrivate*>(_client)->fromJson(_nreply->readAll());
        return _value;
    }
};


EnginioQmlReply::EnginioQmlReply(EnginioQmlClientPrivate *parent, QNetworkReply *reply)
    : EnginioReply(parent, reply, new EnginioQmlReplyPrivate(parent, reply, this))
{}

EnginioQmlReply::~EnginioQmlReply()
{
    EnginioQmlReplyPrivate *priv = static_cast<EnginioQmlReplyPrivate *>(d.take());
    delete priv;
}

QJSValue EnginioQmlReply::data() const
{
    return static_cast<const EnginioQmlReplyPrivate*>(d.data())->data();
}

void EnginioQmlReply::emitFinished()
{
    static_cast<EnginioQmlReplyPrivate*>(d.data())->emitFinished();
}
