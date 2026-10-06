import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtCharts
import DashboardEnv

// Phổ RF đầu vào RX1 của AD9361 (firmware stream_framer.c: DMA 32 mẫu I/Q ->
// FFT 32 điểm -> gói "WAVE" ~300 ms/lần; cấu hình RX qua gói "RFST").
// Mọi xử lý số (dBFS, fftshift, trục tần số, đỉnh, tổng công suất, hiệu
// chuẩn) nằm ở DashboardManager::processWaveMagnitudes() - file này chỉ vẽ.
BoxWrap {
    id: panel
    implicitWidth:  770
    implicitHeight: 275

    readonly property var rf: dashboard.rfController

    // Tooltip
    property bool   tooltipVisible: false
    property real   tooltipX:       0
    property real   tooltipY:       0
    property string tooltipText:    ""

    function fmtX(x) {
        return panel.rf.xIsFrequency ? x.toFixed(3) + " MHz" : "bin " + Math.round(x)
    }

    function populateSeries() {
        waveSeries.clear()
        var pts = panel.rf.waveData
        for (var i = 0; i < pts.length; i++)
            waveSeries.append(pts[i].x, pts[i].y)
        axisX.min = panel.rf.xMin
        axisX.max = panel.rf.xMax
        axisY.min = panel.rf.yMin
        axisY.max = panel.rf.yMax
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 10
        spacing:         4

        // ── Header: tiêu đề + trạng thái luồng + cấu hình RX ─────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text:           qsTr("Phổ RF · RX1")
                font.bold:      true
                font.pixelSize: 13
                color:          AppTheme.textDark
            }

            Rectangle {
                id: statusPill
                radius: 8
                implicitHeight: 18
                implicitWidth:  statusText.implicitWidth + 16
                color: panel.rf.waveSource === "live"    ? AppTheme.successNote
                     : panel.rf.waveSource === "waiting" ? (panel.rf.diagLevel === "error" ? AppTheme.brandRed : AppTheme.warningNote)
                     : AppTheme.textMuted
                Text {
                    id: statusText
                    anchors.centerIn: parent
                    font.pixelSize: 10
                    font.bold:      true
                    color:          AppTheme.textLight
                    text: panel.rf.waveSource === "live"    ? qsTr("Dữ liệu thật")
                        : panel.rf.waveSource === "waiting" ? qsTr("Chưa nhận phổ")
                        : qsTr("Mô phỏng")
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                Layout.alignment: Qt.AlignRight
                font.pixelSize: 11
                color: AppTheme.textMuted
                elide: Text.ElideLeft
                Layout.maximumWidth: panel.width * 0.6
                text: panel.rf.rfStatusValid
                      ? qsTr("LO %1 MHz · Fs %2 MSPS · Gain %3 dB (%4)")
                            .arg(panel.rf.rxLoMHz.toFixed(3)).arg(panel.rf.rxFsMHz.toFixed(2))
                            .arg(panel.rf.rxGainDb).arg(panel.rf.rxGainMode)
                      : (dashboard.isConnected ? qsTr("Chưa nhận cấu hình RX (RFST)") : "")
            }
        }

        // ── Chart ─────────────────────────────────────────────────────────
        Item {
            id: chartArea
            Layout.fillWidth:  true
            Layout.fillHeight: true

            ChartView {
                id:              mainChart
                anchors.fill:    parent
                anchors.margins: -8
                antialiasing:    true
                legend.visible:  false
                backgroundColor: "transparent"

                ValueAxis {
                    id:          axisX
                    min:         -16
                    max:         15
                    tickCount:   5
                    labelsColor: AppTheme.textMuted
                    labelsFont.pixelSize: 10
                    gridLineColor: AppTheme.chartGrid
                    labelFormat: panel.rf.xIsFrequency ? "%.1f" : "%d"
                }

                ValueAxis {
                    id:             axisY
                    min:            -100
                    max:            0
                    tickCount:      6
                    labelsColor:    AppTheme.textMuted
                    labelsFont.pixelSize: 10
                    gridLineColor:  AppTheme.chartGrid
                    labelFormat:    "%d"
                }

                LineSeries {
                    id:    waveSeries
                    axisX: axisX
                    axisY: axisY
                    color: panel.rf.waveSource === "live" ? AppTheme.brandRed : AppTheme.textMuted
                    width: 2.5

                    Component.onCompleted: panel.populateSeries()

                    Connections {
                        target: panel.rf
                        function onWaveDataChanged() { panel.populateSeries() }
                    }
                }
            }

            // Đơn vị trục Y (dBFS khi chưa hiệu chuẩn, dBm khi đã hiệu chuẩn)
            Text {
                x: mainChart.x + mainChart.plotArea.x - 2
                y: mainChart.y + mainChart.plotArea.y - 14
                text: panel.rf.spectrumUnit
                font.pixelSize: 10
                color: AppTheme.textMuted
            }
            Text {
                anchors.right:  parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -2
                text: panel.rf.xIsFrequency ? "MHz" : "bin"
                font.pixelSize: 10
                color: AppTheme.textMuted
            }

            // Đang có phổ nhưng board báo vấn đề (ADC bão hoà, DMA đôi lúc
            // timeout) - băng cảnh báo nhỏ phía trên đồ thị.
            Rectangle {
                visible: panel.rf.waveSource === "live" && panel.rf.diagText.length > 0
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 6
                width:  liveWarnText.implicitWidth + 20
                height: liveWarnText.implicitHeight + 8
                radius: 6
                color:  panel.rf.diagLevel === "error" ? AppTheme.brandRed : AppTheme.warningNote
                Text {
                    id: liveWarnText
                    anchors.centerIn: parent
                    font.pixelSize: 10
                    font.bold: true
                    color: AppTheme.textLight
                    text: panel.rf.diagText
                }
            }

            // Đã kết nối nhưng chưa/mất luồng WAVE - nói rõ thay vì để trống.
            Rectangle {
                visible: panel.rf.waveSource === "waiting"
                anchors.centerIn: parent
                width:  waitText.implicitWidth + 24
                height: waitText.implicitHeight + 16
                radius: 8
                color:  AppTheme.panelBg
                border.color: panel.rf.diagLevel === "error" ? AppTheme.brandRed : AppTheme.warningNote
                border.width: 1
                Text {
                    id: waitText
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 11
                    color: AppTheme.textDark
                    // Lý do cụ thể do chính board báo (gói DIAG): AD9361 lỗi,
                    // DMA không về dữ liệu... - không cần cắm console.
                    text: panel.rf.diagText.length > 0 ? panel.rf.diagText
                          : dashboard.isPortVerified ? qsTr("Chưa nhận phổ RF (WAVE) từ board.")
                          : qsTr("Chưa nhận phổ RF: board chưa trả lời (xem đèn Kết nối).")
                }
            }

            MouseArea {
                id:           chartHover
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton

                onPositionChanged: function(mouse) {
                    var pa = mainChart.plotArea
                    var mx = mouse.x - mainChart.x
                    var my = mouse.y - mainChart.y
                    if (waveSeries.count === 0 ||
                        mx < pa.x || mx > pa.x + pa.width ||
                        my < pa.y || my > pa.y + pa.height) {
                        panel.tooltipVisible = false
                        return
                    }
                    var xRatio = (mx - pa.x) / pa.width
                    var dataX  = axisX.min + xRatio * (axisX.max - axisX.min)

                    // Điểm gần nhất theo trục X (trục X là MHz, không còn là chỉ số).
                    var best = 0
                    var bestD = Number.MAX_VALUE
                    for (var i = 0; i < waveSeries.count; i++) {
                        var d = Math.abs(waveSeries.at(i).x - dataX)
                        if (d < bestD) { bestD = d; best = i }
                    }
                    var p = waveSeries.at(best)
                    panel.tooltipText = p.y.toFixed(1) + " " + panel.rf.spectrumUnit + "  @ " + panel.fmtX(p.x)
                    panel.tooltipVisible = true

                    var tx = mouse.x - tooltipBox.width / 2
                    var ty = mouse.y - tooltipBox.height - 14
                    panel.tooltipX = Math.max(4, Math.min(tx, chartArea.width  - tooltipBox.width  - 4))
                    panel.tooltipY = Math.max(4, Math.min(ty, chartArea.height - tooltipBox.height - 4))
                }
                onExited: panel.tooltipVisible = false
            }

            Rectangle {
                id:      tooltipBox
                visible: panel.tooltipVisible
                x:       panel.tooltipX
                y:       panel.tooltipY
                width:   tooltipLabel.implicitWidth + 20
                height:  28
                color:   "#1E293B"
                radius:  6
                opacity: panel.tooltipVisible ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 100 } }

                Text {
                    id:               tooltipLabel
                    anchors.centerIn: parent
                    text:             panel.tooltipText
                    color:            AppTheme.textLight
                    font.bold:        true
                    font.pixelSize:   11
                }
            }

            Rectangle {
                visible: panel.tooltipVisible
                x:       chartHover.mouseX
                y:       mainChart.y + mainChart.plotArea.y
                width:   1
                height:  mainChart.plotArea.height
                color:   AppTheme.textMuted
                opacity: 0.5
            }
        }

        // ── Footer: kết quả đo + hiệu chuẩn ─────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                font.pixelSize: 12
                font.bold:      true
                color:          AppTheme.textDark
                text: panel.rf.waveData.length > 0
                      ? qsTr("Tổng: %1 %2").arg(panel.rf.totalLevel.toFixed(1)).arg(panel.rf.spectrumUnit)
                      : qsTr("Tổng: --")
            }
            Text {
                font.pixelSize: 11
                color:          AppTheme.textMuted
                text: panel.rf.waveData.length > 0
                      ? qsTr("Đỉnh: %1 %2 @ %3").arg(panel.rf.peakLevel.toFixed(1)).arg(panel.rf.spectrumUnit).arg(panel.fmtX(panel.rf.peakX))
                      : qsTr("Đỉnh: --")
            }

            Item { Layout.fillWidth: true }

            Text {
                font.pixelSize: 11
                color: panel.rf.calMismatch ? AppTheme.warningNote : AppTheme.textMuted
                text: !panel.rf.calibrated ? qsTr("Chưa hiệu chuẩn")
                      : panel.rf.calMismatch ? qsTr("Hiệu chuẩn ở LO/gain khác!")
                      : qsTr("Offset %1 dB").arg(panel.rf.calOffsetDb.toFixed(1))
            }

            TextField {
                id: refInput
                Layout.preferredWidth:  64
                Layout.preferredHeight: 22
                font.pixelSize: 11
                leftPadding: 4; rightPadding: 4; topPadding: 0; bottomPadding: 0
                horizontalAlignment: TextInput.AlignHCenter
                verticalAlignment:   TextInput.AlignVCenter
                color: AppTheme.textDark
                placeholderText: "dBm"
                selectByMouse: true
                // Nhớ mức chuẩn đã nhập lần trước (config.ini, [ui]).
                Component.onCompleted: text = String(dashboard.loadUi("rfRefDbm", ""))
                onEditingFinished: dashboard.saveUi("rfRefDbm", text)
                // Công suất chuẩn (dBm) đưa vào RX1 - cho phép số âm.
                validator: RegularExpressionValidator { regularExpression: /^-?\d{0,3}(\.\d{0,2})?$/ }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Công suất chuẩn (dBm) đang đưa vào RX1")
                background: Rectangle {
                    radius: 5
                    color: AppTheme.elementBg
                    border.color: AppTheme.borderMuted
                    border.width: 1
                }
            }

            Button {
                id: calButton
                implicitHeight: 22
                implicitWidth:  calLabel.implicitWidth + 16
                enabled: panel.rf.waveSource === "live" && refInput.text.length > 0 && !isNaN(parseFloat(refInput.text))
                onClicked: dashboard.calibrateRf(parseFloat(refInput.text))
                background: Rectangle {
                    radius: 5
                    color: calButton.enabled ? (calButton.pressed ? AppTheme.brandRedPressed : AppTheme.brandRed)
                                             : AppTheme.cmdPanelMuted
                }
                contentItem: Text {
                    id: calLabel
                    text: qsTr("Hiệu chuẩn")
                    font.pixelSize: 11
                    font.bold: true
                    color: AppTheme.textLight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment:   Text.AlignVCenter
                }
            }

            Button {
                id: clearCalButton
                implicitHeight: 22
                implicitWidth:  clearLabel.implicitWidth + 14
                enabled: panel.rf.calibrated
                onClicked: dashboard.clearRfCalibration()
                background: Rectangle {
                    radius: 5
                    color: "transparent"
                    border.color: AppTheme.borderMuted
                    border.width: 1
                    opacity: clearCalButton.enabled ? 1.0 : 0.4
                }
                contentItem: Text {
                    id: clearLabel
                    text: qsTr("Xoá")
                    font.pixelSize: 11
                    color: AppTheme.textDark
                    opacity: clearCalButton.enabled ? 1.0 : 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment:   Text.AlignVCenter
                }
            }
        }
    }
}
