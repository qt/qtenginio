#include "enginiobackendconnection_p.h"
#include "enginioclient.h"
#include "enginioclient_p.h"
#include "enginioreply.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/QtEndian>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>
#include <QtCore/quuid.h>
#include <QtNetwork/qtcpsocket.h>

#define CRLF "\r\n"
const static int NIL = 0x00;
const static int FIN = 0x80;
const static int MSB = 0x80;
const static int MSK = 0x80;
const static int OPC = 0x0F;
const static int LEN = 0x7F;

const static quint64 DefaultHeaderLength = 2;
const static quint64 LargePayloadHeaderLength = 8;
const static quint64 MaskingKeyLength = 4;
const static quint64 NormalPayloadMarker = 126;
const static quint64 LargePayloadMarker = 127;
const static quint64 NormalPayloadLengthLimit = 0xFFFF;

namespace {

const QString HttpResponseStatus(QStringLiteral("HTTP/1\\.1\\s([0-9]{3})\\s"));
const QString SecWebSocketAcceptHeader(QStringLiteral("Sec-WebSocket-Accept:\\s(.{28})" CRLF));
const QString UpgradeHeader(QStringLiteral("Upgrade:\\s(.+)" CRLF));
const QString ConnectionHeader(QStringLiteral("Connection:\\s(.+)" CRLF));

QString gBase64EncodedSha1VerificationKey;

void computeBase64EncodedSha1VerificationKey(const QByteArray &base64Key)
{
    // http://tools.ietf.org/html/rfc6455#section-4.2.2 §5./ 4.
    QByteArray webSocketMagicString(QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
    webSocketMagicString.prepend(base64Key);
    gBase64EncodedSha1VerificationKey = QString::fromUtf8(QCryptographicHash::hash(webSocketMagicString, QCryptographicHash::Sha1).toBase64());
}

QByteArray generateMaskingKey()
{
    // The masking key is a 32-bit value chosen at random by the client.
    QByteArray uuid = QUuid::createUuid().toRfc4122();
    QByteArray key = uuid.left(MaskingKeyLength);
    for (int octet = MaskingKeyLength; octet < uuid.size(); ++octet) {
        int index = octet % key.size();
        key[index] = key[index] ^ uuid[octet];
    }
    return key;
}

void maskData(QByteArray &data, const QByteArray &maskingKey )
{
    // Client-to-Server Masking.
    // http://tools.ietf.org/html/rfc6455#section-5.3

    for (int octet = 0; octet < data.size(); ++octet)
        data[octet] = data[octet] ^ maskingKey[octet % maskingKey.size()];
}

int extractResponseStatus(QString responseString)
{
    const QRegularExpression re(HttpResponseStatus);
    QRegularExpressionMatch match = re.match(responseString);
    return match.captured(1).toInt();
}

const QString extractResponseHeader(QString pattern, QString responseString, bool ignoreCase = true)
{
    const QRegularExpression re(pattern);
    QRegularExpressionMatch match = re.match(responseString);

    if (ignoreCase)
        return match.captured(1).toLower();

    return match.captured(1);
}

const QByteArray constructOpeningHandshake(const QUrl& url)
{
    static QString host = QStringLiteral("%1:%2");
    static QString resourceUri = QStringLiteral("%1?%2");
    static QString request = QStringLiteral("GET %1 HTTP/1.1" CRLF
                                            "Host: %2" CRLF
                                            "Upgrade: websocket" CRLF
                                            "Connection: upgrade" CRLF
                                            "Sec-WebSocket-Key: %3" CRLF
                                            "Sec-WebSocket-Version: 13" CRLF
                                            CRLF
                                            );

    // http://tools.ietf.org/html/rfc6455#section-4.1 §2./ 7.
    // The request must include a header field with the name
    // Sec-WebSocket-Key. The value of this header field must be a
    // nonce consisting of a randomly selected 16-byte value that has
    // been base64-encoded.
    // The nonce must be selected randomly for each connection.

    const QByteArray secWebSocketKeyBase64 = EnginioBackendConnection::generateBase64EncodedUniqueKey();
    computeBase64EncodedSha1VerificationKey(secWebSocketKeyBase64);

    return request.arg(resourceUri.arg(url.path(QUrl::FullyEncoded), url.query(QUrl::FullyEncoded))
                       , host.arg(url.host(QUrl::FullyEncoded), QString::number(url.port(8080)))
                       , QString::fromUtf8(secWebSocketKeyBase64)
                       ).toUtf8();
}

const QByteArray constructFrameHeader(bool isFinalFragment
                                      , int opcode
                                      , quint64 payloadLength
                                      , const QByteArray &maskingKey
                                      )
{
    QByteArray frameHeader(DefaultHeaderLength, 0);

    // FIN, x, x, x, Opcode
    frameHeader[0] = frameHeader[0] | (isFinalFragment ? FIN : NIL) | opcode;

    // Masking bit
    frameHeader[1] = frameHeader[1] | MSK;

    // Payload length. Small payload.
    if (payloadLength < NormalPayloadMarker)
        frameHeader[1] = frameHeader[1] | payloadLength;
    else {
        if (payloadLength > NormalPayloadLengthLimit) {
            frameHeader[1] = frameHeader[1] | LargePayloadMarker;
            quint64 lengthBigEndian = qToBigEndian<quint64>(payloadLength);
            QByteArray lengthBytesBigEndian(reinterpret_cast<char*>(&lengthBigEndian), LargePayloadHeaderLength);

            if (lengthBytesBigEndian[0] & MSB) {
                qDebug() << "\t ERROR: Payload too large!" << payloadLength;
                return QByteArray();
            }

            frameHeader.append(lengthBytesBigEndian);
        } else {
            frameHeader[1] = frameHeader[1] | NormalPayloadMarker;
            quint16 lengthBigEndian = qToBigEndian<quint16>(payloadLength);
            frameHeader.append(reinterpret_cast<char*>(&lengthBigEndian), DefaultHeaderLength);
        }
    }

    // Masking-key
    frameHeader.append(maskingKey);

    return frameHeader;
}

} // namespace

/*!
    \brief Class to establish a stateful connection to a backend.
    The communication is based on the WebSocket protocol.
    \sa connectToBackend

    \internal
*/

EnginioBackendConnection::EnginioBackendConnection(QObject *parent)
    : QObject(parent)
    , _protocolOpcode(ContinuationFrameOp)
    , _protocolDecodeState(HandshakePending)
    , _sentCloseFrame(false)
    , _isFinalFragment(false)
    , _isPayloadMasked(false)
    , _payloadLength(0)
    , _tcpSocket(new QTcpSocket(this))
    , _client(new EnginioClient(this))
{
    _tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    _tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    QObject::connect(_client, SIGNAL(error(EnginioReply*)), this, SLOT(onEnginioError(EnginioReply*)));
    QObject::connect(_client, SIGNAL(finished(EnginioReply*)), this, SLOT(onEnginioFinished(EnginioReply*)));

    QObject::connect(_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketConnectionError(QAbstractSocket::SocketError)));
    QObject::connect(_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    QObject::connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
}

/*!
    \brief Generates a unique key useful for identification purposes.

    \return 24-byte base64 encoded nonce created from a 16-byte binary UUID
    \internal
*/

const QByteArray EnginioBackendConnection::generateBase64EncodedUniqueKey()
{
    return QUuid::createUuid().toRfc4122().toBase64();
}

void EnginioBackendConnection::onEnginioError(EnginioReply *reply)
{
    Q_ASSERT(reply);
    qDebug() << "\n\n### EnginioBackendConnection ERROR";
    qDebug() << reply->errorString();
    reply->dumpDebugInfo();
    qDebug() << "\n###\n";
}

void EnginioBackendConnection::onEnginioFinished(EnginioReply *reply)
{
    QJsonValue urlValue = reply->data()[EnginioString::expiringUrl];

    if (!urlValue.isString()) {
        qDebug() << "## Retrieving connection url failed.";
        return;
    }

    qDebug() << "## Initiating WebSocket connection.";

    _socketUrl = QUrl(urlValue.toString());
    _tcpSocket->connectToHost(_socketUrl.host(), _socketUrl.port(8080));

    reply->deleteLater();
}

void EnginioBackendConnection::protocolError(const char* message, WebSocketCloseStatus status)
{
    qWarning() << QLatin1Literal(message) << QStringLiteral("Closing socket.");
    close(status);
    _tcpSocket->close();
}

void EnginioBackendConnection::onSocketConnectionError(QAbstractSocket::SocketError error)
{
    protocolError("Socket connection error.");
    qWarning() << "\t\t->" << error;
}

void EnginioBackendConnection::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
    case QAbstractSocket::ConnectedState:
        qDebug() << "\t -> Starting WebSocket handshake.";
        _protocolDecodeState = HandshakePending;
        _sentCloseFrame = false;
        // The protocol handshake will appear to the HTTP server
        // to be a regular GET request with an Upgrade offer.
        _tcpSocket->write(constructOpeningHandshake(_socketUrl));
        break;
    case QAbstractSocket::ClosingState:
        _protocolDecodeState = HandshakePending;
        _applicationData.clear();
        _payloadLength = 0;
        break;
    case QAbstractSocket::UnconnectedState:
        emit stateChanged(DisconnectedState);
        break;
    default:
        break;
    }
}

