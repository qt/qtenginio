#include "enginioqmlreply.h"
#include "enginioqmlclient_p.h"
#include <Enginio/private/enginioreply_p.h>
#include <QtQml/qjsvalue.h>

class EnginioQmlReplyPrivate : public EnginioReplyPrivate
{
    EnginioQmlClientPrivate *_client;
    mutable QJSValue _value;

public:
    EnginioQmlReplyPrivate(EnginioQmlClientPrivate *client, QNetworkReply *reply)
        : EnginioReplyPrivate(reply)
        , _client(client)
    {
        Q_ASSERT(client);
    }

    QJSValue data() const
    {
        if (!_value.isObject())
            _value = _client->fromJson(_nreply->readAll());
        return _value;
    }
};

EnginioQmlReply::EnginioQmlReply(EnginioQmlClientPrivate *parent, QNetworkReply *reply)
    : EnginioReply(parent, reply, new EnginioQmlReplyPrivate(parent, reply))
{}

QJSValue EnginioQmlReply::data() const
{
    return static_cast<const EnginioQmlReplyPrivate*>(d.data())->data();
}
