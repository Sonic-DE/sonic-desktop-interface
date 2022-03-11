/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krunnersettings.h"

#include <KAboutData>
#include <KConfig>
#include <KLocalizedString>
#include <KPluginFactory>

#include "krunnersettingsbase.h"
#include "krunnersettingsdata.h"

K_PLUGIN_FACTORY_WITH_JSON(KRunnerSettingsFactory, "kcm_krunnersettings.json", registerPlugin<KRunnerSettings>(); registerPlugin<KRunnerSettingsData>();)

KRunnerSettings::KRunnerSettings(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new KRunnerSettingsData(this))
{
    qmlRegisterType<KRunnerSettingsBase>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_krunnersettings"),
                                       i18nc("@title:window KCM display name", "KRunner Settings"),
                                       QStringLiteral("0.1"),
                                       i18nc("@info KCM short description", "System Settings module for configuring KRunner behavior."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Fushan Wen"), QString(), QStringLiteral("qydwhotmail@gmail.com"));
    setAboutData(about);

    setButtons(Apply | Default);

    m_doesShowPluginButton = std::none_of(args.cbegin(), args.cend(), [](const QVariant &arg) {
        return arg.toString() == QLatin1String("openedFromPluginSettings");
    });

    m_consumer = new KActivities::Consumer(this);
    m_historyConfigGroup = KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation)
                               ->group("PlasmaRunnerManager")
                               .group("History");
    m_historyKeys = m_historyConfigGroup.keyList();
}

KRunnerSettings::~KRunnerSettings()
{
}

KRunnerSettingsBase *KRunnerSettings::krunnerSettings() const
{
    return m_data->settings();
}

QStringList KRunnerSettings::historyKeys() const
{
    return m_historyKeys;
}

void KRunnerSettings::configureClearHistoryButton()
{
    const QStringList historyKeys = m_historyConfigGroup.keyList();

    if (historyKeys != m_historyKeys) {
        m_historyKeys = historyKeys;
        Q_EMIT historyKeysChanged();
    }
}

void KRunnerSettings::deleteAllHistory()
{
    m_historyConfigGroup.deleteGroup(KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void KRunnerSettings::deleteHistoryGroup(const QString &key)
{
    if (key.isEmpty()) {
        return;
    }

    m_historyConfigGroup.deleteEntry(key, KConfig::Notify);
    m_historyConfigGroup.sync();
    configureClearHistoryButton();
}

void KRunnerSettings::save()
{
    ManagedConfigModule::save();

    // Combine & write history
    if (!krunnerSettings()->activityAware()) {
        if (!m_historyConfigGroup.hasKey(nullUuid)) {
            QStringList activities = m_consumer->activities();
            activities.removeOne(m_consumer->currentActivity());
            QStringList newHistory = m_historyConfigGroup.readEntry(m_consumer->currentActivity(), QStringList());
            for (const QString &activity : std::as_const(activities)) {
                newHistory.append(m_historyConfigGroup.readEntry(activity, QStringList()));
            }
            newHistory.removeDuplicates();
            m_historyConfigGroup.writeEntry(nullUuid, newHistory, KConfig::Notify);
            m_historyConfigGroup.sync();
        }
    }
}

#include "krunnersettings.moc"
