// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "udisksutils.h"

#include <QCoreApplication>
#include <QLocale>
#include <QStorageInfo>
#include <QDebug>

#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <ddiskmanager.h>

UDisksBlock::UDisksBlock(const QString &devnode)
{
    QString udiskspath = DDiskManager::resolveDeviceNode(devnode,{}).first();
    blk.reset(DDiskManager::createBlockDevice(udiskspath));
}

UDisksBlock::~UDisksBlock()
{
}

bool UDisksBlock::isReadOnly() const
{
    return blk->readOnly();
}

/*
 * To whomever modifying this class in the future:
 * Consult the DFMFileInfo class in dde-file-manager first! Make sure
 * reasonable parity is maintained between these two classes.
 */
QString UDisksBlock::displayName() const
{
    if (blk->mountPoints().contains(QByteArray("/\0", 2))) {
        qDebug() << "UDisksBlock: Device is system disk";
        return QCoreApplication::tr("System Disk");
    }
    
    QString label = blk->idLabel();
    if (label.length() == 0) {
        if (blk->isEncrypted() && blk->cleartextDevice().length() <= 1) {
            QString name = QCoreApplication::tr("%1 Encrypted").arg(QLocale::system().formattedDataSize(sizeTotal()));
            qDebug() << "UDisksBlock: Device is encrypted, display name:" << name;
            return name;
        }
        QString name = QCoreApplication::tr("%1 Volume").arg(QLocale::system().formattedDataSize(sizeTotal()));
        qDebug() << "UDisksBlock: Device has no label, using volume name:" << name;
        return name;
    } else {
        qDebug() << "UDisksBlock: Device label:" << label;
        return label;
    }
}

QString UDisksBlock::iconName() const
{
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
    bool isRemovable = (drv->media() == "thumb" || drv->removable() || drv->mediaRemovable() || drv->ejectable());
    QString iconName = QString(isRemovable ? "drive-removable-media" : "drive-harddisk") + (blk->isEncrypted() ? "-encrypted" : "");
    
    qDebug() << "UDisksBlock: Device type - removable:" << isRemovable << ", encrypted:" << blk->isEncrypted() << ", icon:" << iconName;
    return iconName;
}

QString UDisksBlock::fsType() const
{
    return blk->idType();
}

qint64 UDisksBlock::sizeTotal() const
{
    return static_cast<qint64>(blk->size());
}

qint64 UDisksBlock::sizeUsed() const
{
    QScopedPointer<DBlockDevice> rblk(DDiskManager::createBlockDevice(blk->path()));
    if (rblk->isEncrypted() && rblk->cleartextDevice().length() > 1) {
        rblk.reset(DDiskManager::createBlockDevice(blk->cleartextDevice()));
    }
    if (!rblk->hasFileSystem()) {
        qDebug() << "UDisksBlock: Device has no filesystem, cannot calculate used size";
        return -1;
    }

    QString mp;
    if (!rblk->mountPoints().empty()) {
        mp = rblk->mountPoints().front();
        qDebug() << "UDisksBlock: Device mount point:" << mp;
    }

    if (mp.isEmpty()) {
        qDebug() << "UDisksBlock: Device is not mounted, cannot calculate used size";
        return -1;
    }

    QStorageInfo si(mp);
    return si.bytesTotal() - si.bytesFree();
}

DBlockDevice *UDisksBlock::operator ->()
{
    return blk.data();
}
