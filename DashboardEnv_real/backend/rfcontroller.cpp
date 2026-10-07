#include "rfcontroller.h"
#include <cmath>
#include <limits>

RfController::RfController(QObject *parent)
    : QObject(parent),
    // NaN = chưa đặt (ô ATT hiện "--") cho tới khi nạp từ config.ini/người dùng nhập.
    m_att1(std::numeric_limits<double>::quiet_NaN()),
    m_att2(std::numeric_limits<double>::quiet_NaN()),
    m_att3(std::numeric_limits<double>::quiet_NaN()),
    m_xMin(-16.0), m_xMax(15.0), m_yMin(-100.0), m_yMax(0.0),
    m_xIsFrequency(false),
    m_peakLevel(0.0), m_peakX(0.0), m_totalLevel(0.0), m_totalDbfs(0.0),
    m_waveSource(QStringLiteral("sim")),
    m_rfStatusValid(false),
    m_rxLoMHz(0.0), m_rxFsMHz(0.0), m_rxBwMHz(0.0),
    m_rxGainDb(0), m_fftSize(0),
    m_calibrated(false), m_calOffsetDb(0.0), m_calMismatch(false)
{
}

void RfController::setSpectrum(const QVector<QPointF> &points, bool xIsFrequency,
                               double xMin, double xMax, double yMin, double yMax,
                               double peakLevel, double peakX, double totalLevel, double totalDbfs)
{
    m_waveData.clear();
    m_waveData.reserve(points.size());
    for (const QPointF &p : points)
        m_waveData.append(QVariant::fromValue(p));
    m_xIsFrequency = xIsFrequency;
    m_xMin = xMin;
    m_xMax = xMax;
    m_yMin = yMin;
    m_yMax = yMax;
    m_peakLevel  = peakLevel;
    m_peakX      = peakX;
    m_totalLevel = totalLevel;
    m_totalDbfs  = totalDbfs;
    emit waveDataChanged();
}

void RfController::clearSpectrum()
{
    m_waveData.clear();
    emit waveDataChanged();
}

void RfController::setWaveSource(const QString &src)
{
    if (m_waveSource != src) {
        m_waveSource = src;
        emit waveSourceChanged();
    }
}

void RfController::setRfStatus(bool valid, double loMHz, double fsMHz, double bwMHz,
                               int gainDb, const QString &gainMode, int fftSize)
{
    m_rfStatusValid = valid;
    m_rxLoMHz  = loMHz;
    m_rxFsMHz  = fsMHz;
    m_rxBwMHz  = bwMHz;
    m_rxGainDb = gainDb;
    m_rxGainMode = gainMode;
    m_fftSize  = fftSize;
    emit rfStatusChanged();
}

void RfController::setCalibration(bool calibrated, double offsetDb, bool mismatch,
                                  const QVariantList &points, const QString &info)
{
    if (m_calibrated == calibrated && qFuzzyCompare(m_calOffsetDb + 1.0, offsetDb + 1.0)
        && m_calMismatch == mismatch && m_calPoints == points && m_calInfo == info)
        return;
    m_calibrated  = calibrated;
    m_calOffsetDb = offsetDb;
    m_calMismatch = mismatch;
    m_calPoints   = points;
    m_calInfo     = info;
    emit calibrationChanged();
}

bool RfController::requestSetAtt(int channel, double value, bool force)
{
    bool changed = false;
    if (std::isnan(value))
        return false;           // ô đang "--": không có gì để gửi
    switch (channel) {
    case 1: if (!sameAtt(m_att1, value)) { m_att1 = value; emit att1Changed(); changed = true; } break;
    case 2: if (!sameAtt(m_att2, value)) { m_att2 = value; emit att2Changed(); changed = true; } break;
    case 3: if (!sameAtt(m_att3, value)) { m_att3 = value; emit att3Changed(); changed = true; } break;
    default: return false;
    }
    if (changed || force)
        emit attUpdateRequested(channel, value);
    return changed || force;
}
