/****************************************************************************
** Meta object code from reading C++ file 'TimeControl.hh'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "TimeControl.hh"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TimeControl.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TimeControl_t {
    QByteArrayData data[19];
    char stringdata0[252];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TimeControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TimeControl_t qt_meta_stringdata_TimeControl = {
    {
QT_MOC_LITERAL(0, 0, 11), // "TimeControl"
QT_MOC_LITERAL(1, 12, 7), // "goBack1"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 14), // "goBackDuration"
QT_MOC_LITERAL(4, 36, 10), // "goBackMult"
QT_MOC_LITERAL(5, 47, 6), // "goFwd1"
QT_MOC_LITERAL(6, 54, 13), // "goFwdDuration"
QT_MOC_LITERAL(7, 68, 9), // "goFwdMult"
QT_MOC_LITERAL(8, 78, 26), // "_timeSliderActionTriggered"
QT_MOC_LITERAL(9, 105, 6), // "action"
QT_MOC_LITERAL(10, 112, 23), // "_timeSliderValueChanged"
QT_MOC_LITERAL(11, 136, 5), // "value"
QT_MOC_LITERAL(12, 142, 19), // "_timeSliderReleased"
QT_MOC_LITERAL(13, 162, 18), // "_timeSliderPressed"
QT_MOC_LITERAL(14, 181, 21), // "_timeSliderSetNFrames"
QT_MOC_LITERAL(15, 203, 3), // "val"
QT_MOC_LITERAL(16, 207, 21), // "_setFrameIntervalSecs"
QT_MOC_LITERAL(17, 229, 12), // "_setRealtime"
QT_MOC_LITERAL(18, 242, 9) // "_setSweep"

    },
    "TimeControl\0goBack1\0\0goBackDuration\0"
    "goBackMult\0goFwd1\0goFwdDuration\0"
    "goFwdMult\0_timeSliderActionTriggered\0"
    "action\0_timeSliderValueChanged\0value\0"
    "_timeSliderReleased\0_timeSliderPressed\0"
    "_timeSliderSetNFrames\0val\0"
    "_setFrameIntervalSecs\0_setRealtime\0"
    "_setSweep"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TimeControl[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x0a /* Public */,
       3,    0,   85,    2, 0x0a /* Public */,
       4,    0,   86,    2, 0x0a /* Public */,
       5,    0,   87,    2, 0x0a /* Public */,
       6,    0,   88,    2, 0x0a /* Public */,
       7,    0,   89,    2, 0x0a /* Public */,
       8,    1,   90,    2, 0x0a /* Public */,
      10,    1,   93,    2, 0x0a /* Public */,
      12,    0,   96,    2, 0x0a /* Public */,
      13,    0,   97,    2, 0x0a /* Public */,
      14,    1,   98,    2, 0x0a /* Public */,
      16,    1,  101,    2, 0x0a /* Public */,
      17,    1,  104,    2, 0x0a /* Public */,
      18,    1,  107,    2, 0x0a /* Public */,

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
    QMetaType::Void, QMetaType::Double,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,

       0        // eod
};

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
        case 6: _t->_timeSliderActionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->_timeSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->_timeSliderReleased(); break;
        case 9: _t->_timeSliderPressed(); break;
        case 10: _t->_timeSliderSetNFrames((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->_setFrameIntervalSecs((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->_setRealtime((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->_setSweep((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TimeControl::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_TimeControl.data,
    qt_meta_data_TimeControl,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TimeControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimeControl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TimeControl.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int TimeControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
