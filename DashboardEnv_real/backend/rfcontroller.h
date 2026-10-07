#ifndef RFCONTROLLER_H
#define RFCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QVariantList>
#include <QString>

// Mô hình dữ liệu RF cho QML: 3 kênh suy hao F1956 + phổ RF (RX1 của
// AD9361) nhận từ firmware. Toàn bộ xử lý số (dBFS, fftshift, đỉnh, tổng
// công suất, hiệu chuẩn) nằm ở DashboardManager::processWaveMagnitudes() -
// lớp này chỉ giữ kết quả để QML hiển thị.
class RfController : public QObject {
    Q_OBJECT
    // F1956 thật: bước 0.25dB, 0..31.75 dB - giao thức dây (msgid 38,
    // annuation_q_msg_t) truyền đúng số bước 0.25dB, không làm tròn về dB
    // nguyên nữa (xem DashboardManager::onAttUpdateRequested()).
    Q_PROPERTY(double att1 READ att1 WRITE setAtt1 NOTIFY att1Changed)
    Q_PROPERTY(double att2 READ att2 WRITE setAtt2 NOTIFY att2Changed)
    Q_PROPERTY(double att3 READ att3 WRITE setAtt3 NOTIFY att3Changed)

    // ── Phổ RF ─────────────────────────────────────────────────────────
    // waveData: danh sách QPointF(x = tần số MHz tuyệt đối nếu đã biết LO/Fs
    // từ gói "RFST", ngược lại = chỉ số bin lệch tâm; y = mức theo spectrumUnit).
    Q_PROPERTY(QVariantList waveData READ waveData NOTIFY waveDataChanged)
    Q_PROPERTY(double xMin READ xMin NOTIFY waveDataChanged)
    Q_PROPERTY(double xMax READ xMax NOTIFY waveDataChanged)
    Q_PROPERTY(double yMin READ yMin NOTIFY waveDataChanged)
    Q_PROPERTY(double yMax READ yMax NOTIFY waveDataChanged)
    Q_PROPERTY(bool   xIsFrequency READ xIsFrequency NOTIFY waveDataChanged)
    Q_PROPERTY(double peakLevel READ peakLevel NOTIFY waveDataChanged)
    Q_PROPERTY(double peakX READ peakX NOTIFY waveDataChanged)
    Q_PROPERTY(double totalLevel READ totalLevel NOTIFY waveDataChanged)
    Q_PROPERTY(double totalDbfs READ totalDbfs NOTIFY waveDataChanged)
    // "live" = phổ thật từ board đang về (watchdog 2 s), "sim" = dữ liệu mô
    // phỏng khi chưa kết nối, "waiting" = đã kết nối nhưng chưa có/mất WAVE.
    Q_PROPERTY(QString waveSource READ waveSource NOTIFY waveSourceChanged)
    Q_PROPERTY(QString spectrumUnit READ spectrumUnit NOTIFY calibrationChanged)

    // ── Cấu hình RX AD9361 (gói "RFST" từ firmware) ─────────────────────
    Q_PROPERTY(bool   rfStatusValid READ rfStatusValid NOTIFY rfStatusChanged)
    Q_PROPERTY(double rxLoMHz READ rxLoMHz NOTIFY rfStatusChanged)
    Q_PROPERTY(double rxFsMHz READ rxFsMHz NOTIFY rfStatusChanged)
    Q_PROPERTY(double rxBwMHz READ rxBwMHz NOTIFY rfStatusChanged)
    Q_PROPERTY(int    rxGainDb READ rxGainDb NOTIFY rfStatusChanged)
    Q_PROPERTY(QString rxGainMode READ rxGainMode NOTIFY rfStatusChanged)
    Q_PROPERTY(int    fftSize READ fftSize NOTIFY rfStatusChanged)

    // ── Hiệu chuẩn dBFS -> dBm ──────────────────────────────────────────
    Q_PROPERTY(bool   calibrated READ calibrated NOTIFY calibrationChanged)
    Q_PROPERTY(double calOffsetDb READ calOffsetDb NOTIFY calibrationChanged)
    // true khi LO/gain hiện tại khác lúc hiệu chuẩn -> số dBm không còn đúng.
    Q_PROPERTY(bool   calMismatch READ calMismatch NOTIFY calibrationChanged)
    // Bảng hiệu chuẩn nhiều tần số: mỗi phần tử là map
    // {mhz, label, valid, offsetDb, mismatch, active}. active = điểm đang
    // được dùng (hoặc 1 trong 2 điểm đang nội suy).
    Q_PROPERTY(QVariantList calPoints READ calPoints NOTIFY calibrationChanged)
    // Dòng trạng thái, vd "Offset 26.6 dB @ 2452.4 MHz (nội suy 2450–2455)".
    Q_PROPERTY(QString calInfo READ calInfo NOTIFY calibrationChanged)

