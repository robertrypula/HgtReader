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

#include <math.h>
#include <fstream>
#include <iostream>
#include "COpenGl.h"
#include "CCommons.h"


using namespace std;


COpenGl::COpenGl(QGLFormat glFormat, QWidget *parent) : QGLWidget(glFormat, parent)
{
    // create Earth Buffers
    earthBufferA = new CEarth;
    earthBufferB = new CEarth;

    setFocusPolicy(Qt::StrongFocus);

    // set mutex for safe thread access to drawing state ( !! very important !! )
    drawingState.setDrawingStateMutex(&drawingStateMutex);
    earthBufferReadyToExchange = false;

    // tell cache manager about earths
    cacheManager.setEarthBuffers(earthBufferA, earthBufferB);

    QObject::connect(drawingState.getCamera(), SIGNAL(SIGNALforceResize()), this, SLOT(SLOTforceResize()));

    // start openGL drawing thread
    openGlThread = new COpenGlThread(this);
    openGlThread->start();

    // start terrain loading thread
    terrainLoaderThread = new CTerrainLoaderThread(this);
    terrainLoaderThread->start();

    // start animation thread
    animationThread = new CAnimationThread(this);
    animationThread->start();
}

COpenGl::~COpenGl()
{
    // stop threads
    animationThread->stop();
    animationThread->wait();
    terrainLoaderThread->stop();
    terrainLoaderThread->wait();
    openGlThread->stop();
    openGlThread->wait();

    delete animationThread;
    delete terrainLoaderThread;
    delete openGlThread;

    delete earthBufferA;
    delete earthBufferB;
}

QSize COpenGl::minimumSizeHint() const
{
    return QSize(80, 60);
}

QSize COpenGl::sizeHint() const
{
    return QSize(CONST_DEF_WIDTH, CONST_DEF_HEIGHT);
}

void COpenGl::initializeGL()
{
    QGLWidget::initializeGL();
}

void COpenGl::paintGL()
{
    QGLWidget::paintGL();
}

void COpenGl::resizeEvent(QResizeEvent *event)
{
    drawingState.getCamera()->setNewWindowSize(event->size().width(), event->size().height());
    openGlThread->resizeEvent(event->size().width(), event->size().height());
}

void COpenGl::paintEvent(QPaintEvent *event)
{
    // handled by opengl drawing thread
}

void COpenGl::SLOTforceResize()
{
    openGlThread->resizeEvent();
}

void COpenGl::keyPressEvent(QKeyEvent *event)
{
    if (!drawingState.getCamera()->keyPressEventHandler(event))
        QWidget::keyPressEvent(event);
}

void COpenGl::keyReleaseEvent(QKeyEvent *event)
{
    if (!drawingState.getCamera()->keyReleaseEventHandler(event))
        QWidget::keyReleaseEvent(event);
}

void COpenGl::mousePressEvent(QMouseEvent *event)
{
    drawingState.getCamera()->mousePressEventHandler(event);
}

void COpenGl::mouseReleaseEvent(QMouseEvent *event)
{
    drawingState.getCamera()->mouseReleaseEventHandler(event);
}

void COpenGl::mouseMoveEvent(QMouseEvent *event)
{
    drawingState.getCamera()->mouseMoveEventHandler(event);
}
