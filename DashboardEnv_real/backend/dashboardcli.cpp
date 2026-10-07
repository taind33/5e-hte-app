// dashboardcli.cpp - Terminal kiểu dòng lệnh (CLI) của DashboardManager.
//
// Mọi lệnh gõ đi qua ĐÚNG các hàm mà nút bấm trên giao diện dùng
// (requestAcPower, requestSetAtt, applyDelayTime, refreshPower...) nên có
// cùng giới hạn/kiểm tra, và ô nhập/công tắc trên giao diện cập nhật theo.
#include "dashboardmanager.h"
#include <QSerialPortInfo>
#include <QRegularExpression>
#include <cmath>

namespace {

// Lệnh chức năng của nguồn AC IT-M7700 (programming guide). Gửi qua kênh
// lệnh thô (msgid 43), board trả lời bằng msgid 44.
const char *AC_PROT_CLEAR = "PROT:CLE";       // PROTect:CLEar - xoá trạng thái bảo vệ (reset lỗi)
const char *AC_ERR_QUERY  = "SYST:ERR?";
const char *AC_ERR_CLEAR  = "SYST:CLE";       // xoá hàng đợi lỗi
const char *AC_IDN        = "*IDN?";
const char *AC_POWER      = "MEAS:POW?";
// Nguồn DC Matsusada: đọc trạng thái/giá trị cài đặt.
const char *DC_STATUS     = "#0 STT?";

struct CliCmd { const char *name; const char *usage; const char *desc; };
const CliCmd kCommands[] = {
    { "help",    "help [lệnh]",                   "Danh sách lệnh, hoặc chi tiết 1 lệnh" },
    { "clear",   "clear",                         "Xoá màn hình Terminal" },
    { "status",  "status",                        "Kết nối, firmware, AD9361/DMA, trạng thái nguồn AC/DC" },
    { "ac",      "ac on <V> <Hz> | ac off | ac read | ac clear | ac err | ac idn | ac power | ac raw <lệnh>",
                                                  "Nguồn AC (IT-M7700)" },
    { "dc",      "dc on <V> <A> | dc off | dc read | dc status | dc raw <lệnh>",
                                                  "Nguồn DC (Matsusada)" },
    { "read",    "read ac|dc|all",                "Đọc số đo tức thời (giống nút ⟳ / phím F5)" },
    { "att",     "att <1-3> <dB>",                "Đặt suy hao F1956, 0..31.75 dB, bước 0.25" },
    { "delay",   "delay <0|100|270|400>",          "Đặt giữ chậm quang (ns)" },
    { "reset",   "reset  (rồi: reset yes)",       "Đặt lại hệ thống: tắt AC/DC, ATT 0 dB, delay 0 ns" },
    { "ping",    "ping",                          "Gửi PING tới board, đo thời gian trả lời" },
    { "scan",    "scan",                          "Tự tìm cổng COM + baud đang có board 5E-HTE" },
    { "monitor", "monitor on|off",                "Mỗi giây in số byte/khung nhận được từ board" },
    { "board",   "board <TÊN> [v1 v2 v3]",        "Lệnh chung tới FIRMWARE board (kiểu cũ, msgid 39)" },
    { "cal",     "cal | cal clear <MHz> | cal clear all",
                                                  "Bảng hiệu chuẩn phổ RF theo tần số (dBFS -> dBm)" },
};

QStringList tokens(const QString &line)
{
    return line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}

bool toNum(const QString &s, double &out)
{
    bool ok = false;
    QString t = s;
    t.replace(',', '.');
    out = t.toDouble(&ok);
    return ok && std::isfinite(out);
}

} // namespace

void DashboardManager::cliOut(const QString &text)
{
    m_commandLog.append(text);
    while (m_commandLog.size() > MAX_LOG_LINES)
        m_commandLog.removeFirst();
    emit commandLogChanged();
}

void DashboardManager::cliError(const QString &text)
{
    appendCommandLog("ERR", text);
}

void DashboardManager::clearCommandLog()
{
    m_commandLog.clear();
    emit commandLogChanged();
}

