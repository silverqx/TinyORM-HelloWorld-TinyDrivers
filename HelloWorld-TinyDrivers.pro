QT -= gui

TEMPLATE = app

CONFIG *= cmdline

DEFINES += PROJECT_TINYORM_HELLOWORLD_TINYDRIVERS

SOURCES += $$PWD/main.cpp

# Auto-configure TinyORM library 🔥
include($$TINY_MAIN_DIR/TinyORM/qmake/TinyOrm.pri)
