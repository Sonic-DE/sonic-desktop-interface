#include "calibrationcontroller.h"

#include "device.h"
#include "logging.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QFile>
#include <QFileInfo>
#include <QSocketNotifier>
#include <QTimer>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef Q_OS_LINUX
#include <libudev.h>
#include <sys/stat.h>
#endif

CalibrationController::CalibrationController(QObject *parent)
    : QObject(parent)
{
}

CalibrationController::~CalibrationController()
{
    if (m_state == State::Centering || m_state == State::CollectingExtrema || m_state == State::Applying) {
        restoreOriginal();
    }
    closeDevice();
}

CalibrationController::State CalibrationController::state() const
{
    return m_state;
}

QString CalibrationController::instruction() const
{
    switch (m_state) {
    case State::Idle:
        return i18n("Start calibration with the controls released.");
    case State::Centering:
        return i18n("Keep all sticks centered and controls released.");
    case State::CollectingExtrema:
#ifdef Q_OS_FREEBSD
        return i18n("Center sampling is complete. Finish to apply the X and Y offsets.");
#else
        return i18n("Move every stick and axis through its full range, then finish calibration.");
#endif
    case State::Applying:
        return i18n("Applying and verifying calibration…");
    case State::Completed:
        return i18n("Calibration was applied and saved.");
    case State::Error:
        return m_errorText;
    }
    return {};
}

QString CalibrationController::errorText() const
{
    return m_errorText;
}

bool CalibrationController::supported() const
{
#ifdef Q_OS_LINUX
    return m_device && !m_device->isVirtual() && !m_device->path().isEmpty();
#elif defined(Q_OS_FREEBSD)
    return m_device && !m_device->isVirtual() && !m_device->path().isEmpty();
#else
    return false;
#endif
}

int CalibrationController::axisCount() const
{
#ifdef Q_OS_LINUX
    return m_original.size();
#else
    return 0;
#endif
}

void CalibrationController::setDevice(Device *device)
{
    if (m_device == device) {
        return;
    }
    cancel();
    m_device = device;
    Q_EMIT supportedChanged();
}

void CalibrationController::setState(State state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    Q_EMIT stateChanged();
}

