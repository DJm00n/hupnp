HEADERS += \
    $$SRC_LOC/socket/hmulticast_socket.h \
    $$SRC_LOC/socket/hendpoint.h

SOURCES += \
    $$SRC_LOC/socket/hendpoint.cpp

win32 {
    SOURCES += $$SRC_LOC/socket/win32/hmulticast_socket.cpp
}
else {
    SOURCES += $$SRC_LOC/socket/linux/hmulticast_socket.cpp
}
