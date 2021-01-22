/*
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
 *  Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _KCM_LANDINGPAGE_H
#define _KCM_LANDINGPAGE_H

#include <KQuickAddons/ManagedConfigModule>

class LandingPageData;
class LandingPageGlobalsSettings;

class KCMLandingPage : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(LandingPageGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)

public:
    KCMLandingPage(QObject *parent, const QVariantList &args);
    ~KCMLandingPage() override {}

    LandingPageGlobalsSettings *globalsSettings() const;

    Q_INVOKABLE void openWallpaperDialog();

public Q_SLOTS:
    void save() override;

private:
    LandingPageData *m_data;
};

#endif  // _KCM_LANDINGPAGE_H
