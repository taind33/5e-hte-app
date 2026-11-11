import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DashboardEnv


    BoxWrap {
        id: panel
        implicitWidth: 250
        implicitHeight: 200
        readonly property var delayOptions: [
            { label: "0 ns",   value: 0   },
            { label: "100 ns", value: 100 },
            { label: "270 ns", value: 270 },
            { label: "400 ns", value: 400 }
        ]

        // -1 = chưa chọn lần nào (chạy lần đầu) -> combo hiện "--".
        function currentDelayIndex(){
            for( var i=0; i< delayOptions.length;i++){
                if(delayOptions[i].value=== dashboard.delayTime) return i
            }
                return -1
            }

            ColumnLayout {
                anchors.fill:parent
                anchors.margins: 12
                spacing: 10
                RowLayout{
                spacing: 10
                Image {
                    source:   "qrc:/qt/qml/DashboardEnv/assets/images/DelayIcon.png"
                    width:    24
                    height:   24
                    fillMode: Image.PreserveAspectFit
                }
                Text { text: qsTr("Giữ chậm"); font.bold: true; font.pixelSize: 15; color: AppTheme.textDark;Layout.fillWidth: true }
                }
                    BoxWrap {
                        implicitHeight: 30
                        Layout.fillWidth: true
                        radius : 6
                        RowLayout{
                            anchors.fill: parent
                                          anchors.leftMargin: 3
                                    Text { text: qsTr("Thời gian:"); font.pixelSize: 12; color: AppTheme.textDark; Layout.preferredWidth: 70 }
                                    CustomComboBox {
                                        id: delayCombo
                                        model : panel.delayOptions.map(function(o){return o.label})
                                        currentIndex: panel.currentDelayIndex()
                                        onActivated: {
                                            var ns =  panel.delayOptions[currentIndex].value
                                            dashboard.applyDelayTime(ns)
                                        }
                                    }
                            }
                            }
                    Item { Layout.fillHeight: true }
                }
            }


