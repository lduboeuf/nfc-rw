#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "NFCManager.h"

int main(int argc, char *argv[])
{

    QCoreApplication::setOrganizationName("nfc-rw.lduboeuf");
    QCoreApplication::setApplicationName("nfc-rw");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    NFCManager nfcManager;
    engine.rootContext()->setContextProperty("nfcManager", &nfcManager);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
