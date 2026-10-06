import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DashboardEnv

BoxWrap{
    id: panel
    implicitWidth: 220
    implicitHeight: 272
    // Bắt đỉnh (peak-hold) thật sự nằm ở DashboardManager::m_kichNoPeakV (C++,
    // xem parseIncomingData() msgid 40 / resetSystem()) - trước đây panel này
    // tự tính lại từ dashboard.adcChannels mỗi lần đổi (không có bộ nhớ, nên
    // "bắt đỉnh" không đúng nghĩa: giá trị tụt về ngay khi kênh ADC đọc thấp
    // hơn). Giờ chỉ đọc thẳng dashboard.kichNoPeakVoltage - cập nhật đúng 1
    // lần khi có giá trị âm hơn, giữ nguyên tới khi bấm "Đặt lại".

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: AppTheme.paddingNormal
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: AppTheme.brandRed
            radius: 8

            // Trước đây "Đã phát hiện" cỡ chữ 32px + anchors.centerIn không
            // giới hạn width - chữ dài (tiếng Việt có dấu) tràn gần sát 2 mép
            // Rectangle, nhìn chật/xấu. Giờ gộp cả tiêu đề + dòng điện áp đỉnh
            // vào 1 ColumnLayout căn giữa duy nhất, cỡ chữ giảm còn 22px, và
            // width bị trừ margin 16px mỗi bên (wrapMode WordWrap dự phòng
            // nếu sau này đổi sang chuỗi dài hơn) - luôn cách đều mép khung.
            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width - 32
                spacing: 10

                Text {
                    id: detected_txt
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Đã phát hiện")
                    color: AppTheme.textLight
                    font.pixelSize: 22
                    font.bold: true
                    wrapMode: Text.WordWrap
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 16
                    Text {
                        text: qsTr("Kích nổ (đỉnh):")
                        font.pixelSize: 13
                        color: "#D9D9D9"
                    }
                    Text {
                        text: dashboard.kichNoPeakVoltage.toFixed(1) + " V"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#D9D9D9"
                    }
                }
            }
        }

        Button {
            text: qsTr("ĐẶT LẠI")
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            // Đặt lại TẮT nguồn AC/DC, ATT về 0 dB, delay về 0 ns -> hỏi lại
            // trước, tránh cắt nguồn đang cấp chỉ vì 1 cú bấm nhầm.
            onClicked: resetConfirm.open()
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: parent.clicked()
                cursorShape: Qt.PointingHandCursor
            }
            background: Rectangle {
                color: parent.down ? AppTheme.brandRedPressed : AppTheme.brandRed
                radius: AppTheme.radiusControl
            }
            contentItem: Text {
                text: parent.text
                color: AppTheme.textLight
                font.bold: true
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    // Popup (không phải Item) không được làm con trực tiếp của BoxWrap
    // (default property là list<Item>) -> đặt trong 1 Item chứa.
    Item {
        Dialog {
            id: resetConfirm
            objectName: "resetConfirm"
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: 380
            modal: true
            padding: 18
            topPadding: 14
            onAccepted: dashboard.resetSystem()

            background: Rectangle {
                color: AppTheme.elementBg
                radius: 10
                border.color: AppTheme.borderLight
            }
            header: Text {
                text: qsTr("Đặt lại hệ thống?")
                font.pixelSize: 15
                font.bold: true
                color: AppTheme.textDark
                leftPadding: 18
                topPadding: 16
            }
            contentItem: Text {
                wrapMode: Text.WordWrap
                color: AppTheme.textDark
                font.pixelSize: 12
                lineHeight: 1.2
                text: qsTr("Lệnh này sẽ TẮT ngõ ra nguồn AC và DC, đưa 3 ATT về 0 dB, giữ chậm về 0 ns và xoá đỉnh kích nổ đang giữ.")
            }
            footer: RowLayout {
                spacing: 10
                Item { Layout.fillWidth: true }
                Button {
                    id: cancelBtn
                    text: qsTr("Huỷ")
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 32
                    Layout.bottomMargin: 16
                    onClicked: resetConfirm.reject()
                    background: Rectangle {
                        radius: AppTheme.radiusControl
                        color: cancelBtn.down ? AppTheme.panelBgMuted : AppTheme.panelBg
                        border.color: AppTheme.borderLight
                    }
                    contentItem: Text {
                        text: cancelBtn.text
                        color: AppTheme.textDark
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                Button {
                    id: okBtn
                    text: qsTr("Đặt lại")
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 32
                    Layout.rightMargin: 18
                    Layout.bottomMargin: 16
                    onClicked: resetConfirm.accept()
                    background: Rectangle {
                        radius: AppTheme.radiusControl
                        color: okBtn.down ? AppTheme.brandRedPressed : AppTheme.brandRed
                    }
                    contentItem: Text {
                        text: okBtn.text
                        color: AppTheme.textLight
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
