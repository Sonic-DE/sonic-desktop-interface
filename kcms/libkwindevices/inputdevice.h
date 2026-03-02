/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMatrix4x4>
#include <QMetaProperty>
#include <QObject>
#include <QString>

#include <optional>

#include "kwindevices-logging.h"

class InputDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sysName READ sysName CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QSizeF size READ size CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool supportsLeftHanded READ supportsLeftHanded CONSTANT)
    Q_PROPERTY(bool leftHanded READ isLeftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)
    Q_PROPERTY(bool supportsCalibrationMatrix READ supportsCalibrationMatrix CONSTANT)
    Q_PROPERTY(bool calibrationMatrixIsDefault READ calibrationMatrixIsDefault NOTIFY calibrationMatrixChanged)

    Q_PROPERTY(bool supportsOrientation READ supportsOrientation CONSTANT)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString outputName READ outputName WRITE setOutputName NOTIFY outputNameChanged)
    Q_PROPERTY(QRectF outputArea READ outputArea WRITE setOutputArea NOTIFY outputAreaChanged)
    Q_PROPERTY(QRectF inputArea READ inputArea WRITE setInputArea NOTIFY inputAreaChanged)
    Q_PROPERTY(bool supportsInputArea READ supportsInputArea CONSTANT)
    Q_PROPERTY(bool mapToWorkspace READ isMapToWorkspace WRITE setMapToWorkspace NOTIFY mapToWorkspaceChanged)
    Q_PROPERTY(QString pressureCurve READ pressureCurve WRITE setPressureCurve NOTIFY pressureCurveChanged)
    Q_PROPERTY(bool pressureCurveIsDefault READ pressureCurveIsDefault NOTIFY pressureCurveChanged)
    Q_PROPERTY(quint32 tabletPadButtonCount READ tabletPadButtonCount CONSTANT)
    Q_PROPERTY(double pressureRangeMin READ pressureRangeMin WRITE setPressureRangeMin NOTIFY pressureRangeMinChanged)
    Q_PROPERTY(double pressureRangeMax READ pressureRangeMax WRITE setPressureRangeMax NOTIFY pressureRangeMaxChanged)
    Q_PROPERTY(bool supportsPressureRange READ supportsPressureRange CONSTANT)
    Q_PROPERTY(bool relative READ isRelative WRITE setRelative NOTIFY relativeChanged)
    Q_PROPERTY(quint32 tabletPadDialCount READ tabletPadDialCount CONSTANT)
    Q_PROPERTY(quint32 tabletPadRingCount READ tabletPadRingCount CONSTANT)
    Q_PROPERTY(QList<unsigned int> numModes READ numModes CONSTANT)
    Q_PROPERTY(QList<unsigned int> currentModes READ currentModes NOTIFY currentModesChanged)
    Q_PROPERTY(bool isVirtual READ isVirtual CONSTANT)

public:
    InputDevice(const QString &dbusName, QObject *parent);

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;

    QString name() const
    {
        return m_name.value();
    }

    QString sysName() const
    {
        return m_sysName.value();
    }
    bool isLeftHanded() const
    {
        return false;
    }
    bool supportsLeftHanded() const
    {
        return false;
    }
    void setLeftHanded(bool set);

    bool supportsOrientation() const
    {
        return false;
    }
    int orientation() const
    {
        return false;
    }
    void setOrientation(int ori);

    QString outputName() const
    {
        return m_outputName.value();
    }
    void setOutputName(const QString &outputName);
    QSizeF size() const
    {
        return m_size.value();
    }

    QRectF outputArea() const
    {
        return QRectF(0, 0, 1, 1);
    }
    void setOutputArea(const QRectF &outputArea);

    QRectF inputArea() const
    {
        return QRectF(0, 0, 1, 1);
    }
    void setInputArea(const QRectF &inputArea);
    bool supportsInputArea() const
    {
        return false;
    }

    bool supportsCalibrationMatrix() const
    {
        return false;
    }

    bool isEnabled() const
    {
        return false;
    }

    QMatrix4x4 defaultCalibrationMatrix() const
    {
        return QMatrix4x4();
    }

    QMatrix4x4 calibrationMatrix() const
    {
        return QMatrix4x4();
    }

    void setCalibrationMatrix(const QMatrix4x4 &matrix)
    {
    }

    Q_INVOKABLE void resetCalibrationMatrix()
    {
    }

    bool calibrationMatrixIsDefault() const
    {
        return false;
    }

    void setEnabled(bool enabled);

    bool isMapToWorkspace() const
    {
        return false;
    }
    void setMapToWorkspace(bool mapToWorkspace);

    QString pressureCurve() const
    {
        return QString();
    }
    void setPressureCurve(const QString &curve);
    bool pressureCurveIsDefault() const;

    void setPressureRangeMin(double min)
    {
    }
    double pressureRangeMin() const
    {
        return 0;
    }

    void setPressureRangeMax(double max)
    {
    }
    double pressureRangeMax() const
    {
        return 0;
    }

    bool supportsPressureRange() const
    {
        return false;
    }

    quint32 tabletPadButtonCount() const
    {
        return m_tabletPadButtonCount.value();
    }

    quint32 tabletPadDialCount() const
    {
        return m_tabletPadDialCount.value();
    }

    quint32 tabletPadRingCount() const
    {
        return m_tabletPadRingCount.value();
    }

    QList<unsigned int> numModes() const
    {
        return m_numModes.value();
    }

    QList<unsigned int> currentModes() const
    {
        return m_currentModes.value();
    }

    QString deviceGroup() const
    {
        return m_deviceGroup.value();
    }

    bool tabletPad() const
    {
        return m_tabletPad.value();
    }

    bool tabletTool() const
    {
        return m_tabletTool.value();
    }

    bool isRelative() const
    {
        return m_relative.value();
    }

    void setRelative(bool relative);

    bool isVirtual() const
    {
        return m_isVirtual.value();
    }

