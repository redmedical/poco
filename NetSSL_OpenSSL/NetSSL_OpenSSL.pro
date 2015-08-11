
TARGET = PocoNetSSLmd
include(../common.pri)

INCLUDEPATH += \
    ./../Net/include \
    ./../Util/include \
    ./../Crypto/include

#LIBS +=$(OPENSSL_LIBS)  
INCLUDEPATH +=$(OPENSSL_INC)  

include(NetSSL_OpenSSL.pri)
