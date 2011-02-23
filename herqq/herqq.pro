TEMPLATE = subdirs
CONFIG  += ordered

!CONFIG(DISABLE_QTSOAP) : SUBDIRS += hupnp/lib/qtsoap-2.7-opensource/buildlib
!CONFIG(DISABLE_CORE) : SUBDIRS += hupnp/src/hupnp_core
!CONFIG(DISABLE_TESTAPP) : SUBDIRS += apps/simple_test-app