QStringList DashboardManager::cliCompletions(const QString &prefix) const
{
    static const QStringList words = {
        "help", "clear", "status", "read ac", "read dc", "read all",
        "ac on", "ac off", "ac read", "ac clear", "ac err", "ac idn", "ac power", "ac raw",
        "dc on", "dc off", "dc read", "dc status", "dc raw",
        "att 1", "att 2", "att 3", "delay", "reset", "reset yes",
        "ping", "scan", "monitor on", "monitor off", "board",
        "cal", "cal clear", "cal clear all" };
    QStringList out;
    const QString p = prefix.trimmed().toLower();
    for (const QString &w : words)
        if (w.startsWith(p))
            out << w;
    return out;
}

void DashboardManager::executeCommand(const QString &line)
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return;

    // Tiếng vọng lệnh vào Terminal + file log phiên.
    appendCommandLog("CMD", trimmed);
    writeToLog("[CLI] " + trimmed);

    const QStringList a = tokens(trimmed);
    const QString cmd = a.first().toLower();

    // Kiểu lệnh cũ "TÊN, v1, v2" (có dấu phẩy) -> nhắc dùng "board".
    if (cmd.contains(',')) {
        cliError(QString("Không hiểu lệnh '%1'. Lệnh chung tới firmware giờ là: board <TÊN> [v1 v2 v3]. Gõ help.").arg(trimmed));
        return;
    }
    if (cmd == "help" || cmd == "?")  { cliHelp(a.size() > 1 ? a[1].toLower() : QString()); return; }
    if (cmd == "clear" || cmd == "cls") { clearCommandLog(); return; }
    if (cmd == "status")  { cliStatus(); return; }
    if (cmd == "ac")      { cliAc(a); return; }
    if (cmd == "dc")      { cliDc(a); return; }
    if (cmd == "read")    { cliRead(a); return; }
    if (cmd == "att")     { cliAtt(a); return; }
    if (cmd == "delay")   { cliDelay(a); return; }
    if (cmd == "reset")   { cliReset(a); return; }
    if (cmd == "ping")    { cliPing(); return; }
    if (cmd == "scan")    { startScan(); return; }
    if (cmd == "monitor") { cliMonitor(a); return; }
    if (cmd == "board")   { cliBoard(a, trimmed); return; }
    if (cmd == "cal")     { cliCal(a); return; }

    cliError(QString("Không có lệnh '%1'. Gõ help để xem danh sách lệnh.").arg(a.first()));
}

void DashboardManager::cliHelp(const QString &topic)
{
    if (!topic.isEmpty()) {
        for (const CliCmd &c : kCommands) {
            if (topic == QLatin1String(c.name)) {
                cliOut(QString("%1 — %2").arg(QString::fromUtf8(c.name), QString::fromUtf8(c.desc)));
                cliOut("  Cú pháp: " + QString::fromUtf8(c.usage));
                if (topic == "ac") {
                    cliOut("  Ví dụ: ac on 120 50   (bật 120 V, 50 Hz)");
                    cliOut("         ac clear       (reset lỗi/bảo vệ của nguồn: PROT:CLE)");
                    cliOut("         ac raw MEAS:POW?   (gửi nguyên văn 1 lệnh SCPI bất kỳ, in câu trả lời)");
                } else if (topic == "dc") {
                    cliOut("  Ví dụ: dc on 24 2.5   (bật 24 V, giới hạn dòng 2.5 A)");
                    cliOut("         dc status      (#0 STT? - đọc giá trị cài đặt/trạng thái)");
                    cliOut("         dc raw OVP 30  (tự thêm \"#0 \" nếu thiếu -> gửi \"#0 OVP 30\")");
                } else if (topic == "cal") {
                    cliOut("  Hiệu chuẩn: phát tín hiệu chuẩn vào RX1, nhập công suất (dBm) ở khung Phổ RF,");
                    cliOut("  bấm \"Hiệu chuẩn\" - app tự gán vào điểm tần số gần đỉnh phổ nhất (±2.5 MHz).");
                    cliOut("  Ví dụ: cal              (xem bảng hệ số)");
                    cliOut("         cal clear 2445   (xoá điểm 2445 MHz)");
                    cliOut("         cal clear all    (xoá tất cả)");
                    cliOut("  Danh sách điểm sửa trong config.ini: rf/calPointsMHz=2440,2445,2450,2455,2460");
                } else if (topic == "board") {
                    cliOut("  Gửi lệnh chung tới firmware của board (không phải tới nguồn).");
                    cliOut("  Firmware hiện chỉ xử lý RESET; lệnh khác chỉ được ACK. Ví dụ: board RESET");
                }
                return;
            }
        }
        cliError(QString("Không có lệnh '%1'. Gõ help.").arg(topic));
        return;
    }
    cliOut("──────── LỆNH TERMINAL ────────");
    for (const CliCmd &c : kCommands)
        cliOut(QString("%1  —  %2").arg(QString::fromUtf8(c.usage), QString::fromUtf8(c.desc)));
    cliOut("Lệnh thô tới nguồn (raw): gửi NGUYÊN VĂN 1 lệnh mà danh sách trên không có,");
    cliOut("  vd: ac raw *IDN?  |  ac raw MEAS:POW?  |  ac raw SYST:LOC  |  dc raw #0 STT?");
    cliOut("  Lệnh có '?' là câu hỏi -> in câu trả lời của nguồn. Lệnh ghi (không '?') với AC");
    cliOut("  được kiểm tra bằng SYST:ERR? + OUTP? và báo lại có lỗi hay không.");
    cliOut("Mẹo: ↑/↓ gọi lại lệnh cũ, Tab hoàn thành lệnh, help <lệnh> xem chi tiết.");
}

