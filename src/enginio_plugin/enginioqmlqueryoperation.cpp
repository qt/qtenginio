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
#include "enginioqmlqueryoperation.h"
#include "enginioqmlclient.h"

#include <QJsonDocument>

/*!
 * \qmltype QueryOperation
 * \instantiates EnginioQmlQueryOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for fetching multiple Enginio objects from backend.
 *
 * Enginio Rest API provides two separate API endpoints for fetching objects
 * from backend. \l {https://engin.io/documentation/rest/endpoints/objects/query}
 * {Object queries} can be used to fetch objects of single type using MongoDB
 * style query parameter. Alternatively,
 * \l {https://engin.io/documentation/rest/endpoints/search/fulltext}
 * {fulltext search} can be used to search objects of multiple types where
 * object property contains a search string (defined in \c search parameter).
 *
 * QueryOperation selects correct API endpoint based on given object
 * types and request parameters. If more than one object type and \c search
 * parameter is defined, search API is used, otherwise object query API is used.
 *
 * Operation with more than one object type and no \c search parameter is
 * invalid will cause an error when executed.
 *
 * QueryOperation can be also used to
 * \l {https://engin.io/documentation/rest/endpoints/usergroup_members/query}
 * {query members of usergroup} by defining \c usergroupId property. In this
 * case \c objectTypes and \c search properties will be ignored.
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
 * \obsolete
 * Use objectTypes property instead.
 * Type of objects to query. This property must be set before the operation is
 * executed.
 */

/*!
 * \qmlproperty array QueryOperation::objectTypes
 * Types of objects to query. At least one object type must be defined before
 * the operation is executed.
 */

/*!
 * \qmlproperty object QueryOperation::query
 * Parameter for fine-tuning object query. See
 * \l {https://engin.io/documentation/rest/parameters/queries}
 * {query parameter documentation.}
 */

/*!
 * \qmlproperty object QueryOperation::search
 * Fulltext search parameters. See
 * \l {https://engin.io/documentation/rest/parameters/fulltext_query}
 * {fulltext query documentation.}
 */

/*!
 * \qmlproperty int QueryOperation::limit
 * Maximum number of objects returned from backend.
 */

/*!
 * \qmlproperty int QueryOperation::offset
 * Number of objects skipped from the beginning of returned results.
 */

/*!
 * \qmlproperty array QueryOperation::sort
 * The sort order of results. See
 * \l {https://engin.io/documentation/rest/parameters/sort}
 * {sort parameter documentation.}
 */

/*!
 * \qmlproperty object QueryOperation::include
 * Parameter for defining what data should be included in result objects. See
 * \l {https://engin.io/documentation/rest/parameters/include}
 * {include parameter documentation.}
 */

/*!
 * \qmlproperty string QueryOperation::usergroupId
 * Set this property to valid usergroup ID to query members of that usergroup.
 * Object type of returned objects will be "users" so all object type
 * definitions will be ignored.
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

/*!
 * \qmlmethod string QueryOperation::requestParam(string name)
 * Get request parameter with \a name. If request parameter with \a name has not
 * been set, returns empty string.
 */

/*!
 * \qmlmethod void QueryOperation::setRequestParam(string name, string value)
 * Set request parameter with \a name and \a value to be added to request URL. If
 * request parameter with same \a name has already been set, the old value will
 * be overwritten. Setting parameter with empty \a value will remove already set
 * parameter.
 *
 * Refer to the Enginio REST API documentation for valid parameters and value
 * syntax.
 */

/*!
 * \qmlmethod void QueryOperation::addObjectType(string objectType)
 * Add \a objectType to \c objectTypes array.
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

/*!
 * \qmlmethod list QueryOperation::takeResults()
 * Returns query results if \c model parameter is not specified. Calling this
 * method will remove all received result objects from internal storage and
 * return them in a list.
 */
QVariantList EnginioQmlQueryOperation::takeResults()
{
    QList<EnginioAbstractObject*> results = EnginioQueryOperation::takeResults();
    QVariantList variantResults;
    for (int i = 0; i < results.size(); i++) {
        variantResults.append(QJsonDocument::fromJson(results.at(i)->toEnginioJson()).object());
        delete results.at(i);
    }
    return variantResults;
}

QJsonObject EnginioQmlQueryOperation::query() const
{
    QByteArray queryString = requestParam(QStringLiteral("q")).toUtf8();
    return QJsonDocument::fromJson(queryString).object();
}

void EnginioQmlQueryOperation::setQuery(const QJsonObject &query)
{
    setRequestParam(QStringLiteral("q"),
                    QString(QJsonDocument(query).toJson()).simplified());
}

QJsonObject EnginioQmlQueryOperation::search() const
{
    QByteArray searchString = requestParam(QStringLiteral("search")).toUtf8();
    return QJsonDocument::fromJson(searchString).object();
}

void EnginioQmlQueryOperation::setSearch(const QJsonObject &search)
{
    setRequestParam(QStringLiteral("search"),
                    QString(QJsonDocument(search).toJson()).simplified());
}

int EnginioQmlQueryOperation::limit() const
{
    bool ok;
    int limit = requestParam(QStringLiteral("limit")).toInt(&ok);
    return ok ? limit : -1;
}

void EnginioQmlQueryOperation::setLimit(int limit)
{
    setRequestParam(QStringLiteral("limit"), QString::number(limit));
}

int EnginioQmlQueryOperation::offset() const
{
    bool ok;
    int offset = requestParam(QStringLiteral("offset")).toInt(&ok);
    return ok ? offset : -1;
}

void EnginioQmlQueryOperation::setOffset(int offset)
{
    setRequestParam(QStringLiteral("offset"), QString::number(offset));
}

QJsonArray EnginioQmlQueryOperation::sort() const
{
    QByteArray sortString = requestParam(QStringLiteral("sort")).toUtf8();
    return QJsonDocument::fromJson(sortString).array();
}

void EnginioQmlQueryOperation::setSort(const QJsonArray &sort)
{
    setRequestParam(QStringLiteral("sort"),
                    QString(QJsonDocument(sort).toJson()).simplified());
}

QJsonObject EnginioQmlQueryOperation::include() const
{
    QByteArray includeString = requestParam(QStringLiteral("include")).toUtf8();
    return QJsonDocument::fromJson(includeString).object();
}

void EnginioQmlQueryOperation::setInclude(const QJsonObject &include)
{
    setRequestParam(QStringLiteral("include"),
                    QString(QJsonDocument(include).toJson()).simplified());
}
