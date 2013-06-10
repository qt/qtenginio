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

#include "enginioplugin.h"
#include "enginioqmlclient.h"
#include "enginioqmlmodel.h"
#include "enginiomodel.h"
#include "enginioreply.h"
#include "enginioqmlreply.h"
#include "enginioidentity.h"
#include <Enginio/private/enginioclient_p.h>

#include <qqml.h>
#include <QtQml/qqmlnetworkaccessmanagerfactory.h>
#include <QtQml/qqmlengine.h>

/*!
 * \qmlmodule enginio-plugin
 * \title Enginio QML Plugin
 * 
 * The Enginio QML plugin provides access to the Enginio service through a set of
 * QML types.
 */


/* FIXME if needed
 * \qmlproperty enumeration EnginioReply::errorCode
 * \list
 * \li Error.NoError - No errors.
 * \li Error.UnknownError - Something went wrong but we don't know what.
 * \li Error.NetworkError - Enginio service is unavailable or can't handle request.
 * \li Error.RequestError - Request or reply is invalid.
 * \li Error.InternalError - Enginio service is malfunctioning.
 * \endlist
 */

/*
 * \qmltype Acl
 * \instantiates EnginioAcl
 * \inqmlmodule enginio-plugin
 * \brief Access control list for Enginio objects.
 *
 * Access control list contains a list of subjects and set of permissions
 * granted for each subject.
 *
 * Subjects can be:
 * \list
 *   \li Users
 *   \li Usergroups
 *   \li Well known subjects like "everyone"
 * \endlist
 *
 * And possible permissions in ACL for objects are:
 * \list
 *   \li "read"
 *   \li "update"
 *   \li "delete"
 *   \li "admin"
 * \endlist
 *
 * Subjects are presented as objects with \c id and \c objectType and
 * permissions as Acl::Permission type enumerations.
 *
 * Well known subjects are constants which identify generic Users or Usergroups.
 * For example 'everyone' subject presents all Users (even anonymous) and its
 * value is \c {{ "id": "*", "objectType": "aclSubject" }}.
 */

/*
 * \qmlproperty enumeration Acl::Permission
 * \list
 * \li Acl.ReadPermission - Permission to read object data
 * \li Acl.UpdatePermission - Permission to update object data
 * \li Acl.DeletePermission - Permission to delete object
 * \li Acl.AdminPermission - Permission to read, update and delete object and to
 *     read and change object permissions
 * \endlist
 */

QQmlEngine *g_qmlEngine = 0;

class EnginioNetworkAccessManagerFactory: public QQmlNetworkAccessManagerFactory
{
public:
    virtual QNetworkAccessManager *create(QObject *parent) Q_DECL_OVERRIDE
    {
        Q_UNUSED(parent);
        return EnginioClientPrivate::prepareNetworkManagerInThread();
    }
};

void EnginioPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    g_qmlEngine = engine;

    if (!engine->networkAccessManagerFactory()) {
        static EnginioNetworkAccessManagerFactory factory;
        engine->setNetworkAccessManagerFactory(&factory);
    } else {
        qWarning() << "Enginio client failed to install QQmlNetworkAccessManagerFactory"
                      "on QML engine because a differnt factory is already attached, It"
                      " is recomanded to use QNetworkAccessManager delivered by Enginio";
    }
}

void EnginioPlugin::registerTypes(const char *uri)
{
    // @uri Enginio
    qmlRegisterType<EnginioQmlClient>(uri, 1, 0, "Client");
    qmlRegisterType<EnginioQmlClient>(uri, 1, 0, "Enginio");
    qmlRegisterType<EnginioClient>(uri, 1, 0, "__Enginio");
    qmlRegisterType<EnginioQmlModel>(uri, 1, 0, "EnginioModel");
    qmlRegisterUncreatableType<EnginioReply>(uri, 1, 0, "__EnginioReply", "__EnginioReply cannot be instantiated.");
    qmlRegisterUncreatableType<EnginioQmlReply>(uri, 1, 0, "EnginioReply", "EnginioReply cannot be instantiated.");
    qmlRegisterUncreatableType<EnginioIdentity>(uri, 1, 0, "EnginioIdentity", "EnginioIdentity can not be instantiated directly");
    qmlRegisterType<EnginioAuthentication>(uri, 1, 0, "EnginioAuthentication");
}