void DashboardManager::cliStatus()
{
    cliOut("──────── TRẠNG THÁI ────────");
    if (!m_serialPort->isOpen()) {
        cliOut(QString("Kết nối: chưa mở (COM %1, baud %2)").arg(m_comPort.isEmpty() ? "--" : m_comPort,
                                                                m_baudRate > 0 ? QString::number(m_baudRate) : "--"));
    } else {
        cliOut(QString("Kết nối: %1 @ %2 — %3").arg(m_comPort).arg(m_baudRate)
                   .arg(m_isPortVerified ? "đã xác thực board" : "CHƯA xác thực"));
        cliOut(QString("Nhận: %1 byte, %2 khung đúng, %3 khung lỗi").arg(m_rxBytes).arg(m_rxFramesOk).arg(m_rxFramesBad));
        if (!m_isPortVerified)
            cliOut("Chẩn đoán: " + m_linkDiagnosis);
    }
    if (m_haveDiag) {
        const board_diag_msg_t &d = m_lastDiag;
        cliOut(QString("Firmware: build %1, chạy %2 s").arg(m_boardFirmware).arg(d.uptime_s));
        cliOut(QString("AD9361: %1 | Phổ RF: %2 | DMA ok %3, timeout %4 | đỉnh ADC RX1 %5/2047")
                   .arg(d.ad9361_ok ? QStringLiteral("OK") : QString("LỖI (mã %1)").arg(d.ad9361_status))
                   .arg(d.rf_stream_ok ? "chạy" : "tắt").arg(d.dma_ok).arg(d.dma_timeout).arg(d.rx1_peak));
        cliOut(QString("Board nhận từ PC: %1 byte, %2 khung đúng, %3 khung lỗi")
                   .arg(d.pc_rx_bytes).arg(d.pc_frames_ok).arg(d.pc_frames_bad));
    } else {
        cliOut("Firmware: chưa nhận gói DIAG từ board");
    }
    auto src = [&](const char *name, BasePowerSource *s) {
        cliOut(QString("Nguồn %1: %2, lần đọc cuối %3%4 [%5]").arg(QString::fromLatin1(name))
                   .arg(s->isOn() ? "BẬT" : "TẮT")
                   .arg(s->lastUpdate().isEmpty() ? QStringLiteral("--") : s->lastUpdate())
                   .arg(s->hasData() ? QString(" (%1 V, %2 A)").arg(s->voltage(), 0, 'f', 2).arg(s->current(), 0, 'f', 3) : QString())
                   .arg(s->linkState()));
    };
    src("AC", m_acSource);
    src("DC", m_dcSource);
    cliCal(QStringList{QStringLiteral("cal")});
}