void EnginioBackendConnection::onSocketReadyRead()
{
    //     WebSocket Protocol (RFC6455)
    //     Base Framing Protocol
    //     http://tools.ietf.org/html/rfc6455#section-5.2
    //
    //      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    //     +-+-+-+-+-------+-+-------------+-------------------------------+
    //     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    //     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    //     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    //     | |1|2|3|       |K|             |                               |
    //     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    //     |     Extended payload length continued, if payload len == 127  |
    //     + - - - - - - - - - - - - - - - +-------------------------------+
    //     |                               |Masking-key, if MASK set to 1  |
    //     +-------------------------------+-------------------------------+
    //     | Masking-key (continued)       |          Payload Data         |
    //     +-------------------------------- - - - - - - - - - - - - - - - +
    //     :                     Payload Data continued ...                :
    //     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    //     |                     Payload Data continued ...                |
    //     +---------------------------------------------------------------+

    while (_tcpSocket->bytesAvailable()) {
        switch (_protocolDecodeState) {
        case HandshakePending: {
            // The response is closed by a CRLF line on its own.
            while (_handshakeReplyLines.isEmpty() || _handshakeReplyLines.last() != QStringLiteral(CRLF)) {
                if (!_tcpSocket->canReadLine())
                    return;

                _handshakeReplyLines.append(QString::fromUtf8(_tcpSocket->readLine()));
            }

            QString response = _handshakeReplyLines.join(QString());
            _handshakeReplyLines.clear();

            int statusCode = extractResponseStatus(response);
            QString secWebSocketAccept = extractResponseHeader(SecWebSocketAcceptHeader, response, /* ignoreCase */ false);
            bool hasValidKey = secWebSocketAccept == gBase64EncodedSha1VerificationKey;

            if (statusCode != 101 || !hasValidKey
                    || extractResponseHeader(UpgradeHeader, response) != QStringLiteral("websocket")
                    || extractResponseHeader(ConnectionHeader, response) != QStringLiteral("upgrade")
                    )
                return protocolError("Handshake failed!");

            _protocolDecodeState = FrameHeaderPending;
            emit stateChanged(ConnectedState);
        } // Fall-through.

        case FrameHeaderPending: {
            if (quint64(_tcpSocket->bytesAvailable()) < DefaultHeaderLength)
                return;

            // Large payload.
            if (_payloadLength == LargePayloadMarker) {
                if (quint64(_tcpSocket->bytesAvailable()) < LargePayloadHeaderLength)
                    return;

                char data[LargePayloadHeaderLength];
                if (quint64(_tcpSocket->read(data, LargePayloadHeaderLength)) != LargePayloadHeaderLength)
                    return protocolError("Reading large payload length failed!");

                if (data[0] & MSB)
                    return protocolError("The most significant bit of a large payload length must be 0!", MessageTooBigCloseStatus);

                // 8 bytes interpreted as a 64-bit unsigned integer
                _payloadLength = qFromBigEndian<quint64>(reinterpret_cast<uchar*>(data));
                _protocolDecodeState = PayloadDataPending;

                break;
            }

            char data[DefaultHeaderLength];
            if (quint64(_tcpSocket->read(data, DefaultHeaderLength)) != DefaultHeaderLength)
                return protocolError("Reading header failed!");

            if (!_payloadLength) {
                // This is the initial frame header data.
                _isFinalFragment = (data[0] & FIN);
                _protocolOpcode = static_cast<WebSocketOpcode>(data[0] & OPC);
                _isPayloadMasked = (data[1] & MSK);
                _payloadLength = (data[1] & LEN);

                if (_isPayloadMasked)
                    return protocolError("Invalid masked frame received from server.");

                // For data length 0-125 LEN is the payload length.
                if (_payloadLength < NormalPayloadMarker)
                    _protocolDecodeState = PayloadDataPending;

            } else {
                Q_ASSERT(_payloadLength == NormalPayloadMarker);
                // Normal sized payload: 2 bytes interpreted as the payload
                // length expressed in network byte order (e.g. big endian).
                _payloadLength = qFromBigEndian<quint16>(reinterpret_cast<uchar*>(data));
                _protocolDecodeState = PayloadDataPending;
            }

            break;
        }

        case PayloadDataPending: {
            if (static_cast<quint64>(_tcpSocket->bytesAvailable()) < _payloadLength)
                return;

            if (_protocolOpcode == ConnectionCloseOp) {
                WebSocketCloseStatus closeStatus = UnknownCloseStatus;
                if (_payloadLength >= DefaultHeaderLength) {
                    char data[DefaultHeaderLength];
                    if (quint64(_tcpSocket->read(data, DefaultHeaderLength)) != DefaultHeaderLength)
                        return protocolError("Reading connection close status failed!");

                     closeStatus = static_cast<WebSocketCloseStatus>(qFromBigEndian<quint16>(reinterpret_cast<uchar*>(data)));

                     // The body may contain UTF-8-encoded data with value /reason/,
                     // the interpretation of this data is however not defined by the
                     // specification. Further more the data is not guaranteed to be
                     // human readable, thus it is safe for us to just discard the rest
                     // of the message at this point.
                }

                qDebug() << "Connection closed by the server with status:" << closeStatus;

                QJsonObject data;
                data[EnginioString::messageType] = QStringLiteral("close");
                data[EnginioString::status] = closeStatus;
                emit dataReceived(data);

                close(closeStatus);

                _tcpSocket->close();
                return;
            }

            _applicationData.append(_tcpSocket->read(_payloadLength));
            _protocolDecodeState = FrameHeaderPending;
            _payloadLength = 0;

            if (!_isFinalFragment)
                break;

            if (_protocolOpcode == TextFrameOp) {
                QJsonObject data = QJsonDocument::fromJson(_applicationData).object();
                data[EnginioString::messageType] = QStringLiteral("data");
                emit dataReceived(data);
            } else {
                protocolError("WebSocketOpcode not yet supported.", UnsupportedDataTypeCloseStatus);
                qWarning() << "\t\t->" << _protocolOpcode;
            }

            _applicationData.clear();

            break;
        }
        }
    }
}

