/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchoosermanpages.h"

ComponentChooserManpages::ComponentChooserManpages(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/man"),
                       QString(),
                       QStringLiteral("org.kde.khelpcenter.desktop"),
                       i18n("Select default manual page viewer"))
{
}

static const QStringList manMimetypes{"x-scheme-handler/man", "x-scheme-handler/info"};

QStringList ComponentChooserManpages::mimeTypes() const
{
    return manMimetypes;
}
