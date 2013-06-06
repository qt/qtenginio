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

#include "enginioqmlmodel.h"

/*!
  \qmltype EnginioModel
  \instantiates EnginioQmlModel
  \inqmlmodule enginio-plugin
  \ingroup engino-qml
  \target EnginioModelQml

  \brief Makes accessing collections of objects easy

  The query property of the model takes a JSON object.

  To get a model of each object of type "objects.city" use:
  \snippet models.qml model

  It is then possible to use a regular Qt Quick ListView
  to display the list of cities that the backend contains.
  \snippet models.qml view
  Note that the properties of the objects can be directly accessed.
  In this example, we have the type "objects.city" in the backend
  with two properties: "name" and "population".

  The model supports several function to modify the data, for example it is
  easy to add another city to the backend by appending it:
  \snippet models.qml append

  The QML version of EnginioModel supports the same functionality as the C++ version.
  \l {EnginioModelCpp}{EnginioModel C++}
*/

EnginioQmlModel::EnginioQmlModel(QObject *parent)
    : EnginioModel(parent)
{
}
