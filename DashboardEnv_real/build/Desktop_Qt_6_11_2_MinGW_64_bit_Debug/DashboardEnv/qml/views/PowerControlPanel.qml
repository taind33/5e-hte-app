import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import DashboardEnv

Item {
    id : panel
    implicitHeight: 200
    implicitWidth: 250

    // Giá trị "đang soạn" (staged) cho 4 ô AC/DC giờ sống hẳn ở backend
    // (dashboard.stagedAcVolt/stagedAcFreq/stagedDcVolt/stagedDcCurrent -
    // xem DashboardManager) thay vì Qt.labs.settings ở đây: (1) Qt.labs.settings
    // đã bị Qt báo deprecate ("Please use the one from QtCore instead"),
    // (2) gộp về đúng 1 QSettings m_settings dùng chung với comPort/baudRate/
    // sessionName thay vì 2 cơ chế lưu khác nhau trong cùng 1 app. Vẫn CHỈ
    // lưu/hiển thị lên UI - KHÔNG tự gửi lệnh xuống nguồn AC/DC khi khởi
    // động app (chỉ khi bấm Bật/Tắt mới thực sự gửi, xem CustomSwitch bên
    // dưới) - đúng invariant "init không được động vào phần cứng".

    ColumnLayout {
        anchors.fill:  parent
        spacing:   10

        BoxWrap {
            Layout.fillWidth: true
            implicitHeight: panel.height/2-5
            ColumnLayout {
                anchors.fill : parent
                anchors.margins: 10
                spacing:  4
                RowLayout {
                    spacing :5
                    Image {
                        source:   "qrc:/qt/qml/DashboardEnv/assets/images/ACicon.png"
                        width:    24
                        height:   24
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        text: qsTr("Điện áp AC"); font.bold:  true;
                        font.pixelSize:  15;
                        color:AppTheme.textDark
                    }
                }
                RowLayout {

                    Layout.fillWidth:  true
                    spacing : 8
                    ColumnLayout {
                        Layout.preferredWidth: 60
                        spacing :3
                        Text {
                            text:qsTr("Điện áp"); font.pixelSize:  11;font.weight: Font.Medium;
                            color:AppTheme.textMuted
                        }
                        // ClampedTextField (qml/components/ClampedTextField.qml):
                        // RegularExpressionValidator (theo đúng yêu cầu) CHỈ lọc
                        // ký tự lúc gõ, còn khoảng giá trị thật (80-150V, lấy từ
                        // dashboard.acVoltMin/Max - board_config.h) được kẹp bằng
                        // JS ngay trong lúc gõ, không đợi rời ô.
                        ClampedTextField {
                            id: acVoltField
                            placeholderText: "--"
                            color: AppTheme.textDark
                            font.weight: Font.Medium
                            Layout.preferredWidth: 56
                            leftPadding: 6
                            rightPadding: 6
                            minValue: dashboard.acVoltMin
                            maxValue: dashboard.acVoltMax
                            decimals: 0
                            value: dashboard.stagedAcVolt
                            onCommitted: (v) => dashboard.stagedAcVolt = v
                            background:BoxWrap {
                                anchors.fill: parent
                                radius : 6
                            }
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 60
                        spacing :3
                        Text {
                            text:qsTr("Tần số"); font.pixelSize:  11;font.weight: Font.Medium;
                            color: AppTheme.textMuted
                        }
                        // acFreqMin/Max: firmware không giới hạn tần số AC, đây
                        // chỉ là chặn hợp lý phía UI (test rig chạy tới 1000Hz,
                        // không phải điện lưới 45-65Hz) - xem dashboardmanager.h.
                        ClampedTextField {
                            id: acFreqField
                            placeholderText: "--"
                            color: AppTheme.textDark
                            font.weight: Font.Medium
                            Layout.preferredWidth: 56
                            leftPadding: 6
                            rightPadding: 6
                            minValue: dashboard.acFreqMin
                            maxValue: dashboard.acFreqMax
                            decimals: 0
                            value: dashboard.stagedAcFreq
                            onCommitted: (v) => dashboard.stagedAcFreq = v
                            background:BoxWrap {
                                anchors.fill: parent
                                radius : 6
                            }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    ColumnLayout {
                        spacing :3
                        Layout.alignment: Qt.AlignVCenter
                        Text {
                            text : qsTr("Bật/Tắt") ; font.pixelSize: 11 ; font.weight: Font.Medium;
                            color :AppTheme.textMuted
                            Layout.alignment:  Qt.AlignHCenter
                        }
                        CustomSwitch {
                            id: acSwitch
                            objectName: "acPowerSwitch"
                            Layout.alignment: Qt.AlignHCenter
                            checked: dashboard.acSource.isOn
                            // Đang chờ nguồn xác nhận -> khoá, tránh bấm dồn lệnh.
                            enabled: !dashboard.acSource.pending
                            // onToggled (CHỈ khi người dùng bấm). requestAcPower() kiểm
                            // tra đã kết nối + đã nhập đủ giá trị (thiếu thì báo và không
                            // gửi). Bấm vào Switch làm MẤT binding "checked: isOn" - gắn
                            // lại ngay để công tắc luôn theo trạng thái backend (đang chờ
                            // = trạng thái yêu cầu, xong = trạng thái nguồn xác nhận).
                            onToggled: {
                                dashboard.requestAcPower(checked)
                                checked = Qt.binding(function() { return dashboard.acSource.isOn })
                            }
                            ToolTip.visible: hovered && dashboard.acSource.linkMessage.length > 0
                            ToolTip.text: dashboard.acSource.linkMessage
                        }
                    }
                }
            }
        }

        BoxWrap {
            Layout.fillWidth: true
            implicitHeight: panel.height/2-5
            ColumnLayout {
                anchors.fill : parent
                anchors.margins: 10
                spacing:  4

                RowLayout {
                    spacing :5
                    Image {
                        source:   "qrc:/qt/qml/DashboardEnv/assets/images/DCicon.png"
                        width:    24
                        height:   24
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {

                        text: qsTr("Điện áp DC"); font.bold:  true;
                        font.pixelSize:  15;
                        color: AppTheme.textDark
                    }
                }
                RowLayout {
                    Layout.fillWidth:  true
                    spacing : 8

                    ColumnLayout {
                        Layout.preferredWidth: 60
                        spacing :3
                        Text {
                            text:qsTr("Điện áp"); font.pixelSize:  11; font.weight: Font.Medium;
                            color: AppTheme.textMuted
                        }
                        // dcVoltMin/Max: firmware Device_ControlDCPwr() clamp [0,36].
                        ClampedTextField {
                            id: dcVoltField
                            placeholderText: "--"
                            color: AppTheme.textDark
                            font.weight: Font.Medium
                            Layout.preferredWidth: 56
                            leftPadding: 6
                            rightPadding: 6
                            minValue: dashboard.dcVoltMin
                            maxValue: dashboard.dcVoltMax
                            decimals: 0
                            value: dashboard.stagedDcVolt
                            onCommitted: (v) => dashboard.stagedDcVolt = v
                            background:BoxWrap {
                                anchors.fill: parent
                                radius : 6
                            }
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: 70
                        spacing :3
                        Text {
                            text:qsTr("Dòng điện"); font.pixelSize:  11; font.weight: Font.Medium;
                            color: AppTheme.textMuted
                        }
                        // dcCurrentMin/Max: firmware Device_ControlDCPwr() clamp
                        // [0,3] (cập nhật theo yêu cầu thực tế, trước ghi nhầm 5A).
                        ClampedTextField {
                            id: dcCurrentField
                            placeholderText: "--"
                            color: AppTheme.textDark
                            font.weight: Font.Medium
                            Layout.preferredWidth: 56
                            leftPadding: 6
                            rightPadding: 6
                            minValue: dashboard.dcCurrentMin
                            maxValue: dashboard.dcCurrentMax
                            decimals: 2
                            value: dashboard.stagedDcCurrent
                            onCommitted: (v) => dashboard.stagedDcCurrent = v
                            background: BoxWrap {
                                anchors.fill: parent
                                radius : 6
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout {
                        spacing :3
                        Layout.alignment: Qt.AlignVCenter
                        Text {
                            text : qsTr("Bật/Tắt") ; font.pixelSize: 11; font.weight: Font.Medium ;
                            color : AppTheme.textMuted
                            Layout.alignment:  Qt.AlignHCenter
                        }
                        CustomSwitch {
                            id: dcSwitch
                            objectName: "dcPowerSwitch"
                            Layout.alignment:  Qt.AlignHCenter
                            checked: dashboard.dcSource.isOn
                            enabled: !dashboard.dcSource.pending
                            // Xem chú thích công tắc AC ở trên.
                            onToggled: {
                                dashboard.requestDcPower(checked)
                                checked = Qt.binding(function() { return dashboard.dcSource.isOn })
                            }
                            ToolTip.visible: hovered && dashboard.dcSource.linkMessage.length > 0
                            ToolTip.text: dashboard.dcSource.linkMessage
                        }
                    }
                }
            }
        }
    }
}
