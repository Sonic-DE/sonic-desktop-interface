/**
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <KCModuleDataV2>

extern "C" {
#include <libwacom/libwacom.h>
}

class TabletsModel;

class TabletModuleData : public KCModuleDataV2
{
    Q_OBJECT

public:
    TabletModuleData(QObject *parent = nullptr);
    ~TabletModuleData() override;

private:
    void updateRelevance();

    WacomDeviceDatabase *m_db = nullptr;
    TabletsModel *m_tabletsModel = nullptr;
};
