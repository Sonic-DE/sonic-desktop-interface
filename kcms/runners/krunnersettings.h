/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRUNNERSETTINGS_H
#define KRUNNERSETTINGS_H

#include <KActivities/Consumer>
#include <KConfigGroup>
#include <KQuickAddons/ManagedConfigModule>

class KRunnerSettingsBase;
class KRunnerSettingsData;

class KRunnerSettings : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(KRunnerSettingsBase *krunnerSettings READ krunnerSettings CONSTANT)
    Q_PROPERTY(QStringList historyKeys READ historyKeys NOTIFY historyKeysChanged)
    Q_PROPERTY(bool doesShowPluginButton MEMBER m_doesShowPluginButton CONSTANT)

public:
    KRunnerSettings(QObject *parent, const QVariantList &args);
    ~KRunnerSettings() override;

    KRunnerSettingsBase *krunnerSettings() const;
    QStringList historyKeys() const;

    Q_INVOKABLE void deleteAllHistory();
    Q_INVOKABLE void deleteHistoryGroup(const QString &key);

Q_SIGNALS:
    void historyKeysChanged();

public Q_SLOTS:
    void save() override;

private:
    void configureClearHistoryButton();

    KRunnerSettingsData *m_data;
    KActivities::Consumer *m_consumer;
    KConfigGroup m_historyConfigGroup;
    QStringList m_historyKeys;

    bool m_doesShowPluginButton;

    const QString nullUuid = QStringLiteral("00000000-0000-0000-0000-000000000000");
};

#endif // KRUNNERSETTINGS_H
