/****************************************************************************
** Meta object code from reading C++ file 'DisplayManager.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DisplayManager.hh"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DisplayManager.hh' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDisplayManagerENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSDisplayManagerENDCLASS = QtMocHelpers::stringData(
    "DisplayManager",
    "frameResized",
    "",
    "width",
    "height",
    "colorMapRedefineReceived",
    "string",
    "fieldName",
    "ColorMap",
    "newColorMap",
    "_howto",
    "_about",
    "_showClick",
    "_freeze",
    "_unzoom",
    "_refresh",
    "_changeField",
    "fieldId",
    "guiMode",
    "_openFile",
    "_saveFile",
    "_changeFieldVariable",
    "value"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDisplayManagerENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   86,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    2,   91,    2, 0x0a,    4 /* Public */,
      10,    0,   96,    2, 0x09,    7 /* Protected */,
      11,    0,   97,    2, 0x09,    8 /* Protected */,
      12,    0,   98,    2, 0x09,    9 /* Protected */,
      13,    0,   99,    2, 0x09,   10 /* Protected */,
      14,    0,  100,    2, 0x09,   11 /* Protected */,
      15,    0,  101,    2, 0x09,   12 /* Protected */,
      16,    2,  102,    2, 0x09,   13 /* Protected */,
      19,    0,  107,    2, 0x09,   16 /* Protected */,
      20,    0,  108,    2, 0x09,   17 /* Protected */,
      21,    1,  109,    2, 0x09,   18 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   17,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   22,

       0        // eod
};

Q_CONSTINIT const QMetaObject DisplayManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSDisplayManagerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDisplayManagerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDisplayManagerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DisplayManager, std::true_type>,
        // method 'frameResized'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const int, std::false_type>,
        // method 'colorMapRedefineReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<string, std::false_type>,
        QtPrivate::TypeAndForceComplete<ColorMap, std::false_type>,
        // method '_howto'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_about'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_showClick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
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
        // method '_openFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_saveFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_changeFieldVariable'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void DisplayManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DisplayManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameResized((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->colorMapRedefineReceived((*reinterpret_cast< std::add_pointer_t<string>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ColorMap>>(_a[2]))); break;
        case 2: _t->_howto(); break;
        case 3: _t->_about(); break;
        case 4: _t->_showClick(); break;
        case 5: _t->_freeze(); break;
        case 6: _t->_unzoom(); break;
        case 7: _t->_refresh(); break;
        case 8: _t->_changeField((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 9: _t->_openFile(); break;
        case 10: _t->_saveFile(); break;
        case 11: _t->_changeFieldVariable((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DisplayManager::*)(const int , const int );
            if (_t _q_method = &DisplayManager::frameResized; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *DisplayManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisplayManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDisplayManagerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int DisplayManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void DisplayManager::frameResized(const int _t1, const int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
