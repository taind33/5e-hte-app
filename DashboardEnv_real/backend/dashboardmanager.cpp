#include "dashboardmanager.h"
#include "kria_helper.h"
#include "stdlib.h"
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <limits>
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace {
constexpr int STREAM_MAGIC_LEN = 4;

QString gainModeName(uint8_t mode)
{
    switch (mode) {
    case 0: return QStringLiteral("MGC");
    case 1: return QStringLiteral("AGC nhanh");
    case 2: return QStringLiteral("AGC chậm");
    case 3: return QStringLiteral("AGC lai");
    default: return QStringLiteral("?");
    }
}

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

// "V/A/Hz" có số lẻ hợp lý cho dòng log Terminal.
QString fmtNum(double v, int decimals) { return QString::number(v, 'f', decimals); }
} // namespace

// chưa sửa datatype cho AC DC
DashboardManager::DashboardManager(QObject *parent)
    : QObject(parent),
    m_acSource(new ACsource(this)),
    m_dcSource(new DCsource(this)),
    m_rfController(new RfController(this)),
    m_serialPort(new QSerialPort(this)),
    m_simTimer(new QTimer(this)),
    // Giá trị khởi tạo mặc định ("chưa từng chạy lần nào") - bị ghi đè ngay
    // dưới đây bằng QSettings nếu đã có phiên trước. Đây CHỈ là nạp giá trị
    // hiển thị lên UI, KHÔNG gửi lệnh gì xuống thiết bị (cổng còn chưa mở) -
    // đúng yêu cầu "khi init app giá trị không động vào [phần cứng]".
    // Chạy lần đầu (chưa có config.ini): mọi giá trị người dùng chọn/nhập đều
    // "chưa đặt" -> giao diện hiện "--" (yêu cầu: không tự điền mặc định).
    m_comPort(), m_baudRate(0), m_delayTime(-1),
    m_sessionName(),
    m_logFile(nullptr),
    m_logStream(nullptr),
    m_isPortVerified(false),
    m_pingTimer(new QTimer(this)),
    m_pingTimeoutTimer(new QTimer(this)),
    m_rxBytes(0), m_rxFramesOk(0), m_rxFramesBad(0),
    m_linkDiagTimer(new QTimer(this)),
    m_unverifiedSeconds(0),
    m_haveDiag(false),
    m_notifiedAd9361Fail(false), m_notifiedDmaDead(false), m_notifiedClip(false),
    m_cliPingPending(false), m_resetArmed(false),
    m_monitor(false), m_monitorTimer(new QTimer(this)),
    m_monBytes(0), m_monOk(0), m_monBad(0),
    m_scanning(false), m_scanPort(nullptr), m_scanTimer(new QTimer(this)),
    m_scanPortIdx(0), m_scanBaudIdx(0), m_scanBytes(0),
    m_kichNoPeakV(0.0),
    m_stagedAcVolt(kNaN),
    m_stagedAcFreq(kNaN),
    m_stagedDcVolt(kNaN),
    m_stagedDcCurrent(kNaN),
    m_waveWatchdog(new QTimer(this)),
    m_waveSeenThisSession(false),
    m_rfStatusValid(false),
    m_rxLoHz(0), m_rxFsHz(0), m_rxGainDb(0),
    m_lastTotalDbfs(0.0),
    m_lastSpectrumLive(false),
    m_calibrated(false), m_calOffsetDb(0.0), m_calLoHz(0), m_calGainDb(0),
    m_simSpectrumTick(0),
    m_acPendingOn(false), m_acPrevOn(false),
    m_dcPendingOn(false), m_dcPrevOn(false),
    m_acGotPws(false), m_dcGotPws(false),
    m_acWaitKind(0), m_dcWaitKind(0),
    m_acPowerTimer(new QTimer(this)),
    m_dcPowerTimer(new QTimer(this)),
    m_configPath(resolveConfigPath()),
    m_settings(m_configPath, QSettings::IniFormat)
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &DashboardManager::handleSerialRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &DashboardManager::handleSerialError);
    connect(m_rfController,&RfController::attUpdateRequested, this ,&DashboardManager::onAttUpdateRequested );
    connect(m_simTimer, &QTimer::timeout, this, &DashboardManager::generateSimulatedWave);
    m_simTimer->start(8); // ~60Hz

    connect(m_pingTimer, &QTimer::timeout, this, &DashboardManager::sendPingProbe);
    m_pingTimer->setInterval(PING_INTERVAL_MS);
    m_pingTimeoutTimer->setSingleShot(true);
    m_pingTimeoutTimer->setInterval(PING_TIMEOUT_MS);
    connect(m_pingTimeoutTimer, &QTimer::timeout, this, &DashboardManager::onPingTimeout);

    m_waveWatchdog->setSingleShot(true);
    m_waveWatchdog->setInterval(WAVE_WATCHDOG_MS);
    connect(m_waveWatchdog, &QTimer::timeout, this, &DashboardManager::onWaveWatchdog);

    std::memset(&m_lastDiag, 0, sizeof(m_lastDiag));
    std::memset(&m_scanVipRx, 0, sizeof(m_scanVipRx));
    VIP_Decode_Init(&m_scanVipStatus);
    m_monitorTimer->setInterval(1000);
    connect(m_monitorTimer, &QTimer::timeout, this, &DashboardManager::monitorTick);
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout, this, [this] { scanFinishCandidate(false); });
    m_linkDiagTimer->setInterval(1000);
    connect(m_linkDiagTimer, &QTimer::timeout, this, [this] {
        m_unverifiedSeconds = m_isPortVerified ? 0 : m_unverifiedSeconds + 1;
        // Thông báo sau ~4 s không xác thực được (đủ 2 chu kỳ PING) - 1 lần
        // cho mỗi loại nguyên nhân trong phiên.
        updateLinkDiagnosis(m_unverifiedSeconds >= 4);
    });

    m_acPowerTimer->setSingleShot(true);
    m_acPowerTimer->setInterval(POWER_REPLY_TIMEOUT_MS);
    connect(m_acPowerTimer, &QTimer::timeout, this, &DashboardManager::onAcPowerTimeout);
    m_dcPowerTimer->setSingleShot(true);
    m_dcPowerTimer->setInterval(POWER_REPLY_TIMEOUT_MS);
    connect(m_dcPowerTimer, &QTimer::timeout, this, &DashboardManager::onDcPowerTimeout);

    VIP_Decode_Init(&m_vipStatus);
    refreshPorts();

    // Nạp lại cấu hình lần làm việc trước từ config.ini (xem
    // resolveConfigPath()). Chỉ gán thẳng vào biến thành viên/thuộc tính
    // hiển thị - KHÔNG mở cổng, KHÔNG gửi lệnh nào xuống thiết bị (invariant
    // "init không được động vào phần cứng"). Khoá chưa có -> "chưa đặt" ("--").
    migrateOldSettings();
    m_comPort     = m_settings.value("connection/comPort", QString()).toString();
    m_baudRate    = m_settings.value("connection/baudRate", 0).toInt();
    m_sessionName = m_settings.value("connection/sessionName", QString()).toString();
    m_delayTime   = m_settings.value("delay/ns", -1).toInt();

    // ATT: gán thẳng qua setAttN() (chỉ cập nhật thuộc tính hiển thị trên
    // RfController), TUYỆT ĐỐI không gọi requestSetAtt() ở đây - đó mới là
    // đường phát lệnh xuống dây. Chưa lưu -> NaN -> ô hiện "--".
    m_rfController->setAtt1(loadOptionalDouble(m_settings, "att/ch1", 0.0, attMax()));
    m_rfController->setAtt2(loadOptionalDouble(m_settings, "att/ch2", 0.0, attMax()));
    m_rfController->setAtt3(loadOptionalDouble(m_settings, "att/ch3", 0.0, attMax()));

    // Staged Power UI - kẹp lại theo giới hạn thật phòng khi .ini cũ còn giá
    // trị ngoài khoảng. Chuỗi rỗng/thiếu khoá -> NaN ("--").
    m_stagedAcVolt    = loadOptionalDouble(m_settings, "power/acVolt",    acVoltMin(),    acVoltMax());
    m_stagedAcFreq    = loadOptionalDouble(m_settings, "power/acFreq",    acFreqMin(),    acFreqMax());
    m_stagedDcVolt    = loadOptionalDouble(m_settings, "power/dcVolt",    dcVoltMin(),    dcVoltMax());
    m_stagedDcCurrent = loadOptionalDouble(m_settings, "power/dcCurrent", dcCurrentMin(), dcCurrentMax());

    // Cổng COM đã lưu nhưng hiện không cắm: báo (sau khi QML đã kết nối
    // signal), KHÔNG tự chọn cổng khác.
    if (!m_comPort.isEmpty() && !isPortAvailable(m_comPort)) {
        const QString missing = m_comPort;
        QTimer::singleShot(0, this, [this, missing] {
            emit systemNotification("Warning", QString("Cổng %1 đã lưu lần trước hiện không có - hãy chọn lại cổng COM.").arg(missing));
        });
    }
    QTimer::singleShot(0, this, [this] {
        appendCommandLog("INFO", "Cấu hình: " + QDir::toNativeSeparators(m_configPath));
    });

    // Hiệu chuẩn phổ RF (dBFS -> dBm) đã lưu từ lần trước, nếu có.
    m_calibrated = m_settings.contains("rf/calOffsetDb");
    m_calOffsetDb = m_settings.value("rf/calOffsetDb", 0.0).toDouble();
    m_calLoHz     = m_settings.value("rf/calLoHz", 0).toULongLong();
    m_calGainDb   = m_settings.value("rf/calGainDb", 0).toInt();
    updateCalibrationState();
}

DashboardManager::~DashboardManager() {
    stopSession();
}



