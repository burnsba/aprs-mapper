#-------------------------------------------------
#
# Project created by QtCreator 2011-02-20T16:30:09
#
#-------------------------------------------------

QT       += core gui sql network xml xml xml xml xml xml

TARGET = aprs_mapper_v2
TEMPLATE = app
CONFIG += x86
SOURCES +=\
        aprsmain.cpp \
    point.cpp \
    map.cpp \
    mapgraphicsscene.cpp \
    connection.cpp \
    profilemenu.cpp \
    profile.cpp \
    parserthread.cpp \
    errorhandler.cpp \
    coordinate.cpp \
    main.cpp \
    overlay.cpp \
    parser.cpp \
    database.cpp \
    createnew.cpp \
    loadprofile.cpp \
    connectionmenu.cpp \
    aprs_parser.cpp \
    mydebug.cpp \
    about.cpp \
    packetslist.cpp \
    detailedpacketsinfo.cpp

HEADERS  += aprsmain.h \
    connection.h \
    profilemenu.h \
    profile.h \
    point.h \
    parserthread.h \
    mapgraphicsscene.h \
    map.h \
    errorhandler.h \
    coordinate.h \
    parser.h \
    overlay.h \
    packettypes.h \
    database.h \
    createnew.h \
    loadprofile.h \
    connectionmenu.h \
    aprs_parser.h \
    stringify.h \
    mydebug.h \
    about.h \
    packetslist.h \
    detailedpacketsinfo.h

FORMS  += aprsmain.ui \
    profilemenu.ui \
    createnew.ui \
    loadprofile.ui \
    connectionmenu.ui \
    about.ui \
    packetslist.ui \
    detailedpacketsinfo.ui

OTHER_FILES += \
    resource/profile.xml \
    resource/aprs_icons.png \
    resource/red-light.png \
    resource/green-light.png

RESOURCES += \
    resource/resources.qrc
