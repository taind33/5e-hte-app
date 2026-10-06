import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import DashboardEnv

BoxWrap{
    id: panel
    implicitHeight: 200
    implicitWidth:  250

    ColumnLayout{
        anchors.fill:  parent
        anchors.margins: AppTheme.paddingNormal
        spacing: 8

        RowLayout {
            spacing: 6
            Image {
                source:   "qrc:/qt/qml/DashboardEnv/assets/images/Connected.png"
                width:    24
                height:   24
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text : qsTr("Kết nối")
                font.pixelSize :15
                font.bold: true
                color :AppTheme.textDark
            }
            // Lý do ngắn gọn khi cổng đã mở mà CHƯA xác thực được board (chi
            // tiết đầy đủ: rê chuột vào đèn, hoặc thông báo đỏ sau ~4 s).
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                font.pixelSize: 10
                color: AppTheme.warningNote
                visible: dashboard.isConnected && !dashboard.isPortVerified
                text: dashboard.linkDiagnosisShort
            }
            Item {
                Layout.fillWidth: true
                visible: !(dashboard.isConnected && !dashboard.isPortVerified)
            }

            // Đèn xác thực đúng COM - xanh khi board 5E-HTE thật đã trả lời
            // PING (dashboard.isPortVerified), xám khi chưa mở cổng, đỏ nhạt
            // khi cổng mở nhưng chưa/không thấy phản hồi (khả năng nhầm cổng
            // AC/DC nguồn qua cùng hub USB - xem sendPingProbe() phía C++).
            Rectangle {
                id: verifyLight
                width: 10; height: 10
                radius: 5
                Layout.alignment: Qt.AlignVCenter
                color: !dashboard.isConnected ? AppTheme.textMuted
                     : dashboard.isPortVerified ? AppTheme.successNote
                     : AppTheme.warningNote

                // Lý do cụ thể (đếm byte/khung nhận được) - xem
                // DashboardManager::updateLinkDiagnosis().
                ToolTip {
                    visible: lightHover.containsMouse
                    delay: 300
                    width: 320
                    contentItem: Text {
                        wrapMode: Text.WordWrap
                        font.pixelSize: 11
                        color: AppTheme.textLight
                        text: !dashboard.isConnected ? qsTr("Chưa kết nối")
                              : dashboard.linkDiagnosis
                    }
                    background: Rectangle { color: AppTheme.textDark; radius: 6 }
                }

                MouseArea {
                    id: lightHover
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                }
            }
        }

        BoxWrap{
            implicitHeight: 30
            Layout.fillWidth: true
            radius : 6
            RowLayout{
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 8
                Text { text: qsTr("Chọn COM:"); font.pixelSize: 12; color: AppTheme.textDark;font.weight: Font.Medium; Layout.preferredWidth: 70 }
                CustomComboBox {
                    id: comNameCbb
                    model: dashboard.availablePorts
                    // Cổng đã lưu không còn trong danh sách (hoặc chưa từng chọn)
                    // -> -1 = "--", KHÔNG tự chọn cổng đầu tiên.
                    currentIndex: dashboard.availablePorts.indexOf(dashboard.comPort)
                    onActivated: (index) => dashboard.comPort = currentText
                    onPressedChanged: if(pressed) dashboard.refreshPorts()
                }
            }

        }
        BoxWrap {
            implicitHeight: 30
            Layout.fillWidth: true
            radius : 6
            RowLayout{
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 8
                Text { text: qsTr("Chọn Baud:"); font.pixelSize: 12; color: AppTheme.textDark;font.weight : Font.Medium; Layout.preferredWidth: 70 }
                CustomComboBox {
                    id: baudCbb
                    Layout.fillWidth: true
                    model: ["9600", "19200", "38400", "115200","921600"]
                    currentIndex: model.indexOf(dashboard.baudRate.toString())
                    onActivated: (index) => dashboard.baudRate = parseInt(currentText)
                }
            }

        }
        BoxWrap {
            implicitHeight: 30
            Layout.fillWidth: true
            radius : 6
            RowLayout{
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 8
                Text { text: qsTr("Tên phiên:"); font.pixelSize: 12; color: AppTheme.textDark;font.weight : Font.Medium; Layout.preferredWidth: 70 }
                CustomBoxFiled {
                    id: sessionNameField
                    text:dashboard.sessionName
                    placeholderText: "--"
                    font.weight : Font.Light
                    onEditingFinished: dashboard.sessionName= text
                }
            }
        }

        // Item co giãn hút hết khoảng trống còn lại của panel khi cửa sổ
        // scale to ra - TRƯỚC đây hàng nút bên dưới tự "Layout.fillHeight:
        // parent" (thực chất là gán 1 Item cho property bool, JS coi object
        // là truthy nên vô tình chạy đúng như "true", nhưng để hàng nút hút
        // hết chiều cao dư ra) khiến 2 nút bị kéo cao bất thường/sai tỉ lệ
        // khi panel lớn lên. Giờ khoảng trống dư đó dồn vào Item rỗng này,
        // còn hàng nút giữ chiều cao cố định hợp lý (xem preferredHeight bên
        // dưới) ở mọi kích thước cửa sổ.
        Item { Layout.fillHeight: true }

        // Trước đây mỗi nút có thêm 1 MouseArea con lồng bên trong contentItem
        // (Text) chỉ để gọi lại onClicked - nhưng MouseArea con đó "ăn" mất sự
        // kiện chuột trước khi Button gốc kịp nhận, nên "startbutton.down"
        // (dùng để đổi màu khi nhấn) KHÔNG BAO GIỜ true - nút mất hẳn phản hồi
        // khi bấm, nhìn phẳng/không rõ ràng như đang bị "scale nhỏ". Bỏ hẳn các
        // MouseArea thừa này, để Button tự xử lý hover/press (đã đủ, không cần
        // thêm gì) - đồng thời tăng chiều cao/cỡ chữ + thêm bóng đổ cho nút rõ
        // ràng, nổi bật hơn, đồng bộ với các card số (segment display) khác.
        //
        // Chiều cao 36 -> 42: lần trước tăng lên 44 làm tràn ngân sách vì lúc
        // đó margin/spacing của các hàng phía trên (COM/Baud/Tên phiên) CÙNG
        // tăng theo cỡ chữ đợt sửa đó. Lần này CHỈ tăng riêng actionRow, giữ
        // nguyên 3 hàng field (implicitHeight:30) và spacing (8) như cũ, nên
        // ngân sách dư ra đúng bằng phần Item{Layout.fillHeight:true} co lại,
        // không đụng tới 3 hàng field phía trên - đã dựng ảnh thật kiểm tra
        // bằng offscreen QQuickView (xem verify_layout.md) để xác nhận không
        // còn tràn/cắt panel trước khi gửi.
        RowLayout{
            id: actionRow
            spacing: 10
            Layout.fillWidth: true
            Layout.preferredHeight: 42

            Button{
                id : startbutton
                text : dashboard.isConnected ? qsTr("Ngắt kết nối") : qsTr("BẮT ĐẦU PHIÊN")
                Layout.fillWidth: true
                Layout.fillHeight: true
                hoverEnabled: true

                background:  Rectangle{
                    id: startBg
                    color : startbutton.down ? AppTheme.brandRedPressed
                          : startbutton.hovered ? AppTheme.brandRedHover
                          : AppTheme.brandRed
                    radius :  AppTheme.radiusControl

                    Behavior on color { ColorAnimation { duration: 100 } }
                }
                contentItem:  Text {
                    text : startbutton.text
                    color: AppTheme.textLight
                    font.bold: true
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MultiEffect {
                    source:                 startBg
                    anchors.fill:           startBg
                    shadowEnabled:          true
                    shadowColor:            "#40000000"
                    shadowBlur:             0.3
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset:   2
                    shadowScale:            1.0
                    z: -1
                }

                onClicked: {
                    if(dashboard.isConnected) {
                        dashboard.stopSession()
                        comNameCbb.enabled = true
                        baudCbb.enabled = true
                    }
                    else {
                        dashboard.startSession()
                        comNameCbb.enabled = false
                        baudCbb.enabled = false
                    }
                }
            }

            Button{
                id: openbutton
                text : qsTr("MỞ LOG")
                Layout.fillWidth: true
                Layout.fillHeight: true
                hoverEnabled: true

                background:  Rectangle{
                    id: openBg
                    color : openbutton.down ? AppTheme.brandRedPressed
                          : openbutton.hovered ? AppTheme.brandRedHover
                          : AppTheme.brandRed
                    radius : AppTheme.radiusControl

                    Behavior on color { ColorAnimation { duration: 100 } }
                }
                contentItem:  Text {
                    text : openbutton.text
                    color: AppTheme.textLight
                    font.bold: true
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MultiEffect {
                    source:                 openBg
                    anchors.fill:           openBg
                    shadowEnabled:          true
                    shadowColor:            "#40000000"
                    shadowBlur:             0.3
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset:   2
                    shadowScale:            1.0
                    z: -1
                }

                onClicked:  dashboard.openLogFile()
            }
        }
    }
}
