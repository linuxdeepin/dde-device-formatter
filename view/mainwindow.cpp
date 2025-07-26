// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include <QPainter>
#include <QImage>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QImage>
#include <QProgressBar>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextBrowser>
#include <QFile>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QtConcurrent>
#include <DWindowManagerHelper>
#include "dialogs/messagedialog.h"
#include "utils/udisksutils.h"

#include <dblockdevice.h>
#include <ddiskmanager.h>
#include <dudisksjob.h>

MainWindow::MainWindow(const QString &path, QWidget *parent)
    : DDialog(parent),
      m_diskm(new DDiskManager)
{
    DPlatformWindowHandle handle(this);
    Q_UNUSED(handle)

    setWindowFlags(windowFlags() | Qt::Popup);
    setObjectName("UsbDeviceFormatter");
    m_diskm->setWatchChanges(true);
    m_formatPath = path;
    m_formatType = UDisksBlock(path).fsType();
    m_simulationProgressValue = 0;
    initUI();
    //    initStyleSheet();
    initConnect();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 2, 0, 0);

    QPixmap pixmap(24, 24);
    pixmap.fill(QColor(Qt::transparent));
    QIcon transparentIcon(pixmap);

    QString title = tr("Format");
    setTitle(title);

    m_pageStack = new QStackedWidget(this);
    m_pageStack->setFixedSize(width(), 340);
    m_mainPage = new MainPage(m_formatType, this);
    m_mainPage->setTargetPath(m_formatPath);
    m_warnPage = new WarnPage(this);
    m_formatingPage = new FormatingPage(this);
    m_finishPage = new FinishPage(this);
    m_errorPage = new ErrorPage("", this);

    m_pageStack->addWidget(m_mainPage);
    m_pageStack->addWidget(m_warnPage);
    m_pageStack->addWidget(m_formatingPage);
    m_pageStack->addWidget(m_finishPage);
    m_pageStack->addWidget(m_errorPage);

    m_comfirmButton = new QPushButton(tr("Format"), this);
    m_comfirmButton->setFixedSize(160, 36);
    m_comfirmButton->setObjectName("ComfirmButton");

    mainLayout->addWidget(m_pageStack);
    mainLayout->addSpacing(10);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_comfirmButton, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(34);
    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    addContent(centralWidget);
}

