/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "inputdevice.h"

#include <QList>

InputDevice::InputDevice(const QString &dbusName, QObject *parent)
    : QObject(parent)
{
    connect(this, &InputDevice::leftHandedChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::orientationChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::outputNameChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::outputAreaChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::enabledChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::mapToWorkspaceChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureCurveChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::inputAreaChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureRangeMinChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureRangeMaxChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::relativeChanged, this, &InputDevice::needsSaveChanged);
}

void InputDevice::save()
{
    m_outputName.save();
    m_relative.save();
}

bool InputDevice::isSaveNeeded() const
{
    return false;
}

void InputDevice::defaults()
{
    m_outputName.resetFromDefaults();
    m_relative.resetFromDefaults();
}

bool InputDevice::isDefaults() const
{
    return true;
}

void InputDevice::load()
{
    m_outputName.resetFromSaved();
    m_relative.resetFromSaved();
}

void InputDevice::setOrientation(int ori)
{
}

void InputDevice::setOutputName(const QString &outputName)
{
    m_outputName.set(outputName);
}

void InputDevice::setLeftHanded(bool set)
{
}

void InputDevice::setOutputArea(const QRectF &outputArea)
{
}

void InputDevice::setInputArea(const QRectF &inputArea)
{
}

void InputDevice::setEnabled(bool enabled)
{
}

void InputDevice::setMapToWorkspace(bool mapToWorkspace)
{
}

void InputDevice::setPressureCurve(const QString &curve)
{
}

bool InputDevice::pressureCurveIsDefault() const
{
    return true;
}

QString InputDevice::serializeMatrix(const QMatrix4x4 &matrix)
{
    QString result;
    for (int i = 0; i < 16; i++) {
        result.append(QString::number(matrix.constData()[i]));
        if (i != 15) {
            result.append(QLatin1Char(','));
        }
    }
    return result;
}

QMatrix4x4 InputDevice::deserializeMatrix(const QString &matrix)
{
    const auto items = QStringView(matrix).split(QLatin1Char(','));
    if (items.size() == 16) {
        QList<float> data;
        data.reserve(16);
        std::ranges::transform(std::as_const(items), std::back_inserter(data), [](const QStringView &item) {
            return item.toFloat();
        });

        return QMatrix4x4{data.constData()};
    }

    return QMatrix4x4{};
}

void InputDevice::setRelative(bool relative)
{
    m_relative.set(relative);
}

#include "moc_inputdevice.cpp"
