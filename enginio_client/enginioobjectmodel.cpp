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

#include "enginioobjectmodel_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class EnginioObjectModel
 * \inmodule enginio-client
 * \brief List model for objects derived from EnginioAbstractObject
 */

EnginioObjectModelPrivate::EnginioObjectModelPrivate(EnginioObjectModel *model) :
    q_ptr(model)
{
}

EnginioObjectModelPrivate::~EnginioObjectModelPrivate()
{
    while (!m_objects.isEmpty())
        delete m_objects.takeFirst();
}


EnginioObjectModel::EnginioObjectModel(QObject *parent) :
    QAbstractListModel(parent),
    d_ptr(new EnginioObjectModelPrivate(this))
{
    qDebug() << this << "created";
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioObjectModel::EnginioObjectModel(EnginioObjectModelPrivate &dd,
                                       QObject *parent) :
    QAbstractListModel(parent),
    d_ptr(&dd)
{
    qDebug() << this << "created";
}

/*!
 * Destructor.
 */
EnginioObjectModel::~EnginioObjectModel()
{
    qDebug() << this << "deleted";
    delete d_ptr;
}

int EnginioObjectModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    Q_D(const EnginioObjectModel);
    return d->m_objects.size();
}

QModelIndex EnginioObjectModel::index(int row, int column,
                                      const QModelIndex &parent) const
{
    Q_UNUSED(column)
    Q_UNUSED(parent)
    Q_D(const EnginioObjectModel);
    if (row < 0 || row >= d->m_objects.size())
        return QModelIndex();

    return createIndex(row, column, d->m_objects[row]);
}

QVariant EnginioObjectModel::data(int row, int role) const
{
    QModelIndex modelIndex = index(row, 0);
    if (!modelIndex.isValid())
        return QVariant();

    return data(modelIndex, role);
}

QVariant EnginioObjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EnginioAbstractObject *object = static_cast<EnginioAbstractObject*>(
                index.internalPointer());

    if (role == Enginio::DataRole)
        return QVariant::fromValue(object);
    if (role == Enginio::JsonRole)
        return QVariant(QJsonDocument::fromJson(object->toEnginioJson())
                        .object().toVariantMap());
    if (role == Qt::DisplayRole)
        return QVariant::fromValue(object->id());

    return QVariant();
}

bool EnginioObjectModel::setData (const QModelIndex &index,
                                  const QVariant &value,
                                  int role)
{
    Q_D(EnginioObjectModel);

    if (!index.isValid() || index.row() >= rowCount())
        return false;

    if (role == Enginio::DataRole) {
        if (!value.canConvert<EnginioAbstractObject*>())
            return false;

        EnginioAbstractObject *object = qvariant_cast<EnginioAbstractObject*>(
                    value);
        d->m_objects[index.row()]->fromEnginioJson(
                    QJsonDocument::fromJson(object->toEnginioJson()).object());

        QVector<int> changedRoles = QVector<int>() << Enginio::DataRole <<
                                                      Enginio::JsonRole;
        emit dataChanged(index, index, changedRoles);
    }

    return true;
}

Qt::ItemFlags EnginioObjectModel::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable & Qt::ItemIsEditable;
}

/*!
 * Add objects in \a objects to the model starting on row \a row. If any objects
 * are added to model successfully \c rowsInserted signal is emitted. Please
 * note that model takes the ownership of the objects in \a objects. If \a row
 * is negative, objects will be added to the end of the model. If all objects
 * are inserted successfully this method will return true.
 */
bool EnginioObjectModel::addToModel(QList<EnginioAbstractObject*> objects, int row)
{
    Q_D(EnginioObjectModel);
    qDebug() << this << Q_FUNC_INFO << "row:" << row << "; count:" << objects.size();

    int numObjects = d->m_objects.size();
    if (objects.isEmpty())
        return false;
    if (row > numObjects)
        return false;
    if (row < 0)
        row = numObjects;

    beginInsertRows(QModelIndex(), row, row + objects.size() - 1);
    for (int i = 0; i < objects.size(); ++i) {
        d->m_objects.insert(row + i, objects[i]);
    }
    endInsertRows();

    if (d->m_objects.size() != numObjects)
        emit rowCountChanged(d->m_objects.size());

    return true;
}

/*!
 * Remove \a count rows from model starting from row \a row. If any rows are
 * removed successfully, \c rowsRemoved signal will be emitted. If all \a count
 * rows were removed successfully, this method will return true.
 */
bool EnginioObjectModel::removeFromModel(int row, int count)
{
    Q_D(EnginioObjectModel);
    qDebug() << this << Q_FUNC_INFO << row << count;

    int numObjects = d->m_objects.size();
    if (row >= numObjects)
        return false;

    int removed = 0;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    while (removed < count) {
        EnginioAbstractObject *obj = d->m_objects.takeAt(row);
        delete obj;
        removed++;
        if (row >= d->m_objects.size())
            break;
    }
    endRemoveRows();

    if (d->m_objects.size() != numObjects)
        emit rowCountChanged(d->m_objects.size());

    return removed == count;
}

/*!
 * Remove all rows from model. Emits \c modelReset signal and returns true if
 * all rows were removed successfully.
 */
bool EnginioObjectModel::clear()
{
    Q_D(EnginioObjectModel);
    qDebug() << this << "Model cleared";

    beginResetModel();
    d->m_objects.clear();
    endResetModel();

    return d->m_objects.size() == 0;
}

/*!
 * Find object from model with ID \a id. Returns index to found object or
 * invalid index if object was not found.
 */
QModelIndex EnginioObjectModel::indexFromId(const QString &id) const
{
    Q_D(const EnginioObjectModel);

    for (int row = 0; row < d->m_objects.size(); row++) {
        EnginioAbstractObject *obj = d->m_objects[row];
        if (obj->id() == id)
            return index(row);
    }
    return QModelIndex();
}

/*!
 * Get object in model from index \a index. Returns object from specified index
 * or null pointer if index is invalid. Please note that returned object is
 * owned by model and will be deleted with it.
 */
EnginioAbstractObject * EnginioObjectModel::getObject(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        qWarning() << "invalid index";
        return 0;
    }

    EnginioAbstractObject *object = 0;
    if (index.internalPointer())
        object = static_cast<EnginioAbstractObject*>(index.internalPointer());

    return object;
}
