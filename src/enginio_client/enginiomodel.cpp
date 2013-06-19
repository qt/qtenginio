/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://qt.digia.com/contact-us
**
** This file is part of the Enginio Qt Client Library.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
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
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#include "enginiomodel.h"
#include "enginioreply.h"
#include "enginioclient_p.h"

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>


class EnginioModelPrivate {
    QJsonObject _query;
    EnginioClient *_enginio;
    EnginioClient::Operation _operation;
    EnginioModel *q;
    QVector<QMetaObject::Connection> _connections;

    const static int FullModelReset;
    const static int IncrementalModelUpdate;
    mutable QMap<const EnginioReply*, QPair<int /*row*/, QJsonObject> > _dataChanged;
    QSet<int> _rowsToSync;
    int _latestRequestedOffset;
    bool _canFetchMore;

    enum {
        InvalidRole = -1,
        SyncedRole = Qt::UserRole + 1
    };

    QHash<int, QString> _roles;

    QJsonArray _data; // TODO replace by a sparse array, and add laziness

    class EnginioDestroyed
    {
        EnginioModelPrivate *model;
    public:
        EnginioDestroyed(EnginioModelPrivate *m)
            : model(m)
        {
            Q_ASSERT(m);
        }
        void operator ()()
        {
            model->setEnginio(0);
        }
    };

    class FinishedRequest
    {
        EnginioModelPrivate *model;
    public:
        FinishedRequest(EnginioModelPrivate *m)
            : model(m)
        {
            Q_ASSERT(m);
        }

        void operator ()(const EnginioReply *response)
        {
            model->finishedRequest(response);
        }
    };

    class QueryChanged
    {
        EnginioModelPrivate *model;
    public:
        QueryChanged(EnginioModelPrivate *m)
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
    EnginioModelPrivate(EnginioModel *q_ptr)
        : _enginio(0)
        , _operation()
        , q(q_ptr)
        , _latestRequestedOffset(0)
        , _canFetchMore(false)
    {}

    ~EnginioModelPrivate()
    {
        foreach (const QMetaObject::Connection &connection, _connections)
            QObject::disconnect(connection);
    }

    EnginioClient *enginio() const
    {
        return _enginio;
    }

    void setEnginio(const EnginioClient *enginio)
    {
        if (_enginio) {
            foreach(const QMetaObject::Connection &connection, _connections)
                QObject::disconnect(connection);
            _connections.clear();
        }
        _enginio = const_cast<EnginioClient*>(enginio);
        if (_enginio) {
            _connections.append(QObject::connect(_enginio, &EnginioClient::finished, FinishedRequest(this)));
            _connections.append(QObject::connect(_enginio, &QObject::destroyed, EnginioDestroyed(this)));
            _connections.append(QObject::connect(_enginio, &EnginioClient::backendIdChanged, QueryChanged(this)));
            _connections.append(QObject::connect(_enginio, &EnginioClient::backendSecretChanged, QueryChanged(this)));
            if (!_enginio->backendId().isEmpty() && !_enginio->backendSecret().isEmpty())
                execute();
        }
        emit q->enginioChanged(_enginio);
    }

    QJsonObject query()
    {
        return _query;
    }

    EnginioReply *append(const QJsonObject &value)
    {
        QJsonObject object(value);
        object[EnginioString::objectType] = _query[EnginioString::objectType]; // TODO think about it, it means that not all queries are valid
        q->beginInsertRows(QModelIndex(), _data.count(), _data.count());
        EnginioReply* id = _enginio->create(object, _operation);
        _data.append(value);
        const int row = _data.count() - 1;
        _rowsToSync.insert(row);
        _dataChanged.insert(id, qMakePair(row, object));
        q->endInsertRows();
        return id;
    }

    EnginioReply *remove(int row)
    {
        QJsonObject oldObject = _data.at(row).toObject();
        EnginioReply* id = _enginio->remove(oldObject, _operation);
        _dataChanged.insert(id, qMakePair(row, oldObject));
        QVector<int> roles(1);
        roles.append(SyncedRole);
        emit q->dataChanged(q->index(row), q->index(row) , roles);
        return id;
    }

    EnginioReply *setValue(int row, const QString &role, const QVariant &value)
    {
        int key = _roles.key(role, InvalidRole);
        if (key != InvalidRole) {
            return setData(row, value, key);
        }
        return 0;
    }

