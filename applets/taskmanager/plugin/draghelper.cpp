/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "draghelper.h"

#include <QUrl>

#include "backend.h"

DragHelper::DragHelper(QObject *parent)
    : QObject(parent)
{
}

DragHelper::~DragHelper()
{
}

QVariantMap DragHelper::generateMimeData(const QString &mimeType, const QVariant &mimeData, const QUrl &url) const
{
    QVariantMap mimedata;

    const QByteArray &taskUrlData = Backend::tryDecodeApplicationsUrl(url).toString().toUtf8();
    mimedata.insert(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"), taskUrlData);
    mimedata.insert(mimeType, mimeData);
    mimedata.insert(QStringLiteral("application/x-orgkdeplasmataskmanager_taskbuttonitem"), mimeData);

    return mimedata;
}
