/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ERRORPAGE_H
#define ERRORPAGE_H

#include <QFrame>
class QLabel;

class ErrorPage : public QFrame
{
    Q_OBJECT
public:
    explicit ErrorPage(const QString& errorStr = "", QWidget* parent = 0);
    void initUI();
    void setErrorMsg(const QString& errorMsg);

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    QString m_errorText;
    QLabel* m_errorLabel;
};

#endif // ERRORPAGE_H
