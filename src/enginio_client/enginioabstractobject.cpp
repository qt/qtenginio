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

#include "enginioabstractobject.h"

/*!
 * \class EnginioAbstractObject
 * \inmodule enginio-client
 * \brief Abstract interface for objects stored in the Enginio backend
 *
 * If you have an existing object class or want more flexibility, you can implement
 * this interface in your own class. You must also implement a subclass of
 * EnginioAbstractObjectFactory that can create instances of your custom class
 * and register the factory instance with EnginioClient::registerObjectFactory().
 *
 * /sa EnginioClient
 */

/*!
 * \fn virtual QByteArray EnginioAbstractObject::toEnginioJson(bool isObjectRef) const
 *
 * Returns the object as UTF-8 encoded JSON representation of a valid Enginio object.
 * The \a objectType and \a id properties must be defined, if the object
 * represents an existing Enginio object. If \a isObjectRef is true, object should
 * be represented as a reference to another Enginio object. In that case, resulting
 * JSON should contain only \a id and \a objectType properties.
 */

/*!
 * \fn bool EnginioAbstractObject::fromEnginioJson(const QJsonObject &json)
 *
 * Initializes object from the JSON object, \a json. Returns false if object cannot
 * be initialized from the given JSON object.
 */

 // Why not from QByteArray? Because Enginio library must parse the object
 // anyway to QJsonObject in order to access internal properties.
 

/*!
 * \fn QString EnginioAbstractObject::id() const
 *
 * Returns the object's Enginio ID or empty string if the object is not found.
 *
 */

/*!
 * \fn QString EnginioAbstractObject::objectType() const
 *
 * Returns the object's Enginio type. Note that all user-defined
 * types are prefixed with "objects.".
 */

/*!
 * Time format used on Enginio timestamps. It can be used to convert Enginio
 * timestamp strings to and from QDateTime.
 *
 * \sa QDateTime::fromString(), QDateTime::toString()
 */
QString EnginioAbstractObject::timeFormat()
{
    return QString("yyyy-MM-ddThh:mm:ss.zzzZ");
}
