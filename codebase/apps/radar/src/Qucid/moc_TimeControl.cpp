/****************************************************************************
** Meta object code from reading C++ file 'TimeControl.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "TimeControl.hh"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TimeControl.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.2. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSTimeControlENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSTimeControlENDCLASS = QtMocHelpers::stringData(
    "TimeControl",
    "_timeSliderActionTriggered",
    "",
    "action",
    "_timeSliderValueChanged",
    "value",
    "_timeSliderReleased",
    "_timeSliderPressed"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSTimeControlENDCLASS_t {
    uint offsetsAndSizes[16];
    char stringdata0[12];
    char stringdata1[27];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[24];
    char stringdata5[6];
    char stringdata6[20];
    char stringdata7[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSTimeControlENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSTimeControlENDCLASS_t qt_meta_stringdata_CLASSTimeControlENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "TimeControl"
        QT_MOC_LITERAL(12, 26),  // "_timeSliderActionTriggered"
        QT_MOC_LITERAL(39, 0),  // ""
        QT_MOC_LITERAL(40, 6),  // "action"
        QT_MOC_LITERAL(47, 23),  // "_timeSliderValueChanged"
        QT_MOC_LITERAL(71, 5),  // "value"
        QT_MOC_LITERAL(77, 19),  // "_timeSliderReleased"
        QT_MOC_LITERAL(97, 18)   // "_timeSliderPressed"
    },
    "TimeControl",
    "_timeSliderActionTriggered",
    "",
    "action",
    "_timeSliderValueChanged",
    "value",
    "_timeSliderReleased",
    "_timeSliderPressed"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSTimeControlENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x0a,    1 /* Public */,
       4,    1,   41,    2, 0x0a,    3 /* Public */,
       6,    0,   44,    2, 0x0a,    5 /* Public */,
       7,    0,   45,    2, 0x0a,    6 /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject TimeControl::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSTimeControlENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSTimeControlENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSTimeControlENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TimeControl, std::true_type>,
        // method '_timeSliderActionTriggered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_timeSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_timeSliderReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_timeSliderPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TimeControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TimeControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->_timeSliderActionTriggered((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->_timeSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->_timeSliderReleased(); break;
        case 3: _t->_timeSliderPressed(); break;
        default: ;
        }
    }
}

const QMetaObject *TimeControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimeControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSTimeControlENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int TimeControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
