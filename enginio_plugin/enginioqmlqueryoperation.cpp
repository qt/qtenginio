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

#include "enginioqmlqueryoperation.h"
#include "enginioqmlclient.h"

/*!
 * \qmltype QueryOperation
 * \instantiates EnginioQmlQueryOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for fetching the Enginio objects by type.
 */

/*!
 * \qmlproperty Client QueryOperation::client
 * Enginio client object. This property must be set before the operation is executed.
 */

/*!
 * \qmlproperty ObjectModel QueryOperation::model
 * Object model that gets updated with the query results.
 */

/*!
 * \qmlproperty string QueryOperation::objectType
 * Type of objects to query. This property must be set before the operation is
 * executed.
 */

/*!
 * \qmlsignal QueryOperation::finished()
 *
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal QueryOperation::error(Error error)
 *
 * Emitted when an error occurs while the operation is being executed. \a error contains
 * the error details.
 */

/*!
 * \qmlmethod void QueryOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */

/*!
 * \qmlmethod void QueryOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

EnginioQmlQueryOperation::EnginioQmlQueryOperation(EnginioQmlClient *client,
                                                   EnginioQmlObjectModel *model,
                                                   QObject *parent) :
    EnginioQueryOperation(client, model, parent)
{
}

EnginioQmlClient * EnginioQmlQueryOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

EnginioQmlObjectModel * EnginioQmlQueryOperation::getModel() const
{
    return static_cast<EnginioQmlObjectModel*>(model());
}
