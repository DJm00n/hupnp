TEMPLATE = lib
TARGET   = HUpnp
QT      += network xml
CONFIG  += warn_on dll exceptions thread
DEFINES += H_BUILD_UPNP_CORE_LIB

QMAKE_CXXFLAGS_WARN_ON +=

INCLUDEPATH += \
    ../../include/ \
    ../../lib/qtsoap-2.7-opensource/src

LIBS += -L"../../bin/"
LIBS += -L"../../lib/qtsoap-2.7-opensource/lib"

win32 {
    debug {
        LIBS += -lQtSolutions_SOAP-2.7d
    }
    else {
        LIBS += -lQtSolutions_SOAP-2.7
    }

    LIBS += -lws2_32
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
include (datatypes/datatypes.pri)
include (devicemodel/devicemodel.pri)
include (dataelements/dataelements.pri)
include (devicehosting/devicehosting.pri)

win32 {
    QMAKE_POST_LINK += copy ..\..\lib\qtsoap-2.7-opensource\lib\* ..\..\bin /Y
}
else {
    QMAKE_POST_LINK += cp -fR ../../lib/qtsoap-2.7-opensource/lib/* ../../bin/
}

public_headers.files = $$find(HEADERS, ^((?!_p).)*$)
public_headers.path = ../../deploy/include/HUpnpCore/public

private_headers.files = $$EXPORTED_PRIVATE_HEADERS
private_headers.path = ../../deploy/include/HUpnpCore/private

win32{
    core_lib.files = ../../bin/hupnp.dll ../../bin/QtSolutions_SOAP-2.7*
}
macx{
    core_lib.files = ../../bin/libHUpnp.dylib ../../bin/libQtSolutions_SOAP-2.7.dylib
}
else{
    core_lib.files = ../../bin/libHUpnp.so ../../bin/libQtSolutions_SOAP-2.7.so
}

core_lib.path = ../../deploy/bin/

INSTALLS += public_headers private_headers core_lib
