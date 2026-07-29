#include <KConfigGroup>
#include <KDEDModule>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTimer>

#include <cstring>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(Q_OS_FREEBSD)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/joystick.h>
#include <unistd.h>
#endif

class GameControllerCalibrationApplier : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.GameControllerCalibration")

public:
    GameControllerCalibrationApplier(QObject *parent, const QVariantList &)
        : KDEDModule(parent)
    {
        auto *timer = new QTimer(this);
        timer->setInterval(2000);
        connect(timer, &QTimer::timeout, this, &GameControllerCalibrationApplier::applyProfiles);
        timer->start();
        QTimer::singleShot(0, this, &GameControllerCalibrationApplier::applyProfiles);
    }

public Q_SLOTS:
    Q_SCRIPTABLE bool reloadProfiles()
    {
        m_appliedNodes.clear();
        applyProfiles();
        return true;
    }

private:
    void applyProfiles()
    {
#ifdef Q_OS_LINUX
        for (auto it = m_appliedNodes.begin(); it != m_appliedNodes.end();) {
            if (!QFileInfo::exists(*it))
                it = m_appliedNodes.erase(it);
            else
                ++it;
        }
        auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
        config->reparseConfiguration();
        for (const QString &groupName : config->groupList()) {
            if (!groupName.startsWith(QStringLiteral("Profile_"))) {
                continue;
            }
            KConfigGroup group(config, groupName);
            if (!group.readEntry("Complete", false)) {
                continue;
            }
            const QString node = group.readEntry("DeviceNode", QString());
            const QString expectedName = group.readEntry("DeviceName", QString());
            const int count = group.readEntry("AxisCount", 0);
            const QList<int> flat = group.readEntry("Corrections", QList<int>());
            if (node.isEmpty() || m_appliedNodes.contains(node) || count <= 0 || flat.size() != count * 6) {
                continue;
            }
            const int fd = ::open(QFile::encodeName(node).constData(), O_RDWR | O_CLOEXEC);
            if (fd < 0) {
                continue;
            }
            unsigned char actualAxes = 0;
            if (::ioctl(fd, JSIOCGAXES, &actualAxes) == 0 && actualAxes == count) {
                char nativeName[256] = {};
                if (::ioctl(fd, JSIOCGNAME(sizeof(nativeName)), nativeName) < 0
                    || (!expectedName.isEmpty() && QString::fromLocal8Bit(nativeName) != expectedName)) {
                    ::close(fd);
                    continue;
                }
                QVector<js_corr> corrections(count);
                for (int i = 0; i < count; ++i) {
                    corrections[i].type = flat[i * 6];
                    corrections[i].prec = flat[i * 6 + 1];
                    for (int c = 0; c < 4; ++c) {
                        corrections[i].coef[c] = flat[i * 6 + 2 + c];
                    }
                }
                if (::ioctl(fd, JSIOCSCORR, corrections.constData()) == 0) {
                    QVector<js_corr> readback(count);
                    if (::ioctl(fd, JSIOCGCORR, readback.data()) == 0
                        && std::memcmp(readback.constData(), corrections.constData(), size_t(count) * sizeof(js_corr)) == 0) {
                        m_appliedNodes.insert(node);
                    }
                }
            }
            ::close(fd);
        }
#elif defined(Q_OS_FREEBSD)
        auto config = KSharedConfig::openConfig(QStringLiteral("gamecontrollercalibrationrc"), KConfig::NoGlobals);
        config->reparseConfiguration();
        for (const QString &groupName : config->groupList()) {
            KConfigGroup group(config, groupName);
            if (!groupName.startsWith(QStringLiteral("Profile_")) || !group.readEntry("Complete", false)
                || group.readEntry("Platform", QString()) != QStringLiteral("FreeBSD"))
                continue;
            const QString node = group.readEntry("DeviceNode", QString());
            if (node.isEmpty() || m_appliedNodes.contains(node))
                continue;
            int x = group.readEntry("OffsetX", 0);
            int y = group.readEntry("OffsetY", 0);
            const int fd = ::open(QFile::encodeName(node).constData(), O_RDWR | O_CLOEXEC);
            if (fd < 0)
                continue;
            int readX = 0, readY = 0;
            if (::ioctl(fd, JOY_SET_X_OFFSET, &x) == 0 && ::ioctl(fd, JOY_SET_Y_OFFSET, &y) == 0 && ::ioctl(fd, JOY_GET_X_OFFSET, &readX) == 0
                && ::ioctl(fd, JOY_GET_Y_OFFSET, &readY) == 0 && readX == x && readY == y)
                m_appliedNodes.insert(node);
            ::close(fd);
        }
#endif
    }

    QSet<QString> m_appliedNodes;
};

K_PLUGIN_CLASS_WITH_JSON(GameControllerCalibrationApplier, "kded_gamecontroller_calibration.json")

#include "calibrationapplier.moc"
