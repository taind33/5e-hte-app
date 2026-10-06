#include "acsource.h"

void ACsource::addAcSample(const QDateTime &t, bool vValid, double v, bool iValid, double i, bool fValid, double f)
{
    if (fValid) {
        // sampleX() trước addSample(): cùng 1 lần đọc -> cùng toạ độ X cho
        // cả 3 đồ thị (áp, dòng, tần số).
        const double x = sampleX(t);
        m_freq = f;
        appendPoint(m_freqHistory, x, f, m_freqMax, m_freqMin);
        emit freqChanged();
        emit freqHistoryChanged();
        emit freqMaxChanged();
        emit freqMinChanged();
    }
    if (vValid || iValid) {
        addSample(t, vValid, v, iValid, i);
    } else if (fValid) {
        if (!m_hasData) {
            m_hasData = true;
            emit hasDataChanged();
        }
        m_lastUpdate = t.toString(QStringLiteral("HH:mm:ss"));
        emit linkChanged();
    }
}

void ACsource::resetHistory()
{
    m_freqHistory.clear();
    m_freq = 0.0;
    m_freqMax = NO_MAX;
    m_freqMin = NO_MIN;
    emit freqChanged();
    emit freqHistoryChanged();
    emit freqMaxChanged();
    emit freqMinChanged();
    BasePowerSource::resetHistory();
}