void EnginioBackendConnection::setServiceUrl(const QUrl &serviceUrl)
{
    _client->setServiceUrl(serviceUrl);
}

/*!
    \brief Establish a stateful connection to the backend specified by
    \a backendId and \a backendSecret.
    Optionally, to let the server only send specific messages of interest,
    a \a messageFilter can be provided with the following json scheme:

    {
        "data": {
            objectType: 'objects.todos'
        },
        "event": "create"
    }

    The "event" property can be one of "create", "update" or "delete".

    \internal
*/

void EnginioBackendConnection::connectToBackend(const QByteArray &backendId
                                                , const QByteArray &backendSecret
                                                , const QJsonObject &messageFilter)
{
    qDebug() << "## Requesting WebSocket url.";
    _client->setBackendId(backendId);
    _client->setBackendSecret(backendSecret);
    QUrl url(_client->serviceUrl());
    url.setPath(QStringLiteral("/v1/stream_url"));

    QByteArray filter = QJsonDocument(messageFilter).toJson(QJsonDocument::Compact);
    filter.prepend("filter=");
    url.setQuery(QString::fromUtf8(filter));

    QJsonObject headers;
    headers[ QStringLiteral("Accept") ] = QStringLiteral("application/json");
    QJsonObject data;
    data[EnginioString::headers] = headers;

    emit stateChanged(ConnectingState);
    _client->customRequest(url, QByteArrayLiteral("GET"), data);
}

void EnginioBackendConnection::close(WebSocketCloseStatus closeStatus)
{
    if (_sentCloseFrame)
        return;

    qDebug() << "## Closing WebSocket connection.";

    _sentCloseFrame = true;

    QByteArray payload;
    quint16 closeStatusBigEndian = qToBigEndian<quint16>(closeStatus);
    payload.append(reinterpret_cast<char*>(&closeStatusBigEndian), DefaultHeaderLength);

    QByteArray maskingKey = generateMaskingKey();
    QByteArray message = constructFrameHeader(/*isFinalFragment*/ true, ConnectionCloseOp, payload.size(), maskingKey);
    Q_ASSERT(!message.isEmpty());

    maskData(payload, maskingKey);
    message.append(payload);
    _tcpSocket->write(message);
}
