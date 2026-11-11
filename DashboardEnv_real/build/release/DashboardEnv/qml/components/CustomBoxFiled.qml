import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


TextField {
    id: tfControl
    Layout.fillWidth: true
    font.pixelSize: 12
    color: "#1D2939"
    // Trước leftPadding:4, không có rightPadding - chữ/placeholder dính sát
    // viền ô ngay khi text dài gần hết field. Tăng lên 8 cho thoáng, khớp với
    // padding các TextField khác trong app sau đợt sửa này.
    leftPadding: 8
    rightPadding: 8
    verticalAlignment: TextInput.AlignVCenter
    background: Item {}
}