void DashboardManager::setComPort(const QString &port){
    if (m_comPort != port) {
        m_comPort = port;
        saveSetting("connection/comPort", m_comPort);
        emit comPortChanged();
    }
}
void DashboardManager::setBaudRate(int rate) {
    if (m_baudRate != rate) {
        m_baudRate = rate;
        saveSetting("connection/baudRate", m_baudRate);
        emit baudRateChanged();
    }
}
void DashboardManager::setDelayTime(int time) {
    if (m_delayTime != time) {
        m_delayTime = time;
        // Chỉ lưu lại lựa chọn hiển thị - việc GỬI lệnh xuống board (nếu có)
        // là trách nhiệm của applyDelayTime(), hàm gọi setDelayTime() này.
        saveSetting("delay/ns", m_delayTime);
        emit delayTimeChanged();
    }
}
void DashboardManager::setSessionName(const QString &sessionName){
    if(m_sessionName!= sessionName){
        m_sessionName = sessionName;
        saveSetting("connection/sessionName", m_sessionName);
        emit sessionNameChanged();
    }
}

// Staged Power UI setters - CHỈ lưu giá trị đang soạn trên form, không tự ý
// gửi lệnh xuống nguồn AC/DC. NaN = người dùng xoá trắng ô ("--"), lưu vào
// .ini là chuỗi rỗng. Giá trị số được kẹp theo giới hạn thật.
namespace {
bool sameOptional(double a, double b)
{
    if (std::isnan(a) || std::isnan(b))
        return std::isnan(a) && std::isnan(b);
    return qFuzzyCompare(a + 1.0, b + 1.0);
}
QVariant optionalToVariant(double v)
{
    return std::isnan(v) ? QVariant(QString()) : QVariant(v);
}
} // namespace

void DashboardManager::setStagedAcVolt(double v){
    if (!std::isnan(v)) v = qBound<double>(acVoltMin(), std::round(v), acVoltMax());
    if (!sameOptional(m_stagedAcVolt, v)) {
        m_stagedAcVolt = v;
        saveSetting("power/acVolt", optionalToVariant(v));
        emit stagedAcVoltChanged();
    }
}

void DashboardManager::setStagedAcFreq(double f){
    if (!std::isnan(f)) f = qBound<double>(acFreqMin(), std::round(f), acFreqMax());
    if (!sameOptional(m_stagedAcFreq, f)) {
        m_stagedAcFreq = f;
        saveSetting("power/acFreq", optionalToVariant(f));
        emit stagedAcFreqChanged();
    }
}

void DashboardManager::setStagedDcVolt(double v){
    if (!std::isnan(v)) v = qBound<double>(dcVoltMin(), std::round(v), dcVoltMax());
    if (!sameOptional(m_stagedDcVolt, v)) {
        m_stagedDcVolt = v;
        saveSetting("power/dcVolt", optionalToVariant(v));
        emit stagedDcVoltChanged();
    }
}

void DashboardManager::setStagedDcCurrent(double c){
    if (!std::isnan(c)) c = qBound(dcCurrentMin(), std::round(c * 100.0) / 100.0, dcCurrentMax());
    if (!sameOptional(m_stagedDcCurrent, c)) {
        m_stagedDcCurrent = c;
        saveSetting("power/dcCurrent", optionalToVariant(c));
        emit stagedDcCurrentChanged();
    }
}


void DashboardManager::startSession() {
    stopSession();

    if (m_comPort.isEmpty()) {
        emit systemNotification("Warning", "Chưa chọn cổng COM.");
        return;
    }
    if (m_baudRate <= 0) {
        emit systemNotification("Warning", "Chưa chọn tốc độ baud (board 5E-HTE dùng 115200).");
        return;
    }

    m_serialPort->setPortName(m_comPort);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit systemNotification("Error", "Không thể mở cổng " + m_comPort + " -  " + m_serialPort->errorString());
        emit isConnectedChanged();
        return;
    }
    // Bật DTR + RTS ngay sau khi mở cổng, giống các phần mềm terminal
    // (TeraTerm, Hercules...). QSerialPort trên Windows mặc định KHÔNG bật -
    // trong khi nhiều bộ chuyển USB/RS232 -> RS422/485 lấy nguồn từ DTR/RTS
    // hoặc dùng RTS để bật bộ phát: khi đó cổng mở được nhưng không có byte
    // nào đi/về (đèn cam). Firmware cũ từng chạy qua terminal nên không lộ.
    {
        const bool dtr = m_serialPort->setDataTerminalReady(true);
        const bool rts = m_serialPort->setRequestToSend(true);
        writeToLog(QString("[LINK] DTR %1, RTS %2").arg(dtr ? "ON" : "lỗi", rts ? "ON" : "lỗi"));
    }
    emit isConnectedChanged();
    emit systemNotification("Success",QString("Đã kết nối thành công cổng %1 @ %2 baud").arg(m_comPort).arg(m_baudRate));

    // Bỏ phổ mô phỏng đang hiển thị - từ giờ chỉ vẽ phổ THẬT từ board (gói
    // "WAVE"); nếu sau WAVE_WATCHDOG_MS không có gì, giao diện báo rõ "chưa
    // nhận phổ" thay vì để nguyên đường mô phỏng cũ trông như dữ liệu thật.
    resetRfState();
    m_rfController->setWaveSource(QStringLiteral("waiting"));
    m_waveWatchdog->start();

    // 4 biểu đồ AC/DC chỉ vẽ số ĐO đọc ngược từ nguồn (msgid 42) - phiên mới
    // bắt đầu trống ("--") cho tới khi bấm refresh/F5 hoặc gửi lệnh đặt.
    m_acSource->resetHistory();
    m_dcSource->resetHistory();
    m_acSource->setLink("none", QString());
    m_dcSource->setLink("none", QString());

    // Mở cổng thành công KHÔNG có nghĩa đây đúng là board 5E-HTE - nhiều cổng
    // COM khác (nguồn AC/DC qua cùng hub USB) cũng mở được y hệt. Bắt đầu
    // chu kỳ PING để tự xác nhận qua giao thức VIP (đèn "isPortVerified" trên
    // UI) - xem sendPingProbe()/onPingTimeout().
    m_isPortVerified = false;
    emit isPortVerifiedChanged();
    m_rxBytes = m_rxFramesOk = m_rxFramesBad = 0;
    m_unverifiedSeconds = 0;
    m_linkDiagNotified.clear();
    m_haveDiag = false;
    m_notifiedAd9361Fail = m_notifiedDmaDead = m_notifiedClip = false;
    m_boardFirmware.clear();
    m_boardDiag.clear();
    emit boardDiagChanged();
    m_rfController->setDiag(QString(), QString());
    updateLinkDiagnosis(false);
    m_linkDiagTimer->start();
    startPingCycle();

    QString logsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                      + "/DashboardLogs";
    QDir().mkpath(logsDir);

    QString safeName = m_sessionName.trimmed().isEmpty() ? QStringLiteral("phien") : m_sessionName.trimmed();
    QString timestamp = QDateTime ::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString logPath = QString ("%1/%2_%3.log").arg(logsDir,safeName,timestamp);
    m_logFile = new QFile(logPath, this);

    if(m_logFile->open(QIODevice::WriteOnly|QIODevice::Text)){
        m_logStream = new QTextStream(m_logFile);
        writeToLog(QString("[SESSION START] %1 | Port:%2 | Baud:%3")
                       .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                       .arg(m_comPort).arg(m_baudRate));
    }else{
        emit systemNotification("Warning",
                                QString("Không thể tạo file log tại: %1").arg(logPath));
        delete m_logFile;
        m_logFile   = nullptr;
        m_logStream = nullptr;
    }
}

void DashboardManager::stopSession(){
    stopPingCycle();
    m_linkDiagTimer->stop();
    m_waveWatchdog->stop();
    m_acPowerTimer->stop();
    m_dcPowerTimer->stop();
    m_acWaitKind = m_dcWaitKind = 0;
    if (m_serialPort->isOpen()) {
        // Số cũ vẫn giữ trên đồ thị nhưng đánh dấu không còn kết nối.
        m_acSource->setLink("offline", "Đã ngắt kết nối board");
        m_dcSource->setLink("offline", "Đã ngắt kết nối board");
    }
    if (m_isPortVerified) { m_isPortVerified = false; emit isPortVerifiedChanged(); }

    if (m_serialPort->isOpen()) {
        writeToLog("[SESSION END] " +
                   QDateTime::currentDateTime().toString(Qt::ISODate));
        m_serialPort->close();
        emit isConnectedChanged();
        emit systemNotification("Info", "Đã ngắt kết nối cổng " + m_comPort);
        resetRfState();
        m_rfController->setWaveSource(QStringLiteral("sim"));
        m_rfController->setDiag(QString(), QString());
        updateLinkDiagnosis(false);
    }

    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void DashboardManager::openLogFile()
{
    if (m_logFile && m_logFile->exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_logFile->fileName()));
    } else {
        // Không có file log hiện tại, mở thư mục chứa log
        QString logsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          + "/DashboardLogs";
        QDesktopServices::openUrl(QUrl::fromLocalFile(logsDir));
        emit systemNotification("Info", "Mở thư mục log: " + logsDir);
    }
}

void DashboardManager::resetSystem() {
    // "Đặt lại" = đưa hệ thống về trạng thái an toàn (nút có hộp xác nhận
    // phía QML). Cũng là lệnh reset đỉnh kích nổ bắt được - đúng ý "Khi đặt
    // lại sẽ reset điện áp đọc được" đã xác nhận với người dùng.
    if (m_kichNoPeakV != 0.0) {
        m_kichNoPeakV = 0.0;
        emit kichNoPeakVoltageChanged();
    }
    m_acSource->resetHistory();
    m_dcSource->resetHistory();

    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", "Chưa kết nối board - chỉ xoá số liệu trên màn hình, chưa gửi lệnh đặt lại.");
        return;
    }

    // Gửi "RESET" qua kênh generic/diag (msgid 39). Firmware đưa mọi đầu ra
    // về an toàn: AC/DC OUTPUT OFF (mỗi nguồn trả 1 msgid 42 xác nhận trạng
    // thái thật), 3 ATT = 0 dB, delay quang = 0 ns - KHÔNG khởi động lại board.
    cmd_msg_t reset_cmd;
    std::memset(&reset_cmd, 0, sizeof(cmd_msg_t));
    std::strncpy(reset_cmd.msg, "RESET", sizeof(reset_cmd.msg));
    reset_cmd.value[0] = 0xFF;

    kria_message_t tx_msg;
    static uint8_t reset_seq = 0;
    kria_msg_output_encode(reset_seq++, 39, &reset_cmd, &tx_msg);
    sendKriaFrame(tx_msg);

    m_acWaitKind = 3;
    m_dcWaitKind = 3;
    m_acSource->setLink("pending", "Đang đặt lại (tắt ngõ ra)…");
    m_dcSource->setLink("pending", "Đang đặt lại (tắt ngõ ra)…");
    m_acPowerTimer->start();
    m_dcPowerTimer->start();

    appendCommandLog("TX", "RESET (AC/DC OFF, ATT 0 dB, delay 0 ns)");
    writeToLog("[RESET] system reset requested");
    emit systemNotification("Warning", "Đã gửi lệnh đặt lại toàn bộ hệ thống.");
}

