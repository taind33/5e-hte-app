import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import DashboardEnv
import "../qml/views"
import "../qml/components"

ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 720
    // Trước là literal cứng "VASC Avionics System System Dashboard Control"
    // (lặp "System" 2 lần - lỗi gõ), đã sửa lỗi gõ ngay trong literal.
    // Bỏ bộ song ngữ Translations.qml "tự chế" (không phải Qt Linguist chuẩn)
    // theo đúng yêu cầu - quay lại literal tiếng Việt bọc qsTr() để khi có
    // mẫu Qt Linguist thật (.ts/.qm qua lupdate/lrelease) chỉ cần chạy
    // lupdate là tự trích được hết chuỗi, không phải sửa lại từng file.
    title: qsTr("Bảng Điều Khiển Hệ Thống Avionics VASC")
    color: AppTheme.mainBg

    // ── Nhớ giao diện giữa các lần mở (config.ini, nhóm [ui]) ──
    // Chỉ là trạng thái hiển thị - không liên quan thiết bị.
    Component.onCompleted: {
        var w = Number(dashboard.loadUi("windowWidth", 0))
        var h = Number(dashboard.loadUi("windowHeight", 0))
        if (w >= 800 && h >= 500) { width = w; height = h }
        var x0 = dashboard.loadUi("windowX", "")
        var y0 = dashboard.loadUi("windowY", "")
        if (x0 !== "" && y0 !== "") {
            // Chỉ đặt lại vị trí nếu vẫn nằm trong màn hình hiện có (tránh
            // cửa sổ "biến mất" khi lần trước mở trên màn hình phụ đã rút).
            var xi = Number(x0), yi = Number(y0)
            if (xi > -width + 100 && xi < Screen.desktopAvailableWidth - 100 && yi >= 0 && yi < Screen.desktopAvailableHeight - 100) {
                x = xi; y = yi
            }
        }
        if (String(dashboard.loadUi("maximized", "false")) === "true")
            visibility = Window.Maximized
        AppTheme.isDarkMode = String(dashboard.loadUi("darkMode", "false")) === "true"
    }
    onClosing: {
        var maximized = (visibility === Window.Maximized)
        dashboard.saveUi("maximized", maximized)
        if (!maximized) {
            dashboard.saveUi("windowWidth", width)
            dashboard.saveUi("windowHeight", height)
            dashboard.saveUi("windowX", x)
            dashboard.saveUi("windowY", y)
        }
    }
    Connections {
        target: AppTheme
        function onIsDarkModeChanged() { dashboard.saveUi("darkMode", AppTheme.isDarkMode) }
    }

    // F5: đọc tức thời cả 2 nguồn (AC + DC) - chỉ đọc, không đổi gì trên nguồn.
    Shortcut {
        sequence: "F5"
        context: Qt.ApplicationShortcut
        onActivated: dashboard.refreshAllPower()
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 12
        columns: 5
        rows: 2
        columnSpacing: 12
        rowSpacing: 12

        ConnectionPanel {
            // Trước hardcode color:"#F5F5F5" trong khi 5/8 panel còn lại
            // (PowerControlPanel, CommandLogPanel, Notification, Detect,
            // EnvelopeChartPanel...) dùng mặc định trắng của BoxWrap - làm 3
            // panel này xám nhẹ hơn phần còn lại 1 cách không chủ ý, và bị
            // "đóng băng" trắng/xám dù bật dark mode. Bỏ override, để BoxWrap
            // tự theo AppTheme.elementBg - đồng nhất màu mọi card + theo theme.
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        PowerControlPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        TimeDelay {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Attenuation {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }


        ColumnLayout {
                    Layout.rowSpan: 2
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 12

                    BoxWrap {
                        id: newHalfBox
                        // Tương tự - bỏ hardcode "#FFFFFF" để theo dark mode.
                        radius: 6

                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        Layout.maximumHeight: 100
                        ColumnLayout{
                            Layout.margins: 8
                            Layout.fillWidth:  true
                            Layout.fillHeight: true
                            spacing:  5
                            RowLayout{
                                Item{Layout.fillWidth: true}

                                // Trước là chuyển đổi VN/EN tự chế qua Translations.qml - đã bỏ
                                // (xem comment ở title phía trên, đầu file). Giữ chỗ icon ngôn
                                // ngữ theo ảnh language.png người dùng cung cấp, TẠM để trang
                                // trí/placeholder (chưa gắn logic đổi ngôn ngữ - sẽ nối vào khi
                                // có bộ Qt Linguist thật, .ts/.qm qua lupdate/lrelease theo mẫu
                                // người dùng sẽ gửi).
                                Button {
                                    implicitWidth: 34
                                    implicitHeight: 34
                                    ToolTip.visible: langMouse.containsMouse
                                    ToolTip.text: qsTr("Ngôn ngữ (đang phát triển)")

                                    background: Rectangle {
                                        color: "transparent"
                                        radius: 6
                                    }

                                    contentItem: Item {
                                        implicitWidth: 34
                                        implicitHeight: 34

                                        Image {
                                            source: "qrc:/qt/qml/DashboardEnv/assets/images/language.png"
                                            width: 26
                                            height: 26
                                            anchors.centerIn: parent
                                            fillMode: Image.PreserveAspectFit
                                        }

                                        MouseArea {
                                            id: langMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                    }
                                }

                                Button {
                                    implicitWidth: 34
                                    implicitHeight: 34
                                    // Trước đây 2 nút Sun/Moon không có onClicked - chỉ là hình
                                    // trang trí, AppTheme.isDarkMode/toggleTheme() tồn tại nhưng
                                    // không nút nào gọi tới. Giờ bấm Sun -> luôn về chế độ sáng.
                                    onClicked: if (AppTheme.isDarkMode) AppTheme.toggleTheme()

                                    background: Rectangle {
                                        color: "transparent"
                                        radius: 6
                                    }

                                    contentItem: Item {
                                        implicitWidth: 34
                                        implicitHeight: 34

                                        Image {
                                            source: "qrc:/qt/qml/DashboardEnv/assets/images/Sun.png"
                                            width: 28
                                            height: 28
                                            anchors.centerIn: parent
                                            fillMode: Image.PreserveAspectFit
                                            anchors.verticalCenter: parent.verticalCenter
                                            opacity: AppTheme.isDarkMode ? 0.4 : 1.0
                                        }
                                    }

                                }
                                Button {
                                    implicitWidth: 34
                                    implicitHeight: 34
                                    // Bấm Moon -> luôn về chế độ tối (thay vì chỉ toggle mù mờ,
                                    // 2 nút này đọc như "chọn Sáng" / "chọn Tối", đúng ý đồ ở mockup).
                                    onClicked: if (!AppTheme.isDarkMode) AppTheme.toggleTheme()

                                    background: Rectangle {
                                        color: "transparent"
                                        radius: 6
                                    }

                                    contentItem: Item {
                                        implicitWidth: 34
                                        implicitHeight: 34

                                        Image {
                                            source: "qrc:/qt/qml/DashboardEnv/assets/images/Moon.png"
                                            width: 34
                                            height: 34
                                            anchors.centerIn: parent
                                            fillMode: Image.PreserveAspectFit
                                            anchors.verticalCenter: parent.verticalCenter
                                            opacity: AppTheme.isDarkMode ? 1.0 : 0.4
                                        }
                                    }

                                }
                            }
                            RowLayout{
                                Layout.margins: 10
                                spacing :4
                                ColumnLayout{
                                    spacing :3
                                    Text{
                                        text : qsTr("Phiên bản") + " : 98vf8b4" ; font.pixelSize: 10;
                                    }
                                    // Firmware đang chạy trên board (gói DIAG) - biết ngay
                                    // đã nạp đúng bản mới chưa.
                                    Text{
                                        text : "FW : " + (dashboard.boardFirmware.length > 0 ? dashboard.boardFirmware : "--") ; font.pixelSize: 10;
                                    }
                                    Text{
                                        text : qsTr("Trung tâm Hệ thống Avionics") ; font.pixelSize: 10;
                                    }
                                }
                                Image {
                                    source:   "qrc:/qt/qml/DashboardEnv/assets/images/Viettel_Aerospace_Institute_logo.png"
                                    width:    40
                                    height:   40
                                    fillMode: Image.PreserveAspectFit
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }

                        }

                    }
                    Notification {
                        Layout.fillHeight : true
                        Layout.fillWidth: true
                    }
        }

        RowLayout {
            Layout.columnSpan: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // 4 đồ thị = SỐ ĐO đọc từ chính nguồn (không phải giá trị đặt).
            // Nút ⟳ trên mỗi ô đọc lại nguồn đó (2 ô AC dùng chung 1 lần đọc
            // nên cùng cập nhật), F5 đọc cả 2 nguồn.
            MiniMonitorChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                boxTitle: qsTr("Điện áp AC")
                isAC: true
                unit: "V"
                currentValue: dashboard.acSource.voltage
                dataSource: dashboard.acSource.voltageHistory
                historyMaxVal: dashboard.acSource.voltageMax
                historyMinVal: dashboard.acSource.voltageMin
                powerSource: dashboard.acSource
                refreshEnabled: dashboard.isConnected
                refreshTip: qsTr("Làm mới nguồn AC (F5: làm mới tất cả)")
                onRefreshRequested: dashboard.refreshPower(1)
            }
            MiniMonitorChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                boxTitle: qsTr("Tần số AC")
                isAC: true
                iconSource: "qrc:/qt/qml/DashboardEnv/assets/images/freq.png"
                unit: "Hz"
                currentValue: dashboard.acSource.freq
                dataSource: dashboard.acSource.freqHistory
                historyMaxVal: dashboard.acSource.freqMax
                historyMinVal: dashboard.acSource.freqMin
                powerSource: dashboard.acSource
                refreshEnabled: dashboard.isConnected
                refreshTip: qsTr("Làm mới nguồn AC (F5: làm mới tất cả)")
                onRefreshRequested: dashboard.refreshPower(1)
            }
            MiniMonitorChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                boxTitle: qsTr("Điện áp DC")
                isAC: false
                unit: "V"
                currentValue: dashboard.dcSource.voltage
                dataSource: dashboard.dcSource.voltageHistory
                historyMaxVal: dashboard.dcSource.voltageMax
                historyMinVal: dashboard.dcSource.voltageMin
                powerSource: dashboard.dcSource
                refreshEnabled: dashboard.isConnected
                refreshTip: qsTr("Làm mới nguồn DC (F5: làm mới tất cả)")
                onRefreshRequested: dashboard.refreshPower(2)
            }
            MiniMonitorChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                boxTitle: qsTr("Dòng điện DC")
                isAC: false
                unit: "A"
                decimals: 3
                currentValue: dashboard.dcSource.current
                dataSource: dashboard.dcSource.currentHistory
                historyMaxVal: dashboard.dcSource.currentMax
                historyMinVal: dashboard.dcSource.currentMin
                powerSource: dashboard.dcSource
                refreshEnabled: dashboard.isConnected
                refreshTip: qsTr("Làm mới nguồn DC (F5: làm mới tất cả)")
                onRefreshRequested: dashboard.refreshPower(2)
            }
        }

        CommandLogPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        EnvelopeChartPanel {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Detect {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}