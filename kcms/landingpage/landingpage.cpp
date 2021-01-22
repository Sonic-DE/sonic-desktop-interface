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
#include <QDBusPendingCall>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QScreen>

#include "landingpagedata.h"
#include "landingpage_kdeglobalssettings.h"
#include "landingpage_baloosettings.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMLandingPageFactory, "kcm_landingpage.json", registerPlugin<KCMLandingPage>(); registerPlugin<LandingPageData>();)

KCMLandingPage::KCMLandingPage(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LandingPageData(this))
{
    qmlRegisterType<LandingPageGlobalsSettings>();
    qmlRegisterType<BalooSettings>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_landingpage"),
                                       i18n("Quick Settings"),
                                       QStringLiteral("1.1"),
                                       i18n("Landing page with some basic settings."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);

    setButtons(Apply | Default | Help);
}

LandingPageGlobalsSettings *KCMLandingPage::globalsSettings() const
{
    return m_data->landingPageGlobalsSettings();
}

BalooSettings *KCMLandingPage::balooSettings() const
{
    return m_data->balooSettings();
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

    // Update Baloo config or start/stop Baloo
    if (balooSettings()->indexingEnabled()) {
        // Trying to start baloo when it is already running is fine
        const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
        QProcess::startDetached(exe, QStringList());
    } else {
        QDBusMessage message =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.baloo"), QStringLiteral("/"), QStringLiteral("org.kde.baloo.main"), QStringLiteral("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void KCMLandingPage::openWallpaperDialog()
{
    QString connector;

    QQuickItem *item = mainUi();
    if (!item) {
        return;
    }

    QQuickWindow *quickWindow = item->window();
    if (!quickWindow) {
        return;
    }

    QWindow *window = QQuickRenderControl::renderWindowFor(quickWindow);
    if (!window) {
        return;
    }

    QScreen *screen = window->screen();
    if (screen) {
        connector = screen->name();
    }

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"), QStringLiteral("/PlasmaShell"),
                                                   QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("evaluateScript"));

    QList<QVariant> args;
    args << QStringLiteral(R"(
        let id = screenForConnector("%1");

        if (id >= 0) {
            let desktop = desktopForScreen(id);
            desktop.showConfigurationInterface();
        })").arg(connector);

    message.setArguments(args);

    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

Q_INVOKABLE void KCMLandingPage::openKCM(const QString &kcm)
{
    QProcess::startDetached(QStringLiteral("systemsettings5"), QStringList({kcm}));
}

#include "landingpage.moc"
