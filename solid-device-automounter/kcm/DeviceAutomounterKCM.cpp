/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai UWe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceAutomounterKCM.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QItemSelectionModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <kconfigwidgets_version.h>

#include <KAboutData>
#include <KConfigGroup>
#include <Solid/DeviceNotifier>
#include <Solid/StorageVolume>

#include <KPluginFactory>

#include "AutomounterSettings.h"
#include "DeviceAutomounterData.h"
#include "DeviceModel.h"
#include "LayoutSettings.h"

K_PLUGIN_FACTORY(DeviceAutomounterKCMFactory, registerPlugin<DeviceAutomounterKCM>(); registerPlugin<DeviceAutomounterData>();)

DeviceAutomounterKCM::DeviceAutomounterKCM(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args) // DeviceAutomounterKCMFactory::componentData(), parent)
    , m_settings(new AutomounterSettings(this))
    , m_devices(new DeviceModel(m_settings, this))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_device_automounter"),
                                       i18n("Device Automounter"),
                                       QStringLiteral("2.0"),
                                       QString(),
                                       KAboutLicense::GPL_V2,
                                       i18n("(c) 2009 Trever Fischer, (c) 2015 Kai Uwe Broulik"));
    about->addAuthor(i18n("Trever Fischer"), i18n("Original Author"));
    about->addAuthor(i18n("Kai Uwe Broulik"), i18n("Plasma 5 Port"), QStringLiteral("kde@privat.broulik.de"));

    setAboutData(about);
    setupUi(this);

    addConfig(m_settings, this);

    deviceView->setModel(m_devices);

    deviceView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deviceView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    // TODO: This is a bit generic way to mark changes on the KCM, but improves the current situation.
    // More integration with AutomounterSettings is required to make it fully change-aware.
    connect(m_devices, &QAbstractItemModel::dataChanged, this, [this]() {
        markAsChanged();
    });

    connect(deviceView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DeviceAutomounterKCM::updateForgetDeviceButton);

    connect(forgetDevice, &QAbstractButton::clicked, this, &DeviceAutomounterKCM::forgetSelectedDevices);

    forgetDevice->setEnabled(false);
}

DeviceAutomounterKCM::~DeviceAutomounterKCM()
{
    saveLayout();
}

void DeviceAutomounterKCM::updateForgetDeviceButton()
{
    const auto selectedIndexes = deviceView->selectionModel()->selectedIndexes();
    const bool isAnyDettached = std::any_of(selectedIndexes.cbegin(), selectedIndexes.cend(), [](const auto &idx) {
        return idx.data(DeviceModel::TypeRole) == DeviceModel::Detached;
    });
    forgetDevice->setEnabled(isAnyDettached);
}

void DeviceAutomounterKCM::forgetSelectedDevices()
{
    QItemSelectionModel *selected = deviceView->selectionModel();
    int offset = 0;
    while (!selected->selectedIndexes().isEmpty() && selected->selectedIndexes().size() > offset) {
        if (selected->selectedIndexes()[offset].data(DeviceModel::TypeRole) == DeviceModel::Attached) {
            offset++;
        } else {
            m_devices->forgetDevice(selected->selectedIndexes()[offset].data(DeviceModel::UdiRole).toString());
        }
    }

    markAsChanged();
}

void DeviceAutomounterKCM::load()
{
    KCModule::load();

    m_devices->reload();
    loadLayout();
}

void DeviceAutomounterKCM::save()
{
    KCModule::save();
    saveLayout();

    // Housekeeping before saving.
    // 1. Detect if any of the automount options is set to globally enable automounting
    // 2. Clean-up removed setting groups
    bool enabled = m_devices->automountOnLogin() || m_devices->automountOnPlugin();
    QStringList validDevices;

    for (int i = 1; i < m_devices->rowCount(); ++i) {
        const QModelIndex &parentIdx = m_devices->index(i, 0);
        for (int j = 0; j < m_devices->rowCount(parentIdx); ++j) {
            const QString udi = m_devices->index(j, 0, parentIdx).data(DeviceModel::UdiRole).toString();
            validDevices << udi;
            enabled |= m_settings->deviceSettings(udi)->mountOnLogin() | m_settings->deviceSettings(udi)->mountOnAttach();
        }
    }

    m_settings->setAutomountEnabled(enabled);

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &possibleDevice : knownDevices) {
        if (!validDevices.contains(possibleDevice)) {
            m_settings->removeDeviceGroup(possibleDevice);
        }
    }

    m_settings->save();

    // Now tell kded to automatically load the module if loaded
    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                      QStringLiteral("/kded"),
                                                      QStringLiteral("org.kde.kded5"),
                                                      QStringLiteral("setModuleAutoloading"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter")), QVariant(enabled)});
    dbus.call(msg, QDBus::NoBlock);

    // Load or unload right away
    msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                         QStringLiteral("/kded"),
                                         QStringLiteral("org.kde.kded5"),
                                         enabled ? QStringLiteral("loadModule") : QStringLiteral("unloadModule"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter"))});
    dbus.call(msg, QDBus::NoBlock);
}

void DeviceAutomounterKCM::saveLayout()
{
    QList<int> widths;
    const int nbColumn = m_devices->columnCount();
    widths.reserve(nbColumn);

    for (int i = 0; i < nbColumn; ++i) {
        widths << deviceView->columnWidth(i);
    }

    LayoutSettings::setHeaderWidths(widths);
    // Check DeviceModel.cpp, thats where the magic row numbers come from.
    LayoutSettings::setAttachedExpanded(deviceView->isExpanded(m_devices->index(1, 0)));
    LayoutSettings::setDetachedExpanded(deviceView->isExpanded(m_devices->index(2, 0)));
    LayoutSettings::self()->save();
}

void DeviceAutomounterKCM::loadLayout()
{
    LayoutSettings::self()->load();
    // Reset it first, just in case there isn't any layout saved for a particular column.
    int nbColumn = m_devices->columnCount();
    for (int i = 0; i < nbColumn; ++i) {
        deviceView->resizeColumnToContents(i);
    }

    QList<int> widths = LayoutSettings::headerWidths();
    nbColumn = m_devices->columnCount();
    for (int i = 0; i < nbColumn && i < widths.size(); ++i) {
        deviceView->setColumnWidth(i, widths[i]);
    }

    deviceView->setExpanded(m_devices->index(1, 0), LayoutSettings::attachedExpanded());
    deviceView->setExpanded(m_devices->index(2, 0), LayoutSettings::detachedExpanded());
}

#include "DeviceAutomounterKCM.moc"
