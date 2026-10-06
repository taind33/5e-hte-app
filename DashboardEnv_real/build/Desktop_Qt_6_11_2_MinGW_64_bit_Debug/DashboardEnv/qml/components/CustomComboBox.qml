import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import DashboardEnv

// Viết lại toàn bộ - bản cũ tô toàn màu hex cứng ("#FFFFFF", "#787486",
// "#1D2939"...) nên: (1) không theo dark mode - luôn trắng dù bật theme tối,
// khác hẳn CustomSwitch/mọi component khác trong app đã dùng AppTheme; (2)
// không có phản hồi hover/press nào trên chính hộp combo (chỉ delegate bên
// trong popup mới đổi màu khi hover) nên bấm vào cảm giác "chết", đúng như
// góp ý "khá xấu". Bản này: theo AppTheme đầy đủ (sáng/tối), có hover +
// press + focus state với hiệu ứng chuyển màu mượt (Behavior), mũi tên xoay
// khi mở popup, và popup có đổ bóng nhẹ giống các card khác trong app
// (MultiEffect, cùng công thức shadow đã dùng ở BoxWrap/Notification).
ComboBox {
    id: cbControl
    Layout.fillWidth: true
    Layout.fillHeight: true
    leftPadding: 8
    rightPadding: 24
    hoverEnabled: true

    background: Rectangle {
        id: bg
        implicitHeight: 26
        radius: 6

        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: 2

        color: cbControl.pressed ? AppTheme.panelBgMuted
             : cbControl.hovered ? AppTheme.panelBg
             : AppTheme.elementBg
        border.color: cbControl.visualFocus ? AppTheme.brandRed
                    : cbControl.hovered     ? AppTheme.borderMuted
                    : AppTheme.borderLight
        border.width: cbControl.visualFocus ? 2 : 1

        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }

    indicator: Text {
        x: cbControl.width - width - 8
        y: cbControl.topPadding + (cbControl.availableHeight - height) / 2
        text: "▼"
        font.pixelSize: 8
        color: cbControl.hovered ? AppTheme.textDark : AppTheme.textMuted

        // Xoay ngược mũi tên khi popup đang mở - phản hồi trực quan rằng
        // combo đang "mở", chi tiết nhỏ nhưng làm nó cảm giác mượt hơn hẳn.
        rotation: cbControl.popup.visible ? 180 : 0
        transformOrigin: Item.Center
        Behavior on rotation { NumberAnimation { duration: 140; easing.type: Easing.OutQuad } }
    }

    contentItem: Text {
        // Chưa chọn (chạy lần đầu, hoặc giá trị đã lưu không còn trong danh
        // sách - vd cổng COM đã rút) -> "--".
        text: cbControl.currentIndex < 0 ? "--" : cbControl.currentText
        font.pixelSize: 11
        color: AppTheme.textDark
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    popup: Popup {
        id: cbPopup
        y: cbControl.height + 4
        width: Math.max(120, cbControl.width)
        x: (cbControl.width - width) / 2
        implicitHeight: contentItem.implicitHeight
        padding: 4

        // Hiệu ứng mở/đóng mượt thay vì bật/tắt đột ngột.
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 110 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 80 }
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: cbControl.popup.visible ? cbControl.delegateModel : null
            currentIndex: cbControl.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Item {
            Rectangle {
                id: popupBg
                anchors.fill: parent
                color: AppTheme.elementBg
                border.color: AppTheme.borderLight
                radius: 6
            }
            MultiEffect {
                source:                 popupBg
                anchors.fill:           popupBg
                shadowEnabled:          true
                shadowColor:            "#40000000"
                shadowBlur:             0.3
                shadowHorizontalOffset: 0
                shadowVerticalOffset:   3
                shadowScale:            1.0
            }
        }
    }

    delegate: ItemDelegate {
        id: itemDelegate
        // ListView.view la thuoc tinh attached chuan cua Qt Quick Controls
        // cho delegate ben trong 1 ListView (popup content ben tren) - an
        // toan hon tham chieu id popup tu 1 Component rieng.
        width: ListView.view ? ListView.view.width : implicitWidth
        height: 28
        padding: 0
        leftPadding: 8
        rightPadding: 8
        hoverEnabled: true

        background: Rectangle {
            anchors.fill: parent
            radius: 4
            color: itemDelegate.highlighted ? AppTheme.brandRed
                 : itemDelegate.hovered     ? AppTheme.panelBgMuted
                 : "transparent"
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        contentItem: Text {
            text: modelData
            font.pixelSize: 11
            color: itemDelegate.highlighted ? AppTheme.textLight : AppTheme.textDark
            font.bold: itemDelegate.hovered
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
