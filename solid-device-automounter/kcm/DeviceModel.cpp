/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceModel.h"

#include <QIcon>

#include <KLocalizedString>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include "AutomounterSettings.h"

DeviceModel::DeviceModel(AutomounterSettings *m_settings, QObject *parent)
    : QAbstractItemModel(parent)
    , m_settings(m_settings)
{
    reload();

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &DeviceModel::deviceAttached);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &DeviceModel::deviceRemoved);
}

void DeviceModel::forgetDevice(const QString &udi)
{
    if (m_disconnected.contains(udi)) {
        const int deviceIndex = m_disconnected.indexOf(udi);
        beginRemoveRows(index(2, 0), deviceIndex, deviceIndex);
        m_disconnected.removeOne(udi);
        endRemoveRows();
    } else if (m_attached.contains(udi)) {
        const int deviceIndex = m_attached.indexOf(udi);
        beginRemoveRows(index(1, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();
    }
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("Device");
        case 1:
            return i18nc("As in automount on login", "On Login");
        case 2:
            return i18nc("As in automoount on attach", "On Attach");
        }
    }
    return QVariant();
}

void DeviceModel::deviceAttached(const QString &udi)
{
    const Solid::Device devive(udi);
    auto volume = devive.as<Solid::StorageVolume>();

    if (volume && !volume->isIgnored()) {
        if (m_disconnected.contains(udi)) {
            const int deviceIndex = m_disconnected.indexOf(udi);
            beginRemoveRows(index(1, 0), deviceIndex, deviceIndex);
            m_disconnected.removeOne(udi);
            endRemoveRows();
        }

        addNewDevice(udi);
    }
}

void DeviceModel::deviceRemoved(const QString &udi)
{
    if (m_attached.contains(udi)) {
        const int deviceIndex = m_attached.indexOf(udi);

        beginRemoveRows(index(0, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();

        // We move the device to the "Disconnected" section only if it
        // is a known device, meaning we have some setting for this device.
        // Otherwise the device is not moved to the "Disconnected" section
        // because we need to check whether the device that just got detached is ignored
        // (don't show partition tables and other garbage) but this information
        // is no longer available once the device is gone
        if (m_settings->knownDevices().contains(udi)) {
            beginInsertRows(index(1, 0), m_disconnected.size(), m_disconnected.size());
            m_disconnected << udi;
            endInsertRows();
        }
    }
}

void DeviceModel::addNewDevice(const QString &udi)
{
    m_settings->load();

    const Solid::Device dev(udi);
    if (dev.isValid()) {
        if (dev.is<Solid::StorageAccess>()) {
            const Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();
            if (!access->isIgnored() || !access->isAccessible()) {
                beginInsertRows(index(0, 0), m_attached.size(), m_attached.size());
                m_attached << udi;
                endInsertRows();
            }
        }
    } else {
        beginInsertRows(index(1, 0), m_disconnected.size(), m_disconnected.size());
        m_disconnected << udi;
        endInsertRows();
    }
}

void DeviceModel::reload()
{
    beginResetModel();
    m_attached.clear();
    m_disconnected.clear();

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &dev : knownDevices) {
        addNewDevice(dev);
    }
    endResetModel();
}

QModelIndex DeviceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= columnCount()) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        if (parent.column() > 0 || parent.row() == 0) {
            return QModelIndex();
        }

        const int deviceCount = (parent.row() == 1) ? m_attached.size() : m_disconnected.size();
        if (row < deviceCount) {
            return createIndex(row, column, parent.row());
        }
    } else {
        if (row <= 2) {
            return createIndex(row, column, 3);
        }
    }
    return QModelIndex();
}

