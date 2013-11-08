/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ENGINIOMODELBASE_P_H
#define ENGINIOMODELBASE_P_H


#include <Enginio/enginiomodel.h>
#include <Enginio/enginioreply.h>
#include <Enginio/private/enginioclient_p.h>
#include <Enginio/private/enginiofakereply_p.h>
#include <Enginio/private/enginiodummyreply_p.h>
#include <Enginio/enginioreplybase.h>
#include <Enginio/private/enginiobackendconnection_p.h>
#include <Enginio/enginiomodelbase.h>
#include <Enginio/private/enginiomodelbase_p.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qhash.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qstring.h>
#include <QtCore/quuid.h>
#include <QtCore/qvector.h>

#include <QtCore/private/qabstractitemmodel_p.h>

QT_BEGIN_NAMESPACE

enum {
    DeletedRow = -3,
    NoHintRow = -4,
    InvalidRow = NoHintRow
};

struct EnginioModelPrivateAttachedData
{
    uint ref;
    int row;
    QString id;
    EnginioReplyBase *createReply;
    EnginioModelPrivateAttachedData(int initRow = DeletedRow, const QString &initId = QString())
        : ref()
        , row(initRow)
        , id(initId)
        , createReply()
    {}
};
Q_DECLARE_TYPEINFO(EnginioModelPrivateAttachedData, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const EnginioModelPrivateAttachedData &a)
{
    dbg.nospace() << "EnginioModelPrivateAttachedData(ref:";
    dbg.nospace() << a.ref << ", row: "<< a.row << ", synced: " << (a.ref == 0) << ", id: " << a.id;
    dbg.nospace() << ')';
    return dbg.space();
}
#endif

class AttachedDataContainer
{
    typedef int Row;
    typedef int StorageIndex;
    typedef QString ObjectId;
    typedef QString RequestId;
    typedef EnginioModelPrivateAttachedData AttachedData;

    typedef QHash<Row, StorageIndex> RowIndex;
    RowIndex _rowIndex;

    typedef QHash<ObjectId, StorageIndex> ObjectIdIndex;
    ObjectIdIndex _objectIdIndex;

    typedef QHash<RequestId, QPair<int /*ref*/, StorageIndex> > RequestIdIndex;
    RequestIdIndex _requestIdIndex;

    typedef QHash<StorageIndex, AttachedData> Storage;
    QVector<AttachedData> _storage; // TODO replace by something smarter so we can use pointers instead of index.

    enum { InvalidStorageIndex = InvalidRow };

    StorageIndex append(const AttachedData &data)
    {
        _storage.append(data);
        StorageIndex idx = _storage.count() - 1;
        _rowIndex.insert(data.row, idx);
        _objectIdIndex.insert(data.id, idx);
        return idx;
    }

public:
    bool contains(const ObjectId &id) const
    {
        return _objectIdIndex.contains(id);
    }

    Row rowFromObjectId(const ObjectId &id) const
    {
        Q_ASSERT(contains(id));
        StorageIndex idx = _objectIdIndex.value(id, InvalidStorageIndex);
        return idx == InvalidStorageIndex ? InvalidRow : _storage[idx].row;
    }

    Row rowFromRequestId(const RequestId &id) const
    {
        StorageIndex idx = _requestIdIndex.value(id, qMakePair(0, static_cast<int>(InvalidStorageIndex))).second;
        return idx == InvalidStorageIndex ? InvalidRow : _storage[idx].row;
    }

    bool isSynced(Row row) const
    {
        return _storage[_rowIndex.value(row)].ref == 0;
    }

    void updateAllDataAfterRowRemoval(const int row) {
        _rowIndex.clear();
        _rowIndex.reserve(_storage.count());
        for (StorageIndex i = 0; i < _storage.count() ; ++i) {
            AttachedData &data = _storage[i];
            if (data.row > row)
                --data.row;
            else if (data.row == row)
                data.row = DeletedRow;
            _rowIndex.insert(data.row, i);
        }
    }

    AttachedData &ref(const ObjectId &id, Row row)
    {
        StorageIndex idx = _objectIdIndex.value(id, InvalidStorageIndex);
        if (idx == InvalidStorageIndex) {
            AttachedData data(row, id);
            idx = append(data);
        }
        AttachedData &data = _storage[idx];
        ++data.ref;
        Q_ASSERT(_storage[idx].ref == 1 || _storage[idx].row == row);
        data.row = row;
        return data;
    }

