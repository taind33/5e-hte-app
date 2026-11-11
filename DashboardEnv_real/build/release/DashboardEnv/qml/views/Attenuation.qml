import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DashboardEnv

BoxWrap {
    id: panel
    implicitWidth:  250
    implicitHeight: 200

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 12
        spacing:         20

        RowLayout{
        spacing :10
        Image {
            source:   "qrc:/qt/qml/DashboardEnv/assets/images/ATTIcon.png"
            width:    24
            height:   24
            fillMode: Image.PreserveAspectFit
        }
            Text {
                text:           qsTr("Suy hao")
                font.bold:      true
                font.pixelSize: 15
                color:          AppTheme.textDark
            }
        }

        // Bước 0.25 dB được truyền đúng xuống board (msgid 38, số bước 0.25 dB
        // - xem DashboardManager::onAttUpdateRequested()).

        // ── ATT Row Component (tái sử dụng 3 lần) ───────────
        // ATT 1
        AttRow {
            channel:          1
            currentValue:     dashboard.rfController.att1
            Layout.fillWidth: true
        }

        // ATT 2
        AttRow {
            channel:          2
            currentValue:     dashboard.rfController.att2
            Layout.fillWidth: true
        }

        // ATT 3
        AttRow {
            channel:          3
            currentValue:     dashboard.rfController.att3
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }
    }

    component AttRow: RowLayout {
        id: attRow

        property int channel:         1
        property double currentValue: NaN
        // true trong lúc commit đến từ NÚT GỬI (lệnh tường minh -> luôn gửi,
        // kể cả khi giá trị không đổi); rời ô/Enter chỉ gửi khi giá trị đổi.
        property bool forceSend: false

        spacing: 12

        // F1956 thật bước 0.25dB - làm tròn về bước gần nhất trước khi gửi,
        // tránh gửi 1 giá trị người dùng gõ mà chính phần cứng cũng không thể
        // đặt được (vd 25.3 -> làm tròn 25.25... chọn bội số 0.25 gần nhất).
        function quantize(v) {
            return Math.round(v * 4) / 4
        }

        // NaN (chưa đặt) -> ô trống, hiện placeholder "--".
        function showValue() {
            attInput.value = attRow.currentValue
            attInput.text = isNaN(attRow.currentValue) ? "" : attRow.currentValue.toFixed(2)
        }

        Text {
            text:                  "ATT " + attRow.channel + " :"
            font.pixelSize:        12
            font.bold:             true
            color:                 AppTheme.textDark
            Layout.preferredWidth: 44
        }


        // ClampedTextField (qml/components/ClampedTextField.qml): dùng
        // RegularExpressionValidator đúng như yêu cầu (chỉ lọc ký tự lúc gõ)
        // + tự kẹp khoảng giá trị thật bằng JS ngay trong lúc gõ (không đợi
        // rời ô) - không còn locale: Qt.locale("en_US") (lỗi "Unable to
        // assign QLocale to QString" vì locale của Validator là QString tên
        // locale, không phải QLocale) và không còn lỗ hổng "999 vẫn hợp lệ"
        // của regex đếm ký tự đơn thuần.
        ClampedTextField {
            id:                      attInput
            minValue:                0
            maxValue:                dashboard.attMax
            decimals:                2
            value:                   attRow.currentValue
            horizontalAlignment:     TextInput.AlignHCenter
            verticalAlignment:       TextInput.AlignVCenter
            // Cao 24 (giữ NGUYÊN như bản gốc) - panel Suy hao bị khoá cứng
            // implicitHeight:200 trong GridLayout của Main.qml, đợt trước tôi
            // tăng lên 28 làm tổng nội dung tràn quá khung, ATT 3 bị cắt mất -
            // đúng lỗi trong ảnh bạn gửi. Rộng vẫn giữ 56 (thay vì 50 gốc) vì
            // đây chỉ ảnh hưởng chiều NGANG, panel còn dư chỗ ngang, không gây
            // tràn dọc.
            Layout.preferredWidth:   56
            Layout.preferredHeight:  24
            leftPadding:             6
            rightPadding:            6
            font.pixelSize:          12
            color :                 AppTheme.textDark
            placeholderText: "--"

            // F1956 thật bước 0.25dB - làm tròn về bước gần nhất SAU KHI đã
            // kẹp trong khoảng [0, attMax] (ClampedTextField lo phần kẹp
            // khoảng), rồi mới thực sự gửi lệnh xuống RfController.
            onCommitted: (v) => {
                if (isNaN(v)) {
                    // Ô bị xoá trắng: không có gì để gửi. Bấm nút gửi khi ô
                    // đang "--" thì báo rõ thay vì im lặng.
                    if (attRow.forceSend)
                        dashboard.systemNotification("Warning", qsTr("ATT %1: chưa nhập giá trị - chưa gửi.").arg(attRow.channel))
                    attRow.forceSend = false
                    return
                }
                var q = attRow.quantize(v)
                value = q
                text = q.toFixed(2)
                dashboard.rfController.requestSetAtt(attRow.channel, q, attRow.forceSend)
                attRow.forceSend = false
            }

            Connections {
                target: dashboard.rfController
                function onAtt1Changed() { if (attRow.channel === 1) attRow.showValue() }
                function onAtt2Changed() { if (attRow.channel === 2) attRow.showValue() }
                function onAtt3Changed() { if (attRow.channel === 3) attRow.showValue() }
            }

            background: BoxWrap{
                anchors.fill: parent
                radius : 6
            }
        }

        Button {
            id: sendButton
            // Giữ 24x24 (bằng đúng chiều cao hàng gốc) - lý do như TextField
            // ở trên, không được vượt quá 24 kẻo lại tràn khung panel.
            implicitWidth:  24
            implicitHeight: 24
            Layout.alignment: Qt.AlignVCenter

            background: Rectangle {
                color: "transparent"
                radius: 6
            }

            MouseArea {
                id: sendArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: sendButton.clicked()
            }

            // Icon trước đây 28x28 đặt trong 1 Item chỉ 16x16 -> ảnh tràn ra ngoài
            // ranh giới hàng cao 24px, đè lên ô kế bên (đúng lỗi "ATT tràn ra
            // khỏi ô" người dùng báo). Giờ icon 16x16 nằm gọn trong nút 24x24.
            Image {
                id: iconImage
                source: "qrc:/qt/qml/DashboardEnv/assets/images/SendIcon.png"
                width: 16
                height: 16
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit

                opacity: sendArea.pressed      ? 0.6
                       : sendArea.containsMouse ? 0.8
                       : 1.0

                Behavior on opacity {
                    NumberAnimation { duration: 120 }
                }
            }

            // Chốt giá trị đang gõ trong ô (dù chưa rời ô/nhấn Enter) giống
            // hệt như khi editingFinished tự nhiên - ClampedTextField phát
            // signal "editingFinished" là chuẩn TextInput nên gọi lại được.
            onClicked: {
                attRow.forceSend = true
                attInput.editingFinished()
            }
        }

    }
}