void MainWindow::initConnect()
{
    connect(m_comfirmButton, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(m_diskm.data(), &DDiskManager::jobAdded, [this](const QString &jobs) {
        qDebug() << "MainWindow: Disk manager job added:" << jobs;
        
        QScopedPointer<DUDisksJob> job(DDiskManager::createJob(jobs));
        if (job->operation().contains("format") && job->objects().contains(UDisksBlock(m_formatPath)->path())) {
            qDebug() << "MainWindow: Found matching format job for device:" << m_formatPath;
            qDebug() << "MainWindow: Job operation:" << job->operation();
            
            m_job.reset(DDiskManager::createJob(jobs));
            //非快速格式化按照正常进度显示
            if (m_job->operation().contains("erase")) {
                qDebug() << "MainWindow: Setting up full format with erase operation";
                connect(m_job.data(), &DUDisksJob::progressChanged, [this](double p) {
                    qDebug() << "MainWindow: Format progress:" << (p * 100) << "%";
                    m_formatingPage->setProgress(p);
                });
                connect(m_job.data(), &DUDisksJob::completed, [this](bool r, QString msg) {
                    qDebug() << "MainWindow: Full format completed, result:" << r << ", message:" << msg;
                    this->onFormatingFinished(r);
                });
            }
            //快速格式化不会收到进度更新的信号，这里模拟一个进度条增长的过程
            else if (m_job->operation().contains("mkfs")) {
                qDebug() << "MainWindow: Setting up quick format simulation";
                QTimer *timer = new QTimer();
                m_simulationProgressValue = 0;
                connect(timer, &QTimer::timeout, [this, timer]() {
                    m_simulationProgressValue += 0.06;
                    if (m_simulationProgressValue < 1) {
                        timer->start(300);
                        m_formatingPage->setProgress(m_simulationProgressValue);
                    } else {
                        m_formatingPage->setProgress(0.99);
                    }
                });
                timer->start(300);

                connect(m_job.data(), &DUDisksJob::completed, [this, timer](bool r, QString msg) {
                    qDebug() << "MainWindow: Quick format completed, result:" << r << ", message:" << msg;
                    timer->disconnect();
                    timer->stop();
                    m_simulationProgressValue = 0;
                    if (r) {
                        m_formatingPage->setProgress(1);
                    }

                    connect(timer, &QTimer::timeout, [this, r]() {
                        this->onFormatingFinished(r);
                    });
                    timer->start(200);
                });
            }
        }
    });
    
    connect(m_diskm.data(), &DDiskManager::diskDeviceRemoved, this, [this]() {
        qDebug() << "MainWindow: Disk device removed event triggered";
        
        bool quit = true;
        QStringList &&blkDevStrGroup = DDiskManager::blockDevices({});
        qDebug() << "MainWindow: Checking remaining block devices, count:" << blkDevStrGroup.size();
        
        for (auto blkDevStr : blkDevStrGroup) {
            QScopedPointer<DBlockDevice> blkDev(DDiskManager::createBlockDevice(blkDevStr));
            if (blkDev) {
                QStringList blDevStrArray = blkDevStr.split(QDir::separator());
                QString tagName = blDevStrArray.isEmpty() ? "" : blDevStrArray.last();
                QString devPath = "/dev/" + tagName;
                qDebug() << "MainWindow: Found remaining block device:" << devPath;
                // 当前计算机一个块设备被移除后，检查剩余的块设备是否包含将要格式化的设备
                // 若依然包含，则说明被移除的设备不是将要格式化的设备，因此格式化程序不退出
                if (devPath == m_formatPath) {
                    qDebug() << "MainWindow: Target device" << devPath << "still exists, not quitting";
                    quit = false;
                }
            }
        }

        if (quit) {
            qDebug() << "MainWindow: Target device removed, closing application";
            this->close();
            this->deleteLater();
            ::exit(0);
        }
    });
}

void MainWindow::formatDevice()
{
    qDebug() << "MainWindow: Starting device format process";
    qDebug() << "MainWindow: Format path:" << m_formatPath;
    qDebug() << "MainWindow: Selected filesystem:" << m_mainPage->getSelectedFs();
    qDebug() << "MainWindow: Device label:" << m_mainPage->getLabel();
    qDebug() << "MainWindow: Should erase:" << m_mainPage->shouldErase();
    
    DWindowManagerHelper::setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
    setCloseButtonVisible(false);
    qDebug() << "MainWindow: Disabled window close function during formatting";

    QtConcurrent::run([=] {
        qDebug() << "MainWindow: Format thread started";
        
        UDisksBlock blk(m_formatPath);
        if (!blk->mountPoints().empty()) {
            qDebug() << "MainWindow: Device is mounted, attempting to unmount";
            qDebug() << "MainWindow: Mount points:" << blk->mountPoints();
            
            blk->unmount({});
            QDBusError lastError = blk->lastError();
            if (lastError.isValid()) {
                qWarning() << "MainWindow: Failed to unmount device:" << blk->path() << "Error:" << lastError.name() << ":" << lastError.message();
                QMetaObject::invokeMethod(this, std::bind(&MainWindow::onFormatingFinished, this, false), Qt::ConnectionType::QueuedConnection);
                return;
            }
            qDebug() << "MainWindow: Device unmounted successfully";
        } else {
            qDebug() << "MainWindow: Device is not mounted, proceeding with format";
        }
        
        QVariantMap opt = { { "label", m_mainPage->getLabel() }, { "take-ownership", true } };
        if (m_mainPage->shouldErase()) opt["erase"] = "zero";
        blk->format(m_mainPage->getSelectedFs(), opt);
        QDBusError lastError = blk->lastError();
        if (lastError.isValid()) {
            qWarning() << "MainWindow: Failed to format device:" << blk->path() << "Error:" << lastError.name() << ":" << lastError.message();
            QMetaObject::invokeMethod(this, std::bind(&MainWindow::onFormatingFinished, this, false), Qt::ConnectionType::QueuedConnection);
            return;
        }
        
        qDebug() << "MainWindow: Format operation initiated successfully";
    });
}

bool MainWindow::checkBackup()
{
    return !UDisksBlock(m_formatPath).fsType().isEmpty();
}

void MainWindow::nextStep()
{
    qDebug() << "MainWindow: Next step triggered, current step:" << m_currentStep;
    
    switch (m_currentStep) {
    case Normal:
        qDebug() << "MainWindow: Switching from Normal to Warn page";
        m_pageStack->setCurrentWidget(m_warnPage);
        m_currentStep = Warn;
        m_comfirmButton->setText(tr("Continue"));
        break;
    case Warn:
        qDebug() << "MainWindow: Switching from Warn to Formatting page";
        m_pageStack->setCurrentWidget(m_formatingPage);
        m_currentStep = Formating;
        m_comfirmButton->setText(tr("Formatting..."));
        m_comfirmButton->setEnabled(false);
        formatDevice();
        break;
    case Finished:
        qDebug() << "MainWindow: Format completed, quitting application";
        qApp->quit();
        break;
    case FormattError:
        qDebug() << "MainWindow: Switching from Error back to Main page for retry";
        m_pageStack->setCurrentWidget(m_mainPage);
        m_currentStep = Normal;
        break;
    case RemovedWhenFormattingError:
        qDebug() << "MainWindow: Device removed during formatting, quitting application";
        qApp->quit();
        break;
    default:
        qDebug() << "MainWindow: Unknown step:" << m_currentStep;
        break;
    }
}

void MainWindow::onFormatingFinished(const bool &successful)
{
    qDebug() << "MainWindow: Formatting finished, successful:" << successful;
    
    DWindowManagerHelper::setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);
    setCloseButtonVisible(true);
    qDebug() << "MainWindow: Re-enabled window close function";

    if (successful) {
        qDebug() << "MainWindow: Format successful, showing finish page";
        m_currentStep = Finished;
        m_comfirmButton->setText(tr("Done"));
        m_comfirmButton->setEnabled(true);
        m_pageStack->setCurrentWidget(m_finishPage);

        QTimer::singleShot(0, this, [this] {
            qDebug() << "MainWindow: Attempting to mount formatted device";
            UDisksBlock(this->m_formatPath)->mount({});
        });
    } else {
        qDebug() << "MainWindow: Format failed, checking device status";
        if (!QFile::exists(m_formatPath)) {
            qDebug() << "MainWindow: Device file does not exist, device was removed";
            m_currentStep = RemovedWhenFormattingError;
            m_comfirmButton->setText(tr("Quit"));
            m_errorPage->setErrorMsg(tr("Your disk has been removed"));
        } else {
            qDebug() << "MainWindow: Device still exists, format operation failed";
            m_currentStep = FormattError;
            m_errorPage->setErrorMsg(tr("Failed to format the device"));
            m_comfirmButton->setText(tr("Reformat"));
        }
        m_comfirmButton->setEnabled(true);
        m_pageStack->setCurrentWidget(m_errorPage);
        qDebug() << "MainWindow: Showing error page with step:" << m_currentStep;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow: Close event triggered";
    DDialog::closeEvent(event);
    qDebug() << "MainWindow: Exiting application";
    qApp->exit();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Escape)
        return;

    DDialog::keyPressEvent(event);
}