void CalibrationController::start()
{
#ifdef Q_OS_LINUX
    if (!supported() || (m_state != State::Idle && m_state != State::Completed && m_state != State::Error)) {
        return;
    }
    closeDevice();
    m_errorText.clear();
    Q_EMIT errorTextChanged();

    m_nativePath = findJoystickNode(m_device->path());
    if (m_nativePath.isEmpty()) {
        fail(i18n("Could not uniquely match this controller to a Linux joystick device."), false);
        return;
    }
    m_fd = ::open(QFile::encodeName(m_nativePath).constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (m_fd < 0) {
        fail(i18n("Could not open %1 for calibration: %2", m_nativePath, QString::fromLocal8Bit(std::strerror(errno))), false);
        return;
    }

    unsigned char axes = 0;
    if (::ioctl(m_fd, JSIOCGAXES, &axes) < 0 || axes == 0) {
        fail(i18n("Could not read the controller axis count."), false);
        closeDevice();
        return;
    }
    char nativeName[256] = {};
    if (::ioctl(m_fd, JSIOCGNAME(sizeof(nativeName)), nativeName) < 0) {
        fail(i18n("Could not read the controller identity."), false);
        closeDevice();
        return;
    }
    m_nativeName = QString::fromLocal8Bit(nativeName);
    m_original.resize(axes);
    if (::ioctl(m_fd, JSIOCGCORR, m_original.data()) < 0) {
        fail(i18n("Could not read the existing controller calibration."), false);
        closeDevice();
        return;
    }
    QVector<js_corr> neutral(axes);
    for (js_corr &correction : neutral) {
        correction.type = JS_CORR_NONE;
        correction.prec = 0;
    }
    if (!writeCorrections(neutral)) {
        fail(i18n("Could not initialize controller calibration."), false);
        closeDevice();
        return;
    }

    m_centerMin.fill(std::numeric_limits<int>::max(), axes);
    m_centerMax.fill(std::numeric_limits<int>::min(), axes);
    m_min.fill(std::numeric_limits<int>::max(), axes);
    m_max.fill(std::numeric_limits<int>::min(), axes);
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &CalibrationController::readEvents);
    setState(State::Centering);
    QTimer::singleShot(1200, this, [this] {
        if (m_state == State::Centering) {
            for (int i = 0; i < m_centerMin.size(); ++i) {
                if (m_centerMin[i] == std::numeric_limits<int>::max()) {
                    m_centerMin[i] = m_centerMax[i] = 0;
                }
            }
            setState(State::CollectingExtrema);
        }
    });
#elif defined(Q_OS_FREEBSD)
    if (!supported() || (m_state != State::Idle && m_state != State::Completed && m_state != State::Error))
        return;
    closeDevice();
    m_nativePath = m_device->path();
    m_fd = ::open(QFile::encodeName(m_nativePath).constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (m_fd < 0 || ::ioctl(m_fd, JOY_GET_X_OFFSET, &m_originalXOffset) < 0 || ::ioctl(m_fd, JOY_GET_Y_OFFSET, &m_originalYOffset) < 0) {
        fail(i18n("This controller does not provide the FreeBSD joystick offset API."), false);
        return;
    }
    m_centerXTotal = m_centerYTotal = 0;
    m_centerSamples = 0;
    m_sampleTimer = new QTimer(this);
    m_sampleTimer->setInterval(20);
    connect(m_sampleTimer, &QTimer::timeout, this, [this] {
        struct joystick sample{};
        if (::read(m_fd, &sample, sizeof(sample)) == sizeof(sample)) {
            m_centerXTotal += sample.x;
            m_centerYTotal += sample.y;
            ++m_centerSamples;
        }
    });
    m_sampleTimer->start();
    setState(State::Centering);
    QTimer::singleShot(1200, this, [this] {
        if (m_state == State::Centering) {
            if (m_sampleTimer)
                m_sampleTimer->stop();
            if (m_centerSamples == 0)
                fail(i18n("No joystick samples were received."));
            else
                setState(State::CollectingExtrema);
        }
    });
#else
    fail(i18n("Native calibration is not supported on this platform."), false);
#endif
}

void CalibrationController::finish()
{
#ifdef Q_OS_LINUX
    if (m_state != State::CollectingExtrema || m_fd < 0) {
        return;
    }
    setState(State::Applying);
    QVector<js_corr> calibrated = m_original;
    int changed = 0;
    for (int axis = 0; axis < calibrated.size(); ++axis) {
        const int minValue = m_min[axis];
        const int maxValue = m_max[axis];
        const int centerLow = m_centerMin[axis];
        const int centerHigh = m_centerMax[axis];
        const qint64 lowerRange = qint64(centerLow) - minValue;
        const qint64 upperRange = qint64(maxValue) - centerHigh;
        if (minValue == std::numeric_limits<int>::max() || lowerRange <= 1 || upperRange <= 1) {
            continue;
        }
        const double lowerScale = 32767.0 * 16384.0 / double(lowerRange);
        const double upperScale = 32767.0 * 16384.0 / double(upperRange);
        if (!std::isfinite(lowerScale) || !std::isfinite(upperScale) || std::abs(lowerScale) > std::numeric_limits<int>::max()
            || std::abs(upperScale) > std::numeric_limits<int>::max()) {
            continue;
        }
        js_corr correction{};
        correction.type = JS_CORR_BROKEN;
        correction.prec = qBound(0, centerHigh - centerLow, std::numeric_limits<short>::max());
        correction.coef[0] = centerLow;
        correction.coef[1] = centerHigh;
        correction.coef[2] = qRound(lowerScale);
        correction.coef[3] = qRound(upperScale);
        calibrated[axis] = correction;
        ++changed;
    }
    if (changed == 0) {
        fail(i18n("No axis completed a usable full-range movement."));
        return;
    }
    if (!writeCorrections(calibrated) || !saveProfile(calibrated)) {
        fail(i18n("Could not apply or save the controller calibration."));
        return;
    }
    closeDevice();
    setState(State::Completed);
#elif defined(Q_OS_FREEBSD)
    if (m_state != State::CollectingExtrema || m_fd < 0 || m_centerSamples <= 0)
        return;
    setState(State::Applying);
    const int xOffset = m_originalXOffset + int(m_centerXTotal / m_centerSamples);
    const int yOffset = m_originalYOffset + int(m_centerYTotal / m_centerSamples);
    if (::ioctl(m_fd, JOY_SET_X_OFFSET, &xOffset) < 0 || ::ioctl(m_fd, JOY_SET_Y_OFFSET, &yOffset) < 0) {
        fail(i18n("Could not apply the FreeBSD joystick offsets."));
        return;
    }
    int readX = 0, readY = 0;
    if (::ioctl(m_fd, JOY_GET_X_OFFSET, &readX) < 0 || ::ioctl(m_fd, JOY_GET_Y_OFFSET, &readY) < 0 || readX != xOffset || readY != yOffset) {
        fail(i18n("The FreeBSD joystick offsets could not be verified."));
        return;
    }
    auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
    const QString key = QString::fromLatin1(QCryptographicHash::hash(m_nativePath.toUtf8(), QCryptographicHash::Sha256).toHex());
    KConfigGroup group(config, QStringLiteral("Profile_%1").arg(key));
    group.writeEntry("Complete", false);
    group.writeEntry("Platform", QStringLiteral("FreeBSD"));
    group.writeEntry("DeviceNode", m_nativePath);
    group.writeEntry("BaselineX", m_originalXOffset);
    group.writeEntry("BaselineY", m_originalYOffset);
    group.writeEntry("OffsetX", xOffset);
    group.writeEntry("OffsetY", yOffset);
    if (!config->sync()) {
        fail(i18n("Could not save the FreeBSD calibration profile."));
        return;
    }
    group.writeEntry("Complete", true);
    if (!config->sync()) {
        fail(i18n("Could not complete the FreeBSD calibration profile."));
        return;
    }
    closeDevice();
    setState(State::Completed);
#endif
}

void CalibrationController::cancel()
{
    if (m_fd >= 0) {
        restoreOriginal();
    }
    closeDevice();
    setState(State::Idle);
}

void CalibrationController::reset()
{
#ifdef Q_OS_LINUX
    if (!m_device) {
        return;
    }
    const QString key = profileKey();
    auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
    KConfigGroup group(config, QStringLiteral("Profile_%1").arg(key));
    if (!group.exists()) {
        return;
    }
    const QString node = group.readEntry("DeviceNode", QString());
    const int count = group.readEntry("AxisCount", 0);
    const QList<int> baseline = group.readEntry("Baseline", QList<int>());
    if (count <= 0 || baseline.size() != count * 6) {
        fail(i18n("The saved calibration profile is invalid."), false);
        return;
    }
    m_fd = ::open(QFile::encodeName(node).constData(), O_RDWR | O_CLOEXEC);
    QVector<js_corr> corrections(count);
    for (int i = 0; i < count; ++i) {
        corrections[i].type = baseline[i * 6];
        corrections[i].prec = baseline[i * 6 + 1];
        for (int c = 0; c < 4; ++c) {
            corrections[i].coef[c] = baseline[i * 6 + 2 + c];
        }
    }
    char nativeName[256] = {};
    if (m_fd < 0 || ::ioctl(m_fd, JSIOCGNAME(sizeof(nativeName)), nativeName) < 0
        || QString::fromLocal8Bit(nativeName) != group.readEntry("DeviceName", QString()) || !writeCorrections(corrections)) {
        fail(i18n("Could not restore the original controller calibration."), false);
        closeDevice();
        return;
    }
    group.deleteGroup();
    if (!config->sync()) {
        fail(i18n("The original calibration was restored, but the saved profile could not be removed."), false);
    } else {
        closeDevice();
        setState(State::Idle);
    }
#elif defined(Q_OS_FREEBSD)
    if (!m_device)
        return;
    const QString path = m_device->path();
    const QString key = QString::fromLatin1(QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256).toHex());
    auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
    KConfigGroup group(config, QStringLiteral("Profile_%1").arg(key));
    if (!group.readEntry("Complete", false) || group.readEntry("Platform", QString()) != QStringLiteral("FreeBSD"))
        return;
    int x = group.readEntry("BaselineX", 0);
    int y = group.readEntry("BaselineY", 0);
    m_fd = ::open(QFile::encodeName(path).constData(), O_RDWR | O_CLOEXEC);
    if (m_fd < 0 || ::ioctl(m_fd, JOY_SET_X_OFFSET, &x) < 0 || ::ioctl(m_fd, JOY_SET_Y_OFFSET, &y) < 0) {
        fail(i18n("Could not restore the original FreeBSD joystick offsets."), false);
        return;
    }
    group.deleteGroup();
    if (!config->sync())
        fail(i18n("The offsets were restored, but the profile could not be removed."), false);
    else {
        closeDevice();
        setState(State::Idle);
    }
#endif
}