    AttachedData &ref(Row row)
    {
        StorageIndex idx = _rowIndex.value(row, InvalidStorageIndex);
        Q_ASSERT(idx != InvalidStorageIndex);
        AttachedData &data = _storage[idx];
        ++data.ref;
        return data;
    }

    AttachedData &deref(const ObjectId &id)
    {
        StorageIndex idx = _objectIdIndex.value(id, InvalidStorageIndex);
        Q_ASSERT(idx != InvalidStorageIndex);
        AttachedData &attachedData = _storage[idx];
        if (!--attachedData.ref && id[0] == 't') {
            // TODO it is last ref to a tmp id we should remove it
        }
        return attachedData;
    }

    void insert(const AttachedData &data)
    {
        _storage.append(data);
        StorageIndex idx = _storage.count() - 1;
        _rowIndex.insert(data.row, idx);
        _objectIdIndex.insert(data.id, idx);
    }

    void insertRequestId(const RequestId &id, Row row)
    {
        StorageIndex idx = _rowIndex.value(row, InvalidStorageIndex);
        Q_ASSERT(idx != InvalidStorageIndex);
        _requestIdIndex.insert(id, qMakePair(2, idx));
    }

    /*!
      \internal
      returns true if the request was already handled
    */
    bool markRequestIdAsHandled(const RequestId &id)
    {
        RequestIdIndex::iterator::reference value = _requestIdIndex[id];
        if (value.first) {
            if (--value.first <= 0) {
                _requestIdIndex.remove(id);
                return true;
            }
        } else {
            _requestIdIndex.remove(id);
        }
        return false;
    }

    void initFromArray(const QJsonArray &array)
    {
        const int count = array.count();
        _storage.clear();
        _rowIndex.clear();
        _objectIdIndex.clear();

        _storage.reserve(count);
        _rowIndex.reserve(count);
        _objectIdIndex.reserve(count);

        for (int row = 0; row < count; ++row) {
            QString id = array[row].toObject()[EnginioString::id].toString();
            Q_ASSERT(!id.isEmpty());
            AttachedData data(row, id);
            _storage.append(data);
            _rowIndex.insert(row, row);
            _objectIdIndex.insert(id, row);
        }
    }
};


class ENGINIOCLIENT_EXPORT EnginioModelBasePrivate : public QAbstractItemModelPrivate {
protected:
    EnginioClientBasePrivate *_enginio;
    EnginioClient::Operation _operation;
    EnginioModelBase *q;
    QVector<QMetaObject::Connection> _clientConnections;
    QHash<const EnginioReplyBase*, QMetaObject::Connection> _repliesConnections;

    const static int IncrementalModelUpdate;
    typedef EnginioModelPrivateAttachedData AttachedData;
    AttachedDataContainer _attachedData;
    int _latestRequestedOffset;
    bool _canFetchMore;

    unsigned _rolesCounter;
    QHash<int, QString> _roles;

    QJsonArray _data;

    class NotificationObject {
        // connection object it can be:
        // - null if not yet created
        // - -1 if notifications where disabled with EnginioModel::disableNotifications()
        // - valid pointer to connection object
        EnginioBackendConnection *_connection;

        struct NotificationReceived
        {
            EnginioModelBasePrivate *model;

            void operator ()(QJsonObject data)
            {
                model->receivedNotification(data);
            }
        };
        void removeConnection()
        {
            if (*this) {
                _connection->close();
                delete _connection;
            }
        }

    public:
        NotificationObject()
            : _connection()
        {}

        ~NotificationObject()
        {
            removeConnection();
        }

        operator EnginioBackendConnection*()
        {
            return qintptr(_connection) != -1 ? _connection : 0;
        }

        void disable()
        {
            removeConnection();
            _connection = (EnginioBackendConnection*)-1;
        }

