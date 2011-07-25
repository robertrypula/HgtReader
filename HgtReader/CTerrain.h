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

#ifndef CTERRAIN_H
#define CTERRAIN_H

#include <QVector3D>
#include <QColor>
#include "CEarth.h"
#include "CTerrainData.h"


class CEarth;

class CTerrain
{
public:
    CTerrain();
    ~CTerrain();

    void setEarth(CEarth *earthPtr);
    bool draw();
    void updateTerrainTree();
    unsigned char *getTexturePointer();
    void initTerrainData(double lon, double lat, int lod, const CDrawingStateSnapshot *dss);

private:
    CEarth *earth;
    QVector3D *terrainPointClosestToCam;
    QVector3D terrainPointClosestToCamNormal;
    double terrainPointClosestToCamDistance;
    bool visible;
    bool terrainInCameraFOV;
    CTerrainData *terrainData;
    CTerrain *NWchild;
    CTerrain *NEchild;
    CTerrain *SWchild;
    CTerrain *SEchild;

    void split();
    void merge();
    int getLodToRender();
    void findTerrainPointClosestToCam();
    bool getTerrainVisibility();
};

#endif // CTERRAIN_H
