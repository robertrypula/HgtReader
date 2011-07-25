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
#include "CEarth.h"
#include "CCacheManager.h"
#include "CPerformance.h"


CEarth::CEarth()
{
    drawingStateSnapshot = 0;
}

CEarth::~CEarth()
{
    delete []terrain;
}

void CEarth::initLOD_0()
{
    int i;
    int lo, la;
    double lon, lat;

    terrain = new CTerrain[18];

    i = 0;
    for (lo=0; lo<6; lo++)
        for (la=0; la<3; la++) {
            lon = lo*60.0;
            lat = 90.0 - la*60.0;
            terrain[i].setEarth(this);
            terrain[i].initTerrainData(lon, lat, 0, drawingStateSnapshot);
            i++;
        }
}

void CEarth::setDrawingStateSnapshot(CDrawingStateSnapshot *dss)
{
    drawingStateSnapshot = dss;
}

void CEarth::updateTerrainTree()
{
    CPerformance *performance = CPerformance::getInstance();
    int i;

    performance->terrainsInTree = 0;
    performance->maxLOD = -1;

    for (i=0; i<18; i++) {
        terrain[i].updateTerrainTree();
    }
}

void CEarth::draw()
{
    CPerformance *performance = CPerformance::getInstance();
    int i;

    performance->terrainsQuarterDrawed = 0;
    for (i=0; i<18; i++) {
        terrain[i].draw();
    }
}
