QT += core gui network
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    logger.cpp \
    main.cpp \
    serverlogic.cpp \
    serverui.cpp

HEADERS += \
    logger.h \
    serverlogic.h \
    serverui.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

# Условное подключение GMP.pri
!exists($$PWD/QtBigInt/GMP.pri): {
    include($$PWD/QtBigInt/GMP.pri)
}

# Условное подключение Qt-Secret.pri
!exists($$PWD/Qt-Secret/src/Qt-Secret.pri): {
    include($$PWD/Qt-Secret/src/Qt-Secret.pri)
}
INCLUDEPATH += $$PWD/Qt-Secret/src/Qt-RSA
INCLUDEPATH += $$PWD/QtBigInt/src
QT += network

