/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splititem.h"
#include "splitnode.h"

SplitItem::SplitItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

QQuickItem *SplitItem::darkItem() const
{
    return m_darkItem;
}

void SplitItem::setDarkItem(QQuickItem *item)
{
    if (m_darkItem != item) {
        m_darkItem = item;
        update();
        Q_EMIT darkItemChanged();
    }
}

QQuickItem *SplitItem::lightItem() const
{
    return m_lightItem;
}

void SplitItem::setLightItem(QQuickItem *item)
{
    if (m_lightItem != item) {
        m_lightItem = item;
        update();
        Q_EMIT lightItemChanged();
    }
}

qreal SplitItem::shutter() const
{
    return m_shutter;
}

void SplitItem::setShutter(qreal shutter)
{
    if (m_shutter != shutter) {
        m_shutter = shutter;
        update();
        Q_EMIT shutterChanged();
    }
}

QSGNode *SplitItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_darkItem || !m_darkItem->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    if (!m_lightItem || !m_lightItem->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    SplitNode *node = static_cast<SplitNode *>(oldNode);
    if (!node) {
        node = new SplitNode();
    }

    node->setShutter(m_shutter);
    node->setDark(m_darkItem->textureProvider());
    node->setLight(m_lightItem->textureProvider());
    node->setRect(boundingRect());

    return node;
}

#include "moc_splititem.cpp"
