#include "devicemodel.h"

#include <QDebug>

#include "joydevice.h"

DeviceModel::DeviceModel()
{
    char dev[30];

    for (int i = 0; i < 5; i++) // check the first 5 devices
    {
        sprintf(dev, "/dev/js%d", i); // first look in /dev
        JoyDevice *joy = new JoyDevice(dev);

        if (joy->open() != JoyDevice::SUCCESS) {
            delete joy;
            sprintf(dev, "/dev/input/js%d", i); // then look in /dev/input
            joy = new JoyDevice(dev);

            if (joy->open() != JoyDevice::SUCCESS) {
                delete joy;
                continue; // try next number
            }
        }

        beginInsertRows(QModelIndex(), m_devices.size(), m_devices.size() + 1);
        qDebug() << "insert row " << joy->text();
        m_devices.push_back(joy);
        endInsertRows();
    }

    qDebug() << m_devices;
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    qDebug() << "Requesting " << index.row();
    qDebug() << m_devices[index.row()]->text();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::UserRole:
        return m_devices[index.row()]->text();
    case Qt::UserRole + 1:
        return QVariant::fromValue(m_devices[index.row()]);
    }
    return QVariant();
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}, {Qt::UserRole, "name"}, {Qt::UserRole + 1, "joydevice"}};
}