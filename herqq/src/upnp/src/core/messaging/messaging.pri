HEADERS += \
    $$SRC_LOC/messaging/usn.h \
    $$SRC_LOC/messaging/ssdp.h \
    $$SRC_LOC/messaging/ssdp_p.h \
    $$SRC_LOC/messaging/product_tokens.h \
    $$SRC_LOC/messaging/discovery_messages.h \
    $$SRC_LOC/messaging/event_messages_p.h \
    $$SRC_LOC/messaging/control_messages_p.h \
    $$SRC_LOC/messaging/http_server_p.h \
    $$SRC_LOC/messaging/http_handler_p.h \
    $$SRC_LOC/messaging/http_messaginginfo_p.h \
    $$SRC_LOC/messaging/ssdp_messageheader_objects_p.h \
    $$SRC_LOC/messaging/resource_identifier.h \
    $$SRC_LOC/messaging/multicast_socket.h \
    $$SRC_LOC/messaging/endpoint.h

SOURCES += \
    $$SRC_LOC/messaging/usn.cpp \
    $$SRC_LOC/messaging/ssdp.cpp \
    $$SRC_LOC/messaging/product_tokens.cpp \
    $$SRC_LOC/messaging/discovery_messages.cpp \
    $$SRC_LOC/messaging/event_messages_p.cpp \
    $$SRC_LOC/messaging/control_messages_p.cpp \
    $$SRC_LOC/messaging/http_server_p.cpp \
    $$SRC_LOC/messaging/http_handler_p.cpp \
    $$SRC_LOC/messaging/http_messaginginfo_p.cpp \
    $$SRC_LOC/messaging/ssdp_messageheader_objects_p.cpp \
    $$SRC_LOC/messaging/resource_identifier.cpp \
    $$SRC_LOC/messaging/endpoint.cpp

win32 {
    SOURCES += $$SRC_LOC/messaging/win32/multicast_socket.cpp
}
else {
    SOURCES += $$SRC_LOC/messaging/linux/multicast_socket.cpp
}
