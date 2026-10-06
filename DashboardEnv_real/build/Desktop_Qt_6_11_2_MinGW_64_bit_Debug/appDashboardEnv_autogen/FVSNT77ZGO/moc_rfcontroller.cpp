/****************************************************************************
** Meta object code from reading C++ file 'rfcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/rfcontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rfcontroller.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12RfControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto RfController::qt_create_metaobjectdata<qt_meta_tag_ZN12RfControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RfController",
        "att1Changed",
        "",
        "att2Changed",
        "att3Changed",
        "waveDataChanged",
        "waveSourceChanged",
        "rfStatusChanged",
        "calibrationChanged",
        "diagChanged",
        "attUpdateRequested",
        "channel",
        "value",
        "requestSetAtt",
        "force",
        "att1",
        "att2",
        "att3",
        "waveData",
        "QVariantList",
        "xMin",
        "xMax",
        "yMin",
        "yMax",
        "xIsFrequency",
        "peakLevel",
        "peakX",
        "totalLevel",
        "totalDbfs",
        "waveSource",
        "spectrumUnit",
        "rfStatusValid",
        "rxLoMHz",
        "rxFsMHz",
        "rxBwMHz",
        "rxGainDb",
        "rxGainMode",
        "fftSize",
        "calibrated",
        "calOffsetDb",
        "calMismatch",
        "diagText",
        "diagLevel"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'att1Changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'att2Changed'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'att3Changed'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'waveDataChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'waveSourceChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'rfStatusChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'calibrationChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'diagChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'attUpdateRequested'
        QtMocHelpers::SignalData<void(int, double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 }, { QMetaType::Double, 12 },
        }}),
        // Method 'requestSetAtt'
        QtMocHelpers::MethodData<bool(int, double, bool)>(13, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 11 }, { QMetaType::Double, 12 }, { QMetaType::Bool, 14 },
        }}),
        // Method 'requestSetAtt'
        QtMocHelpers::MethodData<bool(int, double)>(13, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Bool, {{
            { QMetaType::Int, 11 }, { QMetaType::Double, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'att1'
        QtMocHelpers::PropertyData<double>(15, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'att2'
        QtMocHelpers::PropertyData<double>(16, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'att3'
        QtMocHelpers::PropertyData<double>(17, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'waveData'
        QtMocHelpers::PropertyData<QVariantList>(18, 0x80000000 | 19, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'xMin'
        QtMocHelpers::PropertyData<double>(20, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'xMax'
        QtMocHelpers::PropertyData<double>(21, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'yMin'
        QtMocHelpers::PropertyData<double>(22, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'yMax'
        QtMocHelpers::PropertyData<double>(23, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'xIsFrequency'
        QtMocHelpers::PropertyData<bool>(24, QMetaType::Bool, QMC::DefaultPropertyFlags, 3),
        // property 'peakLevel'
        QtMocHelpers::PropertyData<double>(25, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'peakX'
        QtMocHelpers::PropertyData<double>(26, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'totalLevel'
        QtMocHelpers::PropertyData<double>(27, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'totalDbfs'
        QtMocHelpers::PropertyData<double>(28, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'waveSource'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'spectrumUnit'
        QtMocHelpers::PropertyData<QString>(30, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
        // property 'rfStatusValid'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'rxLoMHz'
        QtMocHelpers::PropertyData<double>(32, QMetaType::Double, QMC::DefaultPropertyFlags, 5),
        // property 'rxFsMHz'
        QtMocHelpers::PropertyData<double>(33, QMetaType::Double, QMC::DefaultPropertyFlags, 5),
        // property 'rxBwMHz'
        QtMocHelpers::PropertyData<double>(34, QMetaType::Double, QMC::DefaultPropertyFlags, 5),
        // property 'rxGainDb'
        QtMocHelpers::PropertyData<int>(35, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'rxGainMode'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'fftSize'
        QtMocHelpers::PropertyData<int>(37, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'calibrated'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'calOffsetDb'
        QtMocHelpers::PropertyData<double>(39, QMetaType::Double, QMC::DefaultPropertyFlags, 6),
        // property 'calMismatch'
        QtMocHelpers::PropertyData<bool>(40, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'diagText'
        QtMocHelpers::PropertyData<QString>(41, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
        // property 'diagLevel'
        QtMocHelpers::PropertyData<QString>(42, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RfController, qt_meta_tag_ZN12RfControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RfController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12RfControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12RfControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12RfControllerE_t>.metaTypes,
    nullptr
} };

void RfController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RfController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->att1Changed(); break;
        case 1: _t->att2Changed(); break;
        case 2: _t->att3Changed(); break;
        case 3: _t->waveDataChanged(); break;
        case 4: _t->waveSourceChanged(); break;
        case 5: _t->rfStatusChanged(); break;
        case 6: _t->calibrationChanged(); break;
        case 7: _t->diagChanged(); break;
        case 8: _t->attUpdateRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 9: { bool _r = _t->requestSetAtt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->requestSetAtt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::att1Changed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::att2Changed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::att3Changed, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::waveDataChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::waveSourceChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::rfStatusChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::calibrationChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)()>(_a, &RfController::diagChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (RfController::*)(int , double )>(_a, &RfController::attUpdateRequested, 8))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<double*>(_v) = _t->att1(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->att2(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->att3(); break;
        case 3: *reinterpret_cast<QVariantList*>(_v) = _t->waveData(); break;
        case 4: *reinterpret_cast<double*>(_v) = _t->xMin(); break;
        case 5: *reinterpret_cast<double*>(_v) = _t->xMax(); break;
        case 6: *reinterpret_cast<double*>(_v) = _t->yMin(); break;
        case 7: *reinterpret_cast<double*>(_v) = _t->yMax(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->xIsFrequency(); break;
        case 9: *reinterpret_cast<double*>(_v) = _t->peakLevel(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->peakX(); break;
        case 11: *reinterpret_cast<double*>(_v) = _t->totalLevel(); break;
        case 12: *reinterpret_cast<double*>(_v) = _t->totalDbfs(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->waveSource(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->spectrumUnit(); break;
        case 15: *reinterpret_cast<bool*>(_v) = _t->rfStatusValid(); break;
        case 16: *reinterpret_cast<double*>(_v) = _t->rxLoMHz(); break;
        case 17: *reinterpret_cast<double*>(_v) = _t->rxFsMHz(); break;
        case 18: *reinterpret_cast<double*>(_v) = _t->rxBwMHz(); break;
        case 19: *reinterpret_cast<int*>(_v) = _t->rxGainDb(); break;
        case 20: *reinterpret_cast<QString*>(_v) = _t->rxGainMode(); break;
        case 21: *reinterpret_cast<int*>(_v) = _t->fftSize(); break;
        case 22: *reinterpret_cast<bool*>(_v) = _t->calibrated(); break;
        case 23: *reinterpret_cast<double*>(_v) = _t->calOffsetDb(); break;
        case 24: *reinterpret_cast<bool*>(_v) = _t->calMismatch(); break;
        case 25: *reinterpret_cast<QString*>(_v) = _t->diagText(); break;
        case 26: *reinterpret_cast<QString*>(_v) = _t->diagLevel(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setAtt1(*reinterpret_cast<double*>(_v)); break;
        case 1: _t->setAtt2(*reinterpret_cast<double*>(_v)); break;
        case 2: _t->setAtt3(*reinterpret_cast<double*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *RfController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RfController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12RfControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RfController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    return _id;
}

// SIGNAL 0
void RfController::att1Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void RfController::att2Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void RfController::att3Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void RfController::waveDataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void RfController::waveSourceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void RfController::rfStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void RfController::calibrationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void RfController::diagChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void RfController::attUpdateRequested(int _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}
QT_WARNING_POP