    void setQuery(const QJsonObject &query)
    {
        _query = query;

        if (_query.contains(EnginioString::pageSize)) {
            const int pageSize = _query[EnginioString::pageSize].toDouble();
            const QString limitString(EnginioString::limit);
            const QString offsetString(EnginioString::offset);
            const unsigned limit = _query[limitString].toDouble();
            const unsigned offset = _query[offsetString].toDouble();
            if (limit)
                qWarning() << "EnginioModel::setQuery()" << "'limit' parameter can not be used together with model pagining feature, the value will be ignored";

            if (offset) {
                qWarning() << "EnginioModel::setQuery()" << "'offset' parameter can not be used together with model pagining feature, the value will be ignored";
                _query.remove(offsetString);
            }
            _query[limitString] = pageSize;
            _canFetchMore = true;
        } else {
            _canFetchMore = false;
        }
        emit q->queryChanged(query);
    }

    EnginioClient::Operation operation() const
    {
        return _operation;
    }

    void setOperation(const int operation)
    {
        Q_ASSERT_X(operation >= EnginioClient::ObjectOperation, "setOperation", "Invalid operation specified.");
        _operation = static_cast<EnginioClient::Operation>(operation);
        emit q->operationChanged(_operation);
    }

    void execute()
    {
        if (!_enginio || _enginio->backendId().isEmpty() || _enginio->backendSecret().isEmpty())
            return;
        if (!_query.isEmpty()) {
            const EnginioReply *id = _enginio->query(_query, _operation);
            if (_canFetchMore)
                _latestRequestedOffset = _query[EnginioString::limit].toDouble();
            QObject::connect(id, &EnginioReply::finished, id, &EnginioReply::deleteLater);
            _dataChanged.insert(id, qMakePair(FullModelReset, QJsonObject()));
        }
        QObject::connect(q, &EnginioModel::queryChanged, QueryChanged(this));
        QObject::connect(q, &EnginioModel::operationChanged, QueryChanged(this));
        QObject::connect(q, &EnginioModel::enginioChanged, QueryChanged(this));
    }

    void finishedRequest(const EnginioReply *response)
    {
        // We get all finished requests, check if we started this one
        if (!_dataChanged.contains(response))
            return;

        // ### TODO proper error handling
        // this kind of response happens when the backend id/secret is missing
        if (!response->data()[EnginioString::message].isNull())
            qWarning() << "Enginio: " << response->data()[EnginioString::message].toString();

        QPair<int, QJsonObject> requestInfo = _dataChanged.take(response);
        int row = requestInfo.first;
        if (row == FullModelReset) {
            q->beginResetModel();
            _rowsToSync.clear();
            _data = response->data()[EnginioString::results].toArray();
            syncRoles();
            _canFetchMore = _canFetchMore && _data.count() && (_query[EnginioString::limit].toDouble() <= _data.count());
            q->endResetModel();
        } else if (row == IncrementalModelUpdate) {
            Q_ASSERT(_canFetchMore);
            QJsonArray data(response->data()[EnginioString::results].toArray());
            QJsonObject query(requestInfo.second);
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
        } else {
            _rowsToSync.remove(row);
            // TODO update, insert and remove
            Q_ASSERT(row < _data.count());
            // update or insert data
            QJsonObject currentValue = _data.at(row).toObject();
            QJsonObject oldValue = requestInfo.second;
            QJsonObject newValue(response->data());

            if (response->networkError() != QNetworkReply::NoError) {
                _data.replace(row, oldValue); // FIXME do we have to do more here?
                return;
            }

            if (newValue.isEmpty()) {
                q->beginRemoveRows(QModelIndex(), row, row);
                _data.removeAt(row);
                q->endRemoveRows();
            } else {
                _data.replace(row, newValue);
                if (_data.count() == 1) {
                    q->beginResetModel();
                    syncRoles();
                    q->endResetModel();
                } else {
                    emit q->dataChanged(q->index(row), q->index(row));
                }
            }
        }
    }

    EnginioReply *setData(const int row, const QVariant &value, int role)
    {
        if (role > SyncedRole) {
            _rowsToSync.insert(row);
            const QString roleName(_roles.value(role));
            QJsonObject oldObject = _data.at(row).toObject();
            QJsonObject deltaObject;
            QJsonObject newObject = oldObject;
            deltaObject[roleName] = newObject[roleName] = QJsonValue::fromVariant(value);
            deltaObject[QString::fromUtf8("id")] = newObject[QString::fromUtf8("id")];
            deltaObject[QString::fromUtf8("objectType")] = newObject[QString::fromUtf8("objectType")];
            EnginioReply *id = _enginio->update(deltaObject, _operation);
            _dataChanged.insert(id, qMakePair(row, oldObject));
            _data.replace(row, newObject);
            emit q->dataChanged(q->index(row), q->index(row));
            return id;
        }

        Q_UNIMPLEMENTED();
        return 0;
    }

