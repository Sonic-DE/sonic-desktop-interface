/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DRAGHELPER_H
#define DRAGHELPER_H

#include <QObject>
#include <QVariantMap>

class QUrl;

class DragHelper : public QObject
{
    Q_OBJECT

public:
    explicit DragHelper(QObject *parent = nullptr);
    ~DragHelper() override;

    Q_INVOKABLE QVariantMap generateMimeData(const QString &mimeType, const QVariant &mimeData, const QUrl &url) const;
};

#endif
