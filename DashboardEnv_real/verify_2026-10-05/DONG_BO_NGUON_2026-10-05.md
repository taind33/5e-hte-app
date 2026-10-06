# Đọc ngược nguồn AC/DC, F5, config.ini, ô "--" — 2026-10-05

Đầu vào là bản đồng bộ ngày 01/10 (firmware `5e_hte_app` + app `DashboardEnv_real`). Các chỉnh sửa riêng của anh vẫn được giữ: baud 9600 cho nguồn, `digital_interface_tune_skip_mode = 1`, và đoạn console đang comment trong `main.c`.

**Firmware** sửa 7 file trong `src/` và **không thêm file mới**, nên Vitis chỉ cần build lại, không phải "Add Sources".

**App** sửa 21 file. Có thêm 2 icon `assets/images/refresh.png` và `freq.png` — đây là **icon tạm** để build chạy ngay; anh chép icon của anh đè lên, giữ nguyên tên file.

---

## 1. Xác nhận lệnh bằng phản hồi thật từ nguồn

Trước đây firmware chỉ ghi lệnh ra UART nguồn, nên ACK chỉ có nghĩa là "đã đẩy byte ra". Giờ mỗi lệnh đặt chạy thành một giao dịch có đọc phản hồi.

**AC (IT-M7700), khi BẬT:**

1. `SYST:REM`.
2. Xả hàng lỗi cũ bằng `SYST:ERR?` (tối đa 20 lần), để lỗi tồn từ trước không làm hỏng lệnh mới.
3. Chuỗi lệnh đặt cũ, giữ nguyên thứ tự và các khoảng chờ 50 ms: `PROT:MODE AC`, `NORM:VOLT:AC`, `NORM:FREQ`, `OUTP ON`.
4. `SYST:ERR?` phải trả mã 0.
5. `OUTP?` phải trả `ON`.
6. Đọc `MEAS:VOLT?`, `MEAS:CURR?`, `MEAS:FREQ?`.

**AC, khi TẮT:** chỉ gửi `OUTP OFF`, sau đó cũng xác nhận và đọc như trên. Bản cũ khi tắt vẫn gửi kèm điện áp/tần số, và với ô trống thì thành `NORM:VOLT:AC 80.0` / `NORM:FREQ 0.0`. Lỗi này do test e2e phát hiện và đã sửa.

**DC (Matsusada):**

- BẬT: `#0 REN`, `ICN`, `SW1`, `VCN` như cũ, sau đó đọc `#0 MEAS:VOLT?` và `#0 MEAS:CURR?`.
- TẮT: chỉ `#0 SW0`, rồi đọc.

DC được coi là đã xác nhận khi nguồn trả lời được lệnh đọc. Hiện chưa có lệnh hỏi trạng thái ngõ ra cho DC (xem mục 6).

**ACK** chỉ báo thành công khi nguồn đã xác nhận. Ngay trước ACK, board gửi **msgid 42** với kết quả chi tiết:

| Kết quả | Ý nghĩa | App hiển thị |
|---|---|---|
| OK | Nguồn trả lời, không lỗi, ngõ ra đúng yêu cầu | "Cập nhật hh:mm:ss" |
| NO_REPLY | Một câu hỏi quá 500 ms không có trả lời | "Nguồn không phản hồi — kiểm tra cáp RS232/baud" |
| DEV_ERROR | `SYST:ERR?` báo lỗi | "Nguồn AC báo lỗi lệnh (mã SCPI …)" |
| OUT_MISMATCH | `OUTP?` khác trạng thái yêu cầu | "Ngõ ra đang TẮT, khác yêu cầu"; công tắc hiện trạng thái thật |
| BAD_REPLY | Có trả lời nhưng không đọc được số | "Kiểm tra lệnh đọc trong board_config.h" |
| NOT_READY | UART nguồn trên board không khởi tạo được | báo lỗi |

**Cách firmware chạy giao dịch:**

