/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>

class SplitItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *dark READ darkItem WRITE setDarkItem NOTIFY darkItemChanged)
    Q_PROPERTY(QQuickItem *light READ lightItem WRITE setLightItem NOTIFY lightItemChanged)
    Q_PROPERTY(qreal shutter READ shutter WRITE setShutter NOTIFY shutterChanged)

public:
    explicit SplitItem(QQuickItem *parent = nullptr);

    QQuickItem *lightItem() const;
    void setLightItem(QQuickItem *item);

    QQuickItem *darkItem() const;
    void setDarkItem(QQuickItem *item);

    qreal shutter() const;
    void setShutter(qreal shutter);

Q_SIGNALS:
    void lightItemChanged();
    void darkItemChanged();
    void shutterChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    QPointer<QQuickItem> m_lightItem;
    QPointer<QQuickItem> m_darkItem;
    qreal m_shutter = 0.0;
};
