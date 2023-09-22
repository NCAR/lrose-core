/****************************************************************************
** Meta object code from reading C++ file 'CartWidget.hh'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "CartWidget.hh"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CartWidget.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CartWidget_t {
    QByteArrayData data[26];
    char stringdata0[311];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CartWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CartWidget_t qt_meta_stringdata_CartWidget = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CartWidget"
QT_MOC_LITERAL(1, 11, 15), // "locationClicked"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 3), // "xkm"
QT_MOC_LITERAL(4, 32, 3), // "ykm"
QT_MOC_LITERAL(5, 36, 14), // "const RadxRay*"
QT_MOC_LITERAL(6, 51, 10), // "closestRay"
QT_MOC_LITERAL(7, 62, 12), // "displayImage"
QT_MOC_LITERAL(8, 75, 6), // "size_t"
QT_MOC_LITERAL(9, 82, 9), // "field_num"
QT_MOC_LITERAL(10, 92, 14), // "setArchiveMode"
QT_MOC_LITERAL(11, 107, 12), // "archive_mode"
QT_MOC_LITERAL(12, 120, 10), // "unzoomView"
QT_MOC_LITERAL(13, 131, 6), // "resize"
QT_MOC_LITERAL(14, 138, 5), // "width"
QT_MOC_LITERAL(15, 144, 6), // "height"
QT_MOC_LITERAL(16, 151, 8), // "setRings"
QT_MOC_LITERAL(17, 160, 7), // "enabled"
QT_MOC_LITERAL(18, 168, 8), // "setGrids"
QT_MOC_LITERAL(19, 177, 13), // "setAngleLines"
QT_MOC_LITERAL(20, 191, 17), // "contextMenuCancel"
QT_MOC_LITERAL(21, 209, 26), // "contextMenuParameterColors"
QT_MOC_LITERAL(22, 236, 15), // "contextMenuView"
QT_MOC_LITERAL(23, 252, 17), // "contextMenuEditor"
QT_MOC_LITERAL(24, 270, 18), // "contextMenuExamine"
QT_MOC_LITERAL(25, 289, 21) // "contextMenuDataWidget"

    },
    "CartWidget\0locationClicked\0\0xkm\0ykm\0"
    "const RadxRay*\0closestRay\0displayImage\0"
    "size_t\0field_num\0setArchiveMode\0"
    "archive_mode\0unzoomView\0resize\0width\0"
    "height\0setRings\0enabled\0setGrids\0"
    "setAngleLines\0contextMenuCancel\0"
    "contextMenuParameterColors\0contextMenuView\0"
    "contextMenuEditor\0contextMenuExamine\0"
    "contextMenuDataWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CartWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   91,    2, 0x0a /* Public */,
      10,    1,   94,    2, 0x0a /* Public */,
      12,    0,   97,    2, 0x0a /* Public */,
      13,    2,   98,    2, 0x0a /* Public */,
      16,    1,  103,    2, 0x0a /* Public */,
      18,    1,  106,    2, 0x0a /* Public */,
      19,    1,  109,    2, 0x0a /* Public */,
      20,    0,  112,    2, 0x0a /* Public */,
      21,    0,  113,    2, 0x0a /* Public */,
      22,    0,  114,    2, 0x0a /* Public */,
      23,    0,  115,    2, 0x0a /* Public */,
      24,    0,  116,    2, 0x0a /* Public */,
      25,    0,  117,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 5,    3,    4,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   14,   15,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CartWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CartWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->locationClicked((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< const RadxRay*(*)>(_a[3]))); break;
        case 1: _t->displayImage((*reinterpret_cast< const size_t(*)>(_a[1]))); break;
        case 2: _t->setArchiveMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->unzoomView(); break;
        case 4: _t->resize((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 5: _t->setRings((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 6: _t->setGrids((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 7: _t->setAngleLines((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 8: _t->contextMenuCancel(); break;
        case 9: _t->contextMenuParameterColors(); break;
        case 10: _t->contextMenuView(); break;
        case 11: _t->contextMenuEditor(); break;
        case 12: _t->contextMenuExamine(); break;
        case 13: _t->contextMenuDataWidget(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CartWidget::*)(double , double , const RadxRay * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CartWidget::locationClicked)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CartWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CartWidget.data,
    qt_meta_data_CartWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CartWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CartWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CartWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CartWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void CartWidget::locationClicked(double _t1, double _t2, const RadxRay * _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
