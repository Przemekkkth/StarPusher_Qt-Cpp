greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
    src/game_state.h \
    src/gamescene.h \
    src/level.h \
    src/pixmapmanager.h \
    src/resource_holder.h \
    src/utils.h \
    src/view.h

SOURCES += \
    src/gamescene.cpp \
    src/main.cpp \
    src/pixmapmanager.cpp \
    src/view.cpp

RESOURCES += \
    resources.qrc