        void connectToBackend(EnginioModelBasePrivate *model, EnginioClientBasePrivate *enginio, const QJsonObject &filter)
        {
            if (qintptr(_connection) == -1)
                return;
            Q_ASSERT(model && enginio);
            if (enginio->_serviceUrl != EnginioString::stagingEnginIo)
                return;  // TODO it allows to use notification only on staging
            removeConnection(); // TODO reuse the connecton object
            _connection = new EnginioBackendConnection;
            NotificationReceived receiver = { model };
            QObject::connect(_connection, &EnginioBackendConnection::dataReceived, receiver);
            _connection->connectToBackend(enginio, filter);
        }
    } _notifications;

    struct FinishedRemoveRequest
    {
        EnginioModelBasePrivate *model;
        const QString id;
        EnginioReplyBase *reply;
        void operator ()()
        {
            model->finishedRemoveRequest(reply, id);
        }
    };

    struct FinishedUpdateRequest
    {
        EnginioModelBasePrivate *model;
        const QString id;
        const QJsonObject oldValue;
        EnginioReplyBase *reply;
        void operator ()()
        {
            model->finishedUpdateRequest(reply, id, oldValue);
        }
    };

    struct FinishedCreateRequest
    {
        EnginioModelBasePrivate *model;
        const QString tmpId;
        EnginioReplyBase *reply;
        void operator ()()
        {
            model->finishedCreateRequest(reply, tmpId);
        }
    };

    struct FinishedFullQueryRequest
    {
        EnginioModelBasePrivate *model;
        EnginioReplyBase *reply;
        void operator ()()
        {
            model->finishedFullQueryRequest(reply);
        }
    };

    struct FinishedIncrementalUpdateRequest
    {
        EnginioModelBasePrivate *model;
        const QJsonObject query;
        EnginioReplyBase *reply;
        void operator ()()
        {
            model->finishedIncrementalUpdateRequest(reply, query);
        }
    };

    class QueryChanged
    {
        EnginioModelBasePrivate *model;
    public:
        QueryChanged(EnginioModelBasePrivate *m)
            : model(m)
        {
            Q_ASSERT(m);
        }

        void operator ()()
        {
            model->execute();
        }
    };

public:
    EnginioModelBasePrivate(EnginioModelBase *q_ptr)
        : _enginio(0)
        , _operation()
        , q(q_ptr)
        , _latestRequestedOffset(0)
        , _canFetchMore(false)
        , _rolesCounter(EnginioModel::SyncedRole)
    {
    }

    virtual ~EnginioModelBasePrivate();

    void init()
    {
        QObject::connect(q, &EnginioModelBase::operationChanged, QueryChanged(this));
    }

    void disableNotifications()
    {
        _notifications.disable();
    }

    void receivedNotification(QJsonObject data);
    void receivedRemoveNotification(const QJsonObject &object, int rowHint = NoHintRow);
    void receivedUpdateNotification(const QJsonObject &object, const QString &idHint = QString(), int row = NoHintRow);
    void receivedCreateNotification(const QJsonObject &object);

    EnginioReplyBase *append(const QJsonObject &value)
    {
        QJsonObject object(value);
        QString temporaryId = QString::fromLatin1("tmp") + QUuid::createUuid().toString();
        object[EnginioString::objectType] = queryData(EnginioString::objectType); // TODO think about it, it means that not all queries are valid
        ObjectAdaptor<QJsonObject> aObject(object);
        QNetworkReply *nreply = _enginio->create(aObject, _operation);
        EnginioReplyBase *ereply = _enginio->createReply(nreply);
        FinishedCreateRequest finishedRequest = { this, temporaryId, ereply };
        _repliesConnections.insert(ereply, QObject::connect(ereply, &EnginioReplyBase::dataChanged, finishedRequest));
        object[EnginioString::id] = temporaryId;
        const int row = _data.count();
        AttachedData data(row, temporaryId);
        data.ref = 1;
        data.createReply = ereply;
        if (!row) { // the first item need to update roles
            q->beginResetModel();
            _attachedData.insert(data);
            _data.append(value);
            syncRoles();
            q->endResetModel();
        } else {
            q->beginInsertRows(QModelIndex(), _data.count(), _data.count());
            _attachedData.insert(data);
            _data.append(value);
            q->endInsertRows();
        }
        _attachedData.insertRequestId(ereply->requestId(), row);
        return ereply;
    }

    struct SwapNetworkReplyBase
    {
        EnginioReplyBase *_reply;
        EnginioModelBasePrivate *_model;
        QJsonObject _object;
        QString _tmpId;
        QPointer<EnginioModelBase> _modelGuard;

