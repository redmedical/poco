TEMPLATE = lib
DESTDIR = ../lib
CONFIG -= qt
CONFIG += staticlib
DEFINES += POCO_STATIC


CONFIG(debug, debug|release) {
     mac: TARGET = $$join(TARGET,,,_debug)
     win32: TARGET = $$join(TARGET,,,d)
}

INCLUDEPATH += ./include \
    ./../Foundation/include

win32 {
 QMAKE_CXXFLAGS += /wd4100
}


targ_headers.files = include/*
targ_headers.path = $$OUT_PWD/../include
INSTALLS += targ_headers


#backup
    
#LIBS += \
#    -lws2_32 \
#    -liphlpapi \
#    -lCrypt32

#LIBS += -L"." \
