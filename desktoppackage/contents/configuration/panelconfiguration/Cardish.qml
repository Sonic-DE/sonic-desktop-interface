import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.12 as QQC2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.13 as Kirigami

QQC2.Control {
    padding: 0

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    background: Kirigami.ShadowedRectangle {
        color: PlasmaCore.ColorScope.backgroundColor
        radius: PlasmaCore.Units.smallSpacing

        shadow.size: PlasmaCore.Units.largeSpacing
        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.15)
        shadow.yOffset: PlasmaCore.Units.devicePixelRatio * 2

        border.width: PlasmaCore.Units.devicePixelRatio
        border.color: Qt.tint(PlasmaCore.ColorScope.textColor, Qt.rgba(color.r, color.g, color.b, 0.6))
    }
}