- Giao dịch **không chặn**: trong lúc chờ nguồn trả lời, phổ RF, ADC và lệnh từ PC vẫn chạy.
- Mỗi nguồn chỉ có một câu hỏi đang chờ tại một thời điểm, đúng quy tắc của IT-M7700.
- Câu hỏi đầu tiên bị timeout sẽ kết thúc giao dịch luôn, không chờ hết các câu còn lại.
- Lệnh mới tới khi giao dịch cũ chưa xong: giao dịch cũ bị huỷ, và ACK của nó trả về thất bại.
- Nhấn refresh nhiều lần liên tiếp chỉ tạo một lần đọc, không xếp hàng.

**Toàn bộ chuỗi lệnh gửi nguồn nằm ở `src/board_config.h`**, mục "AC/DC supply READBACK", cùng với timeout và các khoảng chờ. Thử trên bench mà thấy cú pháp khác thì sửa một dòng ở đó rồi build lại.

**Bộ đọc số lấy số cuối cùng trong dòng trả lời**, nên chấp nhận các dạng `12.34`, `#0 12.34`, `+1.234E+01`, `12.34V`. Nếu nguồn có tiếng vọng (echo) lại lệnh, dòng echo được tự bỏ qua.

## 2. Bốn đồ thị: số đo thật, nút ⟳, F5

- **Bốn đồ thị:** Điện áp AC, **Tần số AC** (thay cho Dòng AC), Điện áp DC, Dòng DC. Tất cả là **số đo đọc từ nguồn**, không còn là giá trị đặt, và không còn số mô phỏng khi chưa kết nối.
- **Điểm mới chỉ xuất hiện khi:**
  - bấm ⟳;
  - nhấn **F5** (đọc cả hai nguồn);
  - hoặc ngay sau lệnh bật/tắt, vì phần xác nhận đã đọc số đo.
  
  Không có polling.
- **Nút ⟳** nằm cuối hàng tiêu đề, cạnh ô số:
  - bấm ⟳ ở ô AC nào thì cả hai ô AC cùng cập nhật, vì chúng dùng chung một lần đọc;
  - trong lúc chờ, icon xoay và nút bị khoá;
  - chưa kết nối thì nút mờ.
- **Chân đồ thị:**
  - bình thường: "Cập nhật hh:mm:ss";
  - mất phản hồi: chữ đỏ "Nguồn không phản hồi · hh:mm:ss", rê chuột vào để xem chi tiết;
  - chưa đọc lần nào: "Chưa có số đo", ô số hiện "--".
- **Trục:** Y tự co giãn và chia số tròn; X theo thời gian thật giữa các lần đọc.
- **Công tắc bật/tắt:**
  - trong lúc chờ, hiện trạng thái yêu cầu và bị khoá;
  - sau đó theo đúng trạng thái nguồn xác nhận;
  - nếu thất bại thì quay về trạng thái cũ.
  
  Bản cũ có lỗi: sau lần bấm đầu tiên, công tắc mất liên kết với backend nên không còn cập nhật theo ACK. Lỗi này đã sửa.
- **Dòng DC** hiển thị là **dòng đo thực tế**. Ô "Dòng điện" ở khung nhập vẫn là giới hạn dòng (`ICN`).

## 3. "Đặt lại" và F5

- **F5** chỉ đọc, không thay đổi gì trên thiết bị.
- **"Đặt lại"** vẫn reset hệ thống (AC/DC OFF, ATT 0 dB, delay 0 ns, xoá đỉnh kích nổ), nhưng giờ **hỏi xác nhận trước** vì nó cắt nguồn.
  - Sau khi đặt lại, mỗi nguồn trả msgid 42 xác nhận đã tắt thật.
  - RESET không còn gửi `NORM:VOLT:AC 80.0` / `NORM:FREQ 0.0` như bản cũ, chỉ gửi `OUTP OFF`.

## 4. File `config.ini`

