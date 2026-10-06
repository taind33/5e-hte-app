#ifndef ACSOURCE_H
#define ACSOURCE_H

#include <backend/basepowersource.h>

// Nguồn AC (ITECH IT-M7700): thêm tần số ĐO (MEAS:FREQ?) - đồ thị thứ 2 của
// nguồn AC trên giao diện.
class ACsource : public BasePowerSource
{
    Q_OBJECT
    Q_PROPERTY(double freq READ freq NOTIFY freqChanged FINAL)
    Q_PROPERTY(QVariantList freqHistory READ freqHistory NOTIFY freqHistoryChanged FINAL)
    Q_PROPERTY(double freqMax READ freqMax NOTIFY freqMaxChanged FINAL)
    Q_PROPERTY(double freqMin READ freqMin NOTIFY freqMinChanged FINAL)

public:
    explicit ACsource(QObject *parent = nullptr)
        : BasePowerSource(parent), m_freq(0.0), m_freqMax(NO_MAX), m_freqMin(NO_MIN) {}

    double freq() const { return m_freq; }
    QVariantList freqHistory() const { return m_freqHistory; }
    double freqMax() const { return m_freqMax; }
    double freqMin() const { return m_freqMin; }

    void addAcSample(const QDateTime &t, bool vValid, double v, bool iValid, double i, bool fValid, double f);
    void resetHistory() override;

signals:
    void freqChanged();
    void freqHistoryChanged();
    void freqMaxChanged();
    void freqMinChanged();

private:
    double m_freq;
    double m_freqMax;
    double m_freqMin;
    QVariantList m_freqHistory;
};

#endif // ACSOURCE_H
