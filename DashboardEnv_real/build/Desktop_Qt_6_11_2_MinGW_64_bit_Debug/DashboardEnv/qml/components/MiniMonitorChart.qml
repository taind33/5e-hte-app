import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects
import QtCharts
import DashboardEnv

// Ô đồ thị 1 đại lượng của nguồn chuẩn (áp/dòng/tần số). Chỉ vẽ SỐ ĐO đọc
// ngược từ chính nguồn (msgid 42) - mỗi điểm là 1 lần đọc thật (nút ⟳, F5,
// hoặc lần xác nhận ngay sau lệnh bật/tắt). Chưa có số đo -> "--".
//
//  ┌──────────────────────────────────────────┐
//  │ [icon] Tiêu đề              [ 220.4V ] [⟳]│  ⟳ = đọc lại nguồn này
//  │              (đồ thị)                    │
//  │ Cập nhật 14:32:05          Max: …  Min: …│
//  └──────────────────────────────────────────┘
BoxWrap {
    id: root
    implicitWidth: 250
    implicitHeight: 200

    property string boxTitle: qsTr("Điện áp AC")
    property bool   isAC: true
    property string unit: "V"
    property string iconSource: isAC ? "qrc:/qt/qml/DashboardEnv/assets/images/ACicon.png"
                                     : "qrc:/qt/qml/DashboardEnv/assets/images/DCicon.png"
    property int    decimals: 2

    property var    dataSource: []
    property real   currentValue: 0.0
    property real   historyMaxVal: 0.0
    property real   historyMinVal: 0.0
    property alias  series: lineSeries

    // Nguồn (BasePowerSource) - trạng thái đọc: hasData/pending/linkState/
    // linkMessage/lastUpdate.
    property var    powerSource: null
    property bool   refreshEnabled: true
    property string refreshTip: qsTr("Làm mới (F5: làm mới tất cả)")
    signal refreshRequested()

    readonly property bool hasData: powerSource ? powerSource.hasData : dataSource.length > 0
    readonly property bool pending: powerSource ? powerSource.pending : false
    readonly property string linkState: powerSource ? powerSource.linkState : "none"
    readonly property bool linkBad: ["noreply", "error", "mismatch", "badreply", "notready", "boardnoreply"].indexOf(linkState) >= 0

    onDataSourceChanged: refreshSeries()
    Component.onCompleted: refreshSeries()

    // Trục tự co giãn theo đúng các điểm đang có.
    function refreshSeries() {
        lineSeries.clear()
        pointSeries.clear()
        var n = dataSource.length
        if (n === 0) {
            axisX.min = 0; axisX.max = 1
            axisY.min = 0; axisY.max = 1
            return
        }
        var lo = 1e300, hi = -1e300
        for (var i = 0; i < n; i++) {
            var pt = dataSource[i]
            lineSeries.append(pt.x, pt.y)
            pointSeries.append(pt.x, pt.y)
            if (pt.y < lo) lo = pt.y
            if (pt.y > hi) hi = pt.y
        }
        var x0 = dataSource[0].x, x1 = dataSource[n - 1].x
        if (x1 - x0 < 1e-6) { axisX.min = x0 - 1; axisX.max = x0 + 1 }
        else { var px = (x1 - x0) * 0.04; axisX.min = x0 - px; axisX.max = x1 + px }

        var pad = (hi - lo) > 1e-9 ? (hi - lo) * 0.15 : Math.max(Math.abs(hi) * 0.05, 0.05)
        // Đại lượng không âm (áp/dòng/tần số đo được) -> không kéo trục xuống dưới 0.
        axisY.min = (lo >= 0 && lo - pad < 0) ? 0 : lo - pad
        axisY.max = hi + pad
        axisY.applyNiceNumbers()   // vạch chia tròn số (0/50/100/150...)
    }

    function fmt(v) {
        return (Math.abs(v) >= 100 ? v.toFixed(1) : v.toFixed(root.decimals)) + root.unit
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 2

        // ── Header: icon + tiêu đề ......... [số] [⟳] ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Image {
                source: root.iconSource
                sourceSize.width: 16
                sourceSize.height: 16
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                fillMode: Image.PreserveAspectFit
            }
            // Tiêu đề co lại (…) khi ô hẹp - ô số và nút ⟳ luôn hiện đủ.
            Text {
                Layout.fillWidth: true
                Layout.minimumWidth: 20
                text: root.boxTitle
                elide: Text.ElideRight
                font.pixelSize: 15
                font.bold: true
                color: AppTheme.textDark
            }

            Rectangle {
                id: segment
                color: AppTheme.textDark
                radius: 4
                implicitWidth: Math.max(48, segText.implicitWidth + 12)
                implicitHeight: 25
                opacity: root.linkBad || root.linkState === "offline" ? 0.55 : 1.0

                Text {
                    id: segText
                    anchors.centerIn: parent
                    text: root.hasData ? root.fmt(root.currentValue) : "--"
                    color: AppTheme.textLight
                    font.family: "Seven Segment"
                    font.pointSize: 12
                }
                MultiEffect {
                    source:                  segment
                    anchors.fill:            segment
                    shadowEnabled:           true
                    shadowColor:             "#40000000"
                    shadowBlur:              0.3
                    shadowHorizontalOffset:  0
                    shadowVerticalOffset:    4
                    shadowScale:             1.0
                }
            }

            // Nút làm mới: cùng chiều cao ô số (24 ~ 25 px) -> header không cao
            // thêm, đồ thị không bị thu nhỏ.
            Item {
                id: refreshBtn
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                enabled: root.refreshEnabled && !root.pending
                opacity: enabled ? 1.0 : (root.pending ? 1.0 : 0.35)

                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: refreshArea.pressed ? AppTheme.panelBgMuted
                         : refreshArea.containsMouse && refreshBtn.enabled ? AppTheme.panelBg
                         : "transparent"
                    border.color: refreshArea.containsMouse && refreshBtn.enabled ? AppTheme.borderLight : "transparent"
                }
                Image {
                    id: refreshIcon
                    anchors.centerIn: parent
                    source: "qrc:/qt/qml/DashboardEnv/assets/images/refresh.png"
                    sourceSize.width: 16
                    sourceSize.height: 16
                    width: 16
                    height: 16
                    fillMode: Image.PreserveAspectFit
                    // Đang chờ nguồn trả lời: icon xoay.
                    RotationAnimator on rotation {
                        from: 0; to: 360; duration: 900
                        loops: Animation.Infinite
                        running: root.pending
                    }
                }
                Connections {
                    target: root
                    function onPendingChanged() { if (!root.pending) refreshIcon.rotation = 0 }
                }
                MouseArea {
                    id: refreshArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: refreshBtn.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: if (refreshBtn.enabled) root.refreshRequested()
                }
                ToolTip.visible: refreshArea.containsMouse
                ToolTip.delay: 400
                ToolTip.text: !root.refreshEnabled ? qsTr("Chưa kết nối board")
                            : root.pending ? qsTr("Đang chờ nguồn trả lời…")
                            : root.refreshTip
            }
        }

        ChartView {
            id: chart
            Layout.fillWidth: true
            Layout.fillHeight: true
            antialiasing: true
            legend.visible: false
            margins.top: 4
            margins.bottom: 4
            margins.left: 4
            margins.right: 4
            backgroundColor: "transparent"

            ValueAxis {
                id: axisX
                min: 0
                max: 1
                tickCount: 4
                labelsColor: "transparent"
                gridLineColor: AppTheme.borderMuted
            }

            ValueAxis {
                id: axisY
                min: 0
                max: 1
                tickCount: 4
                labelFormat: "%.4g"
                // Chưa có số đo: ẩn nhãn 0/0.33/0.67/1 vô nghĩa của trục rỗng.
                labelsVisible: root.dataSource.length > 0
                labelsFont.pixelSize: 8
                gridLineColor: AppTheme.borderMuted
                labelsColor: AppTheme.borderMuted
            }

            LineSeries {
                id: lineSeries
                axisX: axisX
                axisY: axisY
                color: AppTheme.brandRed
                width: 1.5
            }
            // Mỗi lần đọc là 1 điểm rời rạc (theo nút bấm) - đánh dấu rõ.
            ScatterSeries {
                id: pointSeries
                axisX: axisX
                axisY: axisY
                color: AppTheme.brandRed
                borderColor: AppTheme.brandRed
                markerSize: 5
            }
        }

        // ── Footer: trạng thái lần đọc gần nhất ......... Max/Min ──
        RowLayout {
            Layout.fillWidth: true

            Text {
                id: statusText
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pixelSize: 9
                readonly property string when: root.powerSource ? root.powerSource.lastUpdate : ""
                text: root.pending ? qsTr("Đang đọc…")
                    : root.linkState === "boardnoreply" ? qsTr("Board không trả kết quả")
                    : root.linkState === "noreply" ? qsTr("Nguồn không phản hồi") + (when ? " · " + when : "")
                    : root.linkState === "notready" ? qsTr("UART nguồn chưa sẵn sàng")
                    : root.linkBad ? qsTr("Lỗi") + (when ? " · " + when : "")
                    : root.linkState === "offline" ? qsTr("Đã ngắt kết nối") + (when ? " · " + when : "")
                    : root.linkState === "unconfirmed" ? qsTr("Chưa xác nhận được")
                    : when ? qsTr("Cập nhật %1").arg(when)
                    : qsTr("Chưa có số đo")
                color: root.linkBad ? AppTheme.brandRed
                     : root.linkState === "unconfirmed" ? AppTheme.warningNote
                     : AppTheme.textMuted

                MouseArea {
                    id: statusArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                }
                ToolTip.visible: statusArea.containsMouse && root.powerSource && root.powerSource.linkMessage.length > 0
                ToolTip.text: root.powerSource ? root.powerSource.linkMessage : ""
            }

            RowLayout {
                spacing: 8
                Text {
                    // Lịch sử rỗng: backend để max < min -> hiện "--".
                    text: "Max: " + (root.historyMaxVal < root.historyMinVal ? "--" : (root.historyMaxVal || 0).toFixed(2))
                    font.pixelSize: 9
                    color: AppTheme.textMuted
                }
                Text {
                    text: "Min: " + (root.historyMaxVal < root.historyMinVal ? "--" : (root.historyMinVal || 0).toFixed(2))
                    font.pixelSize: 9
                    color: AppTheme.textMuted
                }
            }
        }
    }
}
