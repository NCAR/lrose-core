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
    "goBack1",
    "",
    "goBackDuration",
    "goBackMult",
    "goFwd1",
    "goFwdDuration",
    "goFwdMult",
    "_timeSliderActionTriggered",
    "action",
    "_timeSliderValueChanged",
    "value",
    "_timeSliderReleased",
    "_timeSliderPressed",
    "_timeSliderSetNFrames",
    "val"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSTimeControlENDCLASS_t {
    uint offsetsAndSizes[32];
    char stringdata0[12];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[11];
    char stringdata5[7];
    char stringdata6[14];
    char stringdata7[10];
    char stringdata8[27];
    char stringdata9[7];
    char stringdata10[24];
    char stringdata11[6];
    char stringdata12[20];
    char stringdata13[19];
    char stringdata14[22];
    char stringdata15[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSTimeControlENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSTimeControlENDCLASS_t qt_meta_stringdata_CLASSTimeControlENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "TimeControl"
        QT_MOC_LITERAL(12, 7),  // "goBack1"
        QT_MOC_LITERAL(20, 0),  // ""
        QT_MOC_LITERAL(21, 14),  // "goBackDuration"
        QT_MOC_LITERAL(36, 10),  // "goBackMult"
        QT_MOC_LITERAL(47, 6),  // "goFwd1"
        QT_MOC_LITERAL(54, 13),  // "goFwdDuration"
        QT_MOC_LITERAL(68, 9),  // "goFwdMult"
        QT_MOC_LITERAL(78, 26),  // "_timeSliderActionTriggered"
        QT_MOC_LITERAL(105, 6),  // "action"
        QT_MOC_LITERAL(112, 23),  // "_timeSliderValueChanged"
        QT_MOC_LITERAL(136, 5),  // "value"
        QT_MOC_LITERAL(142, 19),  // "_timeSliderReleased"
        QT_MOC_LITERAL(162, 18),  // "_timeSliderPressed"
        QT_MOC_LITERAL(181, 21),  // "_timeSliderSetNFrames"
        QT_MOC_LITERAL(203, 3)   // "val"
    },
    "TimeControl",
    "goBack1",
    "",
    "goBackDuration",
    "goBackMult",
    "goFwd1",
    "goFwdDuration",
    "goFwdMult",
    "_timeSliderActionTriggered",
    "action",
    "_timeSliderValueChanged",
    "value",
    "_timeSliderReleased",
    "_timeSliderPressed",
    "_timeSliderSetNFrames",
    "val"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSTimeControlENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x0a,    1 /* Public */,
       3,    0,   81,    2, 0x0a,    2 /* Public */,
       4,    0,   82,    2, 0x0a,    3 /* Public */,
       5,    0,   83,    2, 0x0a,    4 /* Public */,
       6,    0,   84,    2, 0x0a,    5 /* Public */,
       7,    0,   85,    2, 0x0a,    6 /* Public */,
       8,    1,   86,    2, 0x0a,    7 /* Public */,
      10,    1,   89,    2, 0x0a,    9 /* Public */,
      12,    0,   92,    2, 0x0a,   11 /* Public */,
      13,    0,   93,    2, 0x0a,   12 /* Public */,
      14,    1,   94,    2, 0x0a,   13 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,

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
        // method 'goBack1'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goBackDuration'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goBackMult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goFwd1'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goFwdDuration'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goFwdMult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_timeSliderActionTriggered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_timeSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_timeSliderReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_timeSliderPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_timeSliderSetNFrames'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void TimeControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TimeControl *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->goBack1(); break;
        case 1: _t->goBackDuration(); break;
        case 2: _t->goBackMult(); break;
        case 3: _t->goFwd1(); break;
        case 4: _t->goFwdDuration(); break;
        case 5: _t->goFwdMult(); break;
        case 6: _t->_timeSliderActionTriggered((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->_timeSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->_timeSliderReleased(); break;
        case 9: _t->_timeSliderPressed(); break;
        case 10: _t->_timeSliderSetNFrames((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
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
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
