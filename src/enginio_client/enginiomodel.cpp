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

#include "enginiomodel.h"
#include "enginioreply.h"
#include "enginioclient_p.h"
#include "enginiofakereply_p.h"
#include "enginiodummyreply_p.h"
#include "enginiobackendconnection_p.h"
#include "enginiomodelbase.h"
#include "enginiomodelbase_p.h"

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

QT_BEGIN_NAMESPACE

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

  \note that the EnginioClient does emit the finished and error signals for the model.

  For the QML version of this class see \l {Enginio1::EnginioModel}{EnginioModel (QML)}
*/

namespace  {

struct EnginioModelPrivate1: EnginioModelPrivateT<EnginioModelPrivate1>
{
    typedef EnginioModelPrivateT<EnginioModelPrivate1> Base;
    EnginioModelPrivate1(EnginioModelBase *pub)
        : Base(pub)
    {}

    inline EnginioModel *q() const { return static_cast<EnginioModel*>(Base::q); }

    void emitEnginioChanged(EnginioClientBase *enginio)
    {
        emit q()->enginioChanged(static_cast<EnginioClient*>(enginio));
    }

    void emitQueryChanged(const QJsonObject &query)
    {
        emit q()->queryChanged(query);
    }

    void init()
    {
        QObject::connect(q(), &EnginioModel::queryChanged, QueryChanged(this));
        QObject::connect(q(), &EnginioModel::enginioChanged, QueryChanged(this));
    }
};

} // namespace


#define E_D() EnginioModelPrivate1 *d = static_cast<EnginioModelPrivate1*>(EnginioModelBase::d.data());

/*!
    Constructs a new model with \a parent as QObject parent.
*/
EnginioModel::EnginioModel(QObject *parent)
    : EnginioModelBase(parent, new EnginioModelPrivate1(this))
{
    E_D();
    d->init();
}

/*!
    Destroys the model.
*/
EnginioModel::~EnginioModel()
{}

/*!
    \internal
    Constructs a new model with \a parent as QObject parent.
*/
EnginioModelBase::EnginioModelBase(QObject *parent, EnginioModelPrivate *d_ptr)
    : QAbstractListModel(parent)
    , d(d_ptr)
{}

/*!
    Destroys the model.
*/
EnginioModelBase::~EnginioModelBase()
{}

/*!
  \enum EnginioModel::Roles

  EnginioModel defines roles which represent data used by every object
  stored in the Enginio backend

  \value CreatedAtRole \c When an item was created
  \value UpdatedAtRole \c When an item was updated last time
  \value IdRole \c What is the id of an item
  \value ObjectTypeRole \c What is item type
  \value SyncedRole \c Mark if an item is in sync with the backend
  \omitvalue InvalidRole
  \omitvalue LastRole

  Additionally EnginioModel supports dynamic roles which are mapped
  directly from recieved data. EnginioModel is mapping first item properties
  to role names.

  \note Some objects may not contain value for a static role, it may happen
  for example when an item is not in sync with the backend.

  \sa EnginioModelBase::roleNames()
*/

/*!
  \property EnginioModel::enginio
  \brief The EnginioClient used by the model.

  \sa EnginioClient
*/
EnginioClient *EnginioModel::enginio() const
{
    E_D();
    return d->enginio();
}

void EnginioModel::setEnginio(const EnginioClient *enginio)
{
    E_D();
    if (enginio == d->enginio())
        return;
    d->setEnginio(enginio);
}

/*!
  \property EnginioModel::query
  \brief The query which returns the data for the model.

  Sorting preserved until insertion/deletion

  \sa EnginioClient::query()
*/
QJsonObject EnginioModel::query()
{
    E_D();
    return d->query();
}

void EnginioModel::setQuery(const QJsonObject &query)
{
    E_D();
    if (d->query() == query)
        return;
    return d->setQuery(query);
}

/*!
  \property EnginioModelBase::operation
  \brief The operation type of the query
  \sa EnginioClientBase::Operation, query()
  \return returns the Operation
*/
EnginioClientBase::Operation EnginioModelBase::operation() const
{
    E_D();
    return d->operation();
}

void EnginioModelBase::setOperation(EnginioClientBase::Operation operation)
{
    E_D();
    if (operation == d->operation())
        return;
    d->setOperation(operation);
}

