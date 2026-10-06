/****************************************************************************
** Meta object code from reading C++ file 'acsource.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/acsource.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'acsource.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8ACsourceE_t {};
} // unnamed namespace

template <> constexpr inline auto ACsource::qt_create_metaobjectdata<qt_meta_tag_ZN8ACsourceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ACsource",
        "freqChanged",
        "",
        "freqHistoryChanged",
        "freqMaxChanged",
        "freqMinChanged",
        "freq",
        "freqHistory",
        "QVariantList",
        "freqMax",
        "freqMin"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'freqChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'freqHistoryChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'freqMaxChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'freqMinChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'freq'
        QtMocHelpers::PropertyData<double>(6, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 0),
        // property 'freqHistory'
        QtMocHelpers::PropertyData<QVariantList>(7, 0x80000000 | 8, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Final, 1),
        // property 'freqMax'
        QtMocHelpers::PropertyData<double>(9, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 2),
        // property 'freqMin'
        QtMocHelpers::PropertyData<double>(10, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Final, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ACsource, qt_meta_tag_ZN8ACsourceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ACsource::staticMetaObject = { {
    QMetaObject::SuperData::link<BasePowerSource::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ACsourceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ACsourceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8ACsourceE_t>.metaTypes,
    nullptr
} };

void ACsource::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ACsource *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->freqChanged(); break;
        case 1: _t->freqHistoryChanged(); break;
        case 2: _t->freqMaxChanged(); break;
        case 3: _t->freqMinChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ACsource::*)()>(_a, &ACsource::freqChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ACsource::*)()>(_a, &ACsource::freqHistoryChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ACsource::*)()>(_a, &ACsource::freqMaxChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ACsource::*)()>(_a, &ACsource::freqMinChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<double*>(_v) = _t->freq(); break;
        case 1: *reinterpret_cast<QVariantList*>(_v) = _t->freqHistory(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->freqMax(); break;
        case 3: *reinterpret_cast<double*>(_v) = _t->freqMin(); break;
        default: break;
        }
    }
}

const QMetaObject *ACsource::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ACsource::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ACsourceE_t>.strings))
        return static_cast<void*>(this);
    return BasePowerSource::qt_metacast(_clname);
}

int ACsource::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BasePowerSource::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ACsource::freqChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ACsource::freqHistoryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ACsource::freqMaxChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ACsource::freqMinChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
