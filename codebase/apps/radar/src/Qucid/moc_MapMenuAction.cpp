/****************************************************************************
** Meta object code from reading C++ file 'MapMenuAction.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MapMenuAction.hh"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MapMenuAction.hh' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSMapMenuActionENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMapMenuActionENDCLASS = QtMocHelpers::stringData(
    "MapMenuAction",
    "toggled",
    "",
    "checked",
    "mapStatusToggled",
    "mapIndex"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMapMenuActionENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[14];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[17];
    char stringdata5[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMapMenuActionENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMapMenuActionENDCLASS_t qt_meta_stringdata_CLASSMapMenuActionENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "MapMenuAction"
        QT_MOC_LITERAL(14, 7),  // "toggled"
        QT_MOC_LITERAL(22, 0),  // ""
        QT_MOC_LITERAL(23, 7),  // "checked"
        QT_MOC_LITERAL(31, 16),  // "mapStatusToggled"
        QT_MOC_LITERAL(48, 8)   // "mapIndex"
    },
    "MapMenuAction",
    "toggled",
    "",
    "checked",
    "mapStatusToggled",
    "mapIndex"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMapMenuActionENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x0a,    1 /* Public */,
       4,    2,   29,    2, 0x0a,    3 /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,    3,    5,

       0        // eod
};

Q_CONSTINIT const QMetaObject MapMenuAction::staticMetaObject = { {
    QMetaObject::SuperData::link<QAction::staticMetaObject>(),
    qt_meta_stringdata_CLASSMapMenuActionENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMapMenuActionENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMapMenuActionENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MapMenuAction, std::true_type>,
        // method 'toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'mapStatusToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void MapMenuAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MapMenuAction *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->toggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->mapStatusToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *MapMenuAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MapMenuAction::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMapMenuActionENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QAction::qt_metacast(_clname);
}

int MapMenuAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
