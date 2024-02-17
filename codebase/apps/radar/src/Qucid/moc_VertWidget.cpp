/****************************************************************************
** Meta object code from reading C++ file 'VertWidget.hh'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "VertWidget.hh"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VertWidget.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VertWidget_t {
    QByteArrayData data[12];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VertWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VertWidget_t qt_meta_stringdata_VertWidget = {
    {
QT_MOC_LITERAL(0, 0, 10), // "VertWidget"
QT_MOC_LITERAL(1, 11, 21), // "severalBeamsProcessed"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 10), // "unzoomView"
QT_MOC_LITERAL(4, 45, 5), // "clear"
QT_MOC_LITERAL(5, 51, 7), // "refresh"
QT_MOC_LITERAL(6, 59, 6), // "resize"
QT_MOC_LITERAL(7, 66, 5), // "width"
QT_MOC_LITERAL(8, 72, 6), // "height"
QT_MOC_LITERAL(9, 79, 10), // "paintEvent"
QT_MOC_LITERAL(10, 90, 12), // "QPaintEvent*"
QT_MOC_LITERAL(11, 103, 5) // "event"

    },
    "VertWidget\0severalBeamsProcessed\0\0"
    "unzoomView\0clear\0refresh\0resize\0width\0"
    "height\0paintEvent\0QPaintEvent*\0event"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VertWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   45,    2, 0x0a /* Public */,
       4,    0,   46,    2, 0x0a /* Public */,
       5,    0,   47,    2, 0x0a /* Public */,
       6,    2,   48,    2, 0x0a /* Public */,
       9,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    7,    8,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void VertWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VertWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->severalBeamsProcessed(); break;
        case 1: _t->unzoomView(); break;
        case 2: _t->clear(); break;
        case 3: _t->refresh(); break;
        case 4: _t->resize((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->paintEvent((*reinterpret_cast< QPaintEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VertWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VertWidget::severalBeamsProcessed)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VertWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<CartWidget::staticMetaObject>(),
    qt_meta_stringdata_VertWidget.data,
    qt_meta_data_VertWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VertWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VertWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VertWidget.stringdata0))
        return static_cast<void*>(this);
    return CartWidget::qt_metacast(_clname);
}

int VertWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CartWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void VertWidget::severalBeamsProcessed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