    // Vì sao chưa/không có phổ, theo gói "DIAG" của board (AD9361 lỗi, DMA
    // không về dữ liệu, ADC bão hoà...). "" = không có gì để báo.
    // diagLevel: "" | "warn" | "error".
    Q_PROPERTY(QString diagText READ diagText NOTIFY diagChanged)
    Q_PROPERTY(QString diagLevel READ diagLevel NOTIFY diagChanged)

public:
    explicit RfController(QObject *parent = nullptr);

    double att1() const { return m_att1; }
    double att2() const { return m_att2; }
    double att3() const { return m_att3; }

    // NaN = chưa đặt ("--"). So sánh tay vì qFuzzyCompare không xử lý NaN.
    static bool sameAtt(double a, double b) {
        if (a != a || b != b) return (a != a) && (b != b);
        return qFuzzyCompare(a + 1.0, b + 1.0);
    }
    void setAtt1(double val) { if (!sameAtt(m_att1, val)) { m_att1 = val; emit att1Changed(); } }
    void setAtt2(double val) { if (!sameAtt(m_att2, val)) { m_att2 = val; emit att2Changed(); } }
    void setAtt3(double val) { if (!sameAtt(m_att3, val)) { m_att3 = val; emit att3Changed(); } }

    QVariantList waveData() const { return m_waveData; }
    double xMin() const { return m_xMin; }
    double xMax() const { return m_xMax; }
    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }
    bool   xIsFrequency() const { return m_xIsFrequency; }
    double peakLevel() const { return m_peakLevel; }
    double peakX() const { return m_peakX; }
    double totalLevel() const { return m_totalLevel; }
    double totalDbfs() const { return m_totalDbfs; }
    QString waveSource() const { return m_waveSource; }
    QString spectrumUnit() const { return m_calibrated ? QStringLiteral("dBm") : QStringLiteral("dBFS"); }

    bool   rfStatusValid() const { return m_rfStatusValid; }
    double rxLoMHz() const { return m_rxLoMHz; }
    double rxFsMHz() const { return m_rxFsMHz; }
    double rxBwMHz() const { return m_rxBwMHz; }
    int    rxGainDb() const { return m_rxGainDb; }
    QString rxGainMode() const { return m_rxGainMode; }
    int    fftSize() const { return m_fftSize; }

    bool   calibrated() const { return m_calibrated; }
    double calOffsetDb() const { return m_calOffsetDb; }
    bool   calMismatch() const { return m_calMismatch; }
    QVariantList calPoints() const { return m_calPoints; }
    QString calInfo() const { return m_calInfo; }
    QString diagText() const { return m_diagText; }
    QString diagLevel() const { return m_diagLevel; }
    void setDiag(const QString &text, const QString &level) {
        if (m_diagText == text && m_diagLevel == level) return;
        m_diagText = text; m_diagLevel = level; emit diagChanged();
    }

    // Dùng bởi DashboardManager.
    void setSpectrum(const QVector<QPointF> &points, bool xIsFrequency,
                     double xMin, double xMax, double yMin, double yMax,
                     double peakLevel, double peakX, double totalLevel, double totalDbfs);
    void clearSpectrum();
    void setWaveSource(const QString &src);
    void setRfStatus(bool valid, double loMHz, double fsMHz, double bwMHz,
                     int gainDb, const QString &gainMode, int fftSize);
    void setCalibration(bool calibrated, double offsetDb, bool mismatch,
                        const QVariantList &points, const QString &info);

    // Bấm gửi trên giao diện = lệnh tường minh: LUÔN phát attUpdateRequested
    // khi force=true (nút gửi), kể cả khi giá trị không đổi - trước đây giá
    // trị nạp lại từ .ini lúc mở app không bao giờ gửi xuống board được nếu
    // người dùng không sửa số (requestSetAtt chỉ gửi khi "changed").
    Q_INVOKABLE bool requestSetAtt(int channel, double value, bool force = false);

signals:
    void att1Changed();
    void att2Changed();
    void att3Changed();
    void waveDataChanged();
    void waveSourceChanged();
    void rfStatusChanged();
    void calibrationChanged();
    void diagChanged();
    void attUpdateRequested(int channel, double value);

private:
    double m_att1, m_att2, m_att3;

    QVariantList m_waveData;
    double  m_xMin, m_xMax, m_yMin, m_yMax;
    bool    m_xIsFrequency;
    double  m_peakLevel, m_peakX, m_totalLevel, m_totalDbfs;
    QString m_waveSource;

    bool    m_rfStatusValid;
    double  m_rxLoMHz, m_rxFsMHz, m_rxBwMHz;
    int     m_rxGainDb;
    QString m_rxGainMode;
    int     m_fftSize;

    bool    m_calibrated;
    double  m_calOffsetDb;
    bool    m_calMismatch;
    QVariantList m_calPoints;
    QString m_calInfo;
    QString m_diagText;
    QString m_diagLevel;
};

#endif // RFCONTROLLER_H
