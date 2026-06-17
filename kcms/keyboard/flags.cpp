/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "flags.h"

#include <KCountryFlagEmojiIconEngine>
#include <KLocalizedString>

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QStringList>

// for text handling
#include "keyboard_config.h"
#include "x11_helper.h"
#include "xkb_rules.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(KCM_KEYBOARD_FLAGS, "org.kde.kcm_keyboard.flags", QtWarningMsg)

QIcon Flags::getIcon(const QString &layout)
{
    if (!iconMap.contains(layout)) {
        iconMap[layout] = createIcon(layout);
        const QIcon &icon = iconMap[layout];
        const QString country = getCountryFromLayoutName(layout);
        qCDebug(KCM_KEYBOARD_FLAGS) << "getIcon: layout=" << layout << "country=" << country << "isNull=" << icon.isNull()
                                    << "availableSizes=" << icon.availableSizes();
        // Probe a pixmap to see if the engine actually paints something.
        const QPixmap probe = icon.pixmap(32, 32);
        qCDebug(KCM_KEYBOARD_FLAGS) << "  probe pixmap 32x32: isNull=" << probe.isNull() << "size=" << probe.size();
    }
    return iconMap[layout];
}

QIcon Flags::createIcon(const QString &layout)
{
    const QString country = getCountryFromLayoutName(layout);
    qCDebug(KCM_KEYBOARD_FLAGS) << "createIcon: layout=" << layout << "-> country=" << country;
    return QIcon(new KCountryFlagEmojiIconEngine(country));
}

// static
// const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString &layout) const
{
    QString countryCode = layout;

    if (countryCode == QLatin1String("nec_vndr/jp"))
        return QStringLiteral("JP");

    // KCountryFlagEmojiIconEngine requires an uppercase ISO 3166-1 alpha-2
    // country code; xkeyboard-config layout names are lowercase.
    return countryCode.toUpper();
}

QString Flags::getDefaultCountryCode() const
{
    // The system default layout is whatever X11 reports as the current
    // layout when the user has not configured any overrides. Map it to
    // an ISO 3166-1 alpha-2 country code using the same rules as getIcon.
    const LayoutUnit defaultLayout = X11Helper::getCurrentLayout();
    if (defaultLayout.layout().isEmpty()) {
        return QString();
    }
    return getCountryFromLayoutName(defaultLayout.layout());
}

// TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig)
{
    if (layoutUnit.isEmpty())
        return QStringLiteral("--");

    QString layoutText = layoutUnit.layout();

    for (const auto layouts = keyboardConfig.layouts(); const LayoutUnit &lu : layouts) {
        if (layoutUnit.layout() == lu.layout() && layoutUnit.variant() == lu.variant()) {
            layoutText = lu.getDisplayName();
            break;
        }
    }

    // TODO: good autolabel
    //	if( layoutText == layoutUnit.layout && layoutUnit.getDisplayName() != layoutUnit.layout ) {
    //		layoutText = layoutUnit.getDisplayName();
    //	}

    return layoutText;
}

static QString getDisplayText(const QString &layout, const QString &variant)
{
    if (variant.isEmpty())
        return layout;
    return variant;
}

QString Flags::getLongText(const LayoutUnit &layoutUnit)
{
    QString layoutText = layoutUnit.layout();
    const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layoutUnit.layout());
    if (layoutInfo) {
        layoutText = layoutInfo->description;

        if (!layoutUnit.variant().isEmpty()) {
            const std::optional<VariantInfo> variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant());
            QString variantText = variantInfo ? variantInfo->description : layoutUnit.variant();

            layoutText = getDisplayText(layoutText, variantText);
        }
    }

    return layoutText;
}