        void markAsError(QByteArray msg)
        {
            EnginioFakeReply *nreply = new EnginioFakeReply(_reply, EnginioClientBasePrivate::constructErrorMessage(msg));
            _reply->setNetworkReply(nreply);
        }

        QPair<QString, int> getAndSetCurrentIdRow(EnginioReplyBase *finishedCreateReply)
        {
            QString id = _model->replyData(finishedCreateReply)[EnginioString::id].toString();
            Q_ASSERT(!id.isEmpty());
            _object[EnginioString::id] = id;
            int row = InvalidRow;
            if (Q_LIKELY(_model->_attachedData.contains(_tmpId)))
                row = _model->_attachedData.deref(_tmpId).row;
            else if (Q_LIKELY(_model->_attachedData.contains(id))) {
                // model reset happend in a mean while
                row = _model->_attachedData.rowFromObjectId(id);
            } else {
                // the model was reset, probably with a different query, beacause
                // we have no sign of the id.
            }
            return qMakePair(id, row);
        }

        void swapNetworkReply(EnginioReplyBase *ereply)
        {
            _reply->swapNetworkReply(ereply);
            ereply->deleteLater();
        }
    };

    struct SwapNetworkReplyForRemove
    {
        SwapNetworkReplyBase d;
        EnginioReplyBase *finishedCreateReply;
        void operator ()()
        {
            if (finishedCreateReply->isError()) {
                d.markAsError(EnginioString::Dependent_create_query_failed_so_object_could_not_be_removed);
            } else if (Q_UNLIKELY(!d._modelGuard)) {
                d.markAsError(EnginioString::EnginioModel_was_removed_before_this_request_was_prepared);
            } else {
                QPair<QString, int> tmp = d.getAndSetCurrentIdRow(finishedCreateReply);
                const int row = tmp.second;
                if (Q_UNLIKELY(row == InvalidRow)) {
                    d.markAsError(EnginioString::EnginioModel_The_query_was_changed_before_the_request_could_be_sent);
                    return;
                }
                QString id = tmp.first;
                FinishedRemoveRequest finishedRequest = { d._model, id, d._reply };
                QObject::connect(d._reply, &EnginioReplyBase::dataChanged, finishedRequest);
                EnginioReplyBase *ereply = d._model->removeNow(row, d._object, id);
                d.swapNetworkReply(ereply);
            }
        }
    };

    EnginioReplyBase *remove(int row)
    {
        QJsonObject oldObject = _data.at(row).toObject();
        QString id = oldObject[EnginioString::id].toString();
        if (id.isEmpty())
            return removeDelayed(row, oldObject);
        return removeNow(row, oldObject, id);
    }

    EnginioReplyBase *removeDelayed(int row, const QJsonObject &oldObject)
    {
        // We are about to remove a not synced new item. The item do not have id yet,
        // so we can not make a request now, we need to wait for finished signal.
        EnginioReplyBase *ereply, *createReply;
        QString tmpId;
        Q_ASSERT(oldObject[EnginioString::id].toString().isEmpty());
        delayedOperation(row, &ereply, &tmpId, &createReply);
        SwapNetworkReplyForRemove swapNetworkReply = {{ereply, this, oldObject, tmpId, q}, createReply};
        QObject::connect(createReply, &EnginioReplyBase::dataChanged, swapNetworkReply);
        return ereply;
    }

    EnginioReplyBase *removeNow(int row, const QJsonObject &oldObject, const QString &id)
    {
        Q_ASSERT(!id.isEmpty());
        _attachedData.ref(id, row); // TODO if refcount is > 1 then do not emit dataChanged
        ObjectAdaptor<QJsonObject> aOldObject(oldObject);
        QNetworkReply *nreply = _enginio->remove(aOldObject, _operation);
        EnginioReplyBase *ereply = _enginio->createReply(nreply);
        FinishedRemoveRequest finishedRequest = { this, id, ereply };
        _repliesConnections.insert(ereply, QObject::connect(ereply, &EnginioReplyBase::dataChanged, finishedRequest));
        _attachedData.insertRequestId(ereply->requestId(), row);
        QVector<int> roles(1);
        roles.append(EnginioModel::SyncedRole);
        emit q->dataChanged(q->index(row), q->index(row) , roles);
        return ereply;
    }

