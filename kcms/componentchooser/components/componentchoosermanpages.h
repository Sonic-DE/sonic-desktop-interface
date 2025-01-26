/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERMANPAGES_H
#define COMPONENTCHOOSERMANPAGES_H

#include "componentchooser.h"

class ComponentChooserManpages : public ComponentChooser
{
public:
    ComponentChooserManpages(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
