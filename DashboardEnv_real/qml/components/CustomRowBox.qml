import QtQuick
import QtQuick.Layouts
import QtQuick.Effects // Module bắt buộc trong Qt 6 để dùng MultiEffect nâng cao

Item {
    id: root
    Layout.fillWidth: true
    implicitHeight: 30 // Giữ nguyên chiều cao mong muốn của bạn

    property alias content: container.data

    Rectangle {
        id: bgCard
        anchors.fill: parent
        color: "#FFFFFF"
        radius: 6
        border.color: "#E2E8F0"
        border.width: 1
    }

    MultiEffect {
        source:                  bgCard
        anchors.fill:            bgCard

        shadowEnabled:           true
        shadowColor:             "#40000000"
        shadowBlur:              0.3
        shadowHorizontalOffset:  0
        shadowVerticalOffset:    4
        shadowScale:             1.0
    }

    RowLayout {
        id: container
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 6
    }
}