    EnginioReplyBase *setValue(int row, const QString &role, const QVariant &value)
    {
        int key = _roles.key(role, EnginioModel::InvalidRole);
        return setData(row, value, key);
    }

    EnginioClientBase::Operation operation() const Q_REQUIRED_RESULT
    {
        return _operation;
    }

    void setOperation(const int operation)
    {
        Q_ASSERT_X(operation >= EnginioClientBase::ObjectOperation, "setOperation", "Invalid operation specified.");
        _operation = static_cast<EnginioClientBase::Operation>(operation);
        emit q->operationChanged(_operation);
    }

    void execute()
    {
        if (!_enginio || _enginio->_backendId.isEmpty())
            return;
        if (!queryIsEmpty()) {
            // setup notifications
            QJsonObject filter;
            QJsonObject objectType;
            objectType.insert(EnginioString::objectType, queryData(EnginioString::objectType));
            filter.insert(EnginioString::data, objectType);
            _notifications.connectToBackend(this, _enginio, filter);

            // send full query
            QJsonObject query = queryAsJson();
            ObjectAdaptor<QJsonObject> aQuery(query);
            QNetworkReply *nreply = _enginio->query(aQuery, static_cast<EnginioClientBasePrivate::Operation>(_operation));
            EnginioReplyBase *ereply = _enginio->createReply(nreply);
            if (_canFetchMore)
                _latestRequestedOffset = query[EnginioString::limit].toDouble();
            FinishedFullQueryRequest finshedRequest = { this, ereply };
            _repliesConnections.insert(ereply, QObject::connect(ereply, &EnginioReplyBase::dataChanged, finshedRequest));
            QObject::connect(ereply, &EnginioReplyBase::dataChanged, ereply, &EnginioReplyBase::deleteLater);
        } else {
            fullQueryReset(QJsonArray());
        }
    }

    void finishedIncrementalUpdateRequest(const EnginioReplyBase *reply, const QJsonObject &query)
    {
        _repliesConnections.remove(reply);
        Q_ASSERT(_canFetchMore);
        QJsonArray data(replyData(reply)[EnginioString::results].toArray());
        int offset = query[EnginioString::offset].toDouble();
        int limit = query[EnginioString::limit].toDouble();
        int dataCount = data.count();

        int startingOffset = qMax(offset, _data.count());

        q->beginInsertRows(QModelIndex(), startingOffset, startingOffset + dataCount -1);
        for (int i = 0; i < dataCount; ++i) {
            _data.append(data[i]);
        }

        _canFetchMore = limit <= dataCount;
        q->endInsertRows();
    }

    void finishedFullQueryRequest(const EnginioReplyBase *reply)
    {
        _repliesConnections.remove(reply);
        fullQueryReset(replyData(reply)[EnginioString::results].toArray());
    }

    void fullQueryReset(const QJsonArray &data);

    void finishedCreateRequest(const EnginioReplyBase *reply, const QString &tmpId)
    {
        _repliesConnections.remove(reply);
        if (_attachedData.markRequestIdAsHandled(reply->requestId()))
            return; // request was handled

        int row;
        if (_attachedData.contains(tmpId))
            // this is a common path, we got result of our create request and we still have a dummy
            // item that we want to update.
            row = _attachedData.deref(tmpId).row;
        else {
            // the dummy object doesn't exist anymore, probably it was removed by a full reset
            // or by an initial query.
            QString id = replyData(reply)[EnginioString::id].toString();
            if (_attachedData.contains(id)) {
                // The reset removed the dummy value but it contained the newly created (initial reset
                // and append were reordered)
                row = _attachedData.rowFromObjectId(id);
            } else {
                // we created the item but there is no sign of it. We need to check if we have more or
                // less the same query
                if (queryData(EnginioString::objectType) == replyData(reply)[EnginioString::objectType]) {
                    // the type is the same so we can re-add it
                    receivedCreateNotification(replyData(reply));
                }
                // query was changed too much we are done.
                return;
            }
        }

        if (reply->networkError() != QNetworkReply::NoError) {
            // We tried to create something and we failed, we need to remove tmp
            // item

            // TODO add a signal here so a developer can ask an user for a conflict
            // resolution.
            receivedRemoveNotification(_data[row].toObject(), row);
            return;
        }

        const QJsonObject object = replyData(reply);
        receivedUpdateNotification(object, tmpId, row);
    }

