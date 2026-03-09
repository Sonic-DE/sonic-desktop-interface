/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQuickItem>

class TabletEvents : public QQuickItem
{
    Q_OBJECT
public:
    TabletEvents(QQuickItem *parent = nullptr);
    ~TabletEvents() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void padButtonReceived(const QString &path, uint button, bool pressed);
    void toolButtonReceived(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, uint button, bool pressed);
    void toolDown(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y);
    void toolMotion(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y, double pressure, double tilt_x, double tilt_y);
    void toolUp(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y);
    void dialDelta(int value120);
};
