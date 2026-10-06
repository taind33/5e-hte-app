# Phổ 124 điểm @100 ms, Terminal CLI, kết nối RS422 — 2026-10-05 (đợt 3)

Bản này dựa trên bản "chẩn đoán" trước đó.
- **Firmware:** sửa 8 file trong `src/`, không thêm file mới.
- **App:** sửa 8 file và thêm 1 file mới là `backend/dashboardcli.cpp` (đã khai báo trong `CMakeLists.txt`).

---

## 1. Phổ RF cập nhật mỗi 100 ms

- `RF_WAVE_PERIOD_MS` đổi từ 300 xuống 100.
- Lịch gửi chạy theo lưới cố định 100 ms. Nếu tính "100 ms kể từ khi gửi xong khung trước" thì chu kỳ thực tế bị kéo lên khoảng 124 ms, vì mỗi khung mất khoảng 24 ms để truyền.
- Đo trên giả lập: trung bình **100,3 ms/khung**.
- Tải RS422 115200: khoảng 2,7 KB/s, tức khoảng 25% đường truyền. Phần còn lại vẫn đủ cho lệnh và ACK.

## 2. Số điểm phổ tối đa trong 1 bản tin (phương án B)

**Gói mới "WAVD"** (msgid 39), tối đa 255 byte, gồm:
- `"WAVD"` (4 byte);
- số điểm n (1 byte);
- loại cửa sổ (1 byte);
- n giá trị int16, đơn vị **dBFS × 0,01**.

Như vậy được **124 điểm** trong 1 bản tin (trước là 32).

**Firmware:**
- Tính **DFT 124 điểm** (124 không phải lũy thừa của 2) với bảng twiddle tính sẵn.
- Tải trên A53: khoảng 15 nghìn phép nhân-cộng phức mỗi khung.
- Độ phân giải: **0,48 MHz/điểm** (trước 1,875 MHz).
- Bộ đệm DMA làm tròn lên bội số cache line 32 byte.

**App:**
- Đọc được cả "WAVD" mới lẫn "WAVE" cũ.
- Trục tần số, tổng công suất (Parseval) và hiệu chuẩn dBm vẫn tính như trước.
- Phổ mô phỏng khi chưa kết nối cũng chuyển sang 124 điểm, 100 ms.

## 3–4. Terminal kiểu dòng lệnh

Gõ `help` để xem toàn bộ, `help <lệnh>` để xem chi tiết kèm ví dụ.

| Lệnh | Việc làm |
|---|---|
| `help`, `help ac` | Danh sách lệnh / chi tiết kèm ví dụ |
| `clear` | Xoá màn hình Terminal |
| `status` | Kết nối, số byte/khung nhận được, firmware, AD9361/DMA, nguồn AC/DC |
| `ac on 120 50` / `ac off` | Bật/tắt nguồn AC. Ô nhập trên giao diện cập nhật theo |
| `ac read`, `dc read`, `read all` | Đọc số đo ngay (giống nút ⟳ / F5) |
| **`ac clear`** | **Reset lỗi/bảo vệ của nguồn AC**: gửi `PROT:CLE` (PROTect:CLEar theo manual IT-M7700) |
| `ac err`, `ac idn`, `ac power` | `SYST:ERR?`, `*IDN?`, `MEAS:POW?` |
| `dc on 24 2.5` / `dc off` | Bật/tắt nguồn DC |
| `dc status` | `#0 STT?` (đọc trạng thái/giá trị cài đặt) |
| `att 1 12.75`, `delay 270` | Đặt suy hao / giữ chậm. Giá trị lẻ được làm tròn theo bước 0,25 dB và báo lại |
| `reset` → `reset yes` | Đặt lại hệ thống, phải gõ xác nhận trong 30 s |
| `ping` | Gửi PING, in thời gian board trả lời |
| `scan` | **Tự tìm cổng COM + baud đang có board** (mục 5) |
| `monitor on/off` | Mỗi giây in số byte, khung đúng/lỗi và loại khung nhận được |
| `board <TÊN> [v1 v2 v3]` | Lệnh chung tới **firmware board** (kiểu cũ "TÊN, v1, v2"). Firmware hiện chỉ xử lý `RESET` |

Thao tác bàn phím: **↑/↓** gọi lại lệnh cũ, **Tab** hoàn thành lệnh.

Màu dòng trong Terminal:
- xanh dương: lệnh vừa gõ;
- đỏ: lệnh gửi đi (TX);
- đỏ sẫm: lỗi;
- cam: cảnh báo từ board.

### "raw" là gì

`raw` gửi **nguyên văn** một lệnh tới **nguồn AC hoặc DC**. Dùng cho các lệnh mà danh sách trên chưa có. Board chuyển chuỗi đó sang cổng RS232 của nguồn rồi trả câu trả lời về Terminal.

```
ac raw *IDN?            -> AC → ITECH, M7722, ...
ac raw MEAS:POW?        -> AC → +2.640000E+02          (công suất đo)
ac raw SYST:LOC         -> AC → (đã gửi) [SYST:ERR? = 0, không lỗi] [ngõ ra ON]
ac raw OUTP OFF         -> AC → (đã gửi) [SYST:ERR? = 0, không lỗi] [ngõ ra OFF]
ac raw BOGUS            -> AC → (đã gửi) [LỖI SCPI -113]
dc raw STT?             -> app tự thêm "#0 " -> DC ← #0 STT? -> DC → <trả lời của nguồn>
dc raw OVP 30           -> gửi "#0 OVP 30"
```

Cách board xử lý:
- **Lệnh có `?`** là câu hỏi: in câu trả lời của nguồn.
- **Lệnh ghi (AC):** board hỏi thêm `SYST:ERR?` và `OUTP?`, để biết nguồn có chấp nhận không và ngõ ra đang thế nào. Công tắc AC trên giao diện tự cập nhật theo trạng thái này.
- **Lệnh ghi (DC):** chỉ gửi đi, vì chưa có lệnh hỏi lỗi tương đương.
- **Nguồn đang bận** (đang xác nhận lệnh bật/tắt hoặc đang đọc): raw không chen ngang mà báo "nguồn đang bận, thử lại sau 1–2 s".

**Lưu ý:** raw cho phép gửi **mọi** lệnh, kể cả lệnh đổi điện áp. Hãy dùng các lệnh `ac on/off`, `dc on/off` cho thao tác thường ngày, vì chúng có kiểm tra giới hạn.

## 5. Đèn kết nối vẫn sáng cam

Mình chưa có board thật để thấy đúng nguyên nhân, nên bản này sửa những gì có thể sửa từ phía phần mềm, và giúp app chỉ thẳng ra chỗ hỏng.

1. **Bật DTR và RTS ngay khi mở cổng.** Đây là thay đổi đáng thử nhất.
   - Trên Windows, `QSerialPort` mặc định **không** bật hai chân này. Các phần mềm terminal (TeraTerm, Hercules…) thì bật.
   - Nhiều bộ chuyển USB/RS232 sang RS422/485 **lấy nguồn từ DTR/RTS**, hoặc dùng RTS để bật bộ phát. Khi đó cổng vẫn mở được, nhưng không có byte nào đi hay về, và đèn sáng cam.
   - Firmware tham chiếu cũ từng được thử qua phần mềm terminal, nên nếu đúng nguyên nhân này thì trước đây không lộ ra.
2. **Lý do hiện ngay cạnh đèn**, không còn phải rê chuột:
   - "Không có byte về";
   - "Sai khung (baud/A-B?)";
   - "Không trả PING".
   
   Rê chuột vào đèn hoặc gõ `status` để xem chi tiết.
3. **`scan`** tự thử mọi cổng COM ở 115200 trước, rồi đến 230400, 57600, 460800, 921600, 38400, 19200, 9600. Ở mỗi cổng/baud, app gửi PING và chờ board trả lời.
   - Tìm thấy: app chọn sẵn cổng và baud đó.
   - Nếu board trả lời ở baud **khác** 115200: clock UART RS422 trong HDL/xparameters đang lệch (`XPAR_RS422_PC_CLOCK_FREQ_HZ` = 100 MHz phải khớp clock thật của IP).
