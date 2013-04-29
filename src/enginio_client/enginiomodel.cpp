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

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

class EnginioModelPrivate {
    QJsonObject _query;
    EnginioClient *_enginio;
    EnginioClient::Area _area;
    EnginioModel *q;
    QVector<QMetaObject::Connection> _connections;

    const static int FullModelReset;
    mutable QMap<const EnginioReply*, QPair<int /*row*/, QJsonObject> > _dataChanged;

    const static int SyncedRole = Qt::UserRole + 1;
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
        , _area()
        , q(q_ptr)
    {}

    EnginioClient *enginio() const
    {
        return _enginio;
    }

    void setEnginio(const EnginioClient *enginio)
    {
        if (_enginio) {
            foreach(const QMetaObject::Connection &connection, _connections)
                QObject::disconnect(connection);
        }
        _enginio = const_cast<EnginioClient*>(enginio);
        if (_enginio) {
            _connections.append(QObject::connect(_enginio, &EnginioClient::finished, FinishedRequest(this)));
            _connections.append(QObject::connect(_enginio, &QObject::destroyed, EnginioDestroyed(this)));
        }
        emit q->enginioChanged(_enginio);
    }

    QJsonObject query()
    {
        return _query;
    }

    void setQuery(const QJsonObject &query)
    {
        _query = query;
        emit q->queryChanged(query);
    }

    EnginioClient::Area area() const
    {
        return _area;
    }

    void setArea(const int area)
    {
        _area = static_cast<EnginioClient::Area>(area);
        emit q->areaChanged(_area);
    }

    void execute()
    {
        if (!_query.isEmpty()) {
            const EnginioReply *id = _enginio->query(_query, _area);
            _dataChanged.insert(id, qMakePair(FullModelReset, QJsonObject()));
        }
        QObject::connect(q, &EnginioModel::queryChanged, QueryChanged(this));
        QObject::connect(q, &EnginioModel::areaChanged, QueryChanged(this));
        QObject::connect(q, &EnginioModel::enginioChanged, QueryChanged(this));
    }

    void finishedRequest(const EnginioReply *response)
    {
        QPair<int, QJsonObject> requestInfo = _dataChanged.take(response);
        int row = requestInfo.first;
        if (row == FullModelReset) {
            q->beginResetModel();
            _data = response->data()[QStringLiteral("results")].toArray();
            syncRoles();
            q->endResetModel();
        } else {
            // TODO update, insert and remove
        }
    }

    void syncRoles()
    {
        // estimate new roles:
        _roles.clear();
        int idx = SyncedRole;
        _roles[idx++] = QStringLiteral("_synced"); // TODO Use a proper name, can we make it an attached property in qml? Does it make sense to try?
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
        const QJsonValue value = _data.at(row);
        const QJsonObject object = value.toObject();
        if (role > Qt::UserRole && !object.isEmpty())
            return object[_roles.value(role)];

        Q_UNIMPLEMENTED();
        return value;
    }
};

const int EnginioModelPrivate::FullModelReset = -1;

EnginioModel::EnginioModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new EnginioModelPrivate(this))
{}

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

EnginioClient::Area EnginioModel::area() const
{
    return d->area();
}

void EnginioModel::setArea(const int area)
{
    if (area == d->area())
        return;
    d->setArea(area);
}
void EnginioModel::execute()
{
    d->execute();
}

Qt::ItemFlags EnginioModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant EnginioModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= d->rowCount())
        return QVariant();

    return d->data(index.row(), role);
}

int EnginioModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->rowCount();
}

QHash<int, QByteArray> EnginioModel::roleNames() const
{
    return d->roleNames();
}
