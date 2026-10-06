#ifndef BASEPOWERSOURCE_H
#define BASEPOWERSOURCE_H

#include <QObject>
#include <QVariantList>
#include <QPointF>
#include <QDateTime>
#include <QString>

// Một nguồn chuẩn (AC hoặc DC) như app nhìn thấy:
//  - voltage/current + lịch sử: CHỈ là số ĐO đọc ngược từ chính nguồn
//    (firmware hỏi MEAS:VOLT?/MEAS:CURR? rồi gửi lên bằng msgid 42) - không
//    bao giờ là giá trị đặt. Chưa đọc lần nào -> hasData=false, UI hiện "--".
//  - isOn: trạng thái ngõ ra. Sau khi bấm công tắc hiển thị trạng thái YÊU
//    CẦU trong lúc chờ xác nhận (pending=true), rồi theo đúng trạng thái
//    nguồn xác nhận (OUTP?) hoặc quay lại trạng thái cũ nếu thất bại.
//  - linkState/linkMessage/lastUpdate: kết quả lần trao đổi gần nhất với
//    nguồn, để vận hành viên biết số trên màn hình còn đáng tin hay không.
class BasePowerSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged FINAL)
    Q_PROPERTY(double current READ current NOTIFY currentChanged FINAL)
    Q_PROPERTY(bool hasData READ hasData NOTIFY hasDataChanged FINAL)
    Q_PROPERTY(bool isOn READ isOn WRITE setIsOn NOTIFY isOnChanged FINAL)
    Q_PROPERTY(QVariantList voltageHistory READ voltageHistory NOTIFY voltageHistoryChanged FINAL)
    Q_PROPERTY(QVariantList currentHistory READ currentHistory NOTIFY currentHistoryChanged FINAL)
    Q_PROPERTY(double voltageMax READ voltageMax  NOTIFY voltageMaxChanged FINAL)
    Q_PROPERTY(double voltageMin READ voltageMin NOTIFY voltageMinChanged FINAL)
    Q_PROPERTY(double currentMax READ currentMax  NOTIFY currentMaxChanged FINAL)
    Q_PROPERTY(double currentMin READ currentMin  NOTIFY currentMinChanged FINAL)

    // "none" chưa đọc | "ok" | "pending" đang chờ | "noreply" | "error" |
    // "mismatch" | "badreply" | "notready" | "boardnoreply"
    Q_PROPERTY(QString linkState READ linkState NOTIFY linkChanged FINAL)
    Q_PROPERTY(QString linkMessage READ linkMessage NOTIFY linkChanged FINAL)
    Q_PROPERTY(bool pending READ pending NOTIFY linkChanged FINAL)
    // "hh:mm:ss" của lần đọc thành công gần nhất, "" nếu chưa có.
    Q_PROPERTY(QString lastUpdate READ lastUpdate NOTIFY linkChanged FINAL)

public:
    static constexpr int HISTORY_SIZE = 100;
    static constexpr double NO_MAX = -1e12;     // max < min -> UI hiện "--"
    static constexpr double NO_MIN =  1e12;

    explicit BasePowerSource(QObject *parent = nullptr);

    double voltage() const { return m_voltage; }
    double current() const { return m_current; }
    bool   hasData() const { return m_hasData; }
    bool   isOn() const { return m_isOn; }
    QVariantList voltageHistory() const { return m_voltageHistory; }
    QVariantList currentHistory() const { return m_currentHistory; }
    double voltageMax() const { return m_voltageMax; }
    double voltageMin() const { return m_voltageMin; }
    double currentMax() const { return m_currentMax; }
    double currentMin() const { return m_currentMin; }
    QString linkState() const { return m_linkState; }
    QString linkMessage() const { return m_linkMessage; }
    bool    pending() const { return m_linkState == QLatin1String("pending"); }
    QString lastUpdate() const { return m_lastUpdate; }

    void setIsOn(bool on);
    void setLink(const QString &state, const QString &message);

    // 1 lần đọc thật từ nguồn. vValid/iValid: trường nào nguồn thực sự trả.
    void addSample(const QDateTime &t, bool vValid, double v, bool iValid, double i);

    Q_INVOKABLE virtual void resetHistory();

signals:
    void voltageChanged();
    void currentChanged();
    void hasDataChanged();
    void isOnChanged();
    void voltageHistoryChanged();
    void currentHistoryChanged();
    void voltageMaxChanged();
    void voltageMinChanged();
    void currentMaxChanged();
    void currentMinChanged();
    void linkChanged();

protected:
    // x = giây kể từ mẫu đầu tiên của lịch sử hiện tại (đọc theo nút bấm nên
    // khoảng cách giữa các điểm không đều - trục X theo thời gian thật).
    double sampleX(const QDateTime &t);
    static void appendPoint(QVariantList &list, double x, double y, double &maxVal, double &minVal);

    double m_voltage;
    double m_current;
    bool   m_hasData;
    bool   m_isOn;
    double m_voltageMax, m_voltageMin;
    double m_currentMax, m_currentMin;
    QVariantList m_voltageHistory;
    QVariantList m_currentHistory;
    QDateTime m_t0;
    QString m_linkState;
    QString m_linkMessage;
    QString m_lastUpdate;
};

#endif // BASEPOWERSOURCE_H
