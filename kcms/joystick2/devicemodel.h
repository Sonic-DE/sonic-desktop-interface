#pragma once

#include <QList>
#include <QStandardItemModel>

class JoyDevice;

class DeviceModel : public QAbstractListModel
{
public:
    DeviceModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<JoyDevice *> m_devices;
};