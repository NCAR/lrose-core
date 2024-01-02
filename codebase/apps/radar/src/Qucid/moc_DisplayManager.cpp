/****************************************************************************
** Meta object code from reading C++ file 'DisplayManager.hh'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DisplayManager.hh"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DisplayManager.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DisplayManager_t {
    QByteArrayData data[23];
    char stringdata0[231];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DisplayManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DisplayManager_t qt_meta_stringdata_DisplayManager = {
    {
QT_MOC_LITERAL(0, 0, 14), // "DisplayManager"
QT_MOC_LITERAL(1, 15, 12), // "frameResized"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 5), // "width"
QT_MOC_LITERAL(4, 35, 6), // "height"
QT_MOC_LITERAL(5, 42, 24), // "colorMapRedefineReceived"
QT_MOC_LITERAL(6, 67, 6), // "string"
QT_MOC_LITERAL(7, 74, 9), // "fieldName"
QT_MOC_LITERAL(8, 84, 8), // "ColorMap"
QT_MOC_LITERAL(9, 93, 11), // "newColorMap"
QT_MOC_LITERAL(10, 105, 6), // "_howto"
QT_MOC_LITERAL(11, 112, 6), // "_about"
QT_MOC_LITERAL(12, 119, 10), // "_showClick"
QT_MOC_LITERAL(13, 130, 7), // "_freeze"
QT_MOC_LITERAL(14, 138, 7), // "_unzoom"
QT_MOC_LITERAL(15, 146, 8), // "_refresh"
QT_MOC_LITERAL(16, 155, 12), // "_changeField"
QT_MOC_LITERAL(17, 168, 7), // "fieldId"
QT_MOC_LITERAL(18, 176, 7), // "guiMode"
QT_MOC_LITERAL(19, 184, 9), // "_openFile"
QT_MOC_LITERAL(20, 194, 9), // "_saveFile"
QT_MOC_LITERAL(21, 204, 20), // "_changeFieldVariable"
QT_MOC_LITERAL(22, 225, 5) // "value"

    },
    "DisplayManager\0frameResized\0\0width\0"
    "height\0colorMapRedefineReceived\0string\0"
    "fieldName\0ColorMap\0newColorMap\0_howto\0"
    "_about\0_showClick\0_freeze\0_unzoom\0"
    "_refresh\0_changeField\0fieldId\0guiMode\0"
    "_openFile\0_saveFile\0_changeFieldVariable\0"
    "value"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DisplayManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,   79,    2, 0x0a /* Public */,
      10,    0,   84,    2, 0x09 /* Protected */,
      11,    0,   85,    2, 0x09 /* Protected */,
      12,    0,   86,    2, 0x09 /* Protected */,
      13,    0,   87,    2, 0x09 /* Protected */,
      14,    0,   88,    2, 0x09 /* Protected */,
      15,    0,   89,    2, 0x09 /* Protected */,
      16,    2,   90,    2, 0x09 /* Protected */,
      19,    0,   95,    2, 0x09 /* Protected */,
      20,    0,   96,    2, 0x09 /* Protected */,
      21,    1,   97,    2, 0x09 /* Protected */,

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

void DisplayManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DisplayManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameResized((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 1: _t->colorMapRedefineReceived((*reinterpret_cast< string(*)>(_a[1])),(*reinterpret_cast< ColorMap(*)>(_a[2]))); break;
        case 2: _t->_howto(); break;
        case 3: _t->_about(); break;
        case 4: _t->_showClick(); break;
        case 5: _t->_freeze(); break;
        case 6: _t->_unzoom(); break;
        case 7: _t->_refresh(); break;
        case 8: _t->_changeField((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 9: _t->_openFile(); break;
        case 10: _t->_saveFile(); break;
        case 11: _t->_changeFieldVariable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DisplayManager::*)(const int , const int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DisplayManager::frameResized)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DisplayManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_DisplayManager.data,
    qt_meta_data_DisplayManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DisplayManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisplayManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DisplayManager.stringdata0))
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
            *reinterpret_cast<int*>(_a[0]) = -1;
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
QT_END_MOC_NAMESPACE