void DashboardManager::cliCal(const QStringList &a)
{
    const QString sub = a.size() > 1 ? a[1].toLower() : QString();
    if (sub == "clear") {
        if (a.size() < 3) { cliError("Cú pháp: cal clear <MHz> | cal clear all"); return; }
        if (a[2].toLower() == "all") { clearRfCalibration(); cliOut("Đã xoá toàn bộ bảng hiệu chuẩn."); return; }
        double f = 0.0;
        if (!toNum(a[2], f)) { cliError(QString("Tần số không hợp lệ: '%1'").arg(a[2])); return; }
        const int idx = rfCalNearestPoint(f);
        if (idx < 0 || std::abs(m_calPoints[idx].mhz - f) > 0.01) {
            QStringList l;
            for (const RfCalPoint &p : m_calPoints) l << QString::number(p.mhz, 'f', 1);
            cliError(QString("Không có điểm %1 MHz. Các điểm: %2").arg(a[2], l.join(", ")));
            return;
        }
        if (!m_calPoints[idx].valid) { cliOut(QString("Điểm %1 MHz chưa được hiệu chuẩn.").arg(m_calPoints[idx].mhz, 0, 'f', 1)); return; }
        clearRfCalPoint(m_calPoints[idx].mhz);
        cliOut(QString("Đã xoá điểm %1 MHz.").arg(m_calPoints[idx].mhz, 0, 'f', 1));
        return;
    }
    if (!sub.isEmpty()) { cliError("Cú pháp: cal | cal clear <MHz> | cal clear all"); return; }

    cliOut("Hiệu chuẩn phổ RF (dBm = dBFS + hệ số):");
    for (const RfCalPoint &p : m_calPoints) {
        if (!p.valid) {
            cliOut(QString("  %1 MHz : --").arg(p.mhz, 7, 'f', 1));
            continue;
        }
        const bool mism = m_rfStatusValid && (p.loHz != m_rxLoHz || p.gainDb != m_rxGainDb);
        cliOut(QString("  %1 MHz : %2 dB   (LO %3 MHz, gain %4 dB, %5)%6")
                   .arg(p.mhz, 7, 'f', 1).arg(p.offsetDb, 6, 'f', 2)
                   .arg(static_cast<double>(p.loHz) / 1e6, 0, 'f', 3).arg(p.gainDb)
                   .arg(p.when.isEmpty() ? QStringLiteral("--") : p.when)
                   .arg(mism ? QStringLiteral("  <- LO/gain KHÁC hiện tại") : QString()));
    }
    cliOut("  Đang dùng: " + m_rfController->calInfo());
}

void DashboardManager::cliAc(const QStringList &a)
{
    const QString sub = a.size() > 1 ? a[1].toLower() : QString();
    if (sub == "on") {
        if (a.size() >= 4) {
            double v, f;
            if (!toNum(a[2], v) || !toNum(a[3], f)) { cliError("Cú pháp: ac on <V> <Hz>, vd: ac on 120 50"); return; }
            if (v < acVoltMin() || v > acVoltMax()) { cliError(QString("Điện áp AC phải trong %1..%2 V").arg(acVoltMin()).arg(acVoltMax())); return; }
            if (f < acFreqMin() || f > acFreqMax()) { cliError(QString("Tần số AC phải trong %1..%2 Hz").arg(acFreqMin()).arg(acFreqMax())); return; }
            setStagedAcVolt(v);
            setStagedAcFreq(f);
        } else if (a.size() != 2) {
            cliError("Cú pháp: ac on <V> <Hz>   (hoặc 'ac on' dùng giá trị đang nhập trên giao diện)");
            return;
        }
        requestAcPower(true);
    } else if (sub == "off") {
        requestAcPower(false);
    } else if (sub == "read") {
        refreshPower(PWR_DEV_AC);
    } else if (sub == "clear") {
        sendRawSupply(PWR_DEV_AC, QString::fromLatin1(AC_PROT_CLEAR));
    } else if (sub == "err") {
        sendRawSupply(PWR_DEV_AC, QString::fromLatin1(AC_ERR_QUERY));
    } else if (sub == "errclear") {
        sendRawSupply(PWR_DEV_AC, QString::fromLatin1(AC_ERR_CLEAR));
    } else if (sub == "idn") {
        sendRawSupply(PWR_DEV_AC, QString::fromLatin1(AC_IDN));
    } else if (sub == "power") {
        sendRawSupply(PWR_DEV_AC, QString::fromLatin1(AC_POWER));
    } else if (sub == "raw") {
        const QString text = a.mid(2).join(' ');
        if (text.isEmpty()) { cliError("Cú pháp: ac raw <lệnh SCPI>, vd: ac raw MEAS:POW?"); return; }
        sendRawSupply(PWR_DEV_AC, text);
    } else {
        cliHelp("ac");
    }
}

