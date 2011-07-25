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

#ifndef CTERRAINLOADERTHREAD_H
#define CTERRAINLOADERTHREAD_H

#include <QThread>
#include "COpenGl.h"

class COpenGl;

class CTerrainLoaderThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void SIGNALupdateCacheInfo(int cachedTDCount, int cachedTDInUseCount, int cachedTDNotInUseCount,
                               int cachedTDEmptyEntryCount, unsigned int cacheMinNotInUseTime);

public:
    CTerrainLoaderThread(COpenGl *openGl);

    void stop();
    CEarth *earth;

public slots:
    void SLOTclearCache();

protected:
    void run();

private:
    COpenGl *openGl;
    QTime time;
    CDrawingStateSnapshot dss;
    QMutex doMutex;
    bool doTerminate;
    bool doClearCache;
};


#endif // CTERRAINLOADERTHREAD_H
