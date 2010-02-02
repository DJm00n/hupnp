TEMPLATE = lib
TARGET   = HUtils
QT      -= gui
CONFIG  += warn_on static exceptions

OBJECTS_DIR = obj
DESTDIR     = ../../bin
MOC_DIR     = obj

SRC_LOC = src
include (utils.pri)