- **Vị trí:** cạnh file `.exe`. Nếu thư mục đó không ghi được (ví dụ cài trong Program Files) thì app tự chuyển sang AppData. Đường dẫn thật được in ở Terminal lúc mở app.
- **Nội dung lưu:**
  - cổng COM, baud, tên phiên;
  - lựa chọn giữ chậm;
  - ATT1–3;
  - 4 ô AC/DC (kể cả trạng thái "--");
  - hiệu chuẩn RF và mức chuẩn dBm đã nhập;
  - sáng/tối;
  - kích thước và vị trí cửa sổ.
- **Ghi ngay khi chốt giá trị**, nên app bị tắt đột ngột cũng không mất.
- **Lần đầu chạy bản mới:** app tự chép cấu hình cũ từ `%APPDATA%\VASC\AvionicsDashboard.ini`.
- **Lúc mở app:** chỉ nạp giá trị lên giao diện. **Không gửi lệnh, không tự mở cổng.** Nếu cổng COM đã lưu hiện không có thì app báo, không tự chọn cổng khác.

Ví dụ một file thật sinh ra trong lúc test nằm ở `verify_2026-10-05/config_mau_sau_test.ini`.

## 5. Ô nhập "--"

- **Xoá trắng ô** rồi rời ô hoặc Enter: ô hiện "--" và được lưu như vậy. Không còn nhảy về giá trị cũ.
- **Cài lần đầu:** mọi ô và combobox đều là "--" — COM, baud, giữ chậm, ATT, AC/DC, tên phiên.
- **Chặn thao tác khi thiếu giá trị:**
  - Bật nguồn khi còn "--": app báo rõ ô nào thiếu (ví dụ "Chưa nhập điện áp và tần số AC") và không gửi gì.
  - Bắt đầu phiên khi chưa chọn COM/baud: báo và không mở cổng. Lưu ý board dùng 115200.
  - Bấm gửi ATT khi ô đang "--": báo và không gửi.
- **ATT** lưu giá trị **đã gửi** xuống board. Xoá ô ATT mà không gửi thì lần mở sau vẫn hiện giá trị đã gửi gần nhất.

**Sửa thêm:** ô Terminal trước đây luôn cuộn hụt mất dòng cuối; giờ đã cuộn tới dòng mới nhất.

## 6. Giả định chưa xác nhận trên phần cứng — cần anh kiểm tra

1. **`MEAS:VOLT?` / `MEAS:CURR?` cho AC.** Manual IT-M7700 ghi dạng `MEAS:VOLT:AC?` / `MEAS:CURR:AC?`. Mình dùng dạng anh đưa.
   - Nếu máy không nhận dạng ngắn, app sẽ báo "mã SCPI …" và đồ thị không có số.
   - Khi đó sửa 2 macro `AC_CMD_MEAS_VOLT` / `AC_CMD_MEAS_CURR` trong `board_config.h`.
2. **Lệnh đọc DC** hiện là `#0 MEAS:VOLT?` / `#0 MEAS:CURR?` (giữ tiền tố `#0` như các lệnh DC khác). **Chưa có manual đúng model Matsusada**, nên đây là giả định. Nếu sai thì:
   - nguồn im lặng: app báo "Nguồn DC không phản hồi";
   - hoặc trả dạng khác: app báo "trả lời không đọc được".
   
   Anh gửi model hoặc trang bảng lệnh thì mình sửa cho đúng.
3. **DC chưa có lệnh hỏi trạng thái ngõ ra** (macro `DC_CMD_QUERY_OUTPUT` đang để trống). Vì vậy với DC, "đã xác nhận" chỉ có nghĩa là nguồn còn trả lời, chưa chứng minh relay đã đóng. Có lệnh đó thì điền vào macro là xong.
4. **Timeout 500 ms mỗi câu hỏi** (`PWR_REPLY_TIMEOUT_MS`). Nếu `MEAS` của nguồn thật chậm hơn mức này, tăng giá trị trong `board_config.h`.
5. **`PROT:MODE AC`** giữ nguyên vì anh xác nhận máy không báo lỗi. Nếu lệnh này có lỗi, `SYST:ERR?` sẽ lộ ra ngay ở lần chạy đầu.

