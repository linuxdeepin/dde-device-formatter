// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dplatformwindowhandle.h>
#include <DApplication>
#include <DLog>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QDBusMetaType>
#include "app/cmdmanager.h"
#include "view/mainwindow.h"
#include "dialogs/messagedialog.h"
#include "app/singletonapp.h"
#include "utils/udisksutils.h"
#include <QProcessEnvironment>
#include <X11/Xlib.h>
#include <QGuiApplication>
#include <QScreen>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif

DCORE_USE_NAMESPACE

#include <pwd.h>

int main(int argc, char *argv[])
{
    qDebug() << "Main: Starting dde-device-formatter application";

    // 设置Deepin平台主题
    if (qgetenv("QT_QPA_PLATFORMTHEME").isEmpty()) {
        qputenv("QT_QPA_PLATFORMTHEME", "deepin");
    }

    //Logger
    DLogManager::registerConsoleAppender();

    qDBusRegisterMetaType<QPair<bool, QString>>();
    qDBusRegisterMetaType<QByteArrayList>();

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    bool isWayland = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        isWayland = true;
        qDebug() << "Main: Detected Wayland session";
    } else {
        qDebug() << "Main: Detected X11 session";
    }

    DApplication a(argc, argv);

    //Singleton app handle
    bool isSingletonApp = SingletonApp::instance()->setSingletonApplication("dde-device-formatter");
    if (!isSingletonApp) {
        qDebug() << "Main: Another instance is already running, exiting";
        return 0;
    }

    //Load translation
    QTranslator translator;
    QString locale = QLocale::system().name();
    QString qmFile = QString("%1/dde-device-formatter_%2.qm").arg(QString::fromLatin1(TRANSLATIONS_DIR), locale);
    qDebug() << "Main: System locale:" << locale;
    qDebug() << "Main: Translation file path:" << qmFile;

    if (translator.load(qmFile)) {
        a.installTranslator(&translator);
        qDebug() << "Main: Translation file loaded and installed successfully";
    } else {
        qDebug() << "Main: Failed to load translation file, using default language";
    }

    a.setOrganizationName("deepin");
    a.setApplicationName(QObject::tr("dde device formatter"));
    a.setApplicationVersion("1.0");
    a.setWindowIcon(QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager")));
    a.setQuitOnLastWindowClosed(true);

    //Command line
    CMDManager::instance()->process(a);

    // Check if we need display help text.
    if (CMDManager::instance()->positionalArguments().isEmpty()) {
        qDebug() << "Main: No positional arguments provided, showing help";
        CMDManager::instance()->showHelp();
    }

    //Check if exists path
    const QString path = CMDManager::instance()->getPath();
    if (path.isEmpty() || !path.startsWith("/dev/") || !QFile::exists(path)) {
        QString message = QObject::tr("Device does not exist");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    //Check if the device is read-only
    UDisksBlock blk(path);
    if (blk.isReadOnly()) {
        qDebug() << "Main: Device is read-only, cannot format";
        QString message = QObject::tr("The device is read-only");
        MessageDialog d(message, 0);
        d.exec();
        return 0;
    }

    MainWindow *w = new MainWindow(path);
    w->show();
    QRect rect = w->geometry();
    rect.moveCenter(QGuiApplication::primaryScreen()->availableGeometry().center());
    w->move(rect.x(), rect.y());

    if (CMDManager::instance()->isSet("m")) {
        qDebug() << "Main: Model mode enabled, setting window parent";
        int parentWinId = CMDManager::instance()->getWinId();
        int winId = w->winId();

        if (parentWinId != -1 && !isWayland) {
            qDebug() << "Main: Setting transient window hint for X11";
            Display *display = QX11Info::display();
            if (display) {
                XSetTransientForHint(display, (Window)winId, (Window)parentWinId);
            }
        }
    }

    int code = a.exec();

    qDebug() << "Main: Performing quick exit";
    quick_exit(code);
}
