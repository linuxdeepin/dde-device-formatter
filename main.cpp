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
#include <QtGui/private/qtx11extras_p.h>

DCORE_USE_NAMESPACE

#include <pwd.h>

int main(int argc, char *argv[])
{
    // 在创建QApplication之前设置平台主题
    if (qgetenv("QT_QPA_PLATFORM").isEmpty()) {
        qputenv("QT_QPA_PLATFORM", "dxcb");
    }
    
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
    }

    DApplication a(argc, argv);

    //Singleton app handle
    bool isSingletonApp = SingletonApp::instance()->setSingletonApplication("dde-device-formatter");
    if (!isSingletonApp)
        return 0;

    //Load translation
    QTranslator translator;
    QString locale = QLocale::system().name();
    QString qmFile = QString("%1/dde-device-formatter_%2.qm").arg(QString::fromLatin1(TRANSLATIONS_DIR), locale);

    if (translator.load(qmFile)) {
        a.installTranslator(&translator);
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
        int parentWinId = CMDManager::instance()->getWinId();
        int winId = w->winId();

        if (parentWinId != -1 && !isWayland) {
            Display *display = QX11Info::display();
            if (display) {
                XSetTransientForHint(display, (Window)winId, (Window)parentWinId);
            }
        }
    }

    int code = a.exec();
    quick_exit(code);
}
