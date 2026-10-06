/****************************************************************************
** Meta object code from reading C++ file 'basepowersource.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/basepowersource.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'basepowersource.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15BasePowerSourceE_t {};
} // unnamed namespace

template <> constexpr inline auto BasePowerSource::qt_create_metaobjectdata<qt_meta_tag_ZN15BasePowerSourceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BasePowerSource",
        "voltageChanged",
        "",
        "currentChanged",
        "hasDataChanged",
        "isOnChanged",
        "voltageHistoryChanged",
        "currentHistoryChanged",
        "voltageMaxChanged",
        "voltageMinChanged",
        "currentMaxChanged",
        "currentMinChanged",
        "linkChanged",
        "resetHistory",
        "voltage",
        "current",
        "hasData",
        "isOn",
        "voltageHistory",
        "QVariantList",
        "currentHistory",
        "voltageMax",
        "voltageMin",
        "currentMax",
        "currentMin",
        "linkState",
        "linkMessage",
        "pending",
        "lastUpdate"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'voltageChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'hasDataChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isOnChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voltageHistoryChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentHistoryChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voltageMaxChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voltageMinChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentMaxChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentMinChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'linkChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetHistory'
        QtMocHelpers::MethodData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'voltage'
        QtMocHelpers::PropertyData<double>(14, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 0),
        // property 'current'
        QtMocHelpers::PropertyData<double>(15, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 1),
        // property 'hasData'
        QtMocHelpers::PropertyData<bool>(16, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Final, 2),
        // property 'isOn'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet | QMC::Final, 3),
        // property 'voltageHistory'
        QtMocHelpers::PropertyData<QVariantList>(18, 0x80000000 | 19, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Final, 4),
        // property 'currentHistory'
        QtMocHelpers::PropertyData<QVariantList>(20, 0x80000000 | 19, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Final, 5),
        // property 'voltageMax'
        QtMocHelpers::PropertyData<double>(21, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 6),
        // property 'voltageMin'
        QtMocHelpers::PropertyData<double>(22, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 7),
        // property 'currentMax'
        QtMocHelpers::PropertyData<double>(23, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 8),
        // property 'currentMin'
        QtMocHelpers::PropertyData<double>(24, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 9),
        // property 'linkState'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 10),
        // property 'linkMessage'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 10),
        // property 'pending'
        QtMocHelpers::PropertyData<bool>(27, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Final, 10),
        // property 'lastUpdate'
        QtMocHelpers::PropertyData<QString>(28, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Final, 10),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BasePowerSource, qt_meta_tag_ZN15BasePowerSourceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BasePowerSource::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BasePowerSourceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BasePowerSourceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15BasePowerSourceE_t>.metaTypes,
    nullptr
} };

void BasePowerSource::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BasePowerSource *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->voltageChanged(); break;
        case 1: _t->currentChanged(); break;
        case 2: _t->hasDataChanged(); break;
        case 3: _t->isOnChanged(); break;
        case 4: _t->voltageHistoryChanged(); break;
        case 5: _t->currentHistoryChanged(); break;
        case 6: _t->voltageMaxChanged(); break;
        case 7: _t->voltageMinChanged(); break;
        case 8: _t->currentMaxChanged(); break;
        case 9: _t->currentMinChanged(); break;
        case 10: _t->linkChanged(); break;
        case 11: _t->resetHistory(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::voltageChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::currentChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::hasDataChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::isOnChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::voltageHistoryChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::currentHistoryChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::voltageMaxChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::voltageMinChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::currentMaxChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::currentMinChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (BasePowerSource::*)()>(_a, &BasePowerSource::linkChanged, 10))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<double*>(_v) = _t->voltage(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->current(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->hasData(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isOn(); break;
        case 4: *reinterpret_cast<QVariantList*>(_v) = _t->voltageHistory(); break;
        case 5: *reinterpret_cast<QVariantList*>(_v) = _t->currentHistory(); break;
        case 6: *reinterpret_cast<double*>(_v) = _t->voltageMax(); break;
        case 7: *reinterpret_cast<double*>(_v) = _t->voltageMin(); break;
        case 8: *reinterpret_cast<double*>(_v) = _t->currentMax(); break;
        case 9: *reinterpret_cast<double*>(_v) = _t->currentMin(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->linkState(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->linkMessage(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->pending(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->lastUpdate(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setIsOn(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *BasePowerSource::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BasePowerSource::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15BasePowerSourceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BasePowerSource::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void BasePowerSource::voltageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void BasePowerSource::currentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void BasePowerSource::hasDataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void BasePowerSource::isOnChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void BasePowerSource::voltageHistoryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void BasePowerSource::currentHistoryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void BasePowerSource::voltageMaxChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void BasePowerSource::voltageMinChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void BasePowerSource::currentMaxChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void BasePowerSource::currentMinChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void BasePowerSource::linkChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}
QT_WARNING_POP