## 7. Đã kiểm chứng thế nào

| Hạng mục | Kết quả |
|---|---|
| Firmware build: GCC aarch64 11, cờ Vitis `-Wall -O0 -g3`, 27/27 file | `src/` **0 cảnh báo**, kể cả với `-Wextra` và nhánh UART có ngắt |
| Nguồn giả lập: IT-M7700 (SCPI) và Matsusada (`#0`), qua FIFO 16 byte @9600 | 11 kịch bản: trả lời đúng, im lặng, chậm 700 ms, rác, echo, lỗi -113, relay kẹt, 3 lỗi tồn trong hàng đợi, tắt, RESET — đều cho đúng kết quả (`firmware_scenarios.txt`) |
| Stress thời gian ảo: 90 lệnh / 30 ms xen lệnh AC | 90/90 ACK, 0 byte tràn FIFO, khoảng hở đọc RX lớn nhất 0,02 ms, phổ RF vẫn đều 13 khung / 4,2 s |
| E2E app C++ thật ↔ firmware thật (pty) | **47/47** (`e2e_power.txt`): chạy lần đầu "--", không gửi gì lúc init, chặn bật khi thiếu giá trị, xác nhận AC/DC, F5, mọi kịch bản lỗi, Đặt lại, ghi rồi nạp lại config.ini |
| App mới ↔ firmware 01/10 (chưa có đọc ngược) | **26/26**: lệnh vẫn chạy, được đánh dấu "chưa xác nhận"; refresh báo rõ "firmware cũ chưa hỗ trợ" |
| Phổ RF (hồi quy) | 20/20, tone −20 dBFS @ 2445,625 MHz, hiệu chuẩn −30 dBm |
| GUI thật + backend thật + firmware sim (Qt 6.4): bấm chuột vào công tắc, phím F5 | 9/9, 0 cảnh báo QML (ảnh `gui_qt64_*.png`) |
| Qt 6.11.2 (đúng bản của anh): `qmlcachegen` 18/18 file; render toàn app + hộp "Đặt lại" | 0 lỗi, 0 cảnh báo QML; `qmllint` không có cảnh báo mới (ngoài kiểu `dashboard` không qualify vốn có sẵn trong app) |
| Build C++ bằng CMake (Qt 6.4 / GCC 13) | 0 lỗi, 0 cảnh báo; backend cũng sạch với `-Wall -Wextra` |

**Giới hạn còn lại:**

- Chưa chạy trên nguồn thật và board thật.
- Phần C++ chưa build bằng đúng Qt 6.11.2 / MinGW 13.1 của anh, vì sandbox không có toolchain Windows.
- Khi build, nhớ đặt project ở **đường dẫn ngắn** (ví dụ `D:\Qt\DashboardEnv_real`) và xoá thư mục `build\` cũ. Lỗi `.obj.d` lần trước là do đường dẫn dài 270 ký tự, vượt giới hạn 260 của Windows.

## 8. Thử trên bench (gợi ý)

1. Nạp firmware, mở app, bắt đầu phiên.
2. Nhấn **F5** với cả hai nguồn đang **tắt**. Kỳ vọng: AC "Cập nhật …" với 0 V / 0 Hz, DC tương tự.
   - Nếu báo "không phản hồi": kiểm tra baud trên mặt máy (9600, 8N1), cáp TX/RX.
   - Nếu báo "mã SCPI" hoặc "không đọc được": sửa macro lệnh trong `board_config.h` như mục 6.
3. Nhập giá trị, gạt BẬT AC. Công tắc khoá trong lúc chờ, sau đó hiện thông báo xanh "Nguồn AC đã xác nhận: BẬT" kèm điểm đo đầu tiên.
4. Có thể thử nhanh bằng công cụ không cần app:
   ```
   python tools/vip_test_client.py --port COM5 pwq ac
   python tools/vip_test_client.py --port COM5 ac 110 50 1
   ```
   Dòng `PWS … result=…` cho biết đúng nguồn trả lời gì.