void DashboardManager::sendKriaFrame(const kria_message_t &msg)
{
    if (!m_serialPort->isOpen()) {
        return;
    }

    uint8_t inner[3U + KRIA_MAX_PAYLOAD_LEN];
    uint16_t inner_len = kria_msg_to_send_buffer(inner, &msg);

    uint8_t frame[VIP_MAX_FRAME_LEN];
    uint32_t frame_len = VIP_Encode_Data(VIP_NODE_ID_BOARD /* destination: the board */,
                                         VIP_NODE_ID_HOST /* source: this app */,
                                         VIP_NextTxSeq(), inner, inner_len, frame, sizeof(frame));
    if (frame_len > 0U) {
        m_serialPort->write(reinterpret_cast<const char*>(frame), frame_len);
    }
}

void DashboardManager::startPingCycle()
{
    sendPingProbe();          // gửi ngay 1 phát, không đợi hết interval đầu tiên
    m_pingTimer->start();
}

void DashboardManager::stopPingCycle()
{
    m_pingTimer->stop();
    m_pingTimeoutTimer->stop();
}

void DashboardManager::sendPingProbe()
{
    if (!m_serialPort->isOpen()) { stopPingCycle(); return; }

    uint8_t frame[VIP_MAX_FRAME_LEN];
    uint32_t frame_len = VIP_Encode_Ping(VIP_NODE_ID_BOARD, VIP_NODE_ID_HOST,
                                         VIP_NextTxSeq(), frame, sizeof(frame));
    if (frame_len > 0U) {
        m_serialPort->write(reinterpret_cast<const char*>(frame), frame_len);
    }
    // Nếu không nhận được ACK_STD trong PING_TIMEOUT_MS, coi như chưa/không
    // còn xác thực được (đèn tắt) - parseIncomingData() sẽ dừng timer này lại
    // nếu ACK tới kịp trước khi nó bắn.
    m_pingTimeoutTimer->start();
}

void DashboardManager::onPingTimeout()
{
    if (m_isPortVerified) {
        m_isPortVerified = false;
        emit isPortVerifiedChanged();
        writeToLog("[PING] Mất xác thực - không còn nhận được phản hồi từ board.");
    }
    updateLinkDiagnosis(false);
}

// ─────────────────────────────────────────────────────────────────────────
// Chẩn đoán đường truyền + gói DIAG/LOGT của board
// ─────────────────────────────────────────────────────────────────────────
void DashboardManager::updateLinkDiagnosis(bool notify)
{
    QString kind, text, shortText;
    if (!m_serialPort->isOpen()) {
        kind = "closed";
        text = QStringLiteral("Chưa kết nối");
    } else if (m_isPortVerified) {
        kind = "ok";
        text = QString("Đã xác thực đúng board (%1 byte, %2 khung nhận)").arg(m_rxBytes).arg(m_rxFramesOk);
    } else if (m_rxBytes == 0) {
        kind = "nobytes";
        shortText = QStringLiteral("Không có byte về");
        text = QString("Cổng %1 đã mở nhưng KHÔNG nhận được byte nào từ board. Kiểm tra: board đã chạy firmware "
                       "(nếu chạy Debug trong Vitis phải bấm Resume), đúng cổng COM của RS422, dây RS422 (TX/RX, A/B), "
                       "nguồn board. Gõ 'scan' trong Terminal để tự tìm cổng/baud có board.").arg(m_comPort);
    } else if (m_rxFramesOk == 0) {
        kind = "garbage";
        shortText = QStringLiteral("Sai khung (baud/A-B?)");
        text = QString("Có nhận %1 byte nhưng không giải mã được khung nào").arg(m_rxBytes);
        if (m_rxFramesBad > 0)
            text += QString(" (%1 khung sai CRC)").arg(m_rxFramesBad);
        text += (m_baudRate != 115200)
            ? QString(". Thường do SAI BAUD: board dùng 115200, app đang chọn %1.").arg(m_baudRate)
            : QStringLiteral(". Baud app đã là 115200 -> kiểm tra đảo dây A/B, hoặc baud phía board (clock UART RS422 trong HDL/xparameters).");
    } else {
        kind = "noping";
        shortText = QStringLiteral("Không trả PING");
        text = QString("Board đang gửi dữ liệu (%1 khung) nhưng không trả lời PING của app: hướng PC → board có vấn đề "
                       "(dây TX của bộ chuyển USB-RS422 / A-B), hoặc board đang bận.").arg(m_rxFramesOk);
        if (m_haveDiag)
            text += QString(" Board báo đã nhận %1 byte, %2 khung đúng, %3 khung lỗi từ PC.")
                        .arg(m_lastDiag.pc_rx_bytes).arg(m_lastDiag.pc_frames_ok).arg(m_lastDiag.pc_frames_bad);
    }
    if (m_linkDiagnosis != text || m_linkDiagnosisShort != shortText) {
        m_linkDiagnosis = text;
        m_linkDiagnosisShort = shortText;
        emit linkDiagnosisChanged();
    }
    if (notify && kind != "ok" && kind != "closed" && m_linkDiagNotified != kind) {
        m_linkDiagNotified = kind;
        appendCommandLog("ERR", text);
        writeToLog("[LINK] " + text);
        emit systemNotification("Error", text);
    }
}

void DashboardManager::handleBoardLog(uint8_t level, const QString &text)
{
    const QString tag = level == BOARD_LOG_ERROR ? QStringLiteral("BOARD-LỖI")
                      : level == BOARD_LOG_WARN  ? QStringLiteral("BOARD-CẢNH BÁO")
                                                 : QStringLiteral("BOARD");
    appendCommandLog(tag, text);
    writeToLog(QString("[%1] %2").arg(tag, text));
    if (level == BOARD_LOG_ERROR)
        emit systemNotification("Error", "Board: " + text);
}

void DashboardManager::handleBoardDiag(const board_diag_msg_t &d)
{
    const board_diag_msg_t prev = m_lastDiag;
    const bool hadDiag = m_haveDiag;
    m_lastDiag = d;
    m_haveDiag = true;

    char build[sizeof(d.build) + 1];
    std::memcpy(build, d.build, sizeof(d.build));
    build[sizeof(d.build)] = '\0';
    const QString fw = QString::fromLatin1(build).simplified();
    if (fw != m_boardFirmware) {
        m_boardFirmware = fw;
        appendCommandLog("INFO", "Firmware trên board: build " + fw);
    }

    QVariantMap m;
    m["ad9361Ok"] = d.ad9361_ok != 0;
    m["ad9361Status"] = d.ad9361_status;
    m["rfStreamOk"] = d.rf_stream_ok != 0;
    m["adc7928Ok"] = d.adc7928_ok != 0;
    m["attOk"] = d.att_ok != 0;
    m["delayOk"] = d.odly_ok != 0;
    m["acUartOk"] = d.ac_uart_ok != 0;
    m["dcUartOk"] = d.dc_uart_ok != 0;
    m["uptimeS"] = static_cast<qulonglong>(d.uptime_s);
    m["dmaOk"] = static_cast<qulonglong>(d.dma_ok);
    m["dmaTimeout"] = static_cast<qulonglong>(d.dma_timeout);
    m["dmaStartFail"] = static_cast<qulonglong>(d.dma_start_fail);
    m["rx1Peak"] = d.rx1_peak;
    m["pcRxBytes"] = static_cast<qulonglong>(d.pc_rx_bytes);
    m["pcFramesOk"] = static_cast<qulonglong>(d.pc_frames_ok);
    m["pcFramesBad"] = static_cast<qulonglong>(d.pc_frames_bad);
    m["pcRxDropped"] = static_cast<qulonglong>(d.pc_rx_dropped);
    m["firmware"] = fw;
    m_boardDiag = m;
    emit boardDiagChanged();

    // Thông báo các tình trạng cần người vận hành xử lý - 1 lần mỗi phiên.
    if (!d.ad9361_ok && !m_notifiedAd9361Fail) {
        m_notifiedAd9361Fail = true;
        emit systemNotification("Error", QString("AD9361 khởi tạo lỗi (mã %1) - không có phổ RF. AC/DC/ATT/Delay vẫn dùng được.").arg(d.ad9361_status));
    }
    const bool dmaStuck = d.rf_stream_ok && d.dma_timeout > 0 && hadDiag
                          && d.dma_ok == prev.dma_ok && d.dma_timeout > prev.dma_timeout;
    if (dmaStuck && !m_notifiedDmaDead) {
        m_notifiedDmaDead = true;
        emit systemNotification("Error", QString("RX DMA không về dữ liệu (đã timeout %1 lần) - AD9361 đã khởi tạo nhưng luồng mẫu ADC → FPGA → DMA không chạy.").arg(d.dma_timeout));
    }
    if (d.rf_stream_ok && d.rx1_peak >= 2040 && !m_notifiedClip) {
        m_notifiedClip = true;
        emit systemNotification("Warning", QString("ADC RX1 bão hoà (đỉnh %1/2047) - giảm công suất vào hoặc tăng suy hao; phổ và số dBm sẽ sai.").arg(d.rx1_peak));
    } else if (d.rx1_peak < 1800) {
        m_notifiedClip = false;     // hết bão hoà -> lần sau lại báo
    }
    updateRfDiagnosis();
    if (!m_isPortVerified)
        updateLinkDiagnosis(false);
}

