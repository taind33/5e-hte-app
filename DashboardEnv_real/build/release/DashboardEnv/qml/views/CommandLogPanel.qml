import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DashboardEnv

Item {
    id: panel
    implicitWidth:  250
    implicitHeight: 275

    BoxWrap {
        anchors.fill: parent
    }

    Connections {
        target: dashboard
        function onCommandLogChanged() {
            logFlick.contentY = Math.max(0, logContent.implicitHeight - logFlick.height)
        }
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 12
        spacing:         8

        // ── Header ──────────────────────────────────────────
        RowLayout {
            spacing: 6

            Text {
                text:             qsTr("Terminal")
                font.bold:        true
                font.pixelSize:   13
                color:            AppTheme.textDark
                Layout.fillWidth: true
            }
        }

        // ── Terminal Log View ───────────────────────────────
        Rectangle {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            color:            AppTheme.textLight
            radius:            6
            clip:              true

            Flickable {
                id:              logFlick
                anchors.fill:    parent
                anchors.margins: 8
                contentHeight:   logContent.implicitHeight
                // Cuộn xuống dòng cuối SAU KHI contentHeight đã cập nhật (lúc
                // commandLogChanged chiều cao vẫn là chiều cao cũ -> trước đây
                // luôn hụt mất dòng cuối).
                onContentHeightChanged: contentY = Math.max(0, contentHeight - height)
                clip:            true
                interactive: !logContent.activeFocus
                Column {
                    id:      logContent
                    width:   logFlick.width
                    spacing: 4

                    Repeater {
                        model: logOverride.visible ? [] : dashboard.commandLog

                        delegate: Text {
                            width:          logContent.width
                            text:           modelData
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            wrapMode:       Text.WrapAnywhere
                            color: {
                                if (modelData.indexOf("[ERR]") !== -1 || modelData.indexOf("[BOARD-LỖI]") !== -1)
                                    return AppTheme.brandRedPressed
                                if (modelData.indexOf("[TX]") !== -1)  return AppTheme.textRed
                                if (modelData.indexOf("[CMD]") !== -1) return AppTheme.infoNote
                                if (modelData.indexOf("[BOARD-CẢNH BÁO]") !== -1) return AppTheme.warningNote
                                return AppTheme.textDark
                            }
                        }
                    }
                }
            }

            Rectangle {
                id:      logOverride
                visible: false
                anchors.fill: parent
                color:        AppTheme.textDark
                radius:       6

            }
        }

        // ── Command input row ────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing:          5

            BoxWrap {
                implicitHeight: 30
                Layout.fillWidth: true
                radius : 6
                color : AppTheme.cmdPanelMuted
                borderColor : AppTheme.borderMuted
                RowLayout{
                    anchors.fill: parent
                    anchors.leftMargin: 3
                    Text {
                        text: qsTr("CMD:")
                        font.pixelSize: 12
                        color: AppTheme.textDark
                        font.weight: Font.Medium
                        Layout.preferredWidth: 40
                    }
                    TextField {
                        id: commandfield
                        font.weight: Font.Medium
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: AppTheme.textDark
                        placeholderText: qsTr("Gõ help để xem lệnh")
                        placeholderTextColor: AppTheme.textMuted

                        // Lịch sử lệnh (↑/↓) và hoàn thành bằng Tab.
                        property var history: []
                        property int historyPos: -1
                        property string draft: ""

                        Keys.onUpPressed: {
                            if (history.length === 0) return
                            if (historyPos === -1) { draft = text; historyPos = history.length }
                            if (historyPos > 0) historyPos--
                            text = history[historyPos]
                        }
                        Keys.onDownPressed: {
                            if (historyPos === -1) return
                            historyPos++
                            if (historyPos >= history.length) { historyPos = -1; text = draft }
                            else text = history[historyPos]
                        }
                        Keys.onTabPressed: (event) => {
                            var opts = dashboard.cliCompletions(text)
                            if (opts.length === 1) {
                                text = opts[0] + " "
                            } else if (opts.length > 1) {
                                // phần chung dài nhất của các gợi ý
                                var common = opts[0]
                                for (var i = 1; i < opts.length; i++) {
                                    var j = 0
                                    while (j < common.length && j < opts[i].length && common[j] === opts[i][j]) j++
                                    common = common.substring(0, j)
                                }
                                if (common.length > text.trim().length) text = common
                                else dashboard.executeCommand("help")
                            }
                            cursorPosition = text.length
                            event.accepted = true
                        }
                        background: Rectangle{
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color : AppTheme.textLight
                            radius: 6
                            border.color: AppTheme.borderMuted

                        }
                        Keys.onReturnPressed: submitCmd()
                        Keys.onEnterPressed:  submitCmd()

                        function submitCmd() {
                            var trimmed = commandfield.text.trim()
                            if (trimmed.length === 0) return
                            logOverride.visible = false
                            if (history.length === 0 || history[history.length - 1] !== trimmed) {
                                history.push(trimmed)
                                if (history.length > 50) history.shift()
                            }
                            historyPos = -1
                            draft = ""
                            dashboard.executeCommand(trimmed)
                            commandfield.text = ""
                        }
                    }
                }
            }
        }
    }
}