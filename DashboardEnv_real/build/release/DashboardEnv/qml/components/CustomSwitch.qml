import QtQuick
import QtQuick.Controls
import DashboardEnv

Switch {
    id: control
    text: ""

    indicator: Rectangle {
        implicitWidth:  36
        implicitHeight: 20
        radius:         10

        color:        control.checked ? AppTheme.brandRed : AppTheme.textMuted
        border.color: AppTheme.borderLight
        border.width: 1

        // Viên bi
        Rectangle {
            x:      control.checked ? parent.width - width - 3 : 3
            y:      (parent.height - height) / 2
            width:  14
            height: 14
            radius: 7

            color: "#FFFFFF"

            Behavior on x {
                NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
            }
        }
    }
}