void DashboardManager::cliDc(const QStringList &a)
{
    const QString sub = a.size() > 1 ? a[1].toLower() : QString();
    if (sub == "on") {
        if (a.size() >= 4) {
            double v, i;
            if (!toNum(a[2], v) || !toNum(a[3], i)) { cliError("Cú pháp: dc on <V> <A>, vd: dc on 24 2.5"); return; }
            if (v < dcVoltMin() || v > dcVoltMax()) { cliError(QString("Điện áp DC phải trong %1..%2 V").arg(dcVoltMin()).arg(dcVoltMax())); return; }
            if (i < dcCurrentMin() || i > dcCurrentMax()) { cliError(QString("Dòng DC phải trong %1..%2 A").arg(dcCurrentMin()).arg(dcCurrentMax())); return; }
            setStagedDcVolt(v);
            setStagedDcCurrent(i);
        } else if (a.size() != 2) {
            cliError("Cú pháp: dc on <V> <A>   (hoặc 'dc on' dùng giá trị đang nhập trên giao diện)");
            return;
        }
        requestDcPower(true);
    } else if (sub == "off") {
        requestDcPower(false);
    } else if (sub == "read") {
        refreshPower(PWR_DEV_DC);
    } else if (sub == "status") {
        sendRawSupply(PWR_DEV_DC, QString::fromLatin1(DC_STATUS));
    } else if (sub == "raw") {
        QString text = a.mid(2).join(' ');
        if (text.isEmpty()) { cliError("Cú pháp: dc raw <lệnh>, vd: dc raw #0 STT?"); return; }
        if (!text.startsWith('#'))
            text = "#0 " + text;            // địa chỉ đơn vị như mọi lệnh DC khác
        sendRawSupply(PWR_DEV_DC, text);
    } else {
        cliHelp("dc");
    }
}

void DashboardManager::cliRead(const QStringList &a)
{
    const QString w = a.size() > 1 ? a[1].toLower() : QStringLiteral("all");
    if (w == "ac") refreshPower(PWR_DEV_AC);
    else if (w == "dc") refreshPower(PWR_DEV_DC);
    else if (w == "all") refreshAllPower();
    else cliError("Cú pháp: read ac|dc|all");
}

void DashboardManager::cliAtt(const QStringList &a)
{
    double ch, db;
    if (a.size() != 3 || !toNum(a[1], ch) || !toNum(a[2], db) || (ch != 1 && ch != 2 && ch != 3)) {
        cliError("Cú pháp: att <1-3> <dB>, vd: att 1 12.75");
        return;
    }
    if (db < 0.0 || db > attMax()) {
        cliError(QString("Suy hao phải trong 0..%1 dB").arg(attMax()));
        return;
    }
    const double q = std::round(db * 4.0) / 4.0;     // F1956 bước 0.25 dB
    if (q != db)
        cliOut(QString("  (làm tròn %1 -> %2 dB theo bước 0.25 dB)").arg(db).arg(q, 0, 'f', 2));
    if (!m_serialPort->isOpen()) {
        cliError("Chưa kết nối board.");
        return;
    }
    m_rfController->requestSetAtt(static_cast<int>(ch), q, true);
}

void DashboardManager::cliDelay(const QStringList &a)
{
    double ns;
    if (a.size() != 2 || !toNum(a[1], ns) || (ns != 0 && ns != 100 && ns != 270 && ns != 400)) {
        cliError("Cú pháp: delay <0|100|270|400>");
        return;
    }
    if (!m_serialPort->isOpen()) {
        cliError("Chưa kết nối board.");
        return;
    }
    applyDelayTime(static_cast<int>(ns));
}

void DashboardManager::cliReset(const QStringList &a)
{
    const bool yes = a.size() > 1 && a[1].toLower() == "yes";
    if (yes && m_resetArmed && m_resetArmClock.elapsed() < 30000) {
        m_resetArmed = false;
        resetSystem();
        return;
    }
    if (yes) {
        cliError("Gõ 'reset' trước, rồi 'reset yes' trong vòng 30 s để xác nhận.");
        return;
    }
    m_resetArmed = true;
    m_resetArmClock.start();
    cliOut("Đặt lại sẽ TẮT ngõ ra nguồn AC và DC, ATT về 0 dB, giữ chậm về 0 ns, xoá đỉnh kích nổ.");
    cliOut("Gõ 'reset yes' để xác nhận (trong 30 s).");
}

void DashboardManager::cliBoard(const QStringList &a, const QString &line)
{
    if (a.size() < 2) {
        cliHelp("board");
        return;
    }
    // sendCommand() giữ đúng định dạng cũ: "TÊN, v1, v2, v3" -> msgid 39.
    QStringList parts;
    parts << a[1];
    for (int i = 2; i < a.size(); ++i)
        parts << a[i];
    Q_UNUSED(line);
    sendCommand(parts.join(", "));
}

void DashboardManager::cliPing()
{
    if (!m_serialPort->isOpen()) {
        cliError("Chưa kết nối - bấm BẮT ĐẦU PHIÊN trước (hoặc gõ scan để tự tìm cổng).");
        return;
    }
    m_cliPingPending = true;
    m_cliPingClock.start();
    sendPingProbe();
    QTimer::singleShot(PING_TIMEOUT_MS + 100, this, [this] {
        if (m_cliPingPending) {
            m_cliPingPending = false;
            cliError(QString("PING: board không trả lời trong %1 ms. %2").arg(PING_TIMEOUT_MS).arg(m_linkDiagnosis));
        }
    });
}

void DashboardManager::cliMonitor(const QStringList &a)
{
    const QString w = a.size() > 1 ? a[1].toLower() : QString();
    if (w == "on") {
        m_monitor = true;
        m_monBytes = m_monOk = m_monBad = 0;
        m_monTypes.clear();
        m_monitorTimer->start();
        cliOut("monitor: BẬT - mỗi giây in số byte/khung nhận được (gõ monitor off để tắt).");
    } else if (w == "off") {
        m_monitor = false;
        m_monitorTimer->stop();
        cliOut("monitor: TẮT");
    } else {
        cliError("Cú pháp: monitor on|off");
    }
}

void DashboardManager::monitorTick()
{
    QStringList types;
    for (auto it = m_monTypes.cbegin(); it != m_monTypes.cend(); ++it)
        types << QString("%1×%2").arg(it.key()).arg(it.value());
    cliOut(QString("[MON] %1 byte/s, %2 khung đúng, %3 khung lỗi%4%5")
               .arg(m_monBytes).arg(m_monOk).arg(m_monBad)
               .arg(types.isEmpty() ? QString() : QStringLiteral(" | ") + types.join(' '))
               .arg(m_serialPort->isOpen() ? QString() : QStringLiteral(" (cổng chưa mở)")));
    m_monBytes = m_monOk = m_monBad = 0;
    m_monTypes.clear();
}

// ── Lệnh thô tới nguồn (msgid 43 -> 44) ────────────────────────────────────
void DashboardManager::sendRawSupply(uint8_t dev, const QString &text)
{
    const QString name = dev == PWR_DEV_AC ? QStringLiteral("AC") : QStringLiteral("DC");
    if (!m_serialPort->isOpen()) {
        cliError("Chưa kết nối board.");
        return;
    }
    const QByteArray t = text.toLatin1();
    if (t.size() > static_cast<int>(PWR_RAW_TEXT_MAX) - 1) {
        cliError(QString("Lệnh quá dài (tối đa %1 ký tự).").arg(PWR_RAW_TEXT_MAX - 1));
        return;
    }
    for (char c : t) {
        if (c < 0x20 || c > 0x7E) {
            cliError("Lệnh chỉ được chứa ký tự ASCII in được.");
            return;
        }
    }
    pwr_raw_msg_t m;
    std::memset(&m, 0, sizeof(m));
    std::strncpy(m.msg, "RAW", sizeof(m.msg));
    m.dev = dev;
    m.len = static_cast<uint8_t>(t.size());
    std::memcpy(m.text, t.constData(), static_cast<size_t>(t.size()));
    kria_message_t tx_msg;
    static uint8_t seq = 0;
    kria_msg_output_encode(seq++, 43, &m, &tx_msg);
    sendKriaFrame(tx_msg);
    appendCommandLog("TX", QString("%1 ← %2").arg(name, text));
}