void DashboardManager::updateRfDiagnosis()
{
    if (!m_haveDiag) {
        m_rfController->setDiag(QString(), QString());
        return;
    }
    const board_diag_msg_t &d = m_lastDiag;
    if (!d.ad9361_ok) {
        m_rfController->setDiag(QString("AD9361 khởi tạo lỗi (mã %1) - phổ RF đang tắt.\nXem dòng BOARD-LỖI trong Terminal.").arg(d.ad9361_status), "error");
    } else if (!d.rf_stream_ok) {
        m_rfController->setDiag(QStringLiteral("RX DMA (stream_framer) không khởi tạo được - phổ RF đang tắt."), "error");
    } else if (d.dma_ok == 0 && (d.dma_timeout > 0 || d.dma_start_fail > 0)) {
        m_rfController->setDiag(QString("RX DMA không về dữ liệu: %1 lần timeout, %2 lần lỗi khởi động.\n"
                                        "AD9361 đã khởi tạo; kiểm tra HDL: clock/data AD9361 → axi_ad9361 → DMA.")
                                    .arg(d.dma_timeout).arg(d.dma_start_fail), "error");
    } else if (d.rx1_peak >= 2040) {
        m_rfController->setDiag(QString("ADC RX1 bão hoà (đỉnh %1/2047) - giảm công suất vào / tăng suy hao.").arg(d.rx1_peak), "warn");
    } else if (d.dma_timeout > 0 && d.dma_ok > 0) {
        m_rfController->setDiag(QString("DMA đôi lúc timeout (%1/%2 lần chụp).").arg(d.dma_timeout).arg(d.dma_ok + d.dma_timeout), "warn");
    } else {
        m_rfController->setDiag(QString(), QString());
    }
}

void DashboardManager::sendCommand(const QString &cmd) {
        QString trimmed = cmd.trimmed();
        if(trimmed.isEmpty()) return;

        appendCommandLog("TX", trimmed);
        writeToLog("[CMD TX] " + trimmed);

        if (!m_serialPort->isOpen()) {
            emit systemNotification("Error", "Lệnh thất bại: Cổng kết nối chưa mở!");
            return;
        }
        QStringList parts = trimmed.split(',');
        for(int i = 0; i < parts.size(); ++i) {
            parts[i] = parts[i].trimmed();
        }
        if(parts.isEmpty()) return;

        kria_message_t tx_msg;
        static uint8_t local_seq = 0;
        uint8_t msg_id = 39;

        const void* p_data = nullptr;

        cmd_msg_t generic_cmd_data;

            std::memset(&generic_cmd_data, 0, sizeof(cmd_msg_t));
            msg_id = 39;

            // Giả sử cú pháp nhập lệnh bất kỳ là: "Tên_Lệnh, val1, val2, val3"
            // Ví dụ: "SET_GAIN, 12, 13"
            std::strncpy(generic_cmd_data.msg, parts[0].toUtf8().constData(), sizeof(generic_cmd_data.msg));

            int value_count = 0;
            for (int i = 1; i < parts.size() && value_count < 3; ++i) {
                bool ok;
                uint8_t val = static_cast<uint8_t>(parts[i].toUInt(&ok));
                if (ok) {
                    generic_cmd_data.value[value_count++] = val;
                }
            }

            // KÝ TỰ BÁO HIỆU KẾT THÚC VALUE:
            // Gán phần tử tiếp theo bằng 0xFF để Kria biết phần value có ích đã dừng lại ở đây
            generic_cmd_data.value[value_count] = 0xFF;

            p_data = &generic_cmd_data;

        if (p_data != nullptr) {
            kria_msg_output_encode(local_seq++, msg_id, p_data, &tx_msg);
            sendKriaFrame(tx_msg);
        }
}


bool DashboardManager::requestAcPower(bool on)
{
    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", "Chưa kết nối board - lệnh nguồn AC chưa được gửi.");
        return false;
    }
    if (on && (std::isnan(m_stagedAcVolt) || std::isnan(m_stagedAcFreq))) {
        QStringList missing;
        if (std::isnan(m_stagedAcVolt)) missing << "điện áp";
        if (std::isnan(m_stagedAcFreq)) missing << "tần số";
        emit systemNotification("Warning", "Chưa nhập " + missing.join(" và ") + " AC - chưa bật nguồn.");
        return false;
    }
    // TẮT không cần giá trị: firmware chỉ gửi OUTP OFF cho lệnh tắt.
    setAcPower(on,
               on ? static_cast<uint16_t>(std::lround(m_stagedAcVolt)) : 0,
               on ? static_cast<uint16_t>(std::lround(m_stagedAcFreq)) : 0);
    return true;
}

bool DashboardManager::requestDcPower(bool on)
{
    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", "Chưa kết nối board - lệnh nguồn DC chưa được gửi.");
        return false;
    }
    if (on && (std::isnan(m_stagedDcVolt) || std::isnan(m_stagedDcCurrent))) {
        QStringList missing;
        if (std::isnan(m_stagedDcVolt)) missing << "điện áp";
        if (std::isnan(m_stagedDcCurrent)) missing << "dòng";
        emit systemNotification("Warning", "Chưa nhập " + missing.join(" và ") + " DC - chưa bật nguồn.");
        return false;
    }
    setDcPower(on,
               on ? static_cast<uint16_t>(std::lround(m_stagedDcVolt)) : 0,
               on ? m_stagedDcCurrent : 0.0);
    return true;
}

void DashboardManager::setAcPower(bool on, uint16_t volt, uint16_t freq ){
    // Nhớ trạng thái trước + trạng thái yêu cầu. Công tắc hiện trạng thái
    // YÊU CẦU trong lúc chờ (link "pending"); kết quả cuối cùng theo msgid 42
    // (OUTP? của nguồn) - xem handlePwrStatus().
    if (m_acWaitKind != 2)
        m_acPrevOn = m_acSource->isOn();
    m_acPendingOn = on;
    m_acGotPws = false;
    m_acSource->setIsOn(on);
    fwi_msg_t ac_packet;
    std::memset(&ac_packet, 0, sizeof(fwi_msg_t));
    std ::strncpy (ac_packet.msg, "FWI", sizeof(ac_packet.msg));
    std :: strncpy(ac_packet.isAC,"AC", sizeof(ac_packet.isAC));
    ac_packet.voltage = static_cast<uint16_t> (volt);
    ac_packet.frequency = static_cast<uint16_t> (freq);
    ac_packet.state = on ? 1:0;
    kria_message_t tx_msg ;
    static uint8_t local_seq =0;
    kria_msg_output_encode(local_seq++, 35, &ac_packet, &tx_msg);
    sendKriaFrame(tx_msg);

    if( m_serialPort->isOpen()){
        m_acWaitKind = 2;
        m_acSource->setLink("pending", on ? "Đang bật, chờ nguồn xác nhận…" : "Đang tắt, chờ nguồn xác nhận…");
        m_acPowerTimer->start();
        const QString txt = on ? QString("AC ON %1 V %2 Hz").arg(ac_packet.voltage).arg(ac_packet.frequency)
                               : QStringLiteral("AC OFF");
        appendCommandLog("TX", txt);
        writeToLog("[POWER] " + txt);
    }
}

void DashboardManager:: setDcPower(bool on,uint16_t volt, double cur)
{
    if (m_dcWaitKind != 2)
        m_dcPrevOn = m_dcSource->isOn();
    m_dcPendingOn = on;
    m_dcGotPws = false;
    m_dcSource->setIsOn(on);
    fwi_msg_t dc_packet;
    std::memset(&dc_packet, 0, sizeof(fwi_msg_t));
    std ::strncpy (dc_packet.msg, "FWI", sizeof(dc_packet.msg));
    std :: strncpy(dc_packet.isAC,"DC", sizeof(dc_packet.isAC));
    dc_packet.voltage = static_cast<uint16_t> (volt);
    // Fixed-point x100 (centiamps), e.g. 2.35 A -> 235 - firmware's
    // HandleFwi() divides this back by 100.0f for the DC case.
    dc_packet.frequency = static_cast<uint16_t> (std::lround(std::max(cur, 0.0) * 100.0));
    dc_packet.state = on ? 1:0;
    kria_message_t tx_msg ;
    static uint8_t local_seq =0;
    kria_msg_output_encode(local_seq++, 36, &dc_packet, &tx_msg);
    sendKriaFrame(tx_msg);

    if( m_serialPort->isOpen()){
        m_dcWaitKind = 2;
        m_dcSource->setLink("pending", on ? "Đang bật, chờ nguồn xác nhận…" : "Đang tắt, chờ nguồn xác nhận…");
        m_dcPowerTimer->start();
        const QString txt = on ? QString("DC ON %1 V, giới hạn %2 A").arg(dc_packet.voltage).arg(cur, 0, 'f', 2)
                               : QStringLiteral("DC OFF");
        appendCommandLog("TX", txt);
        writeToLog("[POWER] " + txt);
    }
}

void DashboardManager::sendPwrQuery(uint8_t dev)
{
    pwr_query_msg_t q;
    std::memset(&q, 0, sizeof(q));
    std::strncpy(q.msg, "PWQ", sizeof(q.msg));
    q.dev = dev;
    kria_message_t tx_msg;
    static uint8_t seq = 0;
    kria_msg_output_encode(seq++, 41, &q, &tx_msg);
    sendKriaFrame(tx_msg);
}