    void finishedRemoveRequest(const EnginioReplyBase *response, const QString &id)
    {
        _repliesConnections.remove(response);
        AttachedData &data = _attachedData.deref(id);

        if (_attachedData.markRequestIdAsHandled(response->requestId()))
            return; // request was handled

        int row = data.row;
        if (row == DeletedRow || (response->networkError() != QNetworkReply::NoError && response->backendStatus() != 404)) {
            if (!data.ref) {
                // The item was not removed, because of an error. We assume that the
                // item is in sync
                emit q->dataChanged(q->index(row), q->index(row));
            }
            return;
        }
        receivedRemoveNotification(_data[row].toObject(), row);
    }

    void finishedUpdateRequest(const EnginioReplyBase *reply, const QString &id, const QJsonObject &oldValue)
    {
        _repliesConnections.remove(reply);
        AttachedData &data = _attachedData.deref(id);

        if (_attachedData.markRequestIdAsHandled(reply->requestId()))
            return; // request was handled

        int row = data.row;
        if (row == DeletedRow) {
            // We tried to update something that we already deleted
            // everything should be handled
            return;
        }
        if (reply->networkError() != QNetworkReply::NoError) {
            if (reply->backendStatus() == 404) {
                // We tried to update something that got deleted in between, probably on
                // the server side. Changing operation type to remove, so the cache
                // can be in sync with the server again.

                // TODO add a signal here so a developer can ask an user for a conflict
                // resolution.
                receivedRemoveNotification(_data[row].toObject(), row);
            } else {
                // Try to rollback the change.
                // TODO it is not perfect https://github.com/enginio/enginio-qt/issues/200
                _data.replace(row, oldValue);
                emit q->dataChanged(q->index(row), q->index(row));
            }
            return;
        }
        receivedUpdateNotification(replyData(reply), id, row);
    }

    struct SwapNetworkReplyForSetData
    {
        SwapNetworkReplyBase d;
        QVariant _value;
        int _role;
        EnginioReplyBase *finishedCreateReply;

        void operator ()()
        {

            if (finishedCreateReply->isError()) {
                d.markAsError(EnginioString::Dependent_create_query_failed_so_object_could_not_be_updated);
            } else if (Q_UNLIKELY(!d._modelGuard)) {
                d.markAsError(EnginioString::EnginioModel_was_removed_before_this_request_was_prepared);
            } else {
                QPair<QString, int> tmp = d.getAndSetCurrentIdRow(finishedCreateReply);
                const int row = tmp.second;
                if (Q_UNLIKELY(row == InvalidRow)) {
                    d.markAsError(EnginioString::EnginioModel_The_query_was_changed_before_the_request_could_be_sent);
                    return;
                }
                QString id = tmp.first;
                FinishedUpdateRequest finished = { d._model, id, d._object, d._reply };
                QObject::connect(d._reply, &EnginioReplyBase::dataChanged, finished);
                EnginioReplyBase *ereply = d._model->setDataNow(row, _value, _role, d._object, id);
                d.swapNetworkReply(ereply);
            }
        }
    };

    EnginioReplyBase *setData(const int row, const QVariant &value, int role)
    {
        if (role > EnginioModel::SyncedRole) {
            QJsonObject oldObject = _data.at(row).toObject();
            QString id = oldObject[EnginioString::id].toString();
            if (id.isEmpty())
                return setDataDelyed(row, value, role, oldObject);
            return setDataNow(row, value, role, oldObject, id);
        }
        QNetworkReply *nreply = new EnginioFakeReply(_enginio, EnginioClientBasePrivate::constructErrorMessage(EnginioString::EnginioModel_Trying_to_update_an_object_with_unknown_role));
        EnginioReplyBase *ereply = _enginio->createReply(nreply);
        return ereply;
    }

    void delayedOperation(int row, EnginioReplyBase **newReply, QString *tmpId, EnginioReplyBase **createReply)
    {
        Q_ASSERT(!_attachedData.isSynced(row));
        AttachedData data = _attachedData.ref(row);
        *createReply = data.createReply;
        Q_ASSERT(*createReply);
        *tmpId = data.id;
        Q_ASSERT(tmpId->startsWith('t'));
        EnginioDummyReply *nreply = new EnginioDummyReply(*createReply);
        *newReply = _enginio->createReply(nreply);
    }

