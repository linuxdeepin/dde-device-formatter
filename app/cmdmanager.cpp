// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmdmanager.h"
#include <QStringList>
#include <QDebug>

CMDManager *CMDManager::instance()
{
    static CMDManager * instance;
    if(!instance)
        instance = new CMDManager(0);

    return instance;
}

void CMDManager::process(const QApplication &app)
{
    init();
    m_parser.process(app);
}

void CMDManager::init()
{
    m_parser.addOption(m_modelModeOpt);
    m_parser.setApplicationDescription("Device Formatter");
    m_parser.addPositionalArgument("device-path", "The external device path to format. (required)");
    m_parser.addHelpOption();
    m_parser.addVersionOption();
}

bool CMDManager::isSet(const QString &name) const
{
    return m_parser.isSet(name);
}

QString CMDManager::getPath()
{
    QStringList positionalArguments = m_parser.positionalArguments();
    if (positionalArguments.count() > 0) {
        QString path = m_parser.positionalArguments().at(0);
        qDebug() << "CMDManager: Found device path:" << path;
        return path;
    }

    qDebug() << "CMDManager: No device path provided in arguments";
    return QString();
}

QStringList CMDManager::positionalArguments() const
{
    return m_parser.positionalArguments();
}

void CMDManager::showHelp(int exitCode)
{
    qDebug() << "CMDManager: Showing help with exit code:" << exitCode;
    return m_parser.showHelp(exitCode);
}

int CMDManager::getWinId()
{
    QString winId = m_parser.value(m_modelModeOpt);
    qDebug() << "CMDManager: Raw window ID value:" << (winId.isEmpty() ? "empty" : winId);
    
    if(winId.isEmpty()) {
        qDebug() << "CMDManager: No window ID provided, returning -1";
        return -1;
    }
    return winId.toInt();
}

CMDManager::CMDManager(QObject *parent) :
    QObject(parent),
    m_modelModeOpt(QStringList() << "m" << "model-mode",
                   "Enable model mode.",
                   "window ID")
{
}
