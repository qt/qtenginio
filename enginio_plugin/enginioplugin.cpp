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
#include "enginioqmlidentityauthoperation.h"
#include "enginioqmlobjectmodel.h"
#include "enginioqmlobjectoperation.h"
#include "enginioqmlqueryoperation.h"
#include "enginioerror.h"

#include <qqml.h>

/*!
 * \qmlmodule enginio-plugin
 * \title Enginio QML Plugin
 * 
 * The Enginio QML plugin provides access to the Enginio service through a set of
 * QML types.
 */


/*!
 * \qmltype Error
 * \instantiates EnginioError
 * \inqmlmodule enginio-plugin
 * \brief Used to describe errors that occur during Enginio operations
 */

/*!
 * \qmlproperty enumeration Error::error
 * \list
 * \li Error.NoError - No errors.
 * \li Error.UnknownError - Something went wrong but we don't know what.
 * \li Error.NetworkError - Enginio service is unavailable or can't handle request.
 * \li Error.RequestError - Request or reply is invalid.
 * \li Error.InternalError - Enginio service is malfunctioning.
 * \endlist
 */

QQmlEngine *g_qmlEngine = 0;

void EnginioPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    g_qmlEngine = engine;
}

void EnginioPlugin::registerTypes(const char *uri)
{
    // @uri io.engin
    qmlRegisterType<EnginioQmlClient>(uri, 1, 0, "Client");
    qmlRegisterType<EnginioQmlObjectModel>(uri, 1, 0, "ObjectModel");
    qmlRegisterType<EnginioQmlObjectOperation>(uri, 1, 0, "ObjectOperation");
    qmlRegisterType<EnginioQmlQueryOperation>(uri, 1, 0, "QueryOperation");
    qmlRegisterType<EnginioQmlIdentityAuthOperation>(uri, 1, 0, "IdentityAuthOperation");
    qmlRegisterType<EnginioError>(uri, 1, 0, "Error");
}