    EnginioReplyBase *setDataDelyed(int row, const QVariant &value, int role, const QJsonObject &oldObject)
    {
        // We are about to update a not synced new item. The item do not have id yet,
        // so we can not make a request now, we need to wait for finished signal.
        Q_ASSERT(role > EnginioModel::SyncedRole);
        EnginioReplyBase *ereply, *createReply;
        QString tmpId;
        Q_ASSERT(oldObject[EnginioString::id].toString().isEmpty());
        delayedOperation(row, &ereply, &tmpId, &createReply);
        SwapNetworkReplyForSetData swapNetworkReply = {{ereply, this, oldObject, tmpId, q}, value, role, createReply};
        QObject::connect(createReply, &EnginioReplyBase::dataChanged, swapNetworkReply);
        return ereply;
    }

    EnginioReplyBase *setDataNow(const int row, const QVariant &value, int role, const QJsonObject &oldObject, const QString &id)
    {
        Q_ASSERT(!id.isEmpty());
        Q_ASSERT(role > EnginioModel::SyncedRole);
        const QString roleName(_roles.value(role));
        QJsonObject deltaObject;
        QJsonObject newObject = oldObject;
        deltaObject[roleName] = newObject[roleName] = QJsonValue::fromVariant(value);
        deltaObject[EnginioString::id] = id;
        deltaObject[EnginioString::objectType] = newObject[EnginioString::objectType];
        ObjectAdaptor<QJsonObject> aDeltaObject(deltaObject);
        QNetworkReply *nreply = _enginio->update(aDeltaObject, _operation);
        EnginioReplyBase *ereply = _enginio->createReply(nreply);
        FinishedUpdateRequest finished = { this, id, oldObject, ereply };
        _repliesConnections.insert(ereply, QObject::connect(ereply, &EnginioReplyBase::dataChanged, finished));
        _attachedData.ref(id, row);
        _data.replace(row, newObject);
        _attachedData.insertRequestId(ereply->requestId(), row);
        emit q->dataChanged(q->index(row), q->index(row));
        return ereply;
    }

    void syncRoles();

    QHash<int, QByteArray> roleNames() const Q_REQUIRED_RESULT
    {
        QHash<int, QByteArray> roles;
        roles.reserve(_roles.count());
        for (QHash<int, QString>::const_iterator i = _roles.constBegin();
             i != _roles.constEnd();
             ++i) {
            roles.insert(i.key(), i.value().toUtf8());
        }
        return roles;
    }

    int rowCount() const Q_REQUIRED_RESULT
    {
        return _data.count();
    }

    QVariant data(unsigned row, int role) const Q_REQUIRED_RESULT
    {
        if (role == EnginioModel::SyncedRole) {
            return _attachedData.isSynced(row);
        }

        if (role == Qt::DisplayRole)
            return _data.at(row);

        const QJsonObject object = _data.at(row).toObject();
        if (!object.isEmpty()) {
            const QString roleName = _roles.value(role);
            if (!roleName.isEmpty())
                return object[roleName];
        }

        return QVariant();
    }

    bool canFetchMore() const Q_REQUIRED_RESULT
    {
        return _canFetchMore;
    }

    void fetchMore(int row)
    {
        int currentOffset = _data.count();
        if (!_canFetchMore || currentOffset < _latestRequestedOffset)
            return; // we do not want to spam the server, lets wait for the last fetch

        QJsonObject query(queryAsJson());

        int limit = query[EnginioString::limit].toDouble();
        limit = qMax(row - currentOffset, limit); // check if default limit is not too small

        query[EnginioString::offset] = currentOffset;
        query[EnginioString::limit] = limit;

        qDebug() << Q_FUNC_INFO << query;
        _latestRequestedOffset += limit;
        ObjectAdaptor<QJsonObject> aQuery(query);
        QNetworkReply *nreply = _enginio->query(aQuery, static_cast<EnginioClientBasePrivate::Operation>(_operation));
        EnginioReplyBase *ereply = _enginio->createReply(nreply);
        QObject::connect(ereply, &EnginioReplyBase::dataChanged, ereply, &EnginioReplyBase::deleteLater);
        FinishedIncrementalUpdateRequest finishedRequest = { this, query, ereply };
        _repliesConnections.insert(ereply, QObject::connect(ereply, &EnginioReplyBase::dataChanged, finishedRequest));
    }