Q_SIGNALS:
    void needsSaveChanged();

    void leftHandedChanged();
    void orientationChanged();
    void outputNameChanged();
    void outputAreaChanged();
    void enabledChanged();
    void mapToWorkspaceChanged();
    void pressureCurveChanged();
    void inputAreaChanged();
    void pressureRangeMinChanged();
    void pressureRangeMaxChanged();
    void calibrationMatrixChanged();
    void relativeChanged();
    void currentModesChanged();

private:
    template<typename T>
    struct Prop {
        typedef void (InputDevice::*ChangedSignal)();

        explicit Prop(InputDevice *device, const char *propName, void* defaultValueFunction, void *supported, ChangedSignal changedSignal)
            : m_changedSignalFunction(changedSignal)
            , m_device(device)
        {
            qCDebug(LIBKWINDEVICES) << "there is no" << propName;
        }

        explicit Prop(InputDevice *device, const char *propName)
            : m_changedSignalFunction(nullptr)
            , m_device(device)
        {
            Q_ASSERT(m_device);
        }

        T value() const
        {
            return m_value ? m_value.value() : T();
        }
        void resetFromDefaults()
        {
            if (isSupported()) {
                set(defaultValue());
            }
        }
        void resetFromSaved()
        {
            m_value = {};
            value();
            m_configValue = m_value;
            if (m_changedSignalFunction) {
                (m_device->*m_changedSignalFunction)();
            }
        }

        void set(T newVal)
        {
            if (!m_value) {
                value();
            }

            Q_ASSERT(isSupported());
            if (m_value != newVal) {
                m_value = newVal;
                if (m_changedSignalFunction) {
                    (m_device->*m_changedSignalFunction)();
                }
            }
        }

        T defaultValue() const
        {
            return T();
        }

        bool changed() const
        {
            return m_value.has_value() && m_value.value() != m_configValue;
        }

        void set(const Prop<T> &p)
        {
            set(p.value());
        }

        bool isSupported() const
        {
            return false;
        }

        bool save() {
            qCDebug(LIBKWINDEVICES) << "skipping" << this << m_value.has_value() << isSupported() << m_prop.name();
            return false;
        }

        bool isDefaults() const
        {
            return m_value == defaultValue();
        }

    private:
        QMetaProperty m_prop;
        const ChangedSignal m_changedSignalFunction;
        InputDevice *const m_device;
        mutable std::optional<T> m_configValue;
        mutable std::optional<T> m_value;
    };

    static QString serializeMatrix(const QMatrix4x4 &matrix);
    static QMatrix4x4 deserializeMatrix(const QString &matrix);

    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name");
    Prop<QSizeF> m_size = Prop<QSizeF>(this, "size");
    Prop<QString> m_sysName = Prop<QString>(this, "sysName");

    Prop<QString> m_outputName = Prop<QString>(this, "outputName", nullptr, nullptr, &InputDevice::outputNameChanged);
    Prop<bool> m_relative = Prop<bool>(this, "tabletToolIsRelative", nullptr, nullptr, &InputDevice::relativeChanged);

    Prop<quint32> m_tabletPadButtonCount = Prop<quint32>(this, "tabletPadButtonCount");
    Prop<quint32> m_tabletPadDialCount = Prop<quint32>(this, "tabletPadDialCount");
    Prop<quint32> m_tabletPadRingCount = Prop<quint32>(this, "tabletPadRingCount");
    Prop<QList<unsigned int>> m_numModes = Prop<QList<unsigned int>>(this, "numModes");
    Prop<QList<unsigned int>> m_currentModes = Prop<QList<unsigned int>>(this, "currentModes", nullptr, nullptr, &InputDevice::currentModesChanged);

    Prop<QString> m_deviceGroup = Prop<QString>(this, "deviceGroupId");
    Prop<bool> m_tabletPad = Prop<bool>(this, "tabletPad");
    Prop<bool> m_tabletTool = Prop<bool>(this, "tabletTool");
    Prop<bool> m_isVirtual = Prop<bool>(this, "isVirtual");
};