    void syncRoles()
    {
        // estimate new roles:
        _roles.clear();
        int idx = SyncedRole;
        _roles[idx++] = EnginioString::_synced; // TODO Use a proper name, can we make it an attached property in qml? Does it make sense to try?
        QJsonObject firstObject(_data.first().toObject()); // TODO it expects certain data structure in all objects, add way to specify roles
        for (QJsonObject::const_iterator i = firstObject.constBegin(); i != firstObject.constEnd(); ++i) {
            _roles[idx++] = i.key();
        }
    }

    QHash<int, QByteArray> roleNames() const
    {
        // TODO this is not optimal, but happen once, do we want to do something about it?
        QHash<int, QByteArray> roles;
        roles.reserve(_roles.count());
        for (QHash<int, QString>::const_iterator i = _roles.constBegin();
             i != _roles.constEnd();
             ++i) {
            roles.insert(i.key(), i.value().toUtf8());
        }
        return roles;
    }

    int rowCount() const
    {
        return _data.count();
    }

    QVariant data(unsigned row, int role)
    {
        if (role == SyncedRole)
            return !_rowsToSync.contains(row);

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

    bool canFetchMore() const
    {
        return _canFetchMore;
    }

    void fetchMore(int row)
    {
        int currentOffset = _data.count();
        if (!_canFetchMore || currentOffset < _latestRequestedOffset)
            return; // we do not want to spam the server, lets wait for the last fetch

        QJsonObject query(_query);

        int limit = query[EnginioString::limit].toDouble();
        limit = qMax(row - currentOffset, limit); // check if default limit is not too small

        query[EnginioString::offset] = currentOffset;
        query[EnginioString::limit] = limit;

        qDebug() << Q_FUNC_INFO << query;
        _latestRequestedOffset += limit;
        const EnginioReply *id = _enginio->query(query, _operation);
        _dataChanged.insert(id, qMakePair(IncrementalModelUpdate, query));
    }
};

const int EnginioModelPrivate::FullModelReset = -1;
const int EnginioModelPrivate::IncrementalModelUpdate = -2;


/*!
  \class EnginioModel
  \inmodule enginio-qt
  \ingroup enginio-client
  \target EnginioModelCpp
  \brief EnginioModel represents data from Enginio as a \l QAbstractListModel.

  Example of setting a query.

  The query has to result in a list.

  Inside the delegate you have the magic properties model and index.
  Exposes the data as "model".
  index

  Sorting is done server side, as soon as data is changed locally it will be invalid.


  \note that the EnginioClient does not emit the finished and error signals for the model.

  For the QML version of this class see \l {EnginioModelQml}{EnginioModel (QML)}
*/

/*!
    Constructs a new model with \a parent as QObject parent.
*/
EnginioModel::EnginioModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new EnginioModelPrivate(this))
{}

/*!
    Destroys the model.
*/
EnginioModel::~EnginioModel()
{}


/*!
  \property EnginioModel::enginio
  \brief The EnginioClient used by the model.

  \sa EnginioClient
*/
EnginioClient *EnginioModel::enginio() const
{
    return d->enginio();
}

void EnginioModel::setEnginio(const EnginioClient *enginio)
{
    if (enginio == d->enginio())
        return;
    d->setEnginio(enginio);
}

/*!
  \property EnginioModel::query
  \brief The query which returns the data for the model.

  FIXME example

  Sorting preserved until insertion/deletion

  \sa EnginioClient::query()
*/
QJsonObject EnginioModel::query()
{
    return d->query();
}

void EnginioModel::setQuery(const QJsonObject &query)
{
    if (d->query() == query)
        return;
    return d->setQuery(query);
}

EnginioClient::Operation EnginioModel::operation() const
{
    return d->operation();
}

void EnginioModel::setOperation(const int operation)
{
    if (operation == d->operation())
        return;
    d->setOperation(operation);
}

EnginioReply *EnginioModel::append(const QJsonObject &value)
{
    return d->append(value);
}

EnginioReply *EnginioModel::remove(int row)
{
    if (row >= d->rowCount())
        return 0;// FIXME never return 0

    return d->remove(row);
}

EnginioReply *EnginioModel::setProperty(int row, const QString &role, const QVariant &value)
{
    if (row >= d->rowCount())  // TODO remove as soon as we have a sparse array.
        return 0;  // FIXME never return 0

    return d->setValue(row, role, value);
}

Qt::ItemFlags EnginioModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant EnginioModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->rowCount())
        return QVariant();

    return d->data(index.row(), role);
}

int EnginioModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->rowCount();
}

bool EnginioModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= d->rowCount()) // TODO remove as soon as we have a sparse array.
        return false;

    return d->setData(index.row(), value, role);
}

QHash<int, QByteArray> EnginioModel::roleNames() const
{
    return d->roleNames();
}

void EnginioModel::fetchMore(const QModelIndex &parent)
{
    d->fetchMore(parent.row());
}

bool EnginioModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->canFetchMore();
}