    virtual QJsonObject replyData(const EnginioReplyBase *reply) const = 0;
    virtual QJsonValue queryData(const QString &name) = 0;
    virtual bool queryIsEmpty() const = 0;
    virtual QJsonObject queryAsJson() const = 0;
};


template<typename Derived, typename Types>
struct EnginioModelPrivateT : public EnginioModelBasePrivate
{
    typedef EnginioModelBasePrivate Base;
    typedef typename Types::Reply Reply;
    typedef typename Types::Public Public;
    typedef typename Types::Client Client;
    typedef typename Types::ClientPrivate ClientPrivate;
    typedef typename Types::Data Data;

    Data _query;

    inline Public *q() const { return static_cast<Public*>(Base::q); }

    class EnginioDestroyed
    {
        EnginioModelPrivateT *model;
    public:
        EnginioDestroyed(EnginioModelPrivateT *m)
            : model(m)
        {
            Q_ASSERT(m);
        }
        void operator ()()
        {
            model->setEnginio(0);
        }
    };

    EnginioModelPrivateT(EnginioModelBase *pub)
        : Base(pub)
    {}

    void init()
    {
        Base::init();
        QObject::connect(q(), &Public::queryChanged, QueryChanged(this));
        QObject::connect(q(), &Public::enginioChanged, QueryChanged(this));
    }

    Client *enginio() const Q_REQUIRED_RESULT
    {
        return _enginio ? ClientPrivate::get(_enginio) : 0;
    }

    void setEnginio(const EnginioClientBase *enginio)
    {
        if (_enginio) {
            foreach (const QMetaObject::Connection &connection, _clientConnections)
                QObject::disconnect(connection);
            _clientConnections.clear();
        }
        if (enginio) {
            _enginio = EnginioClientBasePrivate::get(const_cast<EnginioClientBase*>(enginio));
            _clientConnections.append(QObject::connect(enginio, &QObject::destroyed, EnginioDestroyed(this)));
            _clientConnections.append(QObject::connect(enginio, &EnginioClientBase::backendIdChanged, QueryChanged(this)));
        } else {
            _enginio = 0;
        }

        q()->enginioChanged(static_cast<Client*>(const_cast<EnginioClientBase*>(enginio)));
    }

    Data query() Q_REQUIRED_RESULT
    {
        return _query;
    }

    void setQuery(const Data &query)
    {
        _query = query;

        // TODO Enable together with pageing support
//        if (_query.contains(EnginioString::pageSize)) {
//            const int pageSize = _query[EnginioString::pageSize].toDouble();
//            const QString limitString(EnginioString::limit);
//            const QString offsetString(EnginioString::offset);
//            const unsigned limit = _query[limitString].toDouble();
//            const unsigned offset = _query[offsetString].toDouble();
//            if (limit)
//                qWarning() << "EnginioModel::setQuery()" << "'limit' parameter can not be used together with model pagining feature, the value will be ignored";

//            if (offset) {
//                qWarning() << "EnginioModel::setQuery()" << "'offset' parameter can not be used together with model pagining feature, the value will be ignored";
//                _query.remove(offsetString);
//            }
//            _query[limitString] = pageSize;
//            _canFetchMore = true;
//        } else {
//            _canFetchMore = false;
//        }
        emit q()->queryChanged(query);
    }

    Reply *append(const QJsonObject &value) { return static_cast<Reply*>(Base::append(value)); }
    Reply *remove(int row) { return static_cast<Reply*>(Base::remove(row)); }
    Reply *setValue(int row, const QString &role, const QVariant &value) { return static_cast<Reply*>(Base::setValue(row, role, value)); }

    bool queryIsEmpty() const Q_DECL_OVERRIDE
    {
        return ObjectAdaptor<Data>(_query, static_cast<ClientPrivate*>(_enginio)).isEmpty();
    }
};

QT_END_NAMESPACE
#endif // ENGINIOMODELBASE_P_H