void DashboardManager::refreshPower(int dev)
{
    const bool isAc = (dev == PWR_DEV_AC);
    if (dev != PWR_DEV_AC && dev != PWR_DEV_DC)
        return;
    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", "Chưa kết nối board - không đọc được nguồn.");
        return;
    }
    BasePowerSource *src = isAc ? static_cast<BasePowerSource *>(m_acSource) : m_dcSource;
    int &waitKind = isAc ? m_acWaitKind : m_dcWaitKind;
    if (waitKind != 0)
        return;     // đang chờ kết quả của chính nguồn này - kết quả đó cũng có số đo
    waitKind = 1;
    src->setLink("pending", "Đang đọc nguồn…");
    (isAc ? m_acPowerTimer : m_dcPowerTimer)->start();
    sendPwrQuery(static_cast<uint8_t>(dev));
}

void DashboardManager::refreshAllPower()
{
    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", "Chưa kết nối board - không đọc được nguồn.");
        return;
    }
    refreshPower(PWR_DEV_AC);
    refreshPower(PWR_DEV_DC);
}

void DashboardManager::onAcPowerTimeout() { powerTimeout(true); }
void DashboardManager::onDcPowerTimeout() { powerTimeout(false); }

void DashboardManager::powerTimeout(bool isAc)
{
    BasePowerSource *src = isAc ? static_cast<BasePowerSource *>(m_acSource) : m_dcSource;
    int &waitKind = isAc ? m_acWaitKind : m_dcWaitKind;
    const QString name = isAc ? QStringLiteral("AC") : QStringLiteral("DC");
    if (waitKind == 2)      // lệnh đặt không có kết quả -> về trạng thái cũ
        src->setIsOn(isAc ? m_acPrevOn : m_dcPrevOn);
    waitKind = 0;
    const QString msg = QString("Board không trả kết quả nguồn %1 (mất kết nối, hoặc firmware cũ chưa hỗ trợ đọc nguồn).").arg(name);
    src->setLink("boardnoreply", msg);
    appendCommandLog("RX", name + ": không có kết quả từ board");
    writeToLog("[POWER] " + msg);
    emit systemNotification("Error", msg);
}

void DashboardManager::handlePwrStatus(const pwr_status_msg_t &st)
{
    const bool isAc = (st.dev == PWR_DEV_AC);
    if (st.dev != PWR_DEV_AC && st.dev != PWR_DEV_DC)
        return;
    BasePowerSource *src = isAc ? static_cast<BasePowerSource *>(m_acSource) : m_dcSource;
    int  &waitKind = isAc ? m_acWaitKind : m_dcWaitKind;
    bool &gotPws   = isAc ? m_acGotPws : m_dcGotPws;
    const bool pendingOn = isAc ? m_acPendingOn : m_dcPendingOn;
    const bool prevOn    = isAc ? m_acPrevOn : m_dcPrevOn;
    const QString name = isAc ? QStringLiteral("AC") : QStringLiteral("DC");

    // Kết quả này thuộc lệnh đang chờ? (một lần đọc có thể tới trong lúc
    // đang chờ lệnh đặt - vẫn dùng số đo nhưng không kết thúc việc chờ).
    const bool matchesWait =
        (waitKind == 1 && st.reason == PWR_REASON_READ) ||
        (waitKind == 2 && st.reason == PWR_REASON_SET) ||
        (waitKind == 3 && st.reason == PWR_REASON_RESET);
    if (matchesWait) {
        (isAc ? m_acPowerTimer : m_dcPowerTimer)->stop();
        waitKind = 0;
    }
    if (st.reason == PWR_REASON_SET)
        gotPws = true;

    // 1) Số đo -> đồ thị
    const QDateTime now = QDateTime::currentDateTime();
    const bool vV = st.valid & PWR_VALID_VOLT;
    const bool iV = st.valid & PWR_VALID_CURR;
    const bool fV = st.valid & PWR_VALID_FREQ;
    const double v = st.volt_mv / 1000.0;
    const double i = st.curr_ma / 1000.0;
    const double f = st.freq_mhz / 1000.0;
    if (isAc)
        m_acSource->addAcSample(now, vV, v, iV, i, fV, f);
    else
        m_dcSource->addSample(now, vV, v, iV, i);

    // 2) Trạng thái ngõ ra: tin nguồn (OUTP?) nếu có; DC chưa có lệnh hỏi
    //    trạng thái -> lệnh đặt thành công thì lấy trạng thái yêu cầu.
    const bool outKnown = st.valid & PWR_VALID_OUTPUT;
    if (outKnown && st.output <= 1)
        src->setIsOn(st.output == 1);
    else if (st.reason == PWR_REASON_SET)
        src->setIsOn(st.result == PWR_RESULT_OK ? pendingOn : prevOn);
    else if (st.reason == PWR_REASON_RESET && st.result == PWR_RESULT_OK)
        src->setIsOn(false);

    // 3) Kết quả -> nhãn trạng thái, Terminal, thông báo
    QString parts;
    if (outKnown && st.output <= 1) parts += QString(" OUT=%1").arg(st.output ? "ON" : "OFF");
    if (vV) parts += " " + fmtNum(v, 2) + " V";
    if (iV) parts += " " + fmtNum(i, 3) + " A";
    if (fV) parts += " " + fmtNum(f, 2) + " Hz";

    QString state, msg;
    switch (st.result) {
    case PWR_RESULT_OK:
        state = "ok";
        break;
    case PWR_RESULT_NO_REPLY:
        state = "noreply";
        msg = (st.reason == PWR_REASON_READ)
            ? QString("Nguồn %1 không phản hồi - kiểm tra cáp RS232, baud, nguồn đã bật.").arg(name)
            : QString("Đã gửi lệnh tới nguồn %1 nhưng nguồn không phản hồi để xác nhận - kiểm tra cáp RS232/baud.").arg(name);
        break;
    case PWR_RESULT_DEV_ERROR:
        state = "error";
        msg = QString("Nguồn %1 báo lỗi lệnh (mã SCPI %2).").arg(name).arg(st.dev_err);
        break;
    case PWR_RESULT_OUT_MISMATCH:
        state = "mismatch";
        msg = QString("Nguồn %1: ngõ ra đang %2, khác trạng thái yêu cầu.").arg(name, st.output ? "BẬT" : "TẮT");
        break;
    case PWR_RESULT_NOT_READY:
        state = "notready";
        msg = QString("UART nguồn %1 trên board chưa khởi tạo được.").arg(name);
        break;
    case PWR_RESULT_BAD_REPLY:
        state = "badreply";
        msg = QString("Nguồn %1 trả lời không đọc được - kiểm tra lệnh đọc trong board_config.h.").arg(name);
        break;
    default:
        state = "error";
        msg = QString("Nguồn %1: kết quả không xác định (%2).").arg(name).arg(st.result);
        break;
    }
    // Lần đọc chen vào giữa lúc đang chờ lệnh đặt: giữ nhãn "pending".
    if (matchesWait || waitKind == 0)
        src->setLink(state, msg);

    static const char *reasonText[] = { "đọc", "xác nhận lệnh", "đặt lại" };
    const QString reason = st.reason <= 2 ? QString::fromUtf8(reasonText[st.reason]) : QStringLiteral("?");
    appendCommandLog("RX", QString("%1 (%2): %3%4").arg(name, reason,
                     st.result == PWR_RESULT_OK ? QStringLiteral("OK") : state.toUpper(), parts));
    writeToLog(QString("[POWER] %1 %2 result=%3 err=%4%5").arg(name, reason).arg(st.result).arg(st.dev_err).arg(parts));
    if (!msg.isEmpty())
        emit systemNotification("Error", msg);
    else if (st.reason == PWR_REASON_SET)
        emit systemNotification("Success", QString("Nguồn %1 đã xác nhận: %2").arg(name, src->isOn() ? "BẬT" : "TẮT"));
}


void DashboardManager::applyDelayTime(int ns)
{
    setDelayTime(ns);

    timedelay_msg_t delay_data;
    std::memset(&delay_data, 0, sizeof(timedelay_msg_t));
    std::strncpy(delay_data.msg, "DEL", sizeof(delay_data.msg));

    // Must stay uint16_t (matching timedelay_msg_t.value) - 400 does not
    // fit in a uint8_t (was silently truncated to 144, which firmware's
    // OpticalDelay_SetNs() doesn't recognize, so it fell back to 0 ns and
    // ACKed failure - the "400 ns" option was effectively unreachable).
    delay_data.value = static_cast<uint16_t>(ns);

    kria_message_t tx_msg;
    static uint8_t delay_seq = 0;
    kria_msg_output_encode(delay_seq++, 37, &delay_data, &tx_msg);
    sendKriaFrame(tx_msg);

    writeToLog(QString("[DELAY] Set to %1 ns").arg(ns));
    emit systemNotification("Info", QString("Giữ chậm: %1 ns").arg(ns));
}

void DashboardManager::refreshPorts()
{
    QStringList ports;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ports << info.portName();
    if (ports.isEmpty())
        ports << "COM1" << "COM2" << "COM3" << "COM4"; // Fallback khi không có thiết bị
    if (m_availablePorts != ports) {
        m_availablePorts = ports;
        emit availablePortsChanged();
    }
}

bool DashboardManager::isPortAvailable(const QString &portName) const
{
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        if (info.portName() == portName) return true;
    return false;
}

void  DashboardManager::handleSerialRead(){
    // VIP+kria la giao thuc nhi phan (binary) - KHONG duoc cat theo '\n' nhu
    // truoc day, vi mot byte payload/CRC bat ky co the mang gia tri 0x0A va
    // lam vo/cat sai khung mot cach ngau nhien. Moi byte nhan duoc duoc dua
    // thang vao VIP_Decode() (state machine byte-by-byte, tu resync tren
    // SYNC 0xEB90 - xem vip_helper.c) trong parseIncomingData().
    parseIncomingData(m_serialPort->readAll());
}

