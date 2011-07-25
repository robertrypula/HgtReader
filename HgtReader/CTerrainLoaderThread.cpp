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

#include <QDebug>
#include "CTerrainLoaderThread.h"


CTerrainLoaderThread::CTerrainLoaderThread(COpenGl *openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    doTerminate = false;
    doClearCache = false;

    // set earth pointer to earth "B"
    earth = openGl->earthBufferB;

    // set in Earth DrawingStateSnapshot object
    earth->setDrawingStateSnapshot(&dss);
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
    earth->initLOD_0();
}

void CTerrainLoaderThread::stop()
{
    doMutex.lock();
    doTerminate = true;
    doMutex.unlock();
    openGl->earthBufferExchange.wakeOne();
}

void CTerrainLoaderThread::SLOTclearCache()
{
    doMutex.lock();
    doClearCache = true;
    doMutex.unlock();
}

void CTerrainLoaderThread::run()
{
    int cachedTDCount, cachedTDInUseCount, cachedTDNotInUseCount, cachedTDEmptyEntryCount;
    unsigned int cacheMinNotInUseTime;

    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state

    while (true) {
        time.start();

        if (dss.treeUpdating)  earth->updateTerrainTree();

        doMutex.lock();
        if (doClearCache) {
            openGl->cacheManager.cacheClear(earth);
            doClearCache = false;
        }
        if (doTerminate) {
            doMutex.unlock();
            return;
        }
        doMutex.unlock();

        openGl->cacheManager.cacheInfo(&cachedTDCount, &cachedTDInUseCount, &cachedTDNotInUseCount, &cachedTDEmptyEntryCount, &cacheMinNotInUseTime);
        openGl->cacheManager.cacheKeepSize(earth);
        openGl->cacheManager.cacheInfo(&cachedTDCount, &cachedTDInUseCount, &cachedTDNotInUseCount, &cachedTDEmptyEntryCount, &cacheMinNotInUseTime);
        emit SIGNALupdateCacheInfo(cachedTDCount, cachedTDInUseCount, cachedTDNotInUseCount, cachedTDEmptyEntryCount, cacheMinNotInUseTime);


        openGl->earthBufferMutex.lock();
        openGl->earthBufferReadyToExchange = true;
        openGl->earthBufferExchange.wait(&openGl->earthBufferMutex);
        earth->setDrawingStateSnapshot(&dss);
        openGl->earthBufferMutex.unlock();

        openGl->drawingState.getDrawingStateSnapshot(&dss);  // get current scene state

        // update performance info
        openGl->performance.setTerrainTreeUpdatingTime(time.elapsed());
        openGl->performance.updateTerrainTreeUpdatingInfo();
    }
}
