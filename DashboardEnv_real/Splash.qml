import QtQuick
import QtQuick.Window

Window {
    id: splash
    visible: true
    color: "transparent"
    flags: Qt.SplashScreen | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    width: logo.width
    height: logo.height
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2

    Image {
        id: logo
        source: "qrc:/assets/logo/Viettel_Aerospace_Institute_(VTX)_logo.png"
        width: implicitWidth / Screen.devicePixelRatio                                                // đổi số này để to/nhỏ hơn
        height: implicitWidth > 0 ? width * implicitHeight / implicitWidth : 0
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }
}