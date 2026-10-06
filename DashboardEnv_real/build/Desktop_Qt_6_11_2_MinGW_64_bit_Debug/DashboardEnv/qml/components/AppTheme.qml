pragma Singleton
import QtQuick

QtObject {
    id: appTheme

    // false = sáng (mặc định, khớp mockup gốc), true = tối.
    // Đổi qua toggleTheme() - 2 nút Sun/Moon trong Main.qml đã gọi hàm này.
    property bool isDarkMode: false

    // Mọi màu bên dưới là "readonly property color X: isDarkMode ? darkY : lightY".
    // Đây vẫn là binding hợp lệ trong QML (readonly chỉ cấm gán từ bên ngoài,
    // không cấm tự tính lại) - nên khi isDarkMode đổi, MỌI chỗ dùng AppTheme.xxx
    // trong toàn app tự cập nhật ngay, không cần sửa gì thêm ở các file khác.

    readonly property color mainBg:        isDarkMode ? "#1B1D22" : "#D9D9D9"
    readonly property color panelBg:       isDarkMode ? "#24262B" : "#F5F5F5"
    readonly property color elementBg:     isDarkMode ? "#2E3138" : "#FFFFFF"
    readonly property color borderLight:   isDarkMode ? "#3A3D44" : "#E2E8F0"
    readonly property color panelBgMuted:  isDarkMode ? "#2A2D33" : "#E6E6E6"
    readonly property color borderMuted:   isDarkMode ? "#5B5E66" : "#787486"

    // Màu thương hiệu (Viettel Aerospace) - KHÔNG đổi theo theme, giữ nguyên
    // bản sắc dù sáng hay tối.
    readonly property color brandRed:        "#EE0033"
    readonly property color brandRedHover:   "#B42318"
    readonly property color brandRedPressed: "#912018"

    readonly property color cmdPanelMuted: isDarkMode ? "#44474E" : "#B3B3B3"
    readonly property color cmdLogMuted:   isDarkMode ? "#44474E" : "#B3B3B3"

    readonly property color textRed:      "#EE0033"
    readonly property color textLight:    "#F5F5F5"   // chữ trên nền brandRed/tối màu - luôn gần trắng
    // Trước là "#000000" (đen tuyệt đối) - hơi gắt trên nền xám phẳng của mockup;
    // đổi sang than chì đậm, cách phối màu chuẩn hơn cho dashboard chuyên nghiệp
    // (giống bảng đồng hồ Cần chưa dùng ở qml/AppTheme.qml cũ - đã hợp nhất về đây).
    readonly property color textDark:     isDarkMode ? "#F5F5F6" : "#1D2939"
    readonly property color textMuted:    isDarkMode ? "#9AA1AC" : "#667085"
    readonly property color textTerminal: "#F04438"
    readonly property color chartGrid:    isDarkMode ? "#33363D" : "#F2F4F7"

    // Bộ màu trạng thái (status) dùng chung cho badge/thông báo - trước đây mỗi
    // file tự viết hex riêng (Notification.qml có "#2E90FA" cho info,
    // CommandLogPanel.qml có "#334155" cho dòng log mặc định) nên dễ lệch nhau.
    // successNote trước là "#6155F5" (tím) - lạc hoàn toàn khỏi bảng màu đỏ/xám
    // của thương hiệu; đổi sang xanh lá chuẩn cho trạng thái "thành công".
    readonly property color successNote: "#12B76A"
    readonly property color infoNote:    "#2E90FA"
    readonly property color warningNote: "#FF8D28"

    readonly property int radiusPanel:   12
    readonly property int radiusControl: 8
    readonly property int paddingNormal: 16

    function toggleTheme() {
        isDarkMode = !isDarkMode;
    }
}
