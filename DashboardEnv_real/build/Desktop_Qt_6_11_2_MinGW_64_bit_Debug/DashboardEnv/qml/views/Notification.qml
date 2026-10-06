import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import DashboardEnv


Item {
    id: panel
    implicitWidth:  220
    implicitHeight: 308


    BoxWrap {
        anchors.fill: parent
    }

    ListModel {
        id: notifModel
    }

    Connections {
        target: dashboard
        function onSystemNotification(type, message) {
            if (notifModel.count >= 50)
                notifModel.remove(0)

            notifModel.append({
                msgType: type.toLowerCase(),   // "info"|"warning"|"error"|"success"
                msgText: message,
            })

            notifList.positionViewAtEnd()
        }
    }

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 12
        spacing:         8

        RowLayout {
            spacing: 6

            // Icon chuông từ notice.png. Khai cả width/height LẪN
            // Layout.preferredWidth/Height (trong RowLayout, nếu chỉ đặt
            // width/height mà notice.png có kích thước gốc lớn/tỉ lệ lạ, một
            // số trường hợp Layout vẫn ưu tiên implicit size tính từ ảnh gốc
            // thay vì width/height mình gán) - khai rõ cả hai để chắc chắn
            // icon luôn đúng 16x16 dù ảnh gốc bao nhiêu MP. sourceSize giới
            // hạn luôn bộ nhớ giải mã ảnh, tránh việc Qt load ảnh full-res
            // rồi mới scale xuống (cũng là 1 nguyên nhân icon "phồng to" nếu
            // ảnh gốc rất lớn và fillMode tính sai trên 1 khung hình đầu).
            Image {
                source: "qrc:/qt/qml/DashboardEnv/assets/images/notice.png"
                width:  16
                height: 16
                Layout.preferredWidth:  16
                Layout.preferredHeight: 16
                sourceSize.width:  32
                sourceSize.height: 32
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text:             qsTr("Thông báo")
                font.bold:        true
                font.pixelSize:   14
                color:            AppTheme.textDark
                Layout.fillWidth: true
            }

            Rectangle {
                id:      errorBadgeBox
                visible: errorCount > 0
                width:   Math.max(18, errorBadgeText.implicitWidth + 8)
                height:  18
                radius:  9
                color:   AppTheme.brandRed

                property int errorCount: {
                    var c = 0
                    for (var i = 0; i < notifModel.count; i++)
                        if (notifModel.get(i).msgType === "error") c++
                    return c
                }

                Text {
                    id:               errorBadgeText
                    anchors.centerIn: parent
                    text:             errorBadgeBox.errorCount
                    color:            AppTheme.textLight
                    font.pixelSize:   9
                    font.bold:        true
                }
            }

            Text {
                text:           qsTr("Xoá")
                font.pixelSize: 10
                color:          AppTheme.textMuted

                MouseArea {
                    anchors.fill: parent
                    cursorShape:  Qt.PointingHandCursor
                    onClicked:    notifModel.clear()
                }
            }
        }

        ListView {
            id:                      notifList
            Layout.fillWidth:        true
            Layout.fillHeight:       true
            spacing:                 18
            clip:                    true
            model:                   notifModel
            verticalLayoutDirection: ListView.BottomToTop

            delegate:Item{
                id : delegateWrapper
                width: notifList.width
                height: delegateCol.implicitHeight + 20
                anchors.leftMargin: 6
                Rectangle {
                id : notiItem
                width:        notifList.width -10
                height:       delegateCol.implicitHeight +28
                color:        AppTheme.elementBg
                radius:       6

                RowLayout {
                    anchors.fill:    parent
                    anchors.margins: 8
                    spacing:         8

                    Rectangle {
                        width:            22
                        height:           22
                        radius:           11
                        Layout.alignment: Qt.AlignTop
                        color: {
                            switch (msgType) {
                            case "error":   return AppTheme.textRed
                            case "warning": return AppTheme.warningNote
                            case "success": return AppTheme.successNote
                            default:        return AppTheme.infoNote
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            font.bold:        true
                            font.pixelSize:   10
                            color:            AppTheme.textLight
                            text: {
                                switch (msgType) {
                                case "error":   return "✕"
                                case "warning": return "!"
                                case "success": return "✓"
                                default:        return "i"
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        id:               delegateCol
                        Layout.fillWidth: true
                        spacing:          2

                        Text {
                            text:             msgText
                            Layout.fillWidth: true
                            font.pixelSize:   11
                            color:            AppTheme.textDark
                            wrapMode:         Text.WordWrap
                        }
                    }
                }
            }
                MultiEffect {
                    source:                  notiItem
                    anchors.fill:            notiItem

                    shadowEnabled:           true
                    shadowColor:             "#40000000"
                    shadowBlur:              0.3
                    shadowHorizontalOffset:  2
                    shadowVerticalOffset:    4
                    shadowScale:             1.0
                }
            }


            Text {
                anchors.centerIn: parent
                visible:          notifModel.count === 0
                text:             qsTr("Chưa có thông báo")
                font.pixelSize:   11
                color:            AppTheme.textMuted
            }
        }
    }
}