4. **`ping` / `monitor on`** để xem board có trả lời không và đang gửi những gì.

**Kiểm tra firmware đang chạy là bản mới:** ô thông tin phải hiện **"FW : <thời điểm build>"**. Nếu dòng này là "--", app chưa nhận được gói nào từ firmware mới. Nhắc lại lỗi build lần trước: thư mục `5e_hte_app` bị lồng hai cấp, khiến build hỏng và Vitis có thể nạp file ELF cũ.

**Cần anh gửi lại kết quả** của `scan` hoặc `status` (chụp Terminal). Từ đó biết ngay lỗi nằm ở dây, cổng, baud hay firmware.

## 6. Font LED 7 đoạn
Giữ nguyên `"Seven Segment"` cỡ 12 như bản gốc (bản render Qt 6.11.2 hiển thị đúng kiểu LED).

---

## Đã kiểm chứng thế nào

| Hạng mục | Kết quả |
|---|---|
| Firmware build: aarch64 GCC 11, cờ Vitis, 27/27 file | 0 cảnh báo trong `src/`, cả `-Wextra` và nhánh UART có ngắt |
| Phổ "WAVD" từ firmware sim (tone giả lập -20 dBFS) | 124 điểm, đỉnh đúng bin 12 = **-20,00 dBFS**, chu kỳ **100,3 ms** |
| Lệnh thô qua firmware sim (nguồn giả lập) | `*IDN?`, `PROT:CLE`, `OUTP ON/OFF`, `MEAS:POW?`, lệnh sai → SCPI -113, `#0 STT?`, nguồn im lặng → không trả lời: đều đúng |
| E2E Terminal CLI (backend thật ↔ firmware sim) | **24/24**: help, clear, Tab, lệnh sai, scan (bỏ qua cổng chết, tìm đúng board), status, ping, ac/dc on, att làm tròn, delay, monitor, raw, ac clear, dc status, raw OUTP OFF cập nhật công tắc, read all, reset có xác nhận |
| Hồi quy: phổ RF | 20/20 (124 điểm, tone @ 2445,806 MHz, -20,000 dBFS; hiệu chuẩn -30 dBm) |
| Hồi quy: đọc nguồn | 47/47 (3 lần) |
| Hồi quy: firmware cũ | 26/26 |
| Hồi quy: chẩn đoán | 27/27 |
| Hồi quy: stress 90 lệnh | 90/90 ACK, 41 khung phổ / 4,2 s, khoảng hở đọc RX 0,02 ms |
| GUI thật + backend + firmware sim (Qt 6.4), bình thường và AD9361 lỗi | 9/9 mỗi trường hợp |
| Qt 6.11.2 | `qmlcachegen` 18/18 file, render 0 cảnh báo, `qmllint` không có cảnh báo mới |
| Build C++ (CMake) | 0 lỗi, 0 cảnh báo |

**Ghi chú trung thực:** máy giả lập chỉ có 2 CPU và chạy firmware như một tiến trình Linux thường, nên đôi khi bị dừng vài ms. Khi đó FIFO 16 byte mô phỏng tràn và mất byte lệnh. Có 1 lần chạy CLI ra 23/24 vì lý do này (`e2e_cli_run0_LOI_DO_JITTER_SIM.txt`). Đây không phải lỗi firmware:
- Ở chế độ thời gian ảo, firmware không bao giờ để RX quá 0,02 ms mà không đọc.
- Chạy lại với ưu tiên thời gian thực cho sim đều đạt 24/24.

**Giới hạn:**
- Chưa chạy trên board thật.
- C++ chưa build bằng MinGW 13.1 của anh.
- Nhớ giải nén ra chỗ khác rồi **chỉ chép `src/`** vào project Vitis, tránh lồng thư mục như lần trước.
