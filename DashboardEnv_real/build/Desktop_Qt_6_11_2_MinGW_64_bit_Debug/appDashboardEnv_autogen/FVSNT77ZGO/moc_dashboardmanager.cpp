/****************************************************************************
** Meta object code from reading C++ file 'dashboardmanager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/dashboardmanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dashboardmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16DashboardManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto DashboardManager::qt_create_metaobjectdata<qt_meta_tag_ZN16DashboardManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DashboardManager",
        "comPortChanged",
        "",
        "baudRateChanged",
        "delayTimeChanged",
        "sessionNameChanged",
        "isConnectedChanged",
        "availablePortsChanged",
        "commandLogChanged",
        "adcChannelsChanged",
        "isPortVerifiedChanged",
        "linkDiagnosisChanged",
        "boardDiagChanged",
        "scanningChanged",
        "kichNoPeakVoltageChanged",
        "stagedAcVoltChanged",
        "stagedAcFreqChanged",
        "stagedDcVoltChanged",
        "stagedDcCurrentChanged",
        "systemNotification",
        "type",
        "message",
        "handleSerialError",
        "QSerialPort::SerialPortError",
        "error",
        "generateSimulatedWave",
        "onAttUpdateRequested",
        "channel",
        "value",
        "sendPingProbe",
        "onPingTimeout",
        "onWaveWatchdog",
        "onAcPowerTimeout",
        "onDcPowerTimeout",
        "startSession",
        "stopSession",
        "openLogFile",
        "requestAcPower",
        "on",
        "requestDcPower",
        "setAcPower",
        "uint16_t",
        "volt",
        "freq",
        "setDcPower",
        "cur",
        "refreshPower",
        "dev",
        "refreshAllPower",
        "loadUi",
        "QVariant",
        "key",
        "defaultValue",
        "saveUi",
        "applyDelayTime",
        "ns",
        "refreshPorts",
        "isPortAvailable",
        "portName",
        "resetSystem",
        "sendCommand",
        "cmd",
        "executeCommand",
        "line",
        "cliCompletions",
        "prefix",
        "clearCommandLog",
        "calibrateRf",
        "refDbm",
        "clearRfCalibration",
        "clearRfCalPoint",
        "mhz",
        "acSource",
        "ACsource*",
        "dcSource",
        "DCsource*",
        "rfController",
        "RfController*",
        "comPort",
        "baudRate",
        "delayTime",
        "sessionName",
        "isConnected",
        "availablePorts",
        "commandLog",
        "adcChannels",
        "QVariantList",
        "configPath",
        "isPortVerified",
        "linkDiagnosis",
        "linkDiagnosisShort",
        "boardFirmware",
        "boardDiag",
        "QVariantMap",
        "scanning",
        "kichNoPeakVoltage",
        "acVoltMin",
        "acVoltMax",
        "acFreqMin",
        "acFreqMax",
        "dcVoltMin",
        "dcVoltMax",
        "dcCurrentMin",
        "dcCurrentMax",
        "attMax",
        "stagedAcVolt",
        "stagedAcFreq",
        "stagedDcVolt",
        "stagedDcCurrent"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'comPortChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'baudRateChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'delayTimeChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sessionNameChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isConnectedChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'availablePortsChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'commandLogChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'adcChannelsChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isPortVerifiedChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'linkDiagnosisChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'boardDiagChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scanningChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'kichNoPeakVoltageChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stagedAcVoltChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stagedAcFreqChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stagedDcVoltChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stagedDcCurrentChanged'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'systemNotification'
        QtMocHelpers::SignalData<void(QString, QString)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 20 }, { QMetaType::QString, 21 },
        }}),
        // Slot 'handleSerialError'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
        // Slot 'generateSimulatedWave'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAttUpdateRequested'
        QtMocHelpers::SlotData<void(int, double)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 27 }, { QMetaType::Double, 28 },
        }}),
        // Slot 'sendPingProbe'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPingTimeout'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onWaveWatchdog'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAcPowerTimeout'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDcPowerTimeout'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'startSession'
        QtMocHelpers::MethodData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stopSession'
        QtMocHelpers::MethodData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openLogFile'
        QtMocHelpers::MethodData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'requestAcPower'
        QtMocHelpers::MethodData<bool(bool)>(37, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Bool, 38 },
        }}),
        // Method 'requestDcPower'
        QtMocHelpers::MethodData<bool(bool)>(39, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Bool, 38 },
        }}),
        // Method 'setAcPower'
        QtMocHelpers::MethodData<void(bool, uint16_t, uint16_t)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 38 }, { 0x80000000 | 41, 42 }, { 0x80000000 | 41, 43 },
        }}),
        // Method 'setDcPower'
        QtMocHelpers::MethodData<void(bool, uint16_t, double)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 38 }, { 0x80000000 | 41, 42 }, { QMetaType::Double, 45 },
        }}),
        // Method 'refreshPower'
        QtMocHelpers::MethodData<void(int)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 47 },
        }}),
        // Method 'refreshAllPower'
        QtMocHelpers::MethodData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'loadUi'
        QtMocHelpers::MethodData<QVariant(const QString &, const QVariant &) const>(49, 2, QMC::AccessPublic, 0x80000000 | 50, {{
            { QMetaType::QString, 51 }, { 0x80000000 | 50, 52 },
        }}),
        // Method 'loadUi'
        QtMocHelpers::MethodData<QVariant(const QString &) const>(49, 2, QMC::AccessPublic | QMC::MethodCloned, 0x80000000 | 50, {{
            { QMetaType::QString, 51 },
        }}),
        // Method 'saveUi'
        QtMocHelpers::MethodData<void(const QString &, const QVariant &)>(53, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 51 }, { 0x80000000 | 50, 28 },
        }}),
        // Method 'applyDelayTime'
        QtMocHelpers::MethodData<void(int)>(54, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 55 },
        }}),
        // Method 'refreshPorts'
        QtMocHelpers::MethodData<void()>(56, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'isPortAvailable'
        QtMocHelpers::MethodData<bool(const QString &) const>(57, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 58 },
        }}),
        // Method 'resetSystem'
        QtMocHelpers::MethodData<void()>(59, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'sendCommand'
        QtMocHelpers::MethodData<void(const QString &)>(60, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 61 },
        }}),
        // Method 'executeCommand'
        QtMocHelpers::MethodData<void(const QString &)>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 63 },
        }}),
        // Method 'cliCompletions'
        QtMocHelpers::MethodData<QStringList(const QString &) const>(64, 2, QMC::AccessPublic, QMetaType::QStringList, {{
            { QMetaType::QString, 65 },
        }}),
        // Method 'clearCommandLog'
        QtMocHelpers::MethodData<void()>(66, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'calibrateRf'
        QtMocHelpers::MethodData<bool(double)>(67, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Double, 68 },
        }}),
        // Method 'clearRfCalibration'
        QtMocHelpers::MethodData<void()>(69, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearRfCalPoint'
        QtMocHelpers::MethodData<void(double)>(70, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 71 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'acSource'
        QtMocHelpers::PropertyData<ACsource*>(72, 0x80000000 | 73, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'dcSource'
        QtMocHelpers::PropertyData<DCsource*>(74, 0x80000000 | 75, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'rfController'
        QtMocHelpers::PropertyData<RfController*>(76, 0x80000000 | 77, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'comPort'
        QtMocHelpers::PropertyData<QString>(78, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'baudRate'
        QtMocHelpers::PropertyData<int>(79, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'delayTime'
        QtMocHelpers::PropertyData<int>(80, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'sessionName'
        QtMocHelpers::PropertyData<QString>(81, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet | QMC::Final, 3),
        // property 'isConnected'
        QtMocHelpers::PropertyData<bool>(82, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Final, 4),
        // property 'availablePorts'
        QtMocHelpers::PropertyData<QStringList>(83, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Final, 5),
        // property 'commandLog'
        QtMocHelpers::PropertyData<QStringList>(84, QMetaType::QStringList, QMC::DefaultPropertyFlags | QMC::Final, 6),
        // property 'adcChannels'
        QtMocHelpers::PropertyData<QVariantList>(85, 0x80000000 | 86, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Final, 7),
        // property 'configPath'
        QtMocHelpers::PropertyData<QString>(87, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant | QMC::Final),
        // property 'isPortVerified'
        QtMocHelpers::PropertyData<bool>(88, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Final, 8),
        // property 'linkDiagnosis'
        QtMocHelpers::PropertyData<QString>(89, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 9),
        // property 'linkDiagnosisShort'
        QtMocHelpers::PropertyData<QString>(90, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 9),
        // property 'boardFirmware'
        QtMocHelpers::PropertyData<QString>(91, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 10),
        // property 'boardDiag'
        QtMocHelpers::PropertyData<QVariantMap>(92, 0x80000000 | 93, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Final, 10),
        // property 'scanning'
        QtMocHelpers::PropertyData<bool>(94, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Final, 11),
        // property 'kichNoPeakVoltage'
        QtMocHelpers::PropertyData<double>(95, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 12),
        // property 'acVoltMin'
        QtMocHelpers::PropertyData<int>(96, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'acVoltMax'
        QtMocHelpers::PropertyData<int>(97, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'acFreqMin'
        QtMocHelpers::PropertyData<int>(98, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'acFreqMax'
        QtMocHelpers::PropertyData<int>(99, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'dcVoltMin'
        QtMocHelpers::PropertyData<int>(100, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'dcVoltMax'
        QtMocHelpers::PropertyData<int>(101, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'dcCurrentMin'
        QtMocHelpers::PropertyData<double>(102, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'dcCurrentMax'
        QtMocHelpers::PropertyData<double>(103, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'attMax'
        QtMocHelpers::PropertyData<double>(104, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'stagedAcVolt'
        QtMocHelpers::PropertyData<double>(105, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 13),
        // property 'stagedAcFreq'
        QtMocHelpers::PropertyData<double>(106, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 14),
        // property 'stagedDcVolt'
        QtMocHelpers::PropertyData<double>(107, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 15),
        // property 'stagedDcCurrent'
        QtMocHelpers::PropertyData<double>(108, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 16),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DashboardManager, qt_meta_tag_ZN16DashboardManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DashboardManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16DashboardManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16DashboardManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16DashboardManagerE_t>.metaTypes,
    nullptr
} };

void DashboardManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DashboardManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->comPortChanged(); break;
        case 1: _t->baudRateChanged(); break;
        case 2: _t->delayTimeChanged(); break;
        case 3: _t->sessionNameChanged(); break;
        case 4: _t->isConnectedChanged(); break;
        case 5: _t->availablePortsChanged(); break;
        case 6: _t->commandLogChanged(); break;
        case 7: _t->adcChannelsChanged(); break;
        case 8: _t->isPortVerifiedChanged(); break;
        case 9: _t->linkDiagnosisChanged(); break;
        case 10: _t->boardDiagChanged(); break;
        case 11: _t->scanningChanged(); break;
        case 12: _t->kichNoPeakVoltageChanged(); break;
        case 13: _t->stagedAcVoltChanged(); break;
        case 14: _t->stagedAcFreqChanged(); break;
        case 15: _t->stagedDcVoltChanged(); break;
        case 16: _t->stagedDcCurrentChanged(); break;
        case 17: _t->systemNotification((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 18: _t->handleSerialError((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 19: _t->generateSimulatedWave(); break;
        case 20: _t->onAttUpdateRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 21: _t->sendPingProbe(); break;
        case 22: _t->onPingTimeout(); break;
        case 23: _t->onWaveWatchdog(); break;
        case 24: _t->onAcPowerTimeout(); break;
        case 25: _t->onDcPowerTimeout(); break;
        case 26: _t->startSession(); break;
        case 27: _t->stopSession(); break;
        case 28: _t->openLogFile(); break;
        case 29: { bool _r = _t->requestAcPower((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 30: { bool _r = _t->requestDcPower((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 31: _t->setAcPower((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<uint16_t>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<uint16_t>>(_a[3]))); break;
        case 32: _t->setDcPower((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<uint16_t>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 33: _t->refreshPower((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->refreshAllPower(); break;
        case 35: { QVariant _r = _t->loadUi((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2])));
            if (_a[0]) *reinterpret_cast<QVariant*>(_a[0]) = std::move(_r); }  break;
        case 36: { QVariant _r = _t->loadUi((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariant*>(_a[0]) = std::move(_r); }  break;
        case 37: _t->saveUi((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[2]))); break;
        case 38: _t->applyDelayTime((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 39: _t->refreshPorts(); break;
        case 40: { bool _r = _t->isPortAvailable((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 41: _t->resetSystem(); break;
        case 42: _t->sendCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 43: _t->executeCommand((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 44: { QStringList _r = _t->cliCompletions((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 45: _t->clearCommandLog(); break;
        case 46: { bool _r = _t->calibrateRf((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 47: _t->clearRfCalibration(); break;
        case 48: _t->clearRfCalPoint((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::comPortChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::baudRateChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::delayTimeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::sessionNameChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::isConnectedChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::availablePortsChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::commandLogChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::adcChannelsChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::isPortVerifiedChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::linkDiagnosisChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::boardDiagChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::scanningChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::kichNoPeakVoltageChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::stagedAcVoltChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::stagedAcFreqChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::stagedDcVoltChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)()>(_a, &DashboardManager::stagedDcCurrentChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (DashboardManager::*)(QString , QString )>(_a, &DashboardManager::systemNotification, 17))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ACsource* >(); break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< DCsource* >(); break;
        case 2:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< RfController* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<ACsource**>(_v) = _t->acSource(); break;
        case 1: *reinterpret_cast<DCsource**>(_v) = _t->dcSource(); break;
        case 2: *reinterpret_cast<RfController**>(_v) = _t->rfController(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->comPort(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->baudRate(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->delayTime(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->sessionName(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->isConnected(); break;
        case 8: *reinterpret_cast<QStringList*>(_v) = _t->availablePorts(); break;
        case 9: *reinterpret_cast<QStringList*>(_v) = _t->commandLog(); break;
        case 10: *reinterpret_cast<QVariantList*>(_v) = _t->adcChannels(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->configPath(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->isPortVerified(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->linkDiagnosis(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->linkDiagnosisShort(); break;
        case 15: *reinterpret_cast<QString*>(_v) = _t->boardFirmware(); break;
        case 16: *reinterpret_cast<QVariantMap*>(_v) = _t->boardDiag(); break;
        case 17: *reinterpret_cast<bool*>(_v) = _t->scanning(); break;
        case 18: *reinterpret_cast<double*>(_v) = _t->kichNoPeakVoltage(); break;
        case 19: *reinterpret_cast<int*>(_v) = _t->acVoltMin(); break;
        case 20: *reinterpret_cast<int*>(_v) = _t->acVoltMax(); break;
        case 21: *reinterpret_cast<int*>(_v) = _t->acFreqMin(); break;
        case 22: *reinterpret_cast<int*>(_v) = _t->acFreqMax(); break;
        case 23: *reinterpret_cast<int*>(_v) = _t->dcVoltMin(); break;
        case 24: *reinterpret_cast<int*>(_v) = _t->dcVoltMax(); break;
        case 25: *reinterpret_cast<double*>(_v) = _t->dcCurrentMin(); break;
        case 26: *reinterpret_cast<double*>(_v) = _t->dcCurrentMax(); break;
        case 27: *reinterpret_cast<double*>(_v) = _t->attMax(); break;
        case 28: *reinterpret_cast<double*>(_v) = _t->stagedAcVolt(); break;
        case 29: *reinterpret_cast<double*>(_v) = _t->stagedAcFreq(); break;
        case 30: *reinterpret_cast<double*>(_v) = _t->stagedDcVolt(); break;
        case 31: *reinterpret_cast<double*>(_v) = _t->stagedDcCurrent(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setComPort(*reinterpret_cast<QString*>(_v)); break;
        case 4: _t->setBaudRate(*reinterpret_cast<int*>(_v)); break;
        case 5: _t->setDelayTime(*reinterpret_cast<int*>(_v)); break;
        case 6: _t->setSessionName(*reinterpret_cast<QString*>(_v)); break;
        case 28: _t->setStagedAcVolt(*reinterpret_cast<double*>(_v)); break;
        case 29: _t->setStagedAcFreq(*reinterpret_cast<double*>(_v)); break;
        case 30: _t->setStagedDcVolt(*reinterpret_cast<double*>(_v)); break;
        case 31: _t->setStagedDcCurrent(*reinterpret_cast<double*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *DashboardManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DashboardManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16DashboardManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DashboardManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 49)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 49;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 49)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 49;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void DashboardManager::comPortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DashboardManager::baudRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DashboardManager::delayTimeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DashboardManager::sessionNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DashboardManager::isConnectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DashboardManager::availablePortsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DashboardManager::commandLogChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DashboardManager::adcChannelsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void DashboardManager::isPortVerifiedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void DashboardManager::linkDiagnosisChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void DashboardManager::boardDiagChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void DashboardManager::scanningChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void DashboardManager::kichNoPeakVoltageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void DashboardManager::stagedAcVoltChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void DashboardManager::stagedAcFreqChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void DashboardManager::stagedDcVoltChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void DashboardManager::stagedDcCurrentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void DashboardManager::systemNotification(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1, _t2);
}
QT_WARNING_POP
