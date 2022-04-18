QT       += core gui serialbus serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    canbus_interface.cpp \
    chartview.cpp \
    iio_model.cpp \
    main.cpp \
    mainwindow.cpp \
    pid_datamodel.cpp

HEADERS += \
    canbus_interface.h \
    chartview.h \
    iio_model.h \
    mainwindow.h \
    pid_datamodel.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
