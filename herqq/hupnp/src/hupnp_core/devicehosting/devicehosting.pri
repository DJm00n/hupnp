
HEADERS += \
    $$SRC_LOC/devicehosting/hdevicestorage_p.h \
    $$SRC_LOC/devicehosting/hdevicecreator.h \
    $$SRC_LOC/devicehosting/habstracthost_p.h \
    $$SRC_LOC/devicehosting/hobjectcreator_p.h \
    $$SRC_LOC/devicehosting/hdefaultdevice.h \
    $$SRC_LOC/devicehosting/messages/hcontrol_messages_p.h \
    $$SRC_LOC/devicehosting/messages/hevent_messages_p.h \
    $$SRC_LOC/devicehosting/messages/hnt_p.h \
    $$SRC_LOC/devicehosting/messages/hsid_p.h \
    $$SRC_LOC/devicehosting/messages/htimeout_p.h \
    $$SRC_LOC/devicehosting/hdevicehosting_exceptions_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hactioninvoke_proxy_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint.h \
    $$SRC_LOC/devicehosting/controlpoint/hdevicebuild_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_configuration.h \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_configuration_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_dataretriever_p.h \
    $$SRC_LOC/devicehosting/controlpoint/hevent_subscription_p.h \
	$$SRC_LOC/devicehosting/controlpoint/hevent_subscriptionmanager_p.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_p.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_dataretriever_p.h \
    $$SRC_LOC/devicehosting/devicehost/hevent_notifier_p.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_configuration.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_configuration_p.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_ssdp_handler_p.h \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_http_server_p.h \
    $$SRC_LOC/devicehosting/devicehost/hpresence_announcer_p.h \
    $$SRC_LOC/devicehosting/devicehost/hevent_subscriber_p.h

SOURCES += \
    $$SRC_LOC/devicehosting/hdevicestorage_p.cpp \
    $$SRC_LOC/devicehosting/habstracthost_p.cpp \
    $$SRC_LOC/devicehosting/hobjectcreator_p.cpp \
    $$SRC_LOC/devicehosting/hdefaultdevice.cpp \
    $$SRC_LOC/devicehosting/messages/hcontrol_messages_p.cpp \
    $$SRC_LOC/devicehosting/messages/hevent_messages_p.cpp \
    $$SRC_LOC/devicehosting/messages/hnt_p.cpp \
    $$SRC_LOC/devicehosting/messages/hsid_p.cpp \
    $$SRC_LOC/devicehosting/messages/htimeout_p.cpp \
    $$SRC_LOC/devicehosting/hdevicehosting_exceptions_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hactioninvoke_proxy_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hdevicebuild_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_configuration.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hcontrolpoint_dataretriever_p.cpp \
    $$SRC_LOC/devicehosting/controlpoint/hevent_subscription_p.cpp \
	$$SRC_LOC/devicehosting/controlpoint/hevent_subscriptionmanager_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost.cpp \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_dataretriever_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/hevent_notifier_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_configuration.cpp \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_ssdp_handler_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/hdevicehost_http_server_p.cpp \
    $$SRC_LOC/devicehosting/devicehost/hevent_subscriber_p.cpp
