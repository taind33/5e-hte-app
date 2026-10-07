#ifndef DASHBOARDMANAGER_H
#define DASHBOARDMANAGER_H

#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QSettings>
#include <QElapsedTimer>
#include <QMap>
#include <cmath>

#include "acsource.h"
#include "dcsource.h"
#include "rfcontroller.h"
#include "kria_helper.h"

/* This app IS the VIP host node (id 0x00). VIP_NODE_ID_SELF now defaults to
 * 0x00 inside the app's own copy of vip_helper.h, so EVERY translation unit
 * (in particular vip_helper.cpp, which never includes this header) agrees on
 * it - the old per-header override here did not reach vip_helper.cpp and made
 * VIP_Process_Frame() drop every DATA frame from the board as "forward". */
#include "vip_helper.h"
static_assert(VIP_NODE_ID_SELF == VIP_NODE_ID_HOST, "Qt app must be the VIP host node");

class DashboardManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ACsource* acSource READ acSource CONSTANT)
    Q_PROPERTY(DCsource* dcSource READ dcSource CONSTANT)
    Q_PROPERTY(RfController* rfController READ rfController CONSTANT)
    Q_PROPERTY(QString comPort READ comPort WRITE setComPort NOTIFY comPortChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(int delayTime READ delayTime WRITE setDelayTime NOTIFY delayTimeChanged)
    Q_PROPERTY(QString sessionName READ sessionName WRITE setSessionName NOTIFY sessionNameChanged FINAL)
    Q_PROPERTY(bool isConnected READ isConnected  NOTIFY isConnectedChanged FINAL)
    Q_PROPERTY(QStringList availablePorts READ availablePorts  NOTIFY availablePortsChanged FINAL)
    Q_PROPERTY(QStringList commandLog READ commandLog  NOTIFY commandLogChanged FINAL)
    Q_PROPERTY(QVariantList adcChannels READ adcChannels NOTIFY adcChannelsChanged FINAL)
    // Đường dẫn file cấu hình thật đang dùng (config.ini cạnh file .exe, hoặc
    // AppData nếu thư mục .exe không ghi được) - hiện ở tooltip/Terminal.
    Q_PROPERTY(QString configPath READ configPath CONSTANT FINAL)

    // Đèn xác thực đúng COM: true chỉ khi board 5E-HTE thật đã trả lời ACK
    // cho 1 khung PING_STD do app gửi (xem StartPingCycle()/parseIncomingData()).
    // Phân biệt với isConnected (chỉ có nghĩa "mở cổng COM thành công" - cổng
    // của nguồn AC/DC qua cùng hub USB cũng mở được, không có nghĩa là board).
    Q_PROPERTY(bool isPortVerified READ isPortVerified NOTIFY isPortVerifiedChanged FINAL)

    // Vì sao chưa xác thực được board - suy ra từ bộ đếm byte/khung nhận
    // được (đèn cam không còn chỉ là "chưa xác thực"): không có byte nào /
    // có byte nhưng sai khung (baud, A-B) / có khung nhưng không trả PING...
    Q_PROPERTY(QString linkDiagnosis READ linkDiagnosis NOTIFY linkDiagnosisChanged FINAL)
    Q_PROPERTY(QString linkDiagnosisShort READ linkDiagnosisShort NOTIFY linkDiagnosisChanged FINAL)
    // Thông tin board tự báo (gói "DIAG"): firmware build, trạng thái AD9361,
    // DMA... - rỗng nếu board chưa gửi (firmware cũ / chưa kết nối).
    Q_PROPERTY(QString boardFirmware READ boardFirmware NOTIFY boardDiagChanged FINAL)
    Q_PROPERTY(QVariantMap boardDiag READ boardDiag NOTIFY boardDiagChanged FINAL)
    // Đang chạy "scan" (tự tìm cổng COM + baud có board) từ Terminal.
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged FINAL)

    // Điện áp "kích nổ" ở dạng bắt đỉnh (peak-hold) - xem parseIncomingData()
    // msgid 40 và resetSystem(). Thay cho việc Detect.qml tự tính lại mỗi lần
    // (không có bộ nhớ, không phản ánh đúng hành vi peak-detect vật lý).
    Q_PROPERTY(double kichNoPeakVoltage READ kichNoPeakVoltage NOTIFY kichNoPeakVoltageChanged FINAL)

    // Giới hạn thật của nguồn AC/DC - lấy đúng từ board_config.h phía firmware
    // (AC_UVP_VOLTAGE/AC_OVP_LIMIT/DC_MAX_VOLTAGE/DC_MAX_CURRENT), KHÔNG bịa
    // số 999 như validator cũ. Expose ra QML để mọi TextField validator dùng
    // chung 1 nguồn sự thật duy nhất thay vì mỗi file QML tự chép số.
    // Tần số AC: firmware KHÔNG clamp (Device_ControlACPwr chỉ clamp voltage),
    // nên acFreqMin/Max dưới đây chỉ là chặn hợp lý phía UI (lưới điện thế
    // giới thực tế 45-65Hz), không phải giới hạn xác nhận từ firmware.
    Q_PROPERTY(int acVoltMin READ acVoltMin CONSTANT)
    Q_PROPERTY(int acVoltMax READ acVoltMax CONSTANT)
    Q_PROPERTY(int acFreqMin READ acFreqMin CONSTANT)
    Q_PROPERTY(int acFreqMax READ acFreqMax CONSTANT)
    Q_PROPERTY(int dcVoltMin READ dcVoltMin CONSTANT)
    Q_PROPERTY(int dcVoltMax READ dcVoltMax CONSTANT)
    Q_PROPERTY(double dcCurrentMin READ dcCurrentMin CONSTANT)
    Q_PROPERTY(double dcCurrentMax READ dcCurrentMax CONSTANT)
    Q_PROPERTY(double attMax READ attMax CONSTANT)

    // Giá trị "đang nhập" (staged) cho 4 ô Power. NaN = CHƯA NHẬP (ô hiện
    // "--", lưu vào .ini dưới dạng chuỗi rỗng) - không còn tự điền giá trị
    // mặc định khi xoá ô hay khi chạy lần đầu. Trước đây lưu bằng
    // Qt.labs.settings ở PowerControlPanel.qml (QML-side), giờ chuyển hẳn
    // xuống backend để: (1) hết cảnh báo deprecate Qt.labs.settings, (2)
    // dùng chung 1 QSettings m_settings với comPort/baudRate/... thay vì
    // 2 cơ chế lưu khác nhau trong cùng 1 app. Đây CHỈ là giá trị đang
    // soạn trên UI (chưa gửi lệnh) - không đụng phần cứng khi load lại
    // (xem constructor: gán thẳng vào member, không gọi qua setter).
    Q_PROPERTY(double stagedAcVolt READ stagedAcVolt WRITE setStagedAcVolt NOTIFY stagedAcVoltChanged)
    Q_PROPERTY(double stagedAcFreq READ stagedAcFreq WRITE setStagedAcFreq NOTIFY stagedAcFreqChanged)
    Q_PROPERTY(double stagedDcVolt READ stagedDcVolt WRITE setStagedDcVolt NOTIFY stagedDcVoltChanged)
    Q_PROPERTY(double stagedDcCurrent READ stagedDcCurrent WRITE setStagedDcCurrent NOTIFY stagedDcCurrentChanged)

