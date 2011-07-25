/*
 *   -------------------------------------------------------------------------
 *    HgtReader v1.0
 *                                                    (c) Robert Rypula 156520
 *                                   Wroclaw University of Technology - Poland
 *                                                      http://www.pwr.wroc.pl
 *                                                           2011.01 - 2011.06
 *   -------------------------------------------------------------------------
 *
 *   What is this:
 *     - graphic system based on OpenGL to visualize entire Earth including
 *       terrain topography & satellite images
 *     - part of my thesis "Rendering of complex 3D scenes"
 *
 *   What it use:
 *     - Nokia Qt cross-platform C++ application framework
 *     - OpenGL graphic library
 *     - NASA SRTM terrain elevation data:
 *         oryginal dataset
 *           http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/
 *         corrected part of earth:
 *           http://www.viewfinderpanoramas.org/dem3.html
 *         SRTM v4 highest quality SRTM dataset avaiable:
 *           http://srtm.csi.cgiar.org/
 *     - TrueMarble satellite images
 *         free version from Unearthed Outdoors (250m/pix):
 *           http://www.unearthedoutdoors.net/global_data/true_marble/download
 *     - ALGLIB cross-platform numerical analysis and data processing library
 *       for SRTM dataset bicubic interpolation from 90m to 103m (more
 *       flexible LOD division)
 *         ALGLIB website:
 *           http://www.alglib.net/
 *
 *   Contact to author:
 *            phone    +48 505-363-331
 *            e-mail   robert.rypula@gmail.com
 *            GG       1578139
 *
 *                                                   program under GNU licence
 *   -------------------------------------------------------------------------
 */

#ifndef COPENGL_H
#define COPENGL_H

#include <QtOpenGL>
#include <QString>
#include <QMutex>
#include <QTimer>
#include "CCacheManager.h"
#include "CPerformance.h"
#include "CDrawingState.h"
#include "CEarth.h"
#include "COpenGlThread.h"
#include "CTerrainLoaderThread.h"
#include "CAnimationThread.h"

class COpenGlThread;
class CTerrainLoaderThread;
class CAnimationThread;

class COpenGl : public QGLWidget
{
   Q_OBJECT

public:
    COpenGl(QGLFormat glFormat, QWidget *parent);
    ~COpenGl();

    CCacheManager cacheManager;
    CPerformance performance;
    CDrawingState drawingState;
    QMutex drawingStateMutex;
    CEarth *earthBufferA;
    CEarth *earthBufferB;
    QMutex earthBufferMutex;
    QWaitCondition earthBufferExchange;
    bool earthBufferReadyToExchange;
    COpenGlThread *openGlThread;
    CTerrainLoaderThread *terrainLoaderThread;
    CAnimationThread *animationThread;

public slots:
    void SLOTforceResize();

signals:
    protected:
    void initializeGL();
    void paintGL();
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

#endif // COPENGL_H
