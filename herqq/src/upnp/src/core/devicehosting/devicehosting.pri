
HEADERS += \
    $$SRC_LOC/devicehosting/devicestorage_p.h \
    $$SRC_LOC/devicehosting/devicecreator.h \
    $$SRC_LOC/devicehosting/abstracthost_p.h \
    $$SRC_LOC/devicehosting/abstracthost.h \
    $$SRC_LOC/devicehosting/objectcreator_p.h \
    $$SRC_LOC/devicehosting/defaultdevice.h \
    $$SRC_LOC/devicehosting/exceptions_p.h \
    $$SRC_LOC/devicehosting/controlpoint/actioninvoke_proxy_p.h \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_p.h \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint.h \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_configuration.h \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_configuration_p.h \
    $$SRC_LOC/devicehosting/controlpoint/service_subscription_p.h \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_dataretriever_p.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_p.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_dataretriever_p.h \
    $$SRC_LOC/devicehosting/devicehost/event_notifier_p.h \
    $$SRC_LOC/devicehosting/devicehost/service_event_subscriber_p.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_configuration.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_configuration_p.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_ssdp_handler_p.h \
    $$SRC_LOC/devicehosting/devicehost/devicehost_http_server_p.h \
    $$SRC_LOC/devicehosting/devicehost/presence_announcer_p.h

SOURCES += \
    $$SRC_LOC/devicehosting/devicestorage_p.cpp \
    $$SRC_LOC/devicehosting/abstracthost.cpp \
    $$SRC_LOC/devicehosting/objectcreator_p.cpp \
    $$SRC_LOC/devicehosting/defaultdevice.cpp \
    $$SRC_LOC/devicehosting/exceptions_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/actioninvoke_proxy_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint.cpp \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_configuration.cpp \
    $$SRC_LOC/devicehosting/controlpoint/service_subscription_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/controlpoint_dataretriever_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/devicehost.cpp \
    $$SRC_LOC/devicehosting/devicehost/devicehost_dataretriever_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/event_notifier_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/service_event_subscriber_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/devicehost_configuration.cpp \
    $$SRC_LOC/devicehosting/devicehost/devicehost_ssdp_handler_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/devicehost_http_server_p.cpp
