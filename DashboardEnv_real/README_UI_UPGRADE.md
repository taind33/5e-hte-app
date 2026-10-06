# Đợt sửa lỗi - Translation / Validator / Settings / Layout

Đợt này KHÔNG đụng firmware, chỉ sửa app Qt6/CMake (`DashboardEnv`). Áp dụng
bằng cách giải nén đè lên đúng cấu trúc thư mục project hiện tại, sau đó mở
lại trong Qt Creator để CMake tự re-configure (sẽ tự phát hiện file mới
`qml/components/ClampedTextField.qml` và file bị xoá
`qml/components/Translations.qml`) rồi **Rebuild** (không chỉ Build) vì
`CMakeLists.txt` có thay đổi.

> **QUAN TRỌNG - nếu build xong vẫn thấy y hệt lỗi cũ (vd còn thấy tiếng Anh,
> còn thấy dòng chữ đã bảo xoá):** gần như chắc chắn là do Qt Creator dùng
> lại thư mục build cũ (đã cache lại bytecode QML biên dịch từ code cũ).
> Xoá HẲN thư mục build (`build-DashboardEnv-...`, nằm cạnh thư mục project,
> KHÔNG phải trong project) rồi mở lại project để CMake configure lại từ đầu,
> sau đó Rebuild. Bản build clean-từ-đầu là bản đã được kiểm chứng trong đợt
> sửa này (xem mục "Về công cụ kiểm tra" cuối file) - Build/Rebuild thường
> trên 1 thư mục build cũ có thể không phản ánh đúng code mới.

**File mới:** `qml/components/ClampedTextField.qml`
**File bị xoá:** `qml/components/Translations.qml` (không còn được
`CMakeLists.txt` tham chiếu nữa - có thể xoá tay nếu Qt Creator không tự dọn)
**Asset cần có:** `assets/images/language.png` - bạn nói đã thêm sẵn vào máy,
zip này cũng kèm theo (icon tạm demo) để build không lỗi "cannot find source
file" nếu bạn giải nén vào thư mục sạch; icon thật của bạn sẽ ghi đè lên.

## 1. Bỏ bộ song ngữ tự chế, quay lại `qsTr()` (chờ mẫu Qt Linguist thật)

Trước đây tự viết `Translations.qml` (singleton + object tra bảng chuỗi) -
không phải luồng Qt Linguist chuẩn (`qsTr()` + `lupdate`/`lrelease` +
`.ts`/`.qm` + `QTranslator`). Theo đúng yêu cầu: đã xoá hẳn
`Translations.qml`, mọi `Translations.t("key")` trong 8 file QML
(`Main.qml`, `ConnectionPanel.qml`, `PowerControlPanel.qml`,
`Attenuation.qml`, `TimeDelay.qml`, `CommandLogPanel.qml`, `Detect.qml`,
`Notification.qml`, `MiniMonitorChart.qml`) đã quay lại **literal tiếng Việt
gốc bọc trong `qsTr(...)`** - đúng y hệt nội dung tiếng Việt trước đây, không
đổi chữ nào. Bọc sẵn `qsTr()` (thay vì literal trần) để khi bạn gửi mẫu Qt
Linguist thật, chỉ cần chạy `lupdate` là tự trích được hết chuỗi cần dịch mà
không phải sửa lại từng file QML lần nữa.