void DashboardManager::handlePwrRawReply(const pwr_raw_reply_msg_t &r)
{
    const QString name = r.dev == PWR_DEV_AC ? QStringLiteral("AC") : QStringLiteral("DC");
    const int n = std::min<int>(r.len, PWR_RAW_REPLY_MAX);
    const QString text = QString::fromLatin1(r.text, n);
    switch (r.result) {
    case PWR_RESULT_OK:
    case PWR_RESULT_DEV_ERROR: {
        QString line = QString("%1 → %2").arg(name, text.isEmpty() ? QStringLiteral("(đã gửi)") : text);
        if (r.valid & PWR_VALID_ERR)
            line += r.dev_err == 0 ? QStringLiteral("  [SYST:ERR? = 0, không lỗi]")
                                   : QString("  [LỖI SCPI %1]").arg(r.dev_err);
        if ((r.valid & PWR_VALID_OUTPUT) && r.output <= 1) {
            line += QString("  [ngõ ra %1]").arg(r.output ? "ON" : "OFF");
            // Lệnh thô có thể đã bật/tắt ngõ ra -> công tắc theo trạng thái thật.
            if (r.dev == PWR_DEV_AC)
                m_acSource->setIsOn(r.output == 1);
        }
        appendCommandLog(r.result == PWR_RESULT_OK ? "RX" : "ERR", line);
        break;
    }
    case PWR_RESULT_NO_REPLY:
        cliError(QString("%1: nguồn không trả lời (lệnh ghi không có câu trả lời là bình thường với AC/DC; kiểm tra cú pháp/cáp).").arg(name));
        break;
    case PWR_RESULT_BUSY:
        cliError(QString("%1: nguồn đang bận (đang xác nhận lệnh/đọc) - thử lại sau 1-2 s.").arg(name));
        break;
    case PWR_RESULT_NOT_READY:
        cliError(QString("%1: UART nguồn trên board chưa khởi tạo.").arg(name));
        break;
    default:
        cliError(QString("%1: kết quả %2").arg(name).arg(r.result));
        break;
    }
}

// ── scan: tự tìm cổng COM + baud có board ───────────────────────────────────
void DashboardManager::startScan()
{
    if (m_scanning) {
        cliOut("scan đang chạy...");
        return;
    }
    if (m_serialPort->isOpen()) {
        cliOut("scan: ngắt phiên hiện tại để thử các cổng.");
        stopSession();
    }
    m_scanPorts.clear();
    // DASHBOARDENV_SCAN_PORTS="a;b": danh sách cổng cố định (máy test).
    const QString envPorts = qEnvironmentVariable("DASHBOARDENV_SCAN_PORTS");
    if (!envPorts.isEmpty())
        m_scanPorts = envPorts.split(';', Qt::SkipEmptyParts);
    else
        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
            m_scanPorts << info.portName();
    if (m_scanPorts.isEmpty()) {
        cliError("scan: không tìm thấy cổng COM nào trên máy.");
        return;
    }
    // 115200 là baud của firmware; các baud khác để phát hiện trường hợp
    // clock UART phía HDL/xparameters lệch làm baud thật của board khác đi.
    m_scanBauds = { 115200, 230400, 57600, 460800, 921600, 38400, 19200, 9600 };
    m_scanPortIdx = 0;
    m_scanBaudIdx = 0;
    m_scanNotes.clear();
    m_scanning = true;
    emit scanningChanged();
    cliOut(QString("scan: thử %1 cổng × %2 baud, gửi PING và chờ board trả lời...")
               .arg(m_scanPorts.size()).arg(m_scanBauds.size()));
    scanNext();
}

