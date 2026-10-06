// ClampedTextField.qml
//
// TextField dùng chung cho MỌI ô nhập số có giới hạn min/max thật (Power AC/
// DC, Suy hao ATT...).
//
// - RegularExpressionValidator CHỈ lọc ký tự hợp lệ lúc gõ (số, tối đa 1 dấu
//   chấm nếu decimals>0). Chặn khoảng giá trị THẬT nằm ở JS (parseFloat/
//   parseInt không phụ thuộc locale): vượt trên bị kẹp ngay lúc gõ, dưới
//   ngưỡng kẹp khi gõ xong.
// - Ô RỖNG = "chưa đặt": value = NaN, ô hiện "--" (placeholder). Xoá trắng ô
//   rồi rời ô/Enter KHÔNG còn tự quay về giá trị cũ/mặc định nữa - phát
//   committed(NaN) để nơi dùng lưu trạng thái "--".
import QtQuick
import QtQuick.Controls

TextField {
    id: field

    property real minValue: 0
    property real maxValue: 100
    // 0 = số nguyên, >0 = số chữ số thập phân
    property int  decimals: 0
    // Giá trị "đã chốt" - chỉ đổi khi rời ô/nhấn Enter. NaN = chưa đặt.
    property real value: NaN

    // Phát ra khi người dùng chốt xong (rời ô hoặc Enter): số hợp lệ đã kẹp,
    // hoặc NaN nếu ô bị xoá trắng.
    signal committed(real value)

    // true trong lúc field tự sửa lại text của chính nó - chặn onTextChanged
    // gọi đệ quy (setText -> onTextChanged -> setText -> ...).
    property bool _selfEdit: false

    selectByMouse: true
    placeholderText: "--"

    validator: RegularExpressionValidator {
        regularExpression: field.decimals > 0
            ? new RegExp("^\\d{0,6}(\\.\\d{0," + field.decimals + "})?$")
            : /^\d{0,6}$/
    }

    function _format(v) {
        if (isNaN(v)) return ""
        return field.decimals > 0 ? v.toFixed(field.decimals) : String(Math.round(v))
    }

    function _setText(t) {
        _selfEdit = true
        text = t
        _selfEdit = false
    }

    // Giá trị bị đổi từ bên ngoài (nạp .ini, backend kẹp lại...) khi người
    // dùng không đang gõ trong ô -> hiển thị lại cho khớp.
    onValueChanged: if (!activeFocus) _setText(_format(value))

    onTextChanged: {
        if (_selfEdit || text.length === 0) return
        // Đang gõ dở dang (vd "12.") - chờ có số thật để parse.
        if (/\.$/.test(text)) return

        var v = field.decimals > 0 ? parseFloat(text) : parseInt(text, 10)
        if (isNaN(v)) return

        // Chỉ chặn NGOÀI TRÊN lúc đang gõ (không cho gõ vượt max) - chặn dưới
        // để khi gõ xong (gõ "8" rồi "0" thành "80" cho khoảng 80-150 sẽ bị
        // kẹp sai giữa chừng nếu chặn dưới ngay).
        if (v > field.maxValue)
            _setText(field._format(field.maxValue))
    }

    onEditingFinished: {
        var t = field.text.trim()
        if (t.length === 0 || t === ".") {
            // Xoá trắng = chưa đặt ("--").
            field.value = NaN
            _setText("")
            field.committed(NaN)
            return
        }
        var v = field.decimals > 0 ? parseFloat(t) : parseInt(t, 10)
        if (isNaN(v)) {
            field.value = NaN
            _setText("")
            field.committed(NaN)
            return
        }
        v = Math.max(field.minValue, Math.min(field.maxValue, v))
        field.value = v
        _setText(field._format(v))
        field.committed(v)
    }

    Component.onCompleted: _setText(_format(value))
}
