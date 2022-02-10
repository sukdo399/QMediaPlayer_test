QT += multimedia multimediawidgets
QT += widgets
TARGET = videowidget
TEMPLATE = app

HEADERS = \
    videoplayer.h

SOURCES = \
    main.cpp \
    videoplayer.cpp

RESOURCES += \
    resources.qrc

qnx: target.path = /lge/app_ro/testsw/bin/
else: unix:!android: target.path = /lge/app_ro/testsw/bin/
!isEmpty(target.path): INSTALLS += target