QModelIndex DeviceModel::parent(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.internalId() == 3)
            return QModelIndex();
        return createIndex(index.internalId(), 0, 3);
    }
    return QModelIndex();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (!index.parent().isValid()) {
        if (index.row() == 0) {
            return Qt::ItemIsEnabled | (index.column() > 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags);
        } else {
            return (automountOnLogin() && automountOnPlugin()) ? Qt::NoItemFlags : Qt::ItemIsEnabled;
        }
    }

    // Select only detached devices to be removed
    Qt::ItemFlag selectableFlag = index.parent().row() == 2 ? Qt::ItemIsSelectable : Qt::NoItemFlags;

    switch (index.column()) {
    case 0:
        if (automountOnLogin() && automountOnPlugin()) {
            return Qt::NoItemFlags;
        }
        return selectableFlag | Qt::ItemIsEnabled;
    case 1:
        return Qt::ItemIsUserCheckable | selectableFlag | (automountOnLogin() ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    case 2:
        return Qt::ItemIsUserCheckable | selectableFlag | (automountOnPlugin() ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    default:
        Q_UNREACHABLE();
    }
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::CheckStateRole || index.column() == 0) {
        return false;
    }

    if (!index.parent().isValid() && index.row() == 0) {
        switch (index.column()) {
        case 1:
            setAutomaticMountOnLogin(value.toInt() == Qt::Checked);
            break;
        case 2:
            setAutomaticMountOnPlugin(value.toInt() == Qt::Checked);
            break;
        }
    } else {
        const QString &udi = index.data(Qt::UserRole).toString();
        switch (index.column()) {
        case 1:
            m_settings->deviceSettings(udi)->setMountOnLogin(value.toInt() == Qt::Checked);
            break;
        case 2:
            m_settings->deviceSettings(udi)->setMountOnAttach(value.toInt() == Qt::Checked);
            break;
        }
    }
    Q_EMIT dataChanged(index, index);
    return true;
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (role == Qt::DisplayRole && index.column() == 0) {
            switch (index.row()) {
            case 0:
                return i18n("All Devices");
            case 1:
                return i18n("Attached Devices");
            case 2:
                return i18n("Disconnected Devices");
            }
        }
        if (role == Qt::CheckStateRole && index.row() == 0) {
            if (index.column() == 1) {
                return automountOnLogin() ? Qt::Checked : Qt::Unchecked;
            } else if (index.column() == 2) {
                return automountOnPlugin() ? Qt::Checked : Qt::Unchecked;
            }
        }
        return QVariant();
    }

    if (index.parent().row() == 1) {
        if (role == TypeRole) {
            return Attached;
        }

        const QString &udi = m_attached.at(index.row());
        Solid::Device dev(udi);

        if (role == Qt::UserRole) {
            return udi;
        }

        if (index.column() == 0) {
            switch (role) {
            case Qt::DisplayRole:
                return dev.description();
            case Qt::ToolTipRole:
                return i18n("UDI: %1", udi);
            case Qt::DecorationRole:
                return QIcon::fromTheme(dev.icon());
            }
        } else if (index.column() == 1) {
            switch (role) {
            case Qt::CheckStateRole:
                return m_settings->deviceSettings(udi)->mountOnLogin() ? Qt::Checked : Qt::Unchecked;
            case Qt::ToolTipRole:
                if (m_settings->shouldAutomountDevice(udi, AutomounterSettings::Login))
                    return i18n("This device will be automatically mounted at login.");
                return i18n("This device will not be automatically mounted at login.");
            }
        } else if (index.column() == 2) {
            switch (role) {
            case Qt::CheckStateRole:
                return m_settings->deviceSettings(udi)->mountOnAttach() ? Qt::Checked : Qt::Unchecked;
            case Qt::ToolTipRole:
                if (m_settings->shouldAutomountDevice(udi, AutomounterSettings::Attach))
                    return i18n("This device will be automatically mounted when attached.");
                return i18n("This device will not be automatically mounted when attached.");
            }
        }
    } else if (index.parent().row() == 2) {
        if (role == TypeRole) {
            return Detached;
        }

        const QString &udi = m_disconnected[index.row()];

        if (role == Qt::UserRole) {
            return udi;
        }

        if (index.column() == 0) {
            switch (role) {
            case Qt::DisplayRole:
                return m_settings->deviceSettings(udi)->name();
            case Qt::ToolTipRole:
                return i18n("UDI: %1", udi);
            case Qt::DecorationRole:
                return QIcon::fromTheme(m_settings->deviceSettings(udi)->icon());
            }
        } else if (index.column() == 1) {
            switch (role) {
            case Qt::CheckStateRole:
                return m_settings->deviceSettings(udi)->mountOnLogin() ? Qt::Checked : Qt::Unchecked;
            case Qt::ToolTipRole:
                if (m_settings->shouldAutomountDevice(udi, AutomounterSettings::Login))
                    return i18n("This device will be automatically mounted at login.");
                return i18n("This device will not be automatically mounted at login.");
            }
        } else if (index.column() == 2) {
            switch (role) {
            case Qt::CheckStateRole:
                return m_settings->deviceSettings(udi)->mountOnAttach() ? Qt::Checked : Qt::Unchecked;
            case Qt::ToolTipRole:
                if (m_settings->shouldAutomountDevice(udi, AutomounterSettings::Attach))
                    return i18n("This device will be automatically mounted when attached.");
                return i18n("This device will not be automatically mounted when attached.");
            }
        }
    }
    return QVariant();
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 3;
    }
    if (parent.internalId() < 3 || parent.column() > 0) {
        return 0;
    }

    switch (parent.row()) {
    case 0:
        return 0;
    case 1:
        return m_attached.size();
    case 2:
        return m_disconnected.size();
    }

    return 0;
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

bool DeviceModel::automountOnLogin() const
{
    return m_settings->automountOnLogin();
}

bool DeviceModel::automountOnPlugin() const
{
    return m_settings->automountOnPlugin();
}

void DeviceModel::setAutomaticMountOnLogin(bool automaticLogin)
{
    if (automountOnLogin() == automaticLogin) {
        return;
    }

    m_settings->setAutomountOnLogin(automaticLogin);
    for (int parent = 1; parent < rowCount(); parent++) {
        const auto parentIndex = index(parent, 0);
        Q_EMIT dataChanged(index(0, 1, parentIndex), index(rowCount(parentIndex), 1, parentIndex));
    }
}

void DeviceModel::setAutomaticMountOnPlugin(bool automaticAttached)
{
    if (automountOnPlugin() == automaticAttached) {
        return;
    }

    m_settings->setAutomountOnPlugin(automaticAttached);
    for (int parent = 1; parent < rowCount(); parent++) {
        const auto parentIndex = index(parent, 0);
        Q_EMIT dataChanged(index(0, 2, parentIndex), index(rowCount(parentIndex), 2, parentIndex));
    }
}
