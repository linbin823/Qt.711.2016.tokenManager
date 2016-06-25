/****************************************************************************
** Meta object code from reading C++ file 'tokenmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../deleteMe/tokenmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tokenmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_tokenManager_t {
    QByteArrayData data[13];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_tokenManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_tokenManager_t qt_meta_stringdata_tokenManager = {
    {
QT_MOC_LITERAL(0, 0, 12), // "tokenManager"
QT_MOC_LITERAL(1, 13, 24), // "masterPeerMessageUpdated"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 3), // "msg"
QT_MOC_LITERAL(4, 43, 16), // "peerStateChanged"
QT_MOC_LITERAL(5, 60, 12), // "peerState_e&"
QT_MOC_LITERAL(6, 73, 5), // "state"
QT_MOC_LITERAL(7, 79, 21), // "peerErrorStateChanged"
QT_MOC_LITERAL(8, 101, 17), // "peerErrorState_e&"
QT_MOC_LITERAL(9, 119, 20), // "tokenTakeOutOvertime"
QT_MOC_LITERAL(10, 140, 19), // "tokenTakeInOvertime"
QT_MOC_LITERAL(11, 160, 21), // "tokenOrderOutOvertime"
QT_MOC_LITERAL(12, 182, 20) // "tokenOrderInOvertime"

    },
    "tokenManager\0masterPeerMessageUpdated\0"
    "\0msg\0peerStateChanged\0peerState_e&\0"
    "state\0peerErrorStateChanged\0"
    "peerErrorState_e&\0tokenTakeOutOvertime\0"
    "tokenTakeInOvertime\0tokenOrderOutOvertime\0"
    "tokenOrderInOvertime"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_tokenManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    1,   52,    2, 0x06 /* Public */,
       7,    1,   55,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   58,    2, 0x0a /* Public */,
      10,    0,   59,    2, 0x0a /* Public */,
      11,    0,   60,    2, 0x0a /* Public */,
      12,    0,   61,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void tokenManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        tokenManager *_t = static_cast<tokenManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->masterPeerMessageUpdated((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->peerStateChanged((*reinterpret_cast< peerState_e(*)>(_a[1]))); break;
        case 2: _t->peerErrorStateChanged((*reinterpret_cast< peerErrorState_e(*)>(_a[1]))); break;
        case 3: _t->tokenTakeOutOvertime(); break;
        case 4: _t->tokenTakeInOvertime(); break;
        case 5: _t->tokenOrderOutOvertime(); break;
        case 6: _t->tokenOrderInOvertime(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (tokenManager::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&tokenManager::masterPeerMessageUpdated)) {
                *result = 0;
            }
        }
        {
            typedef void (tokenManager::*_t)(peerState_e & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&tokenManager::peerStateChanged)) {
                *result = 1;
            }
        }
        {
            typedef void (tokenManager::*_t)(peerErrorState_e & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&tokenManager::peerErrorStateChanged)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject tokenManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_tokenManager.data,
      qt_meta_data_tokenManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *tokenManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *tokenManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_tokenManager.stringdata0))
        return static_cast<void*>(const_cast< tokenManager*>(this));
    return QObject::qt_metacast(_clname);
}

int tokenManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void tokenManager::masterPeerMessageUpdated(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void tokenManager::peerStateChanged(peerState_e & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void tokenManager::peerErrorStateChanged(peerErrorState_e & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
