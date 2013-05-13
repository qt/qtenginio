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

    view.setSource(QUrl("qrc:///" ENGINIO_SAMPLE_NAME "/" ENGINIO_SAMPLE_NAME ".qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}
