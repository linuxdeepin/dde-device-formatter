// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIALOG_H
#define DIALOG_H

#include <QFrame>
#include <QColor>
#include <QLabel>
#include <QStackedWidget>

#include <DDialog>
#include <dplatformwindowhandle.h>

#include "mainpage.h"
#include "warnpage.h"
#include "formatingpage.h"
#include "finishpage.h"
#include "errorpage.h"

#define BORDER_WIDTH 1
#define BORDER_COLOR QColor(0,0,0,120)
#define BORDER_RADIUS 5
#define WINDOW_SIZE QSize(320, 410)

#define SHADOW_BLUR_RADIUS 10
#define SHADOW_COLOR QColor(0,0,0,20)

class QComboBox;
class QProgressBar;
class QPushButton;
class DUDisksJob;
class DDiskManager;

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class MainWindow : public DDialog
{
    Q_OBJECT
    enum FormatStep{
        Normal = 0,
        Warn,
        Formating,
        Finished,
        FormattError,
        RemovedWhenFormattingError
    };

public:
    MainWindow(const QString& path, QWidget *parent = nullptr);
    ~MainWindow();

    void initUI();
    void initConnect();
    void formatDevice();
    bool checkBackup();

signals:
    void taskFinished(const bool& result);

public slots:
    void nextStep();
    void onFormatingFinished(const bool& successful);

private:
    QPushButton* m_comfirmButton = nullptr;
    QStackedWidget* m_pageStack = nullptr;
    FormatStep m_currentStep = Normal;
    MainPage* m_mainPage = nullptr;
    WarnPage* m_warnPage = nullptr;
    FormatingPage* m_formatingPage = nullptr;
    FinishPage* m_finishPage = nullptr;
    ErrorPage* m_errorPage = nullptr;
    QString m_formatPath;
    QString m_formatType;
    QScopedPointer<DUDisksJob> m_job;
    QScopedPointer<DDiskManager> m_diskm;

    //模拟进度值
    double m_simulationProgressValue;
};

#endif // DIALOG_H
