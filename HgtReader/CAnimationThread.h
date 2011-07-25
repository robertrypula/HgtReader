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

#ifndef CANIMATIONTHREAD_H
#define CANIMATIONTHREAD_H

#include <QThread>
#include "COpenGl.h"


#define BENCHMARK_LOCATIONS 6

class COpenGl;

class CAnimationThread : public QThread
{
    Q_OBJECT

public:
    CAnimationThread(COpenGl *openGl);


    void stop();

public slots:
    void SLOTanimateToEarthPoint(double currEarthPointLon, double currEarthPointLat,
                                 double currEarthPointAlt, double animEarthPointLon,
                                 double animEarthPointLat, double animEarthPointAlt);
    void SLOTstartBenchmark();

protected:
    void run();

private:
    COpenGl *openGl;
    CDrawingStateSnapshot dss;
    QTime time;
    double startLon;
    double startLat;
    double startAlt;
    double stopLon;
    double stopLat;
    double stopAlt;
    double deltaLon;
    double deltaLat;
    double deltaAlt;
    double deltaLonLatLength;
    QMutex doMutex;
    bool doTerminate;
    bool doAnimate;
    double benchmarkLon[BENCHMARK_LOCATIONS];
    double benchmarkLat[BENCHMARK_LOCATIONS];
    double benchmarkAlt[BENCHMARK_LOCATIONS];
    int benchmarkPos;
    bool doBenchmark;

    void manageBenchmark();
    void animateEarthPoint();
};

#endif // CANIMATIONTHREAD_H
