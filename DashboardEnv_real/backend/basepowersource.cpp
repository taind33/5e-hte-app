#include "basepowersource.h"
#include <cmath>

BasePowerSource::BasePowerSource(QObject *parent)
    : QObject(parent),
    m_voltage(0.0), m_current(0.0), m_hasData(false), m_isOn(false),
    m_voltageMax(NO_MAX), m_voltageMin(NO_MIN),
    m_currentMax(NO_MAX), m_currentMin(NO_MIN),
    m_linkState(QStringLiteral("none"))
{
}

void BasePowerSource::setIsOn(bool on)
{
    if (m_isOn == on)
        return;
    m_isOn = on;
    emit isOnChanged();
}

void BasePowerSource::setLink(const QString &state, const QString &message)
{
    if (m_linkState == state && m_linkMessage == message)
        return;
    m_linkState = state;
    m_linkMessage = message;
    emit linkChanged();
}

double BasePowerSource::sampleX(const QDateTime &t)
{
    if (!m_t0.isValid())
        m_t0 = t;
    return static_cast<double>(m_t0.msecsTo(t)) / 1000.0;
}

void BasePowerSource::appendPoint(QVariantList &list, double x, double y, double &maxVal, double &minVal)
{
    list.append(QVariant::fromValue(QPointF(x, y)));
    while (list.size() > HISTORY_SIZE)
        list.removeFirst();
    // Max/Min theo đúng những điểm đang hiển thị (cửa sổ HISTORY_SIZE).
    maxVal = NO_MAX;
    minVal = NO_MIN;
    for (const QVariant &p : std::as_const(list)) {
        const double v = p.toPointF().y();
        if (v > maxVal) maxVal = v;
        if (v < minVal) minVal = v;
    }
}

void BasePowerSource::addSample(const QDateTime &t, bool vValid, double v, bool iValid, double i)
{
    if (!vValid && !iValid)
        return;
    const double x = sampleX(t);
    if (vValid) {
        m_voltage = v;
        appendPoint(m_voltageHistory, x, v, m_voltageMax, m_voltageMin);
        emit voltageChanged();
        emit voltageHistoryChanged();
        emit voltageMaxChanged();
        emit voltageMinChanged();
    }
    if (iValid) {
        m_current = i;
        appendPoint(m_currentHistory, x, i, m_currentMax, m_currentMin);
        emit currentChanged();
        emit currentHistoryChanged();
        emit currentMaxChanged();
        emit currentMinChanged();
    }
    if (!m_hasData) {
        m_hasData = true;
        emit hasDataChanged();
    }
    m_lastUpdate = t.toString(QStringLiteral("HH:mm:ss"));
    emit linkChanged();
}

void BasePowerSource::resetHistory()
{
    m_voltageHistory.clear();
    m_currentHistory.clear();
    m_t0 = QDateTime();
    m_voltageMax = NO_MAX;
    m_voltageMin = NO_MIN;
    m_currentMax = NO_MAX;
    m_currentMin = NO_MIN;
    m_voltage = 0.0;
    m_current = 0.0;
    if (m_hasData) {
        m_hasData = false;
        emit hasDataChanged();
    }
    m_lastUpdate.clear();
    emit voltageChanged();
    emit currentChanged();
    emit voltageHistoryChanged();
    emit currentHistoryChanged();
    emit voltageMaxChanged();
    emit voltageMinChanged();
    emit currentMaxChanged();
    emit currentMinChanged();
    emit linkChanged();
}
