/****************************************************************************
** Meta object code from reading C++ file 'CartManager.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CartManager.hh"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CartManager.hh' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSCartManagerENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCartManagerENDCLASS = QtMocHelpers::stringData(
    "CartManager",
    "_freeze",
    "",
    "_unzoom",
    "_refresh",
    "_changeField",
    "fieldId",
    "guiMode",
    "_horizLocationClicked",
    "xkm",
    "ykm",
    "const RadxRay*",
    "closestRay",
    "_vertLocationClicked",
    "_locationClicked",
    "ray",
    "_showFieldMenu",
    "_placeFieldMenu",
    "_setMapsEnabled",
    "enable",
    "_setProductsEnabled",
    "_setWindsEnabled",
    "_showTimeControl",
    "_placeTimeControl",
    "_circleRadiusSliderValueChanged",
    "value",
    "_brushRadiusSliderValueChanged",
    "_saveImageToFile",
    "interactive",
    "_createRealtimeImageFiles",
    "_createArchiveImageFiles",
    "_createImageFilesAllLevels",
    "_createImageFiles",
    "ShowContextMenu",
    "pos"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCartManagerENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  158,    2, 0x08,    1 /* Private */,
       3,    0,  159,    2, 0x08,    2 /* Private */,
       4,    0,  160,    2, 0x08,    3 /* Private */,
       5,    2,  161,    2, 0x08,    4 /* Private */,
       5,    1,  166,    2, 0x28,    7 /* Private | MethodCloned */,
       8,    3,  169,    2, 0x08,    9 /* Private */,
      13,    3,  176,    2, 0x08,   13 /* Private */,
      14,    3,  183,    2, 0x08,   17 /* Private */,
      16,    0,  190,    2, 0x08,   21 /* Private */,
      17,    0,  191,    2, 0x08,   22 /* Private */,
      18,    1,  192,    2, 0x08,   23 /* Private */,
      20,    1,  195,    2, 0x08,   25 /* Private */,
      21,    1,  198,    2, 0x08,   27 /* Private */,
      22,    0,  201,    2, 0x08,   29 /* Private */,
      23,    0,  202,    2, 0x08,   30 /* Private */,
      24,    1,  203,    2, 0x08,   31 /* Private */,
      26,    1,  206,    2, 0x08,   33 /* Private */,
      27,    1,  209,    2, 0x08,   35 /* Private */,
      27,    0,  212,    2, 0x28,   37 /* Private | MethodCloned */,
      29,    0,  213,    2, 0x08,   38 /* Private */,
      30,    0,  214,    2, 0x08,   39 /* Private */,
      31,    0,  215,    2, 0x08,   40 /* Private */,
      32,    0,  216,    2, 0x08,   41 /* Private */,
      33,    1,  217,    2, 0x08,   42 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    6,    7,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 11,    9,   10,   12,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 11,    9,   10,   12,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 11,    9,   10,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Bool,   28,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   34,

       0        // eod
};

Q_CONSTINIT const QMetaObject CartManager::staticMetaObject = { {
    QMetaObject::SuperData::link<DisplayManager::staticMetaObject>(),
    qt_meta_stringdata_CLASSCartManagerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCartManagerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCartManagerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CartManager, std::true_type>,
        // method '_freeze'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_unzoom'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_changeField'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_changeField'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_horizLocationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_vertLocationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_locationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_showFieldMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_placeFieldMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_setMapsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_setProductsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_setWindsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_showTimeControl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_placeTimeControl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_circleRadiusSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_brushRadiusSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_saveImageToFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_saveImageToFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createRealtimeImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createArchiveImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createImageFilesAllLevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ShowContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>
    >,
    nullptr
} };

void CartManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CartManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->_freeze(); break;
        case 1: _t->_unzoom(); break;
        case 2: _t->_refresh(); break;
        case 3: _t->_changeField((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 4: _t->_changeField((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->_horizLocationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 6: _t->_vertLocationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 7: _t->_locationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 8: _t->_showFieldMenu(); break;
        case 9: _t->_placeFieldMenu(); break;
        case 10: _t->_setMapsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->_setProductsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->_setWindsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->_showTimeControl(); break;
        case 14: _t->_placeTimeControl(); break;
        case 15: _t->_circleRadiusSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->_brushRadiusSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->_saveImageToFile((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->_saveImageToFile(); break;
        case 19: _t->_createRealtimeImageFiles(); break;
        case 20: _t->_createArchiveImageFiles(); break;
        case 21: _t->_createImageFilesAllLevels(); break;
        case 22: _t->_createImageFiles(); break;
        case 23: _t->ShowContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *CartManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CartManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCartManagerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return DisplayManager::qt_metacast(_clname);
}

int CartManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DisplayManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    return _id;
}
QT_WARNING_POP
