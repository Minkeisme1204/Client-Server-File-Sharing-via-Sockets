/****************************************************************************
** Meta object code from reading C++ file 'ServerWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../server/ServerWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ServerWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ServerWindow_t {
    QByteArrayData data[12];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ServerWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ServerWindow_t qt_meta_stringdata_ServerWindow = {
    {
QT_MOC_LITERAL(0, 0, 12), // "ServerWindow"
QT_MOC_LITERAL(1, 13, 13), // "onStartServer"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 12), // "onStopServer"
QT_MOC_LITERAL(4, 41, 16), // "onExecuteCommand"
QT_MOC_LITERAL(5, 58, 16), // "onMetricsClicked"
QT_MOC_LITERAL(6, 75, 16), // "onClientsClicked"
QT_MOC_LITERAL(7, 92, 18), // "onChangeDirClicked"
QT_MOC_LITERAL(8, 111, 16), // "onVerboseClicked"
QT_MOC_LITERAL(9, 128, 18), // "onExportCSVClicked"
QT_MOC_LITERAL(10, 147, 13), // "onQuitClicked"
QT_MOC_LITERAL(11, 161, 8) // "updateUI"

    },
    "ServerWindow\0onStartServer\0\0onStopServer\0"
    "onExecuteCommand\0onMetricsClicked\0"
    "onClientsClicked\0onChangeDirClicked\0"
    "onVerboseClicked\0onExportCSVClicked\0"
    "onQuitClicked\0updateUI"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ServerWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x08 /* Private */,
       3,    0,   65,    2, 0x08 /* Private */,
       4,    0,   66,    2, 0x08 /* Private */,
       5,    0,   67,    2, 0x08 /* Private */,
       6,    0,   68,    2, 0x08 /* Private */,
       7,    0,   69,    2, 0x08 /* Private */,
       8,    0,   70,    2, 0x08 /* Private */,
       9,    0,   71,    2, 0x08 /* Private */,
      10,    0,   72,    2, 0x08 /* Private */,
      11,    0,   73,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ServerWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ServerWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onStartServer(); break;
        case 1: _t->onStopServer(); break;
        case 2: _t->onExecuteCommand(); break;
        case 3: _t->onMetricsClicked(); break;
        case 4: _t->onClientsClicked(); break;
        case 5: _t->onChangeDirClicked(); break;
        case 6: _t->onVerboseClicked(); break;
        case 7: _t->onExportCSVClicked(); break;
        case 8: _t->onQuitClicked(); break;
        case 9: _t->updateUI(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject ServerWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_ServerWindow.data,
    qt_meta_data_ServerWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ServerWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ServerWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ServerWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int ServerWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
