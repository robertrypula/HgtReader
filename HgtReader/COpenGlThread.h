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

#ifndef COPENGLTHREAD_H
#define COPENGLTHREAD_H

#include <QThread>
#include <QTime>
#include "COpenGl.h"
#include "CObjects.h"
#include "CEarth.h"

class COpenGl;

class COpenGlThread : public QThread
{
    Q_OBJECT

public:
    COpenGlThread(COpenGl *openGl);

    void resizeEvent(int w, int h);
    void resizeEvent();
    void stop();
    CEarth *earth;

protected:
    void run();

private:
    COpenGl *openGl;
    CObjects objects;
    QTime time;
    CDrawingStateSnapshot dss;
    QMutex doMutex;
    bool doResize;
    bool doTerminate;
    int doResizeWindowWidth;
    int doResizeWindowHeight;
    int windowWidth;
    int windowHeight;

    void checkSunLightningAndAtmosphere();
    void enableSunLight();
    void disableSunLight();
    void initializeScene();
    void resize(int w, int h, bool forSunRendering = false);
    void drawScene();
};


#endif // COPENGLTHREAD_H
