#ifndef ENGINIOBACKENDCONNECTION_P_H
#define ENGINIOBACKENDCONNECTION_P_H

#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qabstractsocket.h>

class EnginioClient;
class EnginioReply;
class QTcpSocket;
class EnginioBackendConnection : public QObject
{
    Q_OBJECT

    enum WebSocketOpcode
    {
        ContinuationFrameOp = 0x0,
        TextFrameOp = 0x1,
        BinaryFrameOp = 0x2,
        // %x3-7 are reserved for further non-control frames
        ConnectionCloseOp = 0x8,
        PingOp = 0x9,
        PongOp = 0xA
        // %xB-F are reserved for further control frames
    } _protocolOpcode;

    enum ProtocolDecodeState
    {
        HandshakePending,
        FrameHeaderPending,
        PayloadDataPending
    } _protocolDecodeState;

    bool _isFinalFragment;
    bool _isPayloadMasked;
    quint64 _payloadLength;
    QByteArray _applicationData;

    QUrl _socketUrl;
    QTcpSocket *_tcpSocket;
    EnginioClient* _client;

    void protocolError(const char* message);

public:
    enum ConnectionState {
        DisconnectedState,
        ConnectingState,
        ConnectedState
    };

    explicit EnginioBackendConnection(QObject *parent = 0);

    bool isConnected() { return _protocolDecodeState > HandshakePending; }
    void setServiceUrl(const QUrl& serviceUrl);
    void connectToBackend(const QByteArray &backendId, const QByteArray &backendSecret, const QJsonObject& messageFilter = QJsonObject());

signals:
    void stateChanged(ConnectionState state);
    void dataReceived(QJsonObject data);

private slots:
    void onEnginioError(EnginioReply *);
    void onEnginioFinished(EnginioReply *);
    void onSocketStateChanged(QAbstractSocket::SocketState);
    void onSocketConnectionError(QAbstractSocket::SocketError);
    void onSocketReadyRead();
};

#endif // ENGINIOBACKENDCONNECTION_P_H
