/*
 *  Copyright (C) 2021 Marco Martin <mart@kde.org>
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

#include "landingpage.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KGlobalSettings>

#include <QDBusMessage>
#include <QDBusConnection>

#include "landingpagedata.h"
#include "landingpage_kdeglobalssettings.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMLandingPageFactory, "kcm_landingpage.json", registerPlugin<KCMLandingPage>(); registerPlugin<LandingPageData>();)

KCMLandingPage::KCMLandingPage(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LandingPageData(this))
{
    qmlRegisterType<LandingPageGlobalsSettings>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_landingpage"),
                                       i18n("Basic Settings"),
                                       QStringLiteral("1.1"),
                                       i18n("Landing page with some basic settings."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);

    setButtons(Apply | Default | Help);
}

LandingPageGlobalsSettings *KCMLandingPage::globalsSettings() const
{
    return m_data->settings();
}

void KCMLandingPage::save()
{
    ManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(KGlobalSettings::SettingsChanged);
    args.append(KGlobalSettings::SETTINGS_MOUSE);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

#include "landingpage.moc"
