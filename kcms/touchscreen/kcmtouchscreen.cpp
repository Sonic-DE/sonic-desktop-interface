/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtouchscreen.h"
#include "devicesmodel.h"
#include "inputdevice.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardItemModel>

K_PLUGIN_CLASS_WITH_JSON(Touchscreen, "kcm_touchscreen.json")

class OutputsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    OutputsModel()
    {
        auto screens = qGuiApp->screens();
        auto it = new QStandardItem(i18n("Automatic"));
        appendRow(it);

        for (auto screen : screens) {
            auto geo = screen->geometry();
            auto it =
                new QStandardItem(i18nc("model - (x,y widthxheight)", "%1 - (%2,%3 %4×%5)", screen->model(), geo.x(), geo.y(), geo.width(), geo.height()));
            it->setData(screen->name(), Qt::UserRole);
            appendRow(it);
        }

        setItemRoleNames({
            {Qt::DisplayRole, "display"},
            {Qt::UserRole, "name"},
        });
    }

    Q_SCRIPTABLE QString outputNameAt(int row) const
    {
        return item(row)->data(Qt::UserRole).toString();
    }
    Q_SCRIPTABLE int rowForOutputName(const QString &outputName)
    {
        for (int i = 0, c = rowCount(); i < c; ++i) {
            if (item(i)->data(Qt::UserRole) == outputName) {
                return i;
            }
        }

        return 0;
    }
};

Touchscreen::Touchscreen(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list)
    : ManagedConfigModule(parent, metaData, list)
    , m_touchscreensModel(new DevicesModel("touch", this))
{
    qmlRegisterType<OutputsModel>("org.kde.plasma.touchscreen.kcm", 1, 0, "OutputsModel");
    qmlRegisterAnonymousType<InputDevice>("org.kde.plasma.touchscreen.kcm", 1);

    connect(m_touchscreensModel, &DevicesModel::needsSaveChanged, this, &Touchscreen::refreshNeedsSave);
}

Touchscreen::~Touchscreen() = default;

void Touchscreen::refreshNeedsSave()
{
    setNeedsSave(isSaveNeeded());
}

bool Touchscreen::isSaveNeeded() const
{
    return m_touchscreensModel->isSaveNeeded();
}

bool Touchscreen::isDefaults() const
{
    return m_touchscreensModel->isDefaults();
}

void Touchscreen::load()
{
    m_touchscreensModel->load();
}

void Touchscreen::save()
{
    m_touchscreensModel->save();
}

void Touchscreen::defaults()
{
    m_touchscreensModel->defaults();
}

DevicesModel *Touchscreen::touchscreensModel() const
{
    return m_touchscreensModel;
}

#include "kcmtouchscreen.moc"
