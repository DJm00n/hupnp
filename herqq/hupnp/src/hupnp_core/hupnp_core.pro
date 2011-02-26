TEMPLATE = lib
TARGET   = HUpnp
QT      += network xml
QT      -= gui
CONFIG  += warn_on dll thread
DEFINES += H_BUILD_UPNP_CORE_LIB
VERSION = 0.9.1

INCLUDEPATH += ../../include/

CONFIG(DISABLE_QTSOAP) {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtSoap
}
else {
    INCLUDEPATH += ../../lib/qtsoap-2.7-opensource/src
}

LIBS += -L"../../bin/"
!CONFIG(DISABLE_QTSOAP): LIBS += -L"../../lib/qtsoap-2.7-opensource/lib"

debug:DEFINES += DEBUG

win32 {
    debug {
        LIBS += -lQtSolutions_SOAP-2.7d
    }
    else {
        LIBS += -lQtSolutions_SOAP-2.7
    }

    LIBS += -lws2_32
    TARGET_EXT = .dll
}
else {
    LIBS += -lQtSolutions_SOAP-2.7
}

OBJECTS_DIR = obj
DESTDIR     = ../../bin
MOC_DIR     = obj

SRC_LOC = ../utils

include (../utils/utils.pri)

SRC_LOC = .

include (http/http.pri)
include (ssdp/ssdp.pri)
include (socket/socket.pri)
include (general/general.pri)
include (devicemodel/devicemodel.pri)
include (dataelements/dataelements.pri)
include (devicehosting/devicehosting.pri)

!CONFIG(DISABLE_QTSOAP) {
    win32 {
        QMAKE_POST_LINK += copy ..\\..\\lib\\qtsoap-2.7-opensource\\lib\\* ..\\..\\bin /Y
    }
    else {
        QMAKE_POST_LINK += cp -fR ../../lib/qtsoap-2.7-opensource/lib/* ../../bin/
    }
}

isEmpty(PREFIX) {
    PREFIX = ../../deploy
}

INSTLOC_INC = $$PREFIX/include/HUpnpCore
INSTLOC_LIB = $$PREFIX/lib

includes.files += ../../include/HUpnpCore/H*
includes.path = $$INSTLOC_INC

public_headers.files = $$find(HEADERS, ^((?!_p).)*$)*
public_headers.path = $$INSTLOC_INC/public

private_headers.files = $$EXPORTED_PRIVATE_HEADERS
private_headers.path = $$INSTLOC_INC/private

target.path = $$INSTLOC_LIB

INSTALLS += includes public_headers private_headers target
