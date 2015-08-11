
TARGET = PocoCryptomd
include(../common.pri)

    
#LIBS +=$(OPENSSL_LIBS)  
INCLUDEPATH +=$(OPENSSL_INC)    

include(Crypto.pri)