Cụm chuyển ngôn ngữ VN/EN cũ (2 chữ "VN"/"EN" bấm được) đã bỏ theo, vì không
còn cơ chế đổi ngôn ngữ đứng sau nó nữa - thay bằng 1 icon `language.png`
(mục #6 dưới) tạm thời chỉ trang trí, sẽ nối logic thật khi có bộ Linguist.

## 2. Nút "Bắt đầu phiên" / "Mở log" thấp - tăng chiều cao có kiểm chứng thật

`ConnectionPanel.qml`: `actionRow` (chứa 2 nút) từ 36px lên **42px**, cỡ chữ
nút từ 13px lên 14px. Lần trước tăng lên 44px làm tràn khung vì lúc đó SỬA
CẢ margin/spacing các hàng phía trên cùng lúc; lần này CHỈ tăng riêng
`actionRow`, giữ nguyên 3 hàng COM/Baud/Tên phiên và spacing như cũ, nên phần
dư ra chỉ trừ vào `Item{Layout.fillHeight:true}` phía trên nút (không đụng gì
khác).

**Đã dựng ảnh thật bằng code để kiểm tra** (không đoán): build project bằng
CMake + Qt (bộ công cụ Linux gần nhất có sẵn ở môi trường sửa lỗi, xem mục
"Về công cụ kiểm tra" cuối file), sau đó render `ConnectionPanel.qml` thật
trong đúng khung 250×200 (đúng `implicitWidth/Height` khai trong file, tức
đúng ngân sách chật nhất) bằng `QQuickView` chạy chế độ `offscreen`, chụp lại
`grabWindow()` ra PNG - đính kèm `verify_ConnectionPanel.png`. Ảnh xác nhận
2 nút cao 42px hiển thị đầy đủ, không bị cắt/bóp, không tràn khung.

## 3. RegularExpressionValidator đúng cách + hết lỗi "Unable to assign QLocale to QString"

**Nguyên nhân lỗi cũ**: thuộc tính `locale` trên `IntValidator`/
`DoubleValidator`/`RegularExpressionValidator` có kiểu **QString** (tên
locale như `"en_US"`), KHÔNG PHẢI `QLocale`. `Qt.locale("en_US")` trả về 1
object `QLocale`, gán vào property kiểu QString nên QML báo lỗi runtime
"Unable to assign QLocale to QString" và âm thầm không set được locale -
đúng 5 dòng lỗi bạn gửi (`PowerControlPanel.qml:235`, `Attenuation.qml:135`
x3).

**Cách sửa đúng** (file mới `qml/components/ClampedTextField.qml`, dùng
chung cho cả 2 nơi): dùng `RegularExpressionValidator` đúng như yêu cầu,
nhưng CHỈ để lọc ký tự hợp lệ lúc gõ (số, tối đa 1 dấu chấm) - vì regex chỉ
đếm ký tự chứ không hiểu số học nên không thể tự chặn theo khoảng (đây là lý
do bản trước đó dùng regex một mình bị lỗi "vẫn nhập được hơn"). Việc chặn
đúng khoảng giá trị được làm bằng JS (`parseFloat`/`parseInt` - luôn độc lập
locale) ngay trong `onTextChanged`, tức chặn **NGAY LÚC ĐANG GÕ** (gõ "150"
rồi gõ thêm "5" thành "1505" sẽ bị tự sửa lại về "150" ngay lập tức), không
đợi rời khỏi ô mới kiểm tra. Không còn property `locale` nào được dùng ở
validator nữa nên lỗi QLocale/QString biến mất hoàn toàn.

Giới hạn thật áp dụng (theo yêu cầu mới nhất của bạn, đã cập nhật trong
`dashboardmanager.h`):
- AC: 80-150V, tần số tới **1000Hz** (trước ghi nhầm 45-65Hz).
- DC: 0-36V, dòng tới **3A** (trước ghi nhầm 5A).
- ATT: 0-31.75dB, bước 0.25 (giữ nguyên, không đổi).

## 4. QSettings chuyển hẳn xuống backend C++ (hết cảnh báo Qt.labs.settings)

Thêm 4 property mới vào `DashboardManager` (`stagedAcVolt`, `stagedAcFreq`,
`stagedDcVolt`, `stagedDcCurrent`) - đúng khuôn mẫu `comPort`/`baudRate`/
`sessionName` đã có sẵn: getter/setter C++, lưu qua `m_settings.setValue()`
(cùng 1 `QSettings m_settings` dùng chung, không phải cơ chế thứ 2), nạp lại
trong constructor bằng gán thẳng member (không qua setter, không đụng phần
cứng lúc khởi động - đúng invariant đã thống nhất từ đầu). `PowerControlPanel.qml`
bỏ hẳn `import Qt.labs.settings` và khối `Settings{}`/property cục bộ, bind
thẳng 4 ô nhập vào `dashboard.stagedAcVolt/stagedAcFreq/stagedDcVolt/
stagedDcCurrent`. `Main.qml` cũng bỏ khối `Settings{ category:"Language" }`
(không còn cần vì đã bỏ hệ song ngữ tự chế ở mục 1). Cả 2 cảnh báo
`QML Settings: ... Qt.labs.settings is deprecated` bạn gặp đều hết hẳn vì
không còn file nào `import Qt.labs.settings` nữa.

## 5. Xoá dòng chú thích chưa có căn cứ ở Suy hao (và trong cả code backend)

Xoá hẳn dòng "Giao thức hiện tại chỉ gửi được số dB nguyên..." trong
`Attenuation.qml` - đúng góp ý của bạn, chưa có căn cứ xác nhận từ
datasheet/firmware thật nên không nên khẳng định trên giao diện vận hành.

**Cập nhật thêm (bạn chỉ ra đúng)**: cùng 1 khẳng định chưa có căn cứ đó còn
nằm sâu hơn trong code backend - comment ở `kria_helper.h`
(`annuation_msg_t.value`) từng viết chắc chắn "firmware cast thẳng sang float
dB" và log/thông báo trong `dashboardmanager.cpp` (`onAttUpdateRequested()`)
từng ghi "gửi X dB nguyên (giao thức 1 byte)" - cùng một giả định chưa kiểm
chứng. Đã sửa lại thành ghi chú trung thực: byte `value` (1 byte, 0-255) HIỆN
đang được làm tròn về dB nguyên trước khi gửi (lựa chọn an toàn/bảo thủ tạm
thời), nhưng bản thân trường 1 byte đó **không bắt buộc** phải mang dB
nguyên - nhiều IC suy hao số bước như F1956 thực tế nhận lệnh dạng "số bước"
(byte = số bước 0.25dB, giá trị thật = byte×0.25dB), nếu đúng vậy thì cùng 1
byte này tải được trọn 0-31.75dB mà không mất độ phân giải, không cần đổi
wire format.

**Tôi CHƯA tự ý đổi công thức mã hoá** (từ "byte = dB nguyên" sang "byte =
số bước×0.25") vì không có datasheet F1956/tài liệu firmware thật để xác
nhận firmware đọc byte này theo cách nào - đổi sai chiều sẽ khiến suy hao
thực tế gửi ra phần cứng sai lệch (nguy hiểm hơn dòng chữ sai trên UI, vì
ảnh hưởng trực tiếp tới tín hiệu RF thật). Nếu bạn có tài liệu xác nhận cách
firmware xử lý (`Attenuator_SetDb()`/`HandleAtt()` phía firmware, hoặc
datasheet F1956), gửi tôi để sửa đúng công thức - lúc đó sẽ gửi được đúng cả
phần thập phân 0.25dB thay vì làm tròn như hiện tại.

## 6. "Đã phát hiện" - giảm cỡ chữ, cách đều mép khung

`Detect.qml`: cỡ chữ tiêu đề "Đã phát hiện" từ 32px xuống **22px**, gộp
chung với dòng "Kích nổ (đỉnh): x.x V" vào 1 `ColumnLayout` căn giữa, width
bị trừ margin 16px mỗi bên (`wrapMode: WordWrap` dự phòng) - không còn sát
mép khung đỏ. Ảnh `verify_Detect.png` đính kèm là ảnh dựng thật, không phải
mô tả.

## 7. Icon `language.png`

Thêm vào `CMakeLists.txt` (mục `RESOURCES`) và đặt vào đúng cụm góc trên bên
phải `Main.qml` (chỗ 2 nút Sun/Moon), thay cho cụm VN/EN cũ đã bỏ. Hiện TẠM
CHƯA gắn logic đổi ngôn ngữ thật (chỉ hiện tooltip "Ngôn ngữ (đang phát
triển)") - sẽ nối vào khi có bộ Qt Linguist thật theo mẫu bạn gửi.

## 8. Dọn 1 lỗi cấu hình build ẩn: `Qt6::Graphsplugin`

Trong lúc build thật để kiểm chứng (xem mục dưới), CMake báo lỗi cứng ngay
lúc configure:
```
CMake Error at CMakeLists.txt:88 (target_link_libraries):
  Target "appDashboardEnv" links to: Qt6::Graphsplugin
  but the target was not found.
```
`target_link_libraries` có `Qt6::Graphsplugin` nhưng không có `find_package(Qt6
COMPONENTS Graphs)` nào tương ứng, và không file QML nào trong project
`import QtGraphs` cả - đây là 1 target thừa/rác (có thể từ lúc copy template
CMakeLists ban đầu). Đã xoá dòng này khỏi `target_link_libraries`. Nếu máy
bạn build được trước đó dù có dòng này, khả năng cao là do bộ Qt 6.11.2 đầy
đủ của bạn tình cờ có sẵn module Graphs nên CMake tự tạo được target đó -
nhưng nó không được `find_package()` yêu cầu tường minh nên không đảm bảo ổn
định giữa các máy/lần cài đặt khác nhau. Xoá đi không ảnh hưởng gì vì không
ai dùng.

## Về công cụ kiểm tra đã dùng (đúng yêu cầu "dùng công cụ phù hợp")

Môi trường sửa lỗi này chạy Linux, không có sẵn Qt 6.11.2 + MinGW như máy
bạn, nên đã cài bộ Qt6 declarative gần nhất có trên kho Linux
(**Qt 6.4.2**, cùng nhánh Qt6, cùng API `qsTr`/`RegularExpressionValidator`/
`QSettings` đang dùng) để có công cụ THẬT thay vì đoán:

- `qmllint` (từ `qt6-declarative-dev-tools`) chạy trên toàn bộ file QML mới/
  sửa - 0 lỗi mức "Error", chỉ còn vài "Warning" đã xác minh là false-positive
  của bản thân công cụ (ví dụ: không đọc được property của QML singleton khi
  không qua `qt_add_qml_module` thật, hoặc không biết `dashboard` là context
  property gán từ C++) chứ không phải lỗi thật trong code.
- **Build thật bằng CMake** (`qt6-base-dev`, `qt6-declarative-dev`,
  `qt6-serialport-dev`, `qt6-charts-dev`...): cấu hình + biên dịch TOÀN BỘ
  project (C++ backend + toàn bộ QML qua `qmlcachegen`) thành công 100%,
  link ra file thực thi - đây là bằng chứng mạnh hơn `qmllint` vì dùng đúng
  trình biên dịch QML thật của Qt, xác nhận `dashboardmanager.h/.cpp` (property
  mới) và mọi file QML sửa/thêm đều hợp lệ về cú pháp lẫn kiểu dữ liệu.
- **Dựng ảnh thật** (không mô tả bằng lời): viết 1 harness Qt Quick nhỏ độc
  lập nạp thẳng `ConnectionPanel.qml`/`PowerControlPanel.qml`/
  `Attenuation.qml`/`Detect.qml` thật (kèm 1 `MockDashboard.qml` giả lập đúng
  các property/hàm mà 4 file này cần từ `dashboard`) trong đúng kích thước
  khung thật, chạy Qt ở chế độ `offscreen`, `grabWindow()` ra PNG - 4 ảnh
  đính kèm (`verify_ConnectionPanel.png`, `verify_PowerControlPanel.png`,
  `verify_Attenuation.png`, `verify_Detect.png`) là ảnh chụp thật từ code
  chạy thật, không phải mockup HTML như đợt trước.

Riêng module `QtQuick.Effects` (dùng cho đổ bóng `MultiEffect`) không có gói
apt sẵn cho Qt 6.4.2 ở môi trường này (chỉ ảnh hưởng việc DỰNG ẢNH RIÊNG 4
panel qua harness độc lập ở trên, không ảnh hưởng build CMake thật ở mục
trên - bản build CMake thật build/link thành công bình thường vì cách
`qmlcachegen` tìm module lúc build khác lúc chạy độc lập). Đây thuần là giới
hạn gói Linux ở môi trường sửa lỗi - Qt 6.11.2 SDK thật của bạn có sẵn đầy đủ
module này, không cần làm gì thêm.
