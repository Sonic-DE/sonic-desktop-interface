/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MainConfigurationWidget.h"

#include <utils/d_ptr_implementation.h>

#include "ui_MainConfigurationWidgetBase.h"

#include "ActivitiesTab.h"
#include "SwitchingTab.h"
#include "kactivitiesdata.h"

K_PLUGIN_FACTORY_WITH_JSON(ActivitiesKCMFactory, "kcm_activities.json", registerPlugin<MainConfigurationWidget>(); registerPlugin<KActivitiesData>();)

class MainConfigurationWidget::Private : public Ui::MainConfigurationWidgetBase
{
public:
    ActivitiesTab *tabActivities;
    SwitchingTab *tabSwitching;
};

MainConfigurationWidget::MainConfigurationWidget(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , d()
{
    d->setupUi(widget());

    d->verticalLayout->addWidget(d->tabActivities = new ActivitiesTab(widget()));
    d->verticalLayout->addWidget(d->tabSwitching = new SwitchingTab(widget()));

    d->tabActivities->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    d->tabSwitching->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    d->tabSwitching->layout()->setContentsMargins(0, 0, 0, 0);

    addConfig(d->tabSwitching->mainConfig(), d->tabSwitching);
}

MainConfigurationWidget::~MainConfigurationWidget()
{
}

void MainConfigurationWidget::defaults()
{
    KCModule::defaults();
}

void MainConfigurationWidget::load()
{
    KCModule::load();
}

void MainConfigurationWidget::save()
{
    KCModule::save();
}

#include "MainConfigurationWidget.moc"
