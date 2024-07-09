#pragma once

#include <QKeySequence>
#include <QStringList>
#include <QtQml>

/**
 * @brief Represents an input sequence, that could be a variety of types.
 */
class InputSequence
{
    Q_GADGET

    Q_PROPERTY(InputSequence::Type type READ type WRITE setType CONSTANT)
public:
    enum class Type {
        Disabled, /** Emits nothing. */
        Keyboard, /** Emits a keyboard event. */
        Mouse, /** Emits a mouse event. */
        ApplicationDefined /** Tablet events are passed directly to the application. */
    };
    Q_ENUM(Type)

    /**
     * @brief Constructs an empty InputSequence.
     */
    InputSequence();

    /**
     * @brief Constructs an InputSequence from the kcminputrc format read by KWin.
     */
    InputSequence(const QStringList &config);

    Type type() const;
    void setType(Type type);

    /**
     * @return This InputSequence serialized to the kcminputrc format read by KWin.
     */
    QStringList toConfigFormat() const;

    Q_INVOKABLE QString toString() const;

    /**
     * @return The keyboard sequence. Will assert on a non-Keyboard type sequence.
     */
    Q_INVOKABLE QKeySequence keySequence() const;

    /**
     * @return Sets the keyboard sequence. Will assert on a non-Keyboard type sequence.
     */
    Q_INVOKABLE void setKeySequence(const QKeySequence &sequence);

    Q_INVOKABLE Qt::MouseButton mouseButton() const;
    Q_INVOKABLE void setMouseButton(Qt::MouseButton button);

private:
    using KeyData = QKeySequence;

    struct MouseSequence {
        Qt::KeyboardModifiers modifiers;
        Qt::MouseButton button;
    };
    using MouseData = MouseSequence;
    using NoData = std::monostate;

    KeyData &keyData();
    MouseData &mouseData();

    KeyData keyData() const;
    MouseData mouseData() const;

    Type m_type = Type::ApplicationDefined;
    std::variant<KeyData, MouseData, NoData> m_data;
};