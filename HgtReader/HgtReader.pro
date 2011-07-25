#-------------------------------------------------
#
# Project created by QtCreator 2011-05-15T22:57:38
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = HgtReader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CTerrain.cpp \
    CPerformance.cpp \
    COpenGl.cpp \
    CObjects.cpp \
    CHgtFile.cpp \
    CEarthPointsList.cpp \
    CEarth.cpp \
    CDrawingState.cpp \
    CCommons.cpp \
    CCamera.cpp \
    CCacheManager.cpp \
    CAvability.cpp \
    COpenGlThread.cpp \
    CTerrainLoaderThread.cpp \
    CDrawingStateSnapshot.cpp \
    CAnimationThread.cpp \
    CTerrainData.cpp \
    CCachedTerrainDataGroup.cpp \
    CCachedTerrainData.cpp \
    CRawFile.cpp

HEADERS  += mainwindow.h \
    CTerrain.h \
    CPerformance.h \
    COpenGl.h \
    CObjects.h \
    CHgtFile.h \
    CEarthPointsList.h \
    CEarth.h \
    CDrawingState.h \
    CCommons.h \
    CCamera.h \
    CCacheManager.h \
    CAvability.h \
    COpenGlThread.h \
    CTerrainLoaderThread.h \
    CDrawingStateSnapshot.h \
    CAnimationThread.h \
    CTerrainData.h \
    CCachedTerrainDataGroup.h \
    CCachedTerrainData.h \
    CRawFile.h

FORMS    += mainwindow.ui