/*!
  Append \a value to this model local cache and send a create request
  to enginio backend.
  \return reply from backend
  \sa EnginioClient::create()
*/
EnginioReply *EnginioModel::append(const QJsonObject &value)
{
    E_D();
    if (Q_UNLIKELY(!d->enginio())) {
        qWarning("EnginioModel::append(): Enginio client is not set");
        return 0;
    }

    return d->append(value);
}

/*!
  Remove a value from \a row in this model local cache and send
  a remove request to enginio backend.
  \return reply from backend
  \sa EnginioClient::remove()
*/
EnginioReply *EnginioModel::remove(int row)
{
    E_D();
    if (Q_UNLIKELY(!d->enginio())) {
        qWarning("EnginioModel::remove(): Enginio client is not set");
        return 0;
    }

    if (unsigned(row) >= unsigned(d->rowCount())) {
        EnginioClientPrivate *client = EnginioClientPrivate::get(d->enginio());
        QNetworkReply *nreply = new EnginioFakeReply(client, constructErrorMessage(QByteArrayLiteral("EnginioModel::remove: row is out of range")));
        EnginioReply *ereply = new EnginioReply(client, nreply);
        return ereply;
    }

    return d->remove(row);
}

/*!
  Update a value on \a row of this model's local cache
  and send an update request to the Enginio backend.

  The \a role is the property of the object that will be updated to be the new \a value.

  \return reply from backend
  \sa EnginioClient::update()
*/
EnginioReply *EnginioModel::setProperty(int row, const QString &role, const QVariant &value)
{
    E_D();
    if (Q_UNLIKELY(!d->enginio())) {
        qWarning("EnginioModel::setProperty(): Enginio client is not set");
        return 0;
    }

    if (unsigned(row) >= unsigned(d->rowCount())) {  // TODO remove as soon as we have a sparse array.
        EnginioClientPrivate *client = EnginioClientPrivate::get(d->enginio());
        QNetworkReply *nreply = new EnginioFakeReply(client, constructErrorMessage(QByteArrayLiteral("EnginioModel::setProperty: row is out of range")));
        EnginioReply *ereply = new EnginioReply(client, nreply);
        return ereply;
    }

    return d->setValue(row, role, value);
}

/*!
    \overload
    \internal
*/
Qt::ItemFlags EnginioModelBase::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

/*!
    \overload
    Use this function to access the model data at \a index.
    With the \l roleNames() function the mapping of JSON property names to data roles used as \a role is available.
    The data returned will be JSON (for example a string for simple objects, or a JSON Object).
*/
QVariant EnginioModelBase::data(const QModelIndex &index, int role) const
{
    E_D();
    if (!index.isValid() || index.row() < 0 || index.row() >= d->rowCount())
        return QVariant();

    return d->data(index.row(), role);
}

/*!
    \overload
    \internal
*/
int EnginioModelBase::rowCount(const QModelIndex &parent) const
{
    E_D();
    Q_UNUSED(parent);
    return d->rowCount();
}

/*!
    \overload
    \internal
*/
bool EnginioModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    E_D();
    if (index.row() >= d->rowCount()) // TODO remove as soon as we have a sparse array.
        return false;

    EnginioReply *reply = d->setData(index.row(), value, role);
    QObject::connect(reply, &EnginioReply::finished, reply, &EnginioReply::deleteLater);
    return true;
}

/*!
    \overload
    Returns the mapping of the model's roles to names. Use this function to map
    the object property names to the role integers.

    EnginioModel uses heuristics to assign the properties of the objects in the \l query()
    to roles (greater than \l Qt::UserRole). Sometimes if the objects do not share
    the same structure, if for example a property is missing, it may happen that
    a role is missing. In such cases we recommend to overload this method to
    enforce existence of all required roles.

    \note when reimplementating this function, you need to call the base class implementation first and
    take the result into account as shown in the {todos-cpp}{Todos Example}
    \note custom role indexes have to be greater then or equal to \l EnginioModel::LastRole
*/
QHash<int, QByteArray> EnginioModelBase::roleNames() const
{
    E_D();
    return d->roleNames();
}

/*!
    \internal
    Allows to disable notifications for autotests.
*/
void EnginioModelBase::disableNotifications()
{
    E_D();
    d->disableNotifications();
}

/*!
    \overload
    \internal
*/
void EnginioModelBase::fetchMore(const QModelIndex &parent)
{
    E_D();
    d->fetchMore(parent.row());
}

/*!
    \overload
    \internal
*/
bool EnginioModelBase::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    E_D();
    return d->canFetchMore();
}

QT_END_NAMESPACE