void CalibrationController::fail(const QString &message, bool restore)
{
    if (restore && m_fd >= 0 && !restoreOriginal()) {
        qCCritical(KCM_GAMECONTROLLER) << "Failed to restore controller corrections after calibration error";
    }
    closeDevice();
    m_errorText = message;
    Q_EMIT errorTextChanged();
    setState(State::Error);
}

void CalibrationController::closeDevice()
{
#ifdef Q_OS_FREEBSD
    if (m_sampleTimer) {
        delete m_sampleTimer;
        m_sampleTimer = nullptr;
    }
#endif
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier;
        m_notifier = nullptr;
    }
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool CalibrationController::restoreOriginal()
{
#ifdef Q_OS_LINUX
    return m_original.isEmpty() || writeCorrections(m_original);
#elif defined(Q_OS_FREEBSD)
    if (m_fd < 0)
        return true;
    return ::ioctl(m_fd, JOY_SET_X_OFFSET, &m_originalXOffset) == 0 && ::ioctl(m_fd, JOY_SET_Y_OFFSET, &m_originalYOffset) == 0;
#else
    return true;
#endif
}

#ifdef Q_OS_LINUX
QString CalibrationController::findJoystickNode(const QString &eventPath) const
{
    struct stat eventStat{};
    if (::stat(QFile::encodeName(eventPath).constData(), &eventStat) != 0) {
        return {};
    }
    udev *context = udev_new();
    if (!context) {
        return {};
    }
    udev_device *eventDevice = udev_device_new_from_devnum(context, 'c', eventStat.st_rdev);
    if (!eventDevice) {
        udev_unref(context);
        return {};
    }
    udev_device *eventParent = udev_device_get_parent_with_subsystem_devtype(eventDevice, "input", nullptr);
    const char *eventParentPath = eventParent ? udev_device_get_syspath(eventParent) : nullptr;
    QString match;
    udev_enumerate *enumerate = udev_enumerate_new(context);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    udev_list_entry *entry = nullptr;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate))
    {
        udev_device *candidate = udev_device_new_from_syspath(context, udev_list_entry_get_name(entry));
        const char *sysname = candidate ? udev_device_get_sysname(candidate) : nullptr;
        if (!sysname || std::strncmp(sysname, "js", 2) != 0) {
            if (candidate)
                udev_device_unref(candidate);
            continue;
        }
        udev_device *parent = udev_device_get_parent_with_subsystem_devtype(candidate, "input", nullptr);
        const char *parentPath = parent ? udev_device_get_syspath(parent) : nullptr;
        const char *node = udev_device_get_devnode(candidate);
        if (eventParentPath && parentPath && node && std::strcmp(eventParentPath, parentPath) == 0) {
            if (!match.isEmpty()) {
                match.clear();
                udev_device_unref(candidate);
                break;
            }
            match = QString::fromLocal8Bit(node);
        }
        udev_device_unref(candidate);
    }
    udev_enumerate_unref(enumerate);
    udev_device_unref(eventDevice);
    udev_unref(context);
    return match;
}

