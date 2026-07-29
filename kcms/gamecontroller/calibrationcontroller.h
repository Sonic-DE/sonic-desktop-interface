#pragma once

#include <QObject>
#include <QPointer>
#include <QVector>

class Device;
class QSocketNotifier;
class QTimer;

#ifdef Q_OS_LINUX
#include <linux/joystick.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/joystick.h>
#endif

class CalibrationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString instruction READ instruction NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged FINAL)
    Q_PROPERTY(bool supported READ supported NOTIFY supportedChanged FINAL)
    Q_PROPERTY(int axisCount READ axisCount NOTIFY stateChanged FINAL)

public:
    enum class State {
        Idle,
        Centering,
        CollectingExtrema,
        Applying,
        Completed,
        Error,
    };
    Q_ENUM(State)

    explicit CalibrationController(QObject *parent = nullptr);
    ~CalibrationController() override;

    State state() const;
    QString instruction() const;
    QString errorText() const;
    bool supported() const;
    int axisCount() const;

    Q_INVOKABLE void setDevice(Device *device);
    Q_INVOKABLE void start();
    Q_INVOKABLE void finish();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void reset();

Q_SIGNALS:
    void stateChanged();
    void errorTextChanged();
    void supportedChanged();

private:
    void setState(State state);
    void fail(const QString &message, bool restore = true);
    void closeDevice();
    bool restoreOriginal();
#ifdef Q_OS_LINUX
    QString findJoystickNode(const QString &eventPath) const;
    void readEvents();
    bool writeCorrections(const QVector<js_corr> &corrections);
    bool saveProfile(const QVector<js_corr> &corrections);
    QString profileKey() const;
#endif

    QPointer<Device> m_device;
    QSocketNotifier *m_notifier = nullptr;
    State m_state = State::Idle;
    QString m_errorText;
    QString m_nativePath;
    QString m_nativeName;
    int m_fd = -1;
#ifdef Q_OS_LINUX
    QVector<js_corr> m_original;
    QVector<int> m_centerMin;
    QVector<int> m_centerMax;
    QVector<int> m_min;
    QVector<int> m_max;
#elif defined(Q_OS_FREEBSD)
    int m_originalXOffset = 0;
    int m_originalYOffset = 0;
    qint64 m_centerXTotal = 0;
    qint64 m_centerYTotal = 0;
    int m_centerSamples = 0;
    QTimer *m_sampleTimer = nullptr;
#endif
};
