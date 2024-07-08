/****************************************************************************
** Meta object code from reading C++ file 'CartWidget.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CartWidget.hh"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CartWidget.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.0. It"
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
struct qt_meta_stringdata_CLASSCartWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCartWidgetENDCLASS = QtMocHelpers::stringData(
    "CartWidget",
    "locationClicked",
    "",
    "xkm",
    "ykm",
    "const RadxRay*",
    "closestRay",
    "displayImage",
    "size_t",
    "field_num",
    "setArchiveMode",
    "archive_mode",
    "unzoomView",
    "resize",
    "width",
    "height",
    "setRings",
    "enabled",
    "setGrids",
    "setAngleLines",
    "contextMenuCancel",
    "contextMenuParameterColors",
    "contextMenuView",
    "contextMenuEditor",
    "contextMenuExamine",
    "contextMenuDataWidget"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCartWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   98,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,  105,    2, 0x0a,    5 /* Public */,
      10,    1,  108,    2, 0x0a,    7 /* Public */,
      12,    0,  111,    2, 0x0a,    9 /* Public */,
      13,    2,  112,    2, 0x0a,   10 /* Public */,
      16,    1,  117,    2, 0x0a,   13 /* Public */,
      18,    1,  120,    2, 0x0a,   15 /* Public */,
      19,    1,  123,    2, 0x0a,   17 /* Public */,
      20,    0,  126,    2, 0x0a,   19 /* Public */,
      21,    0,  127,    2, 0x0a,   20 /* Public */,
      22,    0,  128,    2, 0x0a,   21 /* Public */,
      23,    0,  129,    2, 0x0a,   22 /* Public */,
      24,    0,  130,    2, 0x0a,   23 /* Public */,
      25,    0,  131,    2, 0x0a,   24 /* Public */,

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

Q_CONSTINIT const QMetaObject CartWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSCartWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCartWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCartWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CartWidget, std::true_type>,
        // method 'locationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method 'displayImage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const size_t, std::false_type>,
        // method 'setArchiveMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'unzoomView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'resize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const int, std::false_type>,
        // method 'setRings'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const bool, std::false_type>,
        // method 'setGrids'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const bool, std::false_type>,
        // method 'setAngleLines'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const bool, std::false_type>,
        // method 'contextMenuCancel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'contextMenuParameterColors'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'contextMenuView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'contextMenuEditor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'contextMenuExamine'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'contextMenuDataWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CartWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CartWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->locationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 1: _t->displayImage((*reinterpret_cast< std::add_pointer_t<size_t>>(_a[1]))); break;
        case 2: _t->setArchiveMode((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->unzoomView(); break;
        case 4: _t->resize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->setRings((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->setGrids((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->setAngleLines((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
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
            if (_t _q_method = &CartWidget::locationClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *CartWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CartWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCartWidgetENDCLASS.stringdata0))
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
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
