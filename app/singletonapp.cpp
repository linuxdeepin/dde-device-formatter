// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singletonapp.h"
#include <QProcess>
#include <QLocalSocket>
#include <QDebug>

SingletonApp::SingletonApp(QObject *parent) : QObject(parent)
{
    initConnections();
}

void SingletonApp::initConnections()
{
    connect(&m_server, &QLocalServer::newConnection, this, &SingletonApp::handleConnection);
}

QString SingletonApp::getServerPathByKey(const QString &key)
{
    QString serverPath = QString("%1/%2").arg(getServerRootPath(), key);
    qDebug() << "SingletonApp: Generated server path:" << serverPath << "for key:" << key;
    return serverPath;
}

QString SingletonApp::getUserId()
{
    QString cmd = "id";
    QProcess p;
    p.start(cmd, (QStringList() << "-u"));
    p.waitForFinished(-1);
    QString uid = p.readLine();
    return uid.trimmed();
}

QString SingletonApp::getServerRootPath()
{
    QString path;
    QString uid = getUserId();
    if(uid == "0")
        path = QString("/tmp");
    else
        path = QString("/run/user/%1").arg(uid);
    return path;
}

SingletonApp *SingletonApp::instance()
{
    static SingletonApp* app = new SingletonApp;
    return app;
}

bool SingletonApp::setSingletonApplication(const QString &key)
{
    qDebug() << "SingletonApp: Setting up singleton application with key:" << key;
    m_key = key;
    QString serverPath = getServerPathByKey(key);
    
    // Try to connect to existing server
    qDebug() << "SingletonApp: Checking if another instance is already running";
    QLocalSocket socket;
    socket.connectToServer(serverPath);
    bool ret = socket.waitForConnected(1000);
    if(ret){
        qDebug() << "SingletonApp: Another instance detected, sending multi-process signal";
        socket.write("MultiProcess");
        socket.flush();
        qDebug() << "SingletonApp: Failed to create singleton - another instance is running";
        return false;
    }
    
    qDebug() << "SingletonApp: No existing instance found, creating new server";
    // Remove any stale server
    QLocalServer::removeServer(serverPath);
    
    // Create new server
    ret = m_server.listen(serverPath);
    return ret;
}

void SingletonApp::readData()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if(!socket) {
        qDebug() << "SingletonApp: readData called but sender is not a QLocalSocket";
        return;
    }

    QByteArray data = socket->readAll();
    qDebug() << "SingletonApp: Received data from client connection:" << data;
    qDebug() << "SingletonApp: Client connection data size:" << data.size() << "bytes";
}

void SingletonApp::handleConnection()
{
    QLocalSocket* socket = m_server.nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &SingletonApp::readData);
}