void DashboardManager::parseIncomingData(const QByteArray &data) {
    for (int i = 0; i < data.size(); ++i) {
        uint8_t byte_in = static_cast<uint8_t>(data.at(i));

        ++m_rxBytes;
        ++m_monBytes;
        // VIP_Decode() chỉ GHI last_error khi gặp lỗi CRC/header (rồi tự bắt
        // đồng bộ lại) -> xoá trước mỗi byte để đếm được số khung lỗi.
        m_vipStatus.last_error = VIP_ERR_NONE;
        const bool gotFrame = VIP_Decode(&m_vipRx, &m_vipStatus, byte_in);
        if (m_vipStatus.last_error != VIP_ERR_NONE) {
            ++m_rxFramesBad;
            ++m_monBad;
        }
        if (!gotFrame) {
            continue;
        }
        ++m_rxFramesOk;
        ++m_monOk;
        if (m_monitor) {
            QString type;
            if (m_vipRx.header.frameType == VIP_FRAME_ACK_STD) type = QStringLiteral("ACK_PING");
            else if (m_vipRx.msg.msgid == 39 && m_vipRx.msg.len >= 4)
                type = QString::fromLatin1(reinterpret_cast<const char *>(m_vipRx.msg.payload64), 4).trimmed().remove(QChar('\0'));
            else {
                switch (m_vipRx.msg.msgid) {
                case 35: type = QStringLiteral("ACK_AC"); break;
                case 36: type = QStringLiteral("ACK_DC"); break;
                case 37: type = QStringLiteral("ACK_DELAY"); break;
                case 38: type = QStringLiteral("ACK_ATT"); break;
                case 40: type = QStringLiteral("ADC"); break;
                case 41: type = QStringLiteral("ACK_READ"); break;
                case 42: type = QStringLiteral("NGUON"); break;
                case 44: type = QStringLiteral("RAW"); break;
                default: type = QString("msg%1").arg(m_vipRx.msg.msgid); break;
                }
            }
            m_monTypes[type] += 1;
        }

        // Xac thuc dung COM: 1 khung ACK_STD that su tu board (dap lai PING
        // do sendPingProbe() gui) chi co the den tu chinh firmware 5E-HTE -
        // khong phai tu nguon AC/DC nhinh cong qua cung hub USB. Kiem tra
        // ngay tren header da giai ma xong, KHONG phu thuoc ket qua cua
        // VIP_Process_Frame() ben duoi (no tra ve IGNORE cho ACK_STD, vi lop
        // do "danh cho muc dich tuong lai" - xem vip_helper.c).
        if (m_vipRx.header.frameType == VIP_FRAME_ACK_STD) {
            m_pingTimeoutTimer->stop();
            if (m_cliPingPending) {
                m_cliPingPending = false;
                cliOut(QString("PING -> board trả lời sau %1 ms").arg(m_cliPingClock.elapsed()));
            }
            if (!m_isPortVerified) {
                m_isPortVerified = true;
                emit isPortVerifiedChanged();
                writeToLog("[PING] Xac thuc dung board 5E-HTE tren cong " + m_comPort);
                updateLinkDiagnosis(false);
            }
            continue;
        }

        // Mot khung VIP hop le (SYNC + header + CRC16 + [kria msg + CRC32])
        // vua duoc rap xong. VIP_Process_Frame() ap dung dinh tuyen (frame
        // khong danh cho node nay se bi bo qua) va tu dong tra loi PING_STD
        // neu firmware gui (hien firmware khong chu dong PING, nhung xu ly
        // doi xung voi phia firmware cho chac).
        uint8_t  reply_buf[VIP_MAX_FRAME_LEN];
        uint32_t reply_len = 0U;
        vip_process_result_t result = VIP_Process_Frame(&m_vipRx, reply_buf, sizeof(reply_buf), &reply_len);

        if (result == VIP_PROCESS_REPLY_READY) {
            if (m_serialPort->isOpen() && reply_len > 0U) {
                m_serialPort->write(reinterpret_cast<const char*>(reply_buf), reply_len);
            }
            continue;
        }
        if (result != VIP_PROCESS_DATA_READY) {
            // FORWARD/IGNORE (frame khong danh cho node nay, ACK khong cai
            // hanh dong nao, hoac dinh tuyen khong xac dinh) - khong co gi
            // de dua len UI.
            continue;
        }

        const kria_message_t &kmsg = m_vipRx.msg;

        if (kmsg.msgid >= 35 && kmsg.msgid <= 38) {
            bool isSuccess = (kmsg.len >= 1 && kmsg.payload64[0] == 1);
            const char *okText = isSuccess ? "THÀNH CÔNG" : "THẤT BẠI";

            switch (kmsg.msgid) {
            case 35: // ACK lệnh AC
            case 36: { // ACK lệnh DC
                // Firmware mới gửi msgid 42 (kết quả thật từ nguồn) NGAY TRƯỚC
                // ACK này - handlePwrStatus() đã cập nhật công tắc/đồ thị/
                // thông báo. Chỉ khi chưa có 42 (firmware cũ: ACK = "đã gửi
                // byte", không xác nhận được nguồn) mới dùng ACK để quyết định.
                const bool isAc = (kmsg.msgid == 35);
                BasePowerSource *src = isAc ? static_cast<BasePowerSource *>(m_acSource) : m_dcSource;
                const bool gotPws = isAc ? m_acGotPws : m_dcGotPws;
                writeToLog(QString("[ACK] Lệnh %1: %2").arg(isAc ? "AC" : "DC", okText));
                if (!gotPws) {
                    int &waitKind = isAc ? m_acWaitKind : m_dcWaitKind;
                    if (waitKind == 2) {
                        (isAc ? m_acPowerTimer : m_dcPowerTimer)->stop();
                        waitKind = 0;
                    }
                    src->setIsOn(isSuccess ? (isAc ? m_acPendingOn : m_dcPendingOn)
                                           : (isAc ? m_acPrevOn : m_dcPrevOn));
                    if (isSuccess) {
                        src->setLink("unconfirmed", "Board đã gửi lệnh nhưng firmware này chưa hỗ trợ xác nhận từ nguồn.");
                    } else {
                        src->setLink("error", "Board từ chối lệnh.");
                        emit systemNotification("Error", QString("Board từ chối lệnh nguồn %1.").arg(isAc ? "AC" : "DC"));
                    }
                }
                break;
            }
            case 37: // Phản hồi làm chậm (Delay)
                writeToLog(QString("[ACK] Lệnh cấu hình Delay: %1").arg(okText));
                if (!isSuccess)
                    emit systemNotification("Error", "Board từ chối lệnh giữ chậm (giá trị không hợp lệ hoặc GPIO chưa sẵn sàng)");
                break;
            case 38: // Phản hồi suy hao (ATT)
                writeToLog(QString("[ACK] Lệnh cấu hình ATT: %1").arg(okText));
                if (!isSuccess)
                    emit systemNotification("Error", "Board từ chối lệnh suy hao (firmware cũ chưa hỗ trợ bước 0.25 dB, hoặc SPI F1956 chưa sẵn sàng)");
                break;
            }
        }

        else if (kmsg.msgid == 41) {
            // Chỉ có ACK khi board TỪ CHỐI yêu cầu đọc (gói sai) - thành công
            // thì câu trả lời là msgid 42.
            if (kmsg.len >= 1 && kmsg.payload64[0] == 0) {
                appendCommandLog("RX", "Board từ chối yêu cầu đọc nguồn");
                emit systemNotification("Error", "Board từ chối yêu cầu đọc nguồn (gói không hợp lệ).");
            }
        }

        else if (kmsg.msgid == 44) {
            if (kmsg.len == sizeof(pwr_raw_reply_msg_t)) {
                pwr_raw_reply_msg_t r;
                std::memcpy(&r, kmsg.payload64, sizeof(r));
                handlePwrRawReply(r);
            }
        }

        else if (kmsg.msgid == 43) {
            if (kmsg.len >= 1 && kmsg.payload64[0] == 0)
                cliError("Board từ chối lệnh thô (gói sai, hoặc ký tự không hợp lệ).");
        }

        else if (kmsg.msgid == 42) {
            if (kmsg.len == sizeof(pwr_status_msg_t)) {
                pwr_status_msg_t st;
                std::memcpy(&st, kmsg.payload64, sizeof(st));
                handlePwrStatus(st);
            }
        }

        else if (kmsg.msgid == 39) {

            if (kmsg.len >= STREAM_MAGIC_LEN + 2 &&
                std::memcmp(kmsg.payload64, "WAVD", STREAM_MAGIC_LEN) == 0) {
                // Firmware mới: n điểm int16 = dBFS x 0.01 (thứ tự bin DFT tự
                // nhiên). Đổi về biên độ tuyến tính |X| = 10^(dB/20)*2048*n để
                // dùng chung đường xử lý (fftshift, Parseval, hiệu chuẩn).
                const int n = kmsg.payload64[STREAM_MAGIC_LEN];
                if (n >= 2 && kmsg.len >= STREAM_MAGIC_LEN + 2 + 2 * n) {
                    QVector<double> mags;
                    mags.reserve(n);
                    const double fullScale = RF_ADC_FULL_SCALE * n;
                    for (int idx = 0; idx < n; ++idx) {
                        const int off = STREAM_MAGIC_LEN + 2 + idx * 2;
                        const int16_t v = static_cast<int16_t>(
                            static_cast<uint16_t>(kmsg.payload64[off])
                            | (static_cast<uint16_t>(kmsg.payload64[off + 1]) << 8));
                        mags.append(std::pow(10.0, (v / 100.0) / 20.0) * fullScale);
                    }
                    processWaveMagnitudes(mags, false);
                }
            }
            else if (kmsg.len > STREAM_MAGIC_LEN &&
                std::memcmp(kmsg.payload64, "WAVE", STREAM_MAGIC_LEN) == 0) {

                // Payload = 4 byte "WAVE" + N biên độ |X_k| uint32 little-
                // endian, thứ tự bin FFT tự nhiên (xem stream_framer.c /
                // CmdDispatcher_SendWaveSpectrum() phía firmware).
                int sampleCount = (kmsg.len - STREAM_MAGIC_LEN) / 4;
                QVector<double> mags;
                mags.reserve(sampleCount);
                for (int idx = 0; idx < sampleCount; ++idx) {
                    int off = STREAM_MAGIC_LEN + idx * 4;
                    uint32_t raw =
                        static_cast<uint32_t>(kmsg.payload64[off + 0])
                        | (static_cast<uint32_t>(kmsg.payload64[off + 1]) << 8)
                        | (static_cast<uint32_t>(kmsg.payload64[off + 2]) << 16)
                        | (static_cast<uint32_t>(kmsg.payload64[off + 3]) << 24);
                    mags.append(static_cast<double>(raw));
                }
                processWaveMagnitudes(mags, false);
            }
            else if (kmsg.len == sizeof(board_diag_msg_t) &&
                     std::memcmp(kmsg.payload64, "DIAG", STREAM_MAGIC_LEN) == 0) {
                board_diag_msg_t d;
                std::memcpy(&d, kmsg.payload64, sizeof(d));
                handleBoardDiag(d);
            }
            else if (kmsg.len > STREAM_MAGIC_LEN &&
                     std::memcmp(kmsg.payload64, "LOGT", STREAM_MAGIC_LEN) == 0) {
                const uint8_t level = kmsg.payload64[STREAM_MAGIC_LEN];
                const int textLen = kmsg.len - STREAM_MAGIC_LEN - 1;
                handleBoardLog(level, textLen > 0
                    ? QString::fromUtf8(reinterpret_cast<const char *>(&kmsg.payload64[STREAM_MAGIC_LEN + 1]), textLen)
                    : QString());
            }
            else if (kmsg.len == sizeof(rf_status_msg_t) &&
                     std::memcmp(kmsg.payload64, "RFST", STREAM_MAGIC_LEN) == 0) {
                rf_status_msg_t st;
                std::memcpy(&st, kmsg.payload64, sizeof(st));
                handleRfStatus(st);
            }
            else {
                // Phan hoi generic/diag (msgid 39, payload64[0] = 1 success/
                // 0 fail - kmsg.msg[] khong gui lai trong ACK, chi co status
                // byte, xem kria_msg_build_ack() phia firmware). Hien ca len
                // o Terminal (truoc day chi co dong TX, khong thay phan hoi).
                bool isSuccess = (kmsg.len > 0 && kmsg.payload64[0] == 1);
                writeToLog(QString("[ACK] Lệnh generic/diag: %1").arg(isSuccess ? "THÀNH CÔNG" : "THẤT BẠI"));
                appendCommandLog("RX", isSuccess ? QStringLiteral("ACK OK") : QStringLiteral("ACK FAIL"));
            }
        }

        else if (kmsg.msgid == 40) {
            // Telemetry ADC7928 8 kenh (device -> host) - xem adc7928_msg_t /
            // CmdDispatcher_SendAdcTelemetry() phia firmware.
            if (kmsg.len == sizeof(adc7928_msg_t)) {
                adc7928_msg_t adc;
                std::memcpy(&adc, kmsg.payload64, sizeof(adc7928_msg_t));

                QVariantList values;
                values.reserve(8);
                for (int ch = 0; ch < 8; ++ch) {
                    values.append(static_cast<int>(adc.raw_code[ch]));
                }
                m_adcChannels = values;
                emit adcChannelsChanged();

                // Kenh 0 = "kich no": mach peak-detect (diode+tu giu dinh)
                // dua ra bien do DUONG cua 1 tin hieu vat ly AM (toi ~-150V)
                // qua bo chia ap - dau tru vi ADC khong biet dau. Bat dinh
                // PHAN MEM: chi cap nhat khi doc duoc 1 gia tri am HON (tri
                // tuyet doi lon hon) so voi dinh dang giu, giong hanh vi
                // "giu dinh" cua chinh mach analog - reset ve 0 chi khi
                // nguoi dung bam "Dat lai" (xem resetSystem() ben duoi).
                double codeToVolts = (static_cast<double>(adc.raw_code[0]) / 4096.0) * ADC7928_FULL_SCALE;
                double candidate = -codeToVolts;
                if (candidate < m_kichNoPeakV) {
                    m_kichNoPeakV = candidate;
                    emit kichNoPeakVoltageChanged();
                }
            }
        }
    }
}


