// SPDX-FileCopyrightText: 2016-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "formatingpage.h"

#include <DWaterProgress>

#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

DWIDGET_USE_NAMESPACE

FormatingPage::FormatingPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FormatingPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    m_progressBox = new DWaterProgress(this);
    m_progressBox->setFixedSize(128,128);
    m_progressBox->start();
    m_progressBox->setValue(1);
    QLabel* formatingLabel = new QLabel(this);
    QString formatingText = tr("Formatting the disk, please wait...");
    formatingLabel->setText(formatingText);
    formatingLabel->setObjectName("StatusLabel");

    mainLayout->addSpacing(29);
    mainLayout->addWidget(m_progressBox, 0 , Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(formatingLabel, 0 , Qt::AlignHCenter);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setObjectName("HintLabel");
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setAlignment(Qt::AlignHCenter);
    m_hintLabel->hide();
    mainLayout->addWidget(m_hintLabel, 0, Qt::AlignHCenter);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void FormatingPage::setProgress(double p)
{
//    qDebug() << "formating page progress = " << QString::number(int(p * 100));
    if (int(p * 100) < 1) {
        return;
    }
    m_progressBox->setValue(int(p * 100));
}

void FormatingPage::setHintText(const QString &text)
{
    if (m_hintLabel) {
        m_hintLabel->setText(text);
        m_hintLabel->show();
    }
}

void FormatingPage::reset()
{
    if (m_hintLabel) {
        m_hintLabel->clear();
        m_hintLabel->hide();
    }
    // setProgress 会跳过 0 值，这里直接复位动画进度
    if (m_progressBox) {
        m_progressBox->setValue(1);
    }
}
