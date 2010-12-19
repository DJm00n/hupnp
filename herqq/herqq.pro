TEMPLATE = subdirs
CONFIG  += ordered

CONFIG += \
    build_qtsoap \
    build_core \
    build_test_app

build_qtsoap {
    SUBDIRS += hupnp/lib/qtsoap-2.7-opensource/buildlib
}

build_core {
    SUBDIRS += hupnp/src/hupnp_core
}

build_test_app {
    SUBDIRS += apps/simple_test-app
}
