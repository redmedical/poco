
include(../common.pri)

TARGET = PocoCryptomd
    
#LIBS +=$(OPENSSL_LIBS)  
INCLUDEPATH +=$(OPENSSL_INC)    

include(Crypto.pri)