void DashboardManager::handleSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        emit systemNotification("Error", "Mất kết nối thiết bị đột ngột (Cáp hở hoặc mất nguồn)!");
        stopSession();
    }
}
void DashboardManager::onAttUpdateRequested(int channel, double value)
{
    // F1956: 7 bit, bước 0.25 dB, 0..31.75 dB (datasheet - firmware
    // board_config.h ATT_STEP_DB/ATT_MAX_STEPS). Gửi thẳng SỐ BƯỚC 0.25 dB
    // (annuation_q_msg_t, msgid 38, 7 byte) - firmware HandleAtt() nhân lại
    // 0.25 rồi Attenuator_SetDb() đổi ra đúng mã 7 bit. Không còn làm tròn
    // về dB nguyên như bản trước.
    long steps = std::lround(value * 4.0);
    steps = std::clamp(steps, 0L, 127L);
    const double sentDb = static_cast<double>(steps) * 0.25;

    annuation_q_msg_t att_data;
    std::memset(&att_data, 0, sizeof(att_data));
    std::strncpy(att_data.msg, "ATQ", sizeof(att_data.msg));
    att_data.att_number = static_cast<uint8_t>(channel);
    att_data.value_qdb  = static_cast<uint16_t>(steps);

    // Lưu giá trị để lần mở app sau hiển thị lại (chỉ hiển thị, không tự
    // gửi xuống board lúc khởi động - xem constructor).
    if (channel >= 1 && channel <= 3) {
        saveSetting(QString("att/ch%1").arg(channel), sentDb);
    }

    kria_message_t tx_msg;
    static uint8_t att_seq = 0;
    kria_msg_output_encode(att_seq++, 38, &att_data, &tx_msg);
    sendKriaFrame(tx_msg);

    if (!m_serialPort->isOpen()) {
        emit systemNotification("Warning", QString("ATT %1: chưa kết nối board - lệnh chưa được gửi").arg(channel));
        return;
    }
    writeToLog(QString("[ATT] CH%1 = %2 dB (gửi %3 bước x 0.25 dB)")
                   .arg(channel).arg(sentDb, 0, 'f', 2).arg(steps));
    emit systemNotification("Info",
                            QString("ATT %1 cập nhật: %2 dB").arg(channel).arg(sentDb, 0, 'f', 2));
}

void DashboardManager::generateSimulatedWave()
{
    if(isConnected()) return ;
    static double t = 0.0;
    t += 0.05;

    // Phổ MÔ PHỎNG khi chưa kết nối - đi qua ĐÚNG đường xử lý của phổ thật
    // (processWaveMagnitudes) để giao diện giống hệt lúc chạy thật, nhưng
    // được đánh dấu "sim" rõ ràng trên giao diện. ~300 ms/lần như firmware.
    if (++m_simSpectrumTick >= 13) {        // ~100 ms, như firmware
        m_simSpectrumTick = 0;
        const int n = 124;      // như firmware ("WAVD", 124 điểm)
        QVector<double> mags(n);
        for (int k = 0; k < n; ++k) {
            double noise = 40.0 + (std::rand() % 400) / 10.0;     // nền nhiễu
            mags[k] = noise;
        }
        // 1 tone di chuyển chậm quanh bin 5, biên độ ~ -20 dBFS
        int toneBin = 19 + static_cast<int>(std::lround(8.0 * std::sin(t * 0.2)));
        mags[(toneBin + n) % n] = RF_ADC_FULL_SCALE * n * 0.1 * (1.0 + 0.05 * std::sin(t));
        processWaveMagnitudes(mags, true);
    }

    // 4 đồ thị nguồn AC/DC KHÔNG còn số mô phỏng: chúng chỉ hiển thị số đo
    // thật đọc từ nguồn (msgid 42), chưa có thì hiện "--".
}


void DashboardManager::writeToLog(const QString &entry)
{
    if (m_logStream) {
        *m_logStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
        << "  " << entry << "\n";
        m_logStream->flush();
    }
}

void DashboardManager::appendCommandLog(const QString &tag, const QString &text)
{
    QString line = QString("[%1] [%2] %3")
                       .arg(QDateTime::currentDateTime().toString("HH:mm:ss"),tag,text);
    m_commandLog.append(line);

    // Giới hạn kích thước buffer
    while (m_commandLog.size() > MAX_LOG_LINES)
        m_commandLog.removeFirst();

    emit commandLogChanged();
}


