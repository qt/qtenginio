#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickView>
#include <QDir>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc,argv);
    QQuickView view;
    const QString appPath = QCoreApplication::applicationDirPath();

    // This allows starting the example without previously defining QML2_IMPORT_PATH.
    QDir qmlImportDir(appPath);
    qmlImportDir.cd("../../../qml");
    view.engine()->addImportPath(qmlImportDir.canonicalPath());

    QString qmlPath = appPath + QDir::separator() + ENGINIO_SAMPLE_NAME ".qml";
    view.setSource(QUrl(qmlPath));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}
