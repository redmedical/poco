#####################################################################
# Main projectfile
#####################################################################
CONFIG -= qt
CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = Foundation Crypto Util XML Net NetSSL_OpenSSL


#cross_compile: CONFIG += nostrip


