TEMPLATE = lib
TARGET   = HUpnp
QT      += network xml
CONFIG  += warn_on dll exceptions
DEFINES += H_BUILD_UPNP_CORE_LIB

QMAKE_CXXFLAGS_WARN_ON +=

INCLUDEPATH += \
    ./../../../../lib/qtsoap-2.7-opensource/src

LIBS += -L"../../../../bin/" -lHCore -lHUtils
LIBS += -L"../../../../lib/qtsoap-2.7-opensource/lib"

win32{
    LIBS += -lQtSolutions_SOAP-2.7d
    LIBS += -lwsock32
}
else{
    LIBS += -lQtSolutions_SOAP-2.7
}

OBJECTS_DIR = obj
DESTDIR     = ../../../../bin
MOC_DIR     = obj

SRC_LOC = .

include (core.pri)
include (utils/utils.pri)
include (basic/basic.pri)
include (messaging/messaging.pri)
include (devicemodel/devicemodel.pri)
include (devicehosting/devicehosting.pri)
include (dataelements/dataelements.pri)
