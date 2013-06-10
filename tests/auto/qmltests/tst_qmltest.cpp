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
#include <QtQuickTest/quicktest.h>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "../common/common.h"

int main(int argc, char** argv)
{
    if (EnginioTests::TESTAPP_ID.isEmpty() || EnginioTests::TESTAPP_SECRET.isEmpty() || EnginioTests::TESTAPP_URL.isEmpty()) {
        qFatal("Needed environment variables ENGINIO_BACKEND_ID, ENGINIO_BACKEND_SECRET and ENGINIO_API_URL are not set!");
        return EXIT_FAILURE;
    }

    QGuiApplication app(argc, argv);
    const QString appPath = QGuiApplication::applicationDirPath();
    // This allows starting the test without previously defining QML2_IMPORT_PATH.
    QDir qmlImportDir(appPath);
    qmlImportDir.cd("../../../qml");
    QByteArray canonicalImportPath = qmlImportDir.canonicalPath().toUtf8();
    qputenv("QML2_IMPORT_PATH", canonicalImportPath);

    QString qmlFilePath(QUICK_TEST_SOURCE_DIR);
    QFile qmltestConfig(qmlFilePath + QDir::separator() + "config.js");

    if (!qmltestConfig.exists()) {
        if (!qmltestConfig.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qFatal("Could not open configuration file for writing: %s", qmltestConfig.fileName().toLocal8Bit().data());
            return EXIT_FAILURE;
        }

        QTextStream out(&qmltestConfig);
        out << "var backendData = {\n" \
            << "    id: \"" << EnginioTests::TESTAPP_ID << "\",\n" \
            << "    secret: \"" << EnginioTests::TESTAPP_SECRET << "\",\n" \
            << "    apiUrl: \"" << EnginioTests::TESTAPP_URL << "\"\n" \
            << "}\n"
            << "var testSourcePath = \"" QUICK_TEST_SOURCE_DIR "\"\n";

        out.flush();
        qmltestConfig.close();
    }

    return quick_test_main(argc, argv, "qmltests", QUICK_TEST_SOURCE_DIR);
}

