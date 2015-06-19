
include(../common.pri)

TARGET = PocoNetSSLmd
INCLUDEPATH += \
    ./../Net/include \
    ./../Util/include \
    ./../Crypto/include

#LIBS +=$(OPENSSL_LIBS)  
INCLUDEPATH +=$(OPENSSL_INC)  

include(NetSSL_OpenSSL.pri)
