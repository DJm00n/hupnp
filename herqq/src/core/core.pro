TEMPLATE = lib
TARGET   = HCore
QT      -= gui
CONFIG  += warn_on dll exceptions
DEFINES += H_BUILD_CORE_LIB

OBJECTS_DIR = obj
DESTDIR     = ../../bin
MOC_DIR     = obj

SRC_LOC = src

include (core.pri)
