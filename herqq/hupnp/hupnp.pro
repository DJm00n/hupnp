TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS  = \
    src/hupnp_core

copy_target.target = FORCE

win32{
    copy_target.commands = copy lib\qtsoap-2.7-opensource\lib\* bin\
}
else{
    copy_target.commands = cp lib/qtsoap-2.7-opensource/lib/* bin/
}

QMAKE_EXTRA_TARGETS += copy_target