// ─────────────────────────────────────────────────────────────────────────
// Phổ RF
// ─────────────────────────────────────────────────────────────────────────
void DashboardManager::processWaveMagnitudes(const QVector<double> &mags, bool simulated)
{
    const int n = mags.size();
    if (n < 2)
        return;

    // dBFS từng bin: |X_k| / (2048*N). Firmware làm tròn |X_k| về số nguyên,
    // nên 0 nghĩa là < 0.5 - dùng 0.5 làm sàn để không ra -inf.
    const double fullScale = RF_ADC_FULL_SCALE * n;
    double sumSq = 0.0;
    QVector<double> dbfs(n);
    for (int k = 0; k < n; ++k) {
        const double m = std::max(mags[k], 0.5);
        dbfs[k] = 20.0 * std::log10(m / fullScale);
        sumSq += mags[k] * mags[k];
    }
    // Tổng công suất trong băng (Parseval): sum|X_k|^2 / (2048*N)^2 - mốc
    // 0 dBFS = sóng sin phức đầy thang. Không bị "scalloping" như số đỉnh
    // (tone lệch giữa 2 bin vẫn cho đúng tổng) -> dùng số này để đo/hiệu chuẩn.
    const double totalDbfs = 10.0 * std::log10(std::max(sumSq, 0.25) / (fullScale * fullScale));

    const bool   haveFreq  = m_rfStatusValid && m_rxFsHz > 0;
    const double binHz     = haveFreq ? static_cast<double>(m_rxFsHz) / n : 1.0;
    const double loMHz     = static_cast<double>(m_rxLoHz) / 1e6;
    const double offset    = m_calibrated ? m_calOffsetDb : 0.0;

    // fftshift: bin N/2..N-1 là tần số âm -> vẽ từ -Fs/2 tới +Fs/2.
    QVector<QPointF> pts;
    pts.reserve(n);
    double peakLevel = -1e9, peakX = 0.0;
    for (int i = 0; i < n; ++i) {
        const int k = (i + n / 2) % n;
        const int rel = i - n / 2;                      // -N/2 .. N/2-1
        const double x = haveFreq ? loMHz + rel * binHz / 1e6 : static_cast<double>(rel);
        const double y = dbfs[k] + offset;
        pts.append(QPointF(x, y));
        if (y > peakLevel) { peakLevel = y; peakX = x; }
    }

    // Trục X trọn băng [-Fs/2, +Fs/2] quanh LO (điểm cuối = +Fs/2 - 1 bin
    // nằm trong khoảng) -> vạch chia tròn số (vd 2410/2425/2440/2455/2470).
    const double half = haveFreq ? static_cast<double>(m_rxFsHz) / 2e6 : n / 2.0;
    const double xMin = haveFreq ? loMHz - half : -half;
    const double xMax = haveFreq ? loMHz + half : half;
    // Trục Y: 100 dB, đỉnh trục làm tròn lên bội 10 trên mốc 0 dBFS (+offset).
    const double yMax = std::ceil((0.0 + offset) / 10.0) * 10.0;
    const double yMin = yMax - 100.0;

    m_lastTotalDbfs   = totalDbfs;
    m_lastSpectrumLive = !simulated;
    m_rfController->setSpectrum(pts, haveFreq, xMin, xMax, yMin, yMax,
                                peakLevel, peakX, totalDbfs + offset, totalDbfs);

    if (!simulated) {
        if (!m_waveSeenThisSession) {
            m_waveSeenThisSession = true;
            writeToLog(QString("[RF] Bắt đầu nhận phổ RF từ board (%1 điểm/khung)").arg(n));
        }
        m_rfController->setWaveSource(QStringLiteral("live"));
        m_waveWatchdog->start();
    } else {
        m_rfController->setWaveSource(QStringLiteral("sim"));
    }
}

void DashboardManager::handleRfStatus(const rf_status_msg_t &st)
{
    const bool changed = !m_rfStatusValid || m_rxLoHz != st.rx_lo_hz
                         || m_rxFsHz != st.rx_fs_hz || m_rxGainDb != st.rx1_gain_db;
    m_rfStatusValid = true;
    m_rxLoHz   = st.rx_lo_hz;
    m_rxFsHz   = st.rx_fs_hz;
    m_rxGainDb = st.rx1_gain_db;
    m_rfController->setRfStatus(true,
                                static_cast<double>(st.rx_lo_hz) / 1e6,
                                static_cast<double>(st.rx_fs_hz) / 1e6,
                                static_cast<double>(st.rx_bw_hz) / 1e6,
                                st.rx1_gain_db, gainModeName(st.gain_mode),
                                st.fft_size);
    if (changed) {
        writeToLog(QString("[RF] AD9361 RX: LO %1 MHz, Fs %2 MSPS, BW %3 MHz, gain %4 dB (%5), FFT %6 điểm")
                       .arg(static_cast<double>(st.rx_lo_hz) / 1e6, 0, 'f', 3)
                       .arg(static_cast<double>(st.rx_fs_hz) / 1e6, 0, 'f', 3)
                       .arg(static_cast<double>(st.rx_bw_hz) / 1e6, 0, 'f', 3)
                       .arg(st.rx1_gain_db).arg(gainModeName(st.gain_mode)).arg(st.fft_size));
    }
    updateCalibrationState();
}

void DashboardManager::onWaveWatchdog()
{
    if (m_serialPort->isOpen()) {
        m_rfController->setWaveSource(QStringLiteral("waiting"));
        if (m_waveSeenThisSession)
            writeToLog("[RF] Mất luồng phổ RF (không nhận được WAVE trong 2 s)");
    }
}

void DashboardManager::resetRfState()
{
    m_waveSeenThisSession = false;
    m_rfStatusValid = false;
    m_lastSpectrumLive = false;
    m_rxLoHz = 0;
    m_rxFsHz = 0;
    m_rxGainDb = 0;
    m_rfController->setRfStatus(false, 0.0, 0.0, 0.0, 0, QString(), 0);
    m_rfController->clearSpectrum();
    updateCalibrationState();
}

void DashboardManager::updateCalibrationState()
{
    // Offset dBFS->dBm chỉ đúng cho đúng LO/gain lúc hiệu chuẩn.
    const bool mismatch = m_calibrated && m_rfStatusValid
                          && (m_calLoHz != m_rxLoHz || m_calGainDb != m_rxGainDb);
    m_rfController->setCalibration(m_calibrated, m_calOffsetDb, mismatch);
}

bool DashboardManager::calibrateRf(double refDbm)
{
    if (!m_lastSpectrumLive || m_rfController->waveSource() != QLatin1String("live")) {
        emit systemNotification("Warning", "Hiệu chuẩn cần phổ THẬT đang về từ board (không dùng dữ liệu mô phỏng).");
        return false;
    }
    if (!m_rfStatusValid) {
        emit systemNotification("Warning", "Chưa nhận cấu hình RX (RFST) từ board - đợi vài giây rồi thử lại.");
        return false;
    }
    m_calOffsetDb = refDbm - m_lastTotalDbfs;
    m_calibrated  = true;
    m_calLoHz     = m_rxLoHz;
    m_calGainDb   = m_rxGainDb;
    saveSetting("rf/calOffsetDb", m_calOffsetDb);
    saveSetting("rf/calLoHz", static_cast<qulonglong>(m_calLoHz));
    saveSetting("rf/calGainDb", m_calGainDb);
    updateCalibrationState();
    writeToLog(QString("[RF] Hiệu chuẩn: chuẩn %1 dBm <-> đo %2 dBFS => offset %3 dB (LO %4 MHz, gain %5 dB)")
                   .arg(refDbm, 0, 'f', 2).arg(m_lastTotalDbfs, 0, 'f', 2).arg(m_calOffsetDb, 0, 'f', 2)
                   .arg(static_cast<double>(m_calLoHz) / 1e6, 0, 'f', 3).arg(m_calGainDb));
    emit systemNotification("Success", QString("Đã hiệu chuẩn RF: offset %1 dB").arg(m_calOffsetDb, 0, 'f', 2));
    return true;
}

void DashboardManager::clearRfCalibration()
{
    m_calibrated = false;
    m_calOffsetDb = 0.0;
    m_calLoHz = 0;
    m_calGainDb = 0;
    m_settings.remove("rf/calOffsetDb");
    m_settings.remove("rf/calLoHz");
    m_settings.remove("rf/calGainDb");
    m_settings.sync();
    updateCalibrationState();
    writeToLog("[RF] Đã xoá hiệu chuẩn - phổ hiển thị theo dBFS");
    emit systemNotification("Info", "Đã xoá hiệu chuẩn RF (đơn vị trở về dBFS)");
}


// ─────────────────────────────────────────────────────────────────────────
// config.ini
// ─────────────────────────────────────────────────────────────────────────
QString DashboardManager::resolveConfigPath()
{
    // 1) Biến môi trường (máy test/nhiều cấu hình): DASHBOARDENV_CONFIG=<file>
    const QString env = qEnvironmentVariable("DASHBOARDENV_CONFIG");
    if (!env.isEmpty())
        return env;

    // 2) config.ini cạnh file .exe - dễ thấy, dễ sao lưu/mang sang máy khác.
    //    Thử mở ghi thật (QFileInfo::isWritable không đáng tin với ACL Windows,
    //    vd khi cài trong Program Files).
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QString local = exeDir + QStringLiteral("/config.ini");
    {
        QFile f(local);
        if (f.open(QIODevice::ReadWrite)) {
            f.close();
            return local;
        }
    }

    // 3) Không ghi được cạnh .exe -> AppData của người dùng.
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (dir.isEmpty())
        dir = QDir::homePath() + QStringLiteral("/.DashboardEnv");
    QDir().mkpath(dir);
    return dir + QStringLiteral("/config.ini");
}

void DashboardManager::migrateOldSettings()
{
    // Bản trước lưu ở %APPDATA%/VASC/AvionicsDashboard.ini (QSettings mặc định
    // theo tên tổ chức/ứng dụng). Lần đầu chạy bản mới (config.ini còn trống)
    // chép sang 1 lần để người dùng không phải nhập lại.
    if (!m_settings.allKeys().isEmpty())
        return;
    if (QCoreApplication::organizationName().isEmpty() || QCoreApplication::applicationName().isEmpty())
        return;
    QSettings old(QSettings::IniFormat, QSettings::UserScope,
                  QCoreApplication::organizationName(), QCoreApplication::applicationName());
    if (QFileInfo(old.fileName()) == QFileInfo(m_configPath))
        return;
    const QStringList keys = old.allKeys();
    if (keys.isEmpty())
        return;
    for (const QString &k : keys)
        m_settings.setValue(k, old.value(k));
    m_settings.setValue("meta/migratedFrom", old.fileName());
    m_settings.sync();
    const QString from = old.fileName();
    QTimer::singleShot(0, this, [this, from] {
        appendCommandLog("INFO", "Đã chép cấu hình cũ từ " + QDir::toNativeSeparators(from));
    });
}

void DashboardManager::saveSetting(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
    // Ghi xuống đĩa ngay - app bị tắt đột ngột cũng không mất giá trị vừa chốt.
    m_settings.sync();
}

double DashboardManager::loadOptionalDouble(const QSettings &s, const QString &key, double lo, double hi)
{
    if (!s.contains(key))
        return kNaN;
    const QString txt = s.value(key).toString().trimmed();
    if (txt.isEmpty())
        return kNaN;
    bool ok = false;
    const double v = txt.toDouble(&ok);
    if (!ok || std::isnan(v))
        return kNaN;
    return qBound(lo, v, hi);
}

QVariant DashboardManager::loadUi(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(QStringLiteral("ui/") + key, defaultValue);
}

void DashboardManager::saveUi(const QString &key, const QVariant &value)
{
    saveSetting(QStringLiteral("ui/") + key, value);
}