void DashboardManager::scanNext()
{
    if (!m_scanning)
        return;
    // Thứ tự: mọi cổng ở 115200 trước (trường hợp thường gặp -> tìm thấy
    // nhanh), rồi mới tới các baud khác. m_scanPortIdx chạy trong 1 baud.
    if (m_scanPortIdx >= m_scanPorts.size()) {
        m_scanPortIdx = 0;
        ++m_scanBaudIdx;
    }
    while (m_scanBaudIdx < m_scanBauds.size() && m_scanPortIdx < m_scanPorts.size()
           && m_scanPorts[m_scanPortIdx].isEmpty()) {      // cổng không mở được: bỏ qua
        if (++m_scanPortIdx >= m_scanPorts.size()) {
            m_scanPortIdx = 0;
            ++m_scanBaudIdx;
        }
    }
    if (m_scanBaudIdx >= m_scanBauds.size()) {
        scanStop(QStringLiteral("scan: KHÔNG tìm thấy board trên cổng nào.") +
                 (m_scanNotes.isEmpty() ? QString(" Không cổng nào có dữ liệu về -> kiểm tra board đã chạy firmware, dây RS422 (TX/RX, A/B), bộ chuyển USB-RS422.")
                                        : QString(" Có dữ liệu nhưng không phải khung hợp lệ ở: ") + m_scanNotes.join("; ")));
        return;
    }
    const QString port = m_scanPorts[m_scanPortIdx];
    const int baud = m_scanBauds[m_scanBaudIdx];

    delete m_scanPort;
    m_scanPort = new QSerialPort(this);
    m_scanPort->setPortName(port);
    m_scanPort->setBaudRate(baud);
    m_scanPort->setDataBits(QSerialPort::Data8);
    m_scanPort->setParity(QSerialPort::NoParity);
    m_scanPort->setStopBits(QSerialPort::OneStop);
    m_scanPort->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_scanPort->open(QIODevice::ReadWrite)) {
        cliOut(QString("  %1: không mở được (%2) - bỏ qua").arg(port, m_scanPort->errorString()));
        m_scanPorts[m_scanPortIdx].clear();       // đánh dấu bỏ qua ở các baud sau
        ++m_scanPortIdx;
        QTimer::singleShot(0, this, &DashboardManager::scanNext);
        return;
    }
    m_scanPort->setDataTerminalReady(true);
    m_scanPort->setRequestToSend(true);
    m_scanPort->clear();
    VIP_Decode_Init(&m_scanVipStatus);
    m_scanBytes = 0;
    connect(m_scanPort, &QSerialPort::readyRead, this, [this] {
        if (!m_scanPort)
            return;
        const QByteArray d = m_scanPort->readAll();
        m_scanBytes += static_cast<quint64>(d.size());
        for (char c : d) {
            if (VIP_Decode(&m_scanVipRx, &m_scanVipStatus, static_cast<uint8_t>(c))
                && m_scanVipRx.header.frameType == VIP_FRAME_ACK_STD
                && m_scanVipRx.header.srcId == VIP_NODE_ID_BOARD) {
                m_scanTimer->stop();
                scanFinishCandidate(true);
                return;
            }
        }
    });
    uint8_t frame[VIP_MAX_FRAME_LEN];
    const uint32_t len = VIP_Encode_Ping(VIP_NODE_ID_BOARD, VIP_NODE_ID_HOST, VIP_NextTxSeq(), frame, sizeof(frame));
    m_scanPort->write(reinterpret_cast<const char *>(frame), len);
    // 900 ms: board có thể đang bận tối đa ~0.5 s (chờ 1 lần chụp DMA).
    m_scanTimer->start(900);
}

void DashboardManager::scanFinishCandidate(bool found)
{
    if (!m_scanning || !m_scanPort)
        return;
    const QString port = m_scanPorts[m_scanPortIdx];
    const int baud = m_scanBauds[m_scanBaudIdx];
    const quint64 bytes = m_scanBytes;
    m_scanPort->close();
    m_scanPort->deleteLater();
    m_scanPort = nullptr;

    if (found) {
        setComPort(port);
        setBaudRate(baud);
        scanStop(QString("scan: TÌM THẤY board ở %1 @ %2 baud - đã chọn sẵn, bấm BẮT ĐẦU PHIÊN.%3")
                     .arg(port).arg(baud)
                     .arg(baud != 115200 ? QString(" CHÚ Ý: baud thật của board KHÁC 115200 -> clock UART RS422 trong HDL/xparameters đang lệch.") : QString()));
        return;
    }
    if (bytes > 0)
        m_scanNotes << QString("%1@%2 (%3 byte)").arg(port).arg(baud).arg(bytes);
    cliOut(QString("  %1 @ %2: không thấy board%3").arg(port).arg(baud)
               .arg(bytes > 0 ? QString(" (có %1 byte không hợp lệ)").arg(bytes) : QString()));
    ++m_scanPortIdx;
    QTimer::singleShot(30, this, &DashboardManager::scanNext);
}

void DashboardManager::scanStop(const QString &summary)
{
    m_scanning = false;
    m_scanTimer->stop();
    if (m_scanPort) {
        m_scanPort->close();
        m_scanPort->deleteLater();
        m_scanPort = nullptr;
    }
    emit scanningChanged();
    if (summary.contains("TÌM THẤY")) {
        appendCommandLog("INFO", summary);
        emit systemNotification("Success", summary);
    } else {
        cliError(summary);
        emit systemNotification("Error", summary);
    }
}