void CalibrationController::readEvents()
{
    js_event event{};
    while (true) {
        const ssize_t size = ::read(m_fd, &event, sizeof(event));
        if (size < 0 && errno == EAGAIN) {
            return;
        }
        if (size != sizeof(event)) {
            fail(i18n("The joystick event stream returned incomplete data."));
            return;
        }
        if (!(event.type & JS_EVENT_AXIS) || event.number >= m_original.size()) {
            continue;
        }
        const int axis = event.number;
        if (m_state == State::Centering) {
            m_centerMin[axis] = qMin(m_centerMin[axis], int(event.value));
            m_centerMax[axis] = qMax(m_centerMax[axis], int(event.value));
        } else if (m_state == State::CollectingExtrema) {
            m_min[axis] = qMin(m_min[axis], int(event.value));
            m_max[axis] = qMax(m_max[axis], int(event.value));
        }
    }
}

bool CalibrationController::writeCorrections(const QVector<js_corr> &corrections)
{
    if (m_fd < 0 || corrections.isEmpty() || ::ioctl(m_fd, JSIOCSCORR, corrections.constData()) < 0) {
        return false;
    }
    QVector<js_corr> readback(corrections.size());
    return ::ioctl(m_fd, JSIOCGCORR, readback.data()) == 0
        && std::memcmp(readback.constData(), corrections.constData(), size_t(corrections.size()) * sizeof(js_corr)) == 0;
}

QString CalibrationController::profileKey() const
{
    if (!m_device) {
        return {};
    }
    const QByteArray identity = m_device->name().toUtf8() + '\0' + m_device->path().toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex());
}

bool CalibrationController::saveProfile(const QVector<js_corr> &corrections)
{
    auto flatten = [](const QVector<js_corr> &values) {
        QList<int> result;
        result.reserve(values.size() * 6);
        for (const js_corr &value : values) {
            result << value.type << value.prec << value.coef[0] << value.coef[1] << value.coef[2] << value.coef[3];
        }
        return result;
    };
    auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
    KConfigGroup group(config, QStringLiteral("Profile_%1").arg(profileKey()));
    const QList<int> existingBaseline = group.readEntry("Baseline", QList<int>());
    group.writeEntry("Complete", false);
    group.writeEntry("DeviceNode", m_nativePath);
    group.writeEntry("DeviceName", m_nativeName);
    group.writeEntry("AxisCount", corrections.size());
    group.writeEntry("Baseline", existingBaseline.isEmpty() ? flatten(m_original) : existingBaseline);
    group.writeEntry("Corrections", flatten(corrections));
    if (!config->sync()) {
        return false;
    }
    group.writeEntry("Complete", true);
    if (!config->sync()) {
        return false;
    }
    QDBusMessage reload = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded6"),
                                                         QStringLiteral("/modules/kded_gamecontroller_calibration"),
                                                         QStringLiteral("org.kde.GameControllerCalibration"),
                                                         QStringLiteral("reloadProfiles"));
    QDBusConnection::sessionBus().asyncCall(reload);
    return true;
}
#endif
