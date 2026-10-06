// BoxWrap.qml
import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import DashboardEnv

Item {
    id:    root
    property alias color:  body.color
    property alias radius: body.radius
    property alias borderColor: body.border.color
    default property alias content: body.children

    // Shadow layer — render TRƯỚC body để bóng nằm dưới
    MultiEffect {
        source:                  body
        anchors.fill:            body

        shadowEnabled:           true
        shadowColor:             "#40000000"
        shadowBlur:              0.3
        shadowHorizontalOffset:  0
        shadowVerticalOffset:    4
        shadowScale:             1.0
    }

    Rectangle {
        id:           body
        anchors.fill: parent
        // Trước đây hardcode "#FFFFFF"/"#EBEFF5" - mọi panel dùng BoxWrap mà
        // không tự set color/borderColor (đa số) sẽ LUÔN trắng dù bật dark
        // mode, vì AppTheme.isDarkMode không đổi được 1 màu hardcode ở đây.
        // Đổi sang token để bật/tắt theme thực sự đổi màu toàn bộ card.
        color:        AppTheme.elementBg
        radius:       12
        border.color: AppTheme.borderLight
        border.width: 1
    }
}