#include "inputsequence.h"

#include <KLocalizedString>

InputSequence::InputSequence() = default;

InputSequence::InputSequence(const QStringList &config)
{
    if (config.empty()) {
        return;
    }

    const QString &type = config.first();
    if (type == "Disabled") {
        setType(Type::Disabled);
    } else if (type == "Key") {
        setType(Type::Keyboard);

        if (config.size() == 2) {
            keyData() = config.last();
        }
    } else if (type == "MouseButton") {
        setType(Type::Mouse);

        if (config.size() == 2) {
            // TODO: load mouse buttons
        }
    } else if (type == "ApplicationDefined") {
        setType(Type::ApplicationDefined);
    }
}

InputSequence::Type InputSequence::type() const
{
    return m_type;
}

void InputSequence::setType(const Type type)
{
    if (m_type != type) {
        m_type = type;

        // Make sure to reset the data when the type changed
        switch (m_type) {
        case Type::Disabled:
        case Type::ApplicationDefined:
            m_data = NoData{};
            break;
        case Type::Keyboard:
            m_data = KeyData{""};
            break;
        case Type::Mouse:
            m_data = MouseData{.button = Qt::LeftButton};
            break;
        default:
            Q_UNREACHABLE();
        }
    }
}

QStringList InputSequence::toConfigFormat() const
{
    switch (m_type) {
    case Type::Disabled:
        return QStringList{"Disabled"};
    case Type::Keyboard: {
        const auto key = keyData().toString(QKeySequence::PortableText);
        return QStringList{"Key", key};
    }
    case Type::Mouse: {
        const auto mouse = mouseData();
        return QStringList{"MouseButton", QString::number(mouse.button)};
    }
    case Type::ApplicationDefined:
        return QStringList{"ApplicationDefined"};
    default:
        Q_UNREACHABLE();
    }
}

QString InputSequence::toString() const
{
    switch (m_type) {
    case Type::Disabled:
        return i18nc("@action:button This action is disabled", "Disabled");
    case Type::Keyboard: {
        if (keyData().isEmpty()) {
            return i18nc("@action:button There is no keybinding", "None");
        }
        return keyData().toString(QKeySequence::NativeText);
    }
    case Type::Mouse: {
        switch (mouseData().button) {
        case Qt::LeftButton:
            return i18nc("@action:button", "Left mouse button");
        case Qt::RightButton:
            return i18nc("@action:button", "Right mouse button");
        case Qt::MiddleButton:
            return i18nc("@action:button", "Middle mouse button");
        default:
            Q_UNREACHABLE();
        }
    }
    case Type::ApplicationDefined:
        return i18nc("@action:button", "Application-defined");
    default:
        Q_UNREACHABLE();
    }
}

QKeySequence InputSequence::keySequence() const
{
    return keyData();
}

void InputSequence::setKeySequence(const QKeySequence &sequence)
{
    keyData() = sequence;
}

Qt::MouseButton InputSequence::mouseButton() const
{
    return mouseData().button;
}

void InputSequence::setMouseButton(const Qt::MouseButton button)
{
    mouseData().button = button;
}

InputSequence::KeyData &InputSequence::keyData()
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData &InputSequence::mouseData()
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}

InputSequence::KeyData InputSequence::keyData() const
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData InputSequence::mouseData() const
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}