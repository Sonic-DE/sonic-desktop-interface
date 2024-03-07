/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QQmlEngine>

#include <KIO/DropJob>

struct DropJobForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(KIO::DropJob)
};