public:
    explicit DashboardManager(QObject *parent = nullptr);
    ~DashboardManager();

    ACsource* acSource() const {return m_acSource;}
    DCsource* dcSource() const {return m_dcSource;}
    RfController* rfController() const {return m_rfController;}


    QString comPort() const {return m_comPort;}
    int baudRate() const { return m_baudRate;}
    int delayTime() const {return m_delayTime;}
    QString sessionName() const {return m_sessionName;}
    bool isConnected () const {return m_serialPort->isOpen();}
    QStringList availablePorts() const { return m_availablePorts;}
    QStringList commandLog() const {return m_commandLog;}
    QVariantList adcChannels() const {return m_adcChannels;}
    bool isPortVerified() const {return m_isPortVerified;}
    QString linkDiagnosis() const {return m_linkDiagnosis;}
    QString linkDiagnosisShort() const {return m_linkDiagnosisShort;}
    QString boardFirmware() const {return m_boardFirmware;}
    QVariantMap boardDiag() const {return m_boardDiag;}
    bool scanning() const {return m_scanning;}
    double kichNoPeakVoltage() const {return m_kichNoPeakV;}

    double stagedAcVolt()    const {return m_stagedAcVolt;}
    double stagedAcFreq()    const {return m_stagedAcFreq;}
    double stagedDcVolt()    const {return m_stagedDcVolt;}
    double stagedDcCurrent() const {return m_stagedDcCurrent;}
    QString configPath()     const {return m_configPath;}

    // Giới hạn nguồn AC/DC - xem board_config.h (firmware) src/board_config.h:
    // AC_UVP_VOLTAGE=80, AC_OVP_LIMIT=150, DC_MAX_VOLTAGE=36, DC_MAX_CURRENT=5.
    static int    acVoltMin()    {return 80;}
    static int    acVoltMax()    {return 150;}
    static int    acFreqMin()    {return 45;}   // UI-only, firmware không giới hạn tần số AC
    static int    acFreqMax()    {return 1000;} // Cập nhật theo yêu cầu thực tế: nguồn AC test rig chạy tần số biến thiên tới 1000Hz, không phải điện lưới 50/60Hz.
    static int    dcVoltMin()    {return 0;}
    static int    dcVoltMax()    {return 36;}
    static double dcCurrentMin() {return 0.0;}
    static double dcCurrentMax() {return 3.0;}  // Cập nhật theo yêu cầu thực tế: 3A (trước ghi nhầm 5A).
    // F1956 thật: bước 0.25dB, 0..31.75 dB - giao thức dây (msgid 38,
    // annuation_q_msg_t) truyền đúng số bước 0.25dB, xem onAttUpdateRequested().
    static double attMax()       {return 31.75;}

    void setComPort(const QString &port);
    void setBaudRate(int rate);
    void setDelayTime(int time);
    void setSessionName(const QString &sessionName);
    void setStagedAcVolt(double v);
    void setStagedAcFreq(double f);
    void setStagedDcVolt(double v);
    void setStagedDcCurrent(double c);

    Q_INVOKABLE void startSession();
    Q_INVOKABLE void stopSession();
    Q_INVOKABLE void openLogFile();
    // Công tắc AC/DC trên giao diện gọi 2 hàm này: kiểm tra đã kết nối và
    // (khi BẬT) đã nhập đủ giá trị - thiếu thì báo rõ và trả false (công tắc
    // tự về vị trí cũ), đủ thì gửi lệnh. Trạng thái ON/OFF cuối cùng theo
    // đúng xác nhận của nguồn (msgid 42), không theo vị trí công tắc.
    Q_INVOKABLE bool requestAcPower(bool on);
    Q_INVOKABLE bool requestDcPower(bool on);
    Q_INVOKABLE void setAcPower(bool on, uint16_t volt, uint16_t freq );
    Q_INVOKABLE void setDcPower(bool on,uint16_t volt, double cur);

    // Đọc giá trị tức thời của nguồn (nút refresh trên đồ thị): dev 1 = AC,
    // 2 = DC. refreshAllPower() = phím F5 (cả 2 nguồn). Chỉ đọc, không đổi
    // gì trên nguồn.
    Q_INVOKABLE void refreshPower(int dev);
    Q_INVOKABLE void refreshAllPower();

    // Lưu/nạp trạng thái giao diện thuần (theme, kích thước cửa sổ, ô công
    // suất chuẩn RF...) vào cùng file config.ini, nhóm [ui].
    Q_INVOKABLE QVariant loadUi(const QString &key, const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void saveUi(const QString &key, const QVariant &value);
    Q_INVOKABLE void applyDelayTime ( int ns);
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE bool isPortAvailable(const QString &portName) const;

    Q_INVOKABLE void resetSystem();
    Q_INVOKABLE void sendCommand(const QString &cmd);

    // ── Terminal kiểu CLI (dashboardcli.cpp) ──
    // 1 dòng người dùng gõ ở ô CMD: help, clear, status, ac/dc/att/delay,
    // read, ac raw/dc raw (lệnh thô tới nguồn), ping, scan, monitor, reset,
    // board (lệnh chung kiểu cũ)... - xem "help".
    Q_INVOKABLE void executeCommand(const QString &line);
    // Hoàn thành lệnh bằng phím Tab: các lệnh/từ khớp tiền tố.
    Q_INVOKABLE QStringList cliCompletions(const QString &prefix) const;
    Q_INVOKABLE void clearCommandLog();

    // Hiệu chuẩn phổ RF dBFS -> dBm: đưa 1 nguồn tín hiệu đã biết công suất
    // (refDbm, đo tại đầu vào RX1) vào board, chờ phổ ổn định rồi gọi hàm này
    // - offset = refDbm - tổng công suất dBFS đang đo. Chỉ nhận khi phổ đang
    // là dữ liệu THẬT từ board (không hiệu chuẩn trên dữ liệu mô phỏng). Lưu
    // vào .ini kèm LO/gain lúc hiệu chuẩn (đổi LO/gain thì offset không còn
    // đúng -> RfController::calMismatch).
    //
    // Hiệu chuẩn NHIỀU TẦN SỐ (2026-10): các điểm tần số lấy từ config.ini
    // (rf/calPointsMHz, mặc định 2440,2445,2450,2455,2460). calibrateRf() lấy
    // tần số ĐỈNH phổ đang đo, gán vào điểm gần nhất (lệch <= RF_CAL_SNAP_MHZ),
    // lưu hệ số riêng cho điểm đó. Khi đo: nội suy tuyến tính theo tần số
    // đỉnh giữa 2 điểm đã hiệu chuẩn gần nhất (ngoài dải -> điểm biên).
    Q_INVOKABLE bool calibrateRf(double refDbm);
    Q_INVOKABLE void clearRfCalibration();              // xoá TẤT CẢ các điểm
    Q_INVOKABLE void clearRfCalPoint(double mhz);       // xoá 1 điểm

signals:
    void comPortChanged();
    void baudRateChanged();
    void delayTimeChanged();
    void sessionNameChanged();
    void isConnectedChanged();
    void availablePortsChanged();
    void commandLogChanged();
    void adcChannelsChanged();
    void isPortVerifiedChanged();
    void linkDiagnosisChanged();
    void boardDiagChanged();
    void scanningChanged();
    void kichNoPeakVoltageChanged();
    void stagedAcVoltChanged();
    void stagedAcFreqChanged();
    void stagedDcVoltChanged();
    void stagedDcCurrentChanged();
    void systemNotification(QString type, QString message);

private slots:
    void handleSerialError(QSerialPort::SerialPortError error);
    void generateSimulatedWave();
    void onAttUpdateRequested(int channel, double value);
    // Gửi 1 khung PING_STD tới địa chỉ board (VIP_NODE_ID_BOARD) - gọi định kỳ
    // trong khi cổng đang mở (xem m_pingTimer). Board thật (firmware đã có sẵn
    // auto-ACK cho PING_STD từ trước, không cần sửa firmware) sẽ trả ACK_STD;
    // nguồn AC/DC nối nhầm cổng qua cùng hub USB thì mở được cổng nhưng không
    // bao giờ trả lời đúng khung này.
    void sendPingProbe();
    // Hết thời gian chờ ACK cho lần PING gần nhất - đánh dấu chưa xác thực.
    void onPingTimeout();
    // Không nhận được gói "WAVE" nào trong WAVE_WATCHDOG_MS khi đang kết nối.
    void onWaveWatchdog();
    // Board không trả msgid 42 trong POWER_REPLY_TIMEOUT_MS.
    void onAcPowerTimeout();
    void onDcPowerTimeout();

private:
    void handleSerialRead();
    void parseIncomingData(const QByteArray &data);
    void writeToLog(const QString &entry);
    void appendCommandLog(const QString &tag, const QString &text);
    void openLogFileInternal();

    /* Wraps `msg` (already built by kria_msg_output_encode()) in a VIP
     * DATA_STD frame (SYNC + header + CRC16 + kria payload + CRC32) and
     * writes it to m_serialPort - this is the one place that talks to the
     * wire, so every command path (AC/DC/delay/attenuator/generic) stays in
     * sync with the framing the firmware's CmdDispatcher_HandleByte()
     * actually expects (see src/vip_helper.h / src/cmd_dispatcher.c). No-op
     * if the port isn't open. */
    void sendKriaFrame(const kria_message_t &msg);

    // Bắt đầu/dừng chu kỳ PING xác thực đúng board (xem sendPingProbe()).
    void startPingCycle();
    void stopPingCycle();

    // Phổ RF: biên độ |X_k| tuyến tính (đúng thứ tự bin FFT của firmware,
    // chưa fftshift) -> dBFS/dBm, fftshift, trục tần số, đỉnh, tổng công suất.
    void processWaveMagnitudes(const QVector<double> &mags, bool simulated);
    void handleRfStatus(const rf_status_msg_t &st);
    void handleBoardDiag(const board_diag_msg_t &d);
    void handlePwrRawReply(const pwr_raw_reply_msg_t &r);
    void sendRawSupply(uint8_t dev, const QString &text);

    // ── CLI (dashboardcli.cpp) ──
    void cliOut(const QString &text);              // dòng không có giờ
    void cliError(const QString &text);
    void cliHelp(const QString &topic);
    void cliStatus();
    void cliAc(const QStringList &a);
    void cliDc(const QStringList &a);
    void cliAtt(const QStringList &a);
    void cliDelay(const QStringList &a);
    void cliRead(const QStringList &a);
    void cliReset(const QStringList &a);
    void cliBoard(const QStringList &a, const QString &line);
    void cliMonitor(const QStringList &a);
    void cliPing();
    void startScan();
    void scanNext();
    void scanFinishCandidate(bool found);
    void scanStop(const QString &summary);
    void monitorTick();
    void handleBoardLog(uint8_t level, const QString &text);
    void updateLinkDiagnosis(bool notify);
    void updateRfDiagnosis();
    void updateCalibrationState();
    void resetRfState();

    // ── Nguồn chuẩn AC/DC: đọc ngược + xác nhận (msgid 41/42) ──
    void handlePwrStatus(const pwr_status_msg_t &st);
    void sendPwrQuery(uint8_t dev);
    void powerTimeout(bool isAc);

    // ── config.ini ──
    static QString resolveConfigPath();
    void migrateOldSettings();
    void saveSetting(const QString &key, const QVariant &value);
    static double loadOptionalDouble(const QSettings &s, const QString &key, double lo, double hi);

    ACsource *m_acSource;
    DCsource *m_dcSource;
    RfController *m_rfController;

    QSerialPort *m_serialPort;
    QTimer *m_simTimer;

    QString m_comPort;
    int m_baudRate;
    int m_delayTime;
    QString m_sessionName;
    QFile *m_logFile;
    QTextStream *m_logStream;
    QStringList m_availablePorts;
    QStringList m_commandLog;
    static constexpr  int MAX_LOG_LINES=200;
    QVariantList m_adcChannels;

    /* VIP outer-framing decode state for bytes coming from m_serialPort -
     * replaces the old raw kria_decode()/m_buffer line-splitting, which was
     * unsafe for a binary protocol (a payload byte could legitimately be
     * '\n' or contain no wire framing at all - see INTEGRATION_NOTES.md). */
    vip_msg_t    m_vipRx;
    vip_status_t m_vipStatus;

    // Xác thực đúng COM (xem sendPingProbe()/onPingTimeout()).
    bool    m_isPortVerified;
    QTimer *m_pingTimer;        // lặp lại, gửi 1 PING mỗi PING_INTERVAL_MS
    QTimer *m_pingTimeoutTimer; // one-shot, hết hạn = chưa/không còn thấy ACK
    static constexpr int PING_INTERVAL_MS = 2000;
    // 1.5 s: board có thể đang bận tối đa ~0.5 s (chờ 1 lần chụp DMA) trước
    // khi xử lý PING - 700 ms cũ dễ báo mất xác thực giả.
    static constexpr int PING_TIMEOUT_MS  = 1500;

    // ── Chẩn đoán đường truyền (đếm từ lúc mở cổng) ──
    quint64 m_rxBytes;
    quint64 m_rxFramesOk;
    quint64 m_rxFramesBad;
    QString m_linkDiagnosis;
    QString m_linkDiagnosisShort;
    QString m_linkDiagNotified;     // loại chẩn đoán đã thông báo trong phiên
    QTimer *m_linkDiagTimer;        // 1 s, chỉ chạy khi đã mở cổng
    int     m_unverifiedSeconds;

    // ── Gói DIAG của board ──
    QString     m_boardFirmware;
    QVariantMap m_boardDiag;
    bool        m_haveDiag;
    board_diag_msg_t m_lastDiag;
    bool        m_notifiedAd9361Fail;
    bool        m_notifiedDmaDead;
    bool        m_notifiedClip;

    // ── CLI state ──
    bool          m_cliPingPending;
    QElapsedTimer m_cliPingClock;
    bool          m_resetArmed;
    QElapsedTimer m_resetArmClock;
    bool          m_monitor;
    QTimer       *m_monitorTimer;
    quint64       m_monBytes, m_monOk, m_monBad;
    QMap<QString, int> m_monTypes;
    QByteArray    m_monLastChunk;

    // ── scan (tự tìm cổng + baud) ──
    bool          m_scanning;
    QSerialPort  *m_scanPort;
    QTimer       *m_scanTimer;
    QStringList   m_scanPorts;
    QList<int>    m_scanBauds;
    int           m_scanPortIdx, m_scanBaudIdx;
    vip_status_t  m_scanVipStatus;
    vip_msg_t     m_scanVipRx;
    quint64       m_scanBytes;
    QStringList   m_scanNotes;

    // Kích nổ - bắt đỉnh phần mềm (xem parseIncomingData() msgid 40).
    double m_kichNoPeakV;

    // Staged Power UI values - xem comment Q_PROPERTY phía trên. NaN = "--".
    double m_stagedAcVolt;
    double m_stagedAcFreq;
    double m_stagedDcVolt;
    double m_stagedDcCurrent;
    // ── RF spectrum ──
    QTimer *m_waveWatchdog;                      // single-shot
    static constexpr int WAVE_WATCHDOG_MS = 2000; // firmware gửi ~300 ms/lần
    bool     m_waveSeenThisSession;
    bool     m_rfStatusValid;
    quint64  m_rxLoHz;
    quint32  m_rxFsHz;
    int      m_rxGainDb;
    double   m_lastTotalDbfs;
    bool     m_lastSpectrumLive;
    // ── Hiệu chuẩn nhiều tần số ──
    struct RfCalPoint {
        double  mhz;        // tần số điểm hiệu chuẩn
        bool    valid;      // đã hiệu chuẩn chưa
        double  offsetDb;   // dBm = dBFS + offsetDb
        quint64 loHz;       // LO RX lúc hiệu chuẩn
        int     gainDb;     // gain RX1 lúc hiệu chuẩn
        QString when;       // thời điểm hiệu chuẩn
    };
    QVector<RfCalPoint> m_calPoints;
    bool     m_calibrated;        // có ít nhất 1 điểm hợp lệ
    double   m_calOffsetDb;       // hệ số đang áp dụng cho phổ hiện tại
    double   m_calFreqMHz;        // tần số dùng để tính hệ số (NaN = chưa thấy tín hiệu)
    double   m_lastPeakMHz;       // tần số đỉnh của khung phổ gần nhất
    double   m_lastPeakProminenceDb; // đỉnh cao hơn nền (trung vị) bao nhiêu dB
    bool     m_lastHaveFreq;
    // Đỉnh phải lệch điểm hiệu chuẩn không quá giá trị này mới được gán.
    static constexpr double RF_CAL_SNAP_MHZ = 2.5;
    // Đỉnh lệch 1 điểm đã hiệu chuẩn không quá giá trị này (~nửa bin
    // 0.48 MHz ở 60 MSPS/124 điểm) thì dùng đúng hệ số điểm đó.
    static constexpr double RF_CAL_EXACT_MHZ = 0.3;
    // Đỉnh phải cao hơn nền nhiễu (trung vị các bin) ít nhất ngần này mới
    // được coi là có tín hiệu (dùng để hiệu chuẩn / chọn hệ số).
    static constexpr double RF_CAL_MIN_PROMINENCE_DB = 15.0;
    void   loadRfCalibration();
    void   saveRfCalPoint(int idx);
    static QString rfCalKey(double mhz);
    double rfCalOffsetAt(double mhz, int *loIdx, int *hiIdx) const;
    int    rfCalNearestPoint(double mhz) const;
    void   cliCal(const QStringList &a);
    int      m_simSpectrumTick;
    // AD9361 ADC 12-bit, axi_adc sign-extend lên 16-bit: biên độ đầy thang
    // của 1 thành phần I/Q = 2048 -> sóng sin phức đầy thang qua FFT N điểm
    // cho |X_k| = 2048*N. Đây là mốc 0 dBFS.
    static constexpr double RF_ADC_FULL_SCALE = 2048.0;

    // ── ACK nguồn AC/DC: trạng thái yêu cầu vs trạng thái trước đó ──
    bool m_acPendingOn, m_acPrevOn;
    bool m_dcPendingOn, m_dcPrevOn;
    // Đã nhận msgid 42 cho lệnh đặt đang chờ chưa (firmware mới luôn gửi 42
    // TRƯỚC ACK; ACK tới mà chưa có 42 = firmware cũ, không xác nhận được).
    bool m_acGotPws, m_dcGotPws;
    // Đang chờ kết quả cho: 0 không chờ, 1 đọc (refresh), 2 lệnh đặt, 3 RESET
    int  m_acWaitKind, m_dcWaitKind;
    QTimer *m_acPowerTimer;     // single-shot
    QTimer *m_dcPowerTimer;
    // Trường hợp xấu nhất phía firmware (xả hàng lỗi SCPI + 5 câu hỏi, mỗi
    // câu timeout 500 ms) vẫn < 4 s; 8 s để không báo động giả.
    static constexpr int POWER_REPLY_TIMEOUT_MS = 8000;

    static constexpr double ADC7928_VREF_VOLTS = 2.5;      // board_config.h
    static constexpr double ADC7928_FULL_SCALE = ADC7928_VREF_VOLTS * 2.0; // RANGE 0..2*VREF

    // config.ini - xem resolveConfigPath(). m_configPath PHẢI khai báo trước
    // m_settings (thứ tự khởi tạo thành viên).
    QString   m_configPath;
    QSettings m_settings;
};

#endif // DASHBOARDMANAGER_H