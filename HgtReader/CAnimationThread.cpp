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

#include "CAnimationThread.h"
#include "CCommons.h"


CAnimationThread::CAnimationThread(COpenGl *openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    doTerminate = false;
    doAnimate = false;
    doBenchmark = false;

    benchmarkLon[0] = 20.088333; benchmarkLat[0] = 49.179444; benchmarkAlt[0] = CONST_EARTH_RADIUS + 2503.000;
    benchmarkLon[1] = 21.101202; benchmarkLat[1] = 47.123456; benchmarkAlt[1] = CONST_EARTH_RADIUS + 1500.0;
    benchmarkLon[2] = 41.101202; benchmarkLat[2] = -17.123456; benchmarkAlt[2] = CONST_EARTH_RADIUS + 2340.0;
    benchmarkLon[3] = 21.101202; benchmarkLat[3] = 37.123456; benchmarkAlt[3] = CONST_EARTH_RADIUS + 9030.0;
    benchmarkLon[4] = 301.101202; benchmarkLat[4] = 47.123456; benchmarkAlt[4] = CONST_EARTH_RADIUS + 34.0;
    benchmarkLon[5] = 20.088333; benchmarkLat[5] = 49.179444; benchmarkAlt[5] = CONST_EARTH_RADIUS + 2503.000;
    benchmarkPos = 0;
}

void CAnimationThread::stop()
{
    doMutex.lock();
    doTerminate = true;
    doMutex.unlock();
}

void CAnimationThread::SLOTanimateToEarthPoint(double currEarthPointLon, double currEarthPointLat, double currEarthPointAlt,
                                               double animEarthPointLon, double animEarthPointLat, double animEarthPointAlt)
{
    doMutex.lock();
    startLon = currEarthPointLon;
    startLat = currEarthPointLat;
    startAlt = currEarthPointAlt;
    stopLon = animEarthPointLon;
    stopLat = animEarthPointLat;
    stopAlt = animEarthPointAlt;

    deltaLon = stopLon - startLon;
    if (deltaLon>180.0)
        deltaLon = deltaLon - 360.0;
    if (deltaLon<-180.0)
        deltaLon = deltaLon + 360.0;

    deltaLat = stopLat - startLat;
    deltaAlt = stopAlt - startAlt;

    if (deltaLon==0 && deltaLat==0 && deltaAlt==0) {
        doAnimate = false;
    } else {
        doAnimate = true;
        time.start();
        deltaLonLatLength = sqrt(deltaLon*deltaLon + deltaLat*deltaLat);
        openGl->performance.addEventToHistory( QString("[ANIM START] lon: ") +
                                               QString::number(startLon, 'f', 6) + QString(" lat: ") +
                                               QString::number(startLat, 'f', 6) + QString(" alt: ") +
                                               QString::number(startAlt - CONST_EARTH_RADIUS, 'f', 3)
                                             );
    }
    doMutex.unlock();
}

void CAnimationThread::SLOTstartBenchmark()
{
    openGl->performance.resetHistory();
    openGl->performance.enableSavingToHistory();
    doMutex.lock();
    doAnimate = false;
    doBenchmark = true;
    benchmarkPos = 0;
    doMutex.unlock();
    openGl->drawingState.getCamera()->setEarthPointLonLatAlt(benchmarkLon[0], benchmarkLat[0], benchmarkAlt[0], true);
}

void CAnimationThread::manageBenchmark()
{
    bool startAnim = false;
    int posAnim;

    doMutex.lock();
    if (!doBenchmark || doAnimate) {
        doMutex.unlock();
        return;
    }

    if (benchmarkPos<BENCHMARK_LOCATIONS-1) {
        startAnim = true;
        posAnim = benchmarkPos;
        benchmarkPos++;
    } else {
        startAnim = false;
        doBenchmark = false;
        openGl->performance.disableSavingToHistory();
    }
    doMutex.unlock();

    if (startAnim)
        SLOTanimateToEarthPoint(benchmarkLon[posAnim], benchmarkLat[posAnim], benchmarkAlt[posAnim],
                                benchmarkLon[posAnim+1], benchmarkLat[posAnim+1], benchmarkAlt[posAnim+1]);
}

void CAnimationThread::run()
{
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state

    while (true) {

        doMutex.lock();
        if (doTerminate) {
            doMutex.unlock();
            return;
        }
        doMutex.unlock();


        // animation code here
        openGl->drawingState.getCamera()->checkInteractKeys();       // interact keys
        animateEarthPoint();
        manageBenchmark();

        msleep(ANIMATION_SPEED_MS);
        openGl->drawingState.getDrawingStateSnapshot(&dss);  // get current scene state
    }
}

void CAnimationThread::animateEarthPoint()
{
    double cosFunc, cosFuncAlt;
    double animPositionAlt, animPosition, animPositionInUnit;
    double lon, lat, alt;
    double timeElapsed = time.elapsed();

    doMutex.lock();

    if (!doAnimate) {
        doMutex.unlock();
        return;
    }

    if (timeElapsed>ANIMATION_EP_DURATION_MS) {
        doAnimate = false;
        openGl->drawingState.getCamera()->setEarthPointLonLatAlt(stopLon, stopLat, stopAlt, true);
        openGl->performance.addEventToHistory( QString("[ANIM STOP] lon: ") +
                                               QString::number(stopLon, 'f', 6) + QString(" lat: ") +
                                               QString::number(stopLat, 'f', 6) + QString(" alt: ") +
                                               QString::number(stopAlt - CONST_EARTH_RADIUS, 'f', 3)
                                             );
        doMutex.unlock();
        return;
    }


    animPositionInUnit = timeElapsed/ANIMATION_EP_DURATION_MS;
    animPosition = animPositionInUnit * CONST_PI;
    animPositionAlt = animPositionInUnit * 2.0 * CONST_PI;
    cosFunc = (cos(-CONST_PI + animPosition )+1.0)/2.0;
    cosFuncAlt = (cos(-CONST_PI + animPositionAlt )+1.0)/2.0;

    lon = startLon + deltaLon*cosFunc;
    lat = startLat + deltaLat*cosFunc;
    alt = startAlt + deltaAlt*cosFunc + ANIMATION_EP_ALT*cosFuncAlt*(deltaLonLatLength/254.56);

    if (lon<0.0) lon += 360.0;
    if (lon>360.0) lon -= 360.0;

    openGl->drawingState.getCamera()->setEarthPointLonLatAlt(lon, lat, alt, true);

    doMutex.unlock();
}
