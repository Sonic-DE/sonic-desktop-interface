// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QQmlEngineExtensionPlugin>

void qml_register_types_org_kde_plasma_emoji();

class EmojierPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    EmojierPlugin(QObject *parent = nullptr)
        : QQmlEngineExtensionPlugin(parent)
    {
        volatile auto registration = &qml_register_types_org_kde_plasma_emoji;
        Q_UNUSED(registration);
    }
};

#include "emojierpluginsource.moc"
