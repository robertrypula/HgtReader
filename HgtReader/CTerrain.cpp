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
#include "CCommons.h"
#include "CTerrain.h"
#include "CCacheManager.h"
#include "CPerformance.h"
#include "CDrawingStateSnapshot.h"


CTerrain::CTerrain()
{
    visible = false;
    terrainInCameraFOV = false;

    terrainPointClosestToCam = 0;
    terrainPointClosestToCamDistance = 2000.0*CONST_1GM; // 2 milions km it's far beyond the maximum position of the camera

    NWchild = 0;
    NEchild = 0;
    SWchild = 0;
    SEchild = 0;

    earth = 0;
    terrainData = 0;
}

CTerrain::~CTerrain()
{
    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;
    CCacheManager *cacheManager = CCacheManager::getInstance();

    cacheManager->cacheTerrainDataFree(earth, &terrainData, dss->dontUseCache);

    if (NWchild!=0) {
        delete NWchild; NWchild = 0;
        delete NEchild; NEchild = 0;
        delete SWchild; SWchild = 0;
        delete SEchild; SEchild = 0;
    }
}

unsigned char *CTerrain::getTexturePointer()
{
    return (unsigned char *)terrainData->texture;
}

void CTerrain::setEarth(CEarth *earthPtr)
{
    earth = earthPtr;
}

void CTerrain::findTerrainPointClosestToCam()
{
    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;
    QVector3D vecCamPos2Terrain;
    double vecCamPos2TerrainDistance;
    int i;

    terrainPointClosestToCamDistance = 2000.0*CONST_1GM; // 2 milions km it's far beyond the maximum position of the camera
    for (i=0; i<81; i++) {
        vecCamPos2Terrain = terrainData->sphere[i] - dss->camPosition;
        vecCamPos2TerrainDistance = vecCamPos2Terrain.length();

        if (vecCamPos2TerrainDistance<terrainPointClosestToCamDistance) {
            terrainPointClosestToCam = &(terrainData->sphere[i]);
            terrainPointClosestToCamDistance = vecCamPos2TerrainDistance;
        }
    }

    // get normal vector of closest to cam point
    terrainPointClosestToCamNormal = (*terrainPointClosestToCam);
    terrainPointClosestToCamNormal.normalize();
}

bool CTerrain::getTerrainVisibility()
{
    bool beyondTheHorizon;
    bool cameraCloseToTerrain;
    QVector3D vecCamPos2TerrainNormal;
    QVector3D vecCamPos2TerrainBehindCameraNormal;
    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;

    findTerrainPointClosestToCam();

    vecCamPos2TerrainNormal = (*terrainPointClosestToCam) - dss->camPosition;
    vecCamPos2TerrainBehindCameraNormal = vecCamPos2TerrainNormal + dss->camLookingDirectionNormal * (10000.0);
    vecCamPos2TerrainBehindCameraNormal.normalize();
    vecCamPos2TerrainNormal.normalize();

    // only for vector angle >90deg terrain is visible (so cosine must be <0.0)
    if (QVector3D::dotProduct(vecCamPos2TerrainNormal, terrainPointClosestToCamNormal)<-0.01)
        beyondTheHorizon = false; else  // <- terrain is visible
        beyondTheHorizon = true;        // <- terrain is invisible

    // camera close to terrain check
    if (terrainPointClosestToCamDistance<=terrainData->mustShowDistance)
        cameraCloseToTerrain = true; else
        cameraCloseToTerrain = false;

    // check if terrain is in camera FOV
    if (QVector3D::dotProduct(dss->camLookingDirectionNormal, vecCamPos2TerrainBehindCameraNormal) > dss->camClippingAngleCosine)
        terrainInCameraFOV = true; else
        terrainInCameraFOV = false;

    return (cameraCloseToTerrain || !beyondTheHorizon) ? true : false;
}

int CTerrain::getLodToRender()
{
    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;

    if (terrainPointClosestToCamDistance <     5.2*CONST_1KM*dss->lodMultiplier) return 13; else
    if (terrainPointClosestToCamDistance <    10.4*CONST_1KM*dss->lodMultiplier) return 12; else
    if (terrainPointClosestToCamDistance <    20.8*CONST_1KM*dss->lodMultiplier) return 11; else
    if (terrainPointClosestToCamDistance <    41.6*CONST_1KM*dss->lodMultiplier) return 10; else
    if (terrainPointClosestToCamDistance <    83.2*CONST_1KM*dss->lodMultiplier) return  9; else
    if (terrainPointClosestToCamDistance <   166.4*CONST_1KM*dss->lodMultiplier) return  8; else
    if (terrainPointClosestToCamDistance <   332.8*CONST_1KM*dss->lodMultiplier) return  7; else
    if (terrainPointClosestToCamDistance <   665.6*CONST_1KM*dss->lodMultiplier) return  6; else
    if (terrainPointClosestToCamDistance <  1331.2*CONST_1KM*dss->lodMultiplier) return  5; else
    if (terrainPointClosestToCamDistance <  2662.5*CONST_1KM*dss->lodMultiplier) return  4; else
    if (terrainPointClosestToCamDistance <  5324.9*CONST_1KM*dss->lodMultiplier) return  3; else
    if (terrainPointClosestToCamDistance < 10649.9*CONST_1KM*dss->lodMultiplier) return  2; else
    if (terrainPointClosestToCamDistance < 21299.7*CONST_1KM*dss->lodMultiplier) return  1; else
                                                                                 return  0;
}

void CTerrain::updateTerrainTree()
{
    if (terrainData==0)
        qFatal("Terrain in tree - terrainData pointer is NULL!");

    CPerformance *performance = CPerformance::getInstance();
    int LODtoRender;

    visible = getTerrainVisibility();
    if (!visible) {
        merge();
        performance->terrainsInTree++;
        return;
    }

    LODtoRender = getLodToRender();
    if (LODtoRender<terrainData->LOD)
        visible = false;

    // split terrain when LOD to render is bigger that current terrain LOD
    if (LODtoRender>terrainData->LOD) {
        split();
        NWchild->updateTerrainTree();
        NEchild->updateTerrainTree();
        SWchild->updateTerrainTree();
        SEchild->updateTerrainTree();
    } else
        if (LODtoRender<=terrainData->LOD) {
            merge();
        }

    // performance info
    performance->terrainsInTree++;
    if (terrainData->LOD > performance->maxLOD)
        performance->maxLOD = terrainData->LOD;
}

void CTerrain::initTerrainData(double lon, double lat, int lod, const CDrawingStateSnapshot *dss)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    bool TDfound;

    if (!dss->dontUseCache) {
        TDfound = cacheManager->cacheTerrainDataFind(lon, lat, lod, earth, &terrainData);
        if (!TDfound) {
            terrainData = new CTerrainData();
            terrainData->initTerrainData(lon, lat, lod, dss);
            cacheManager->cacheTerrainDataRegister(earth, &terrainData);
        }
    } else {
        terrainData = new CTerrainData();
        terrainData->initTerrainData(lon, lat, lod, dss);
    }
}

void CTerrain::split()
{
    if (NWchild!=0) return;

    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;

    NWchild = new CTerrain(); NWchild->setEarth(earth);
    NEchild = new CTerrain(); NEchild->setEarth(earth);
    SWchild = new CTerrain(); SWchild->setEarth(earth);
    SEchild = new CTerrain(); SEchild->setEarth(earth);

    NWchild->initTerrainData(terrainData->topLeftLon, terrainData->topLeftLat,
                             terrainData->LOD+1, dss);
    NEchild->initTerrainData(terrainData->topLeftLon+(terrainData->degreeSize/2.0), terrainData->topLeftLat,
                             terrainData->LOD+1, dss);
    SWchild->initTerrainData(terrainData->topLeftLon, terrainData->topLeftLat-(terrainData->degreeSize/2.0),
                             terrainData->LOD+1, dss);
    SEchild->initTerrainData(terrainData->topLeftLon+(terrainData->degreeSize/2.0), terrainData->topLeftLat-(terrainData->degreeSize/2.0),
                             terrainData->LOD+1, dss);
}

void CTerrain::merge()
{
    if (NWchild!=0) {
        delete NWchild; NWchild = 0;
        delete NEchild; NEchild = 0;
        delete SWchild; SWchild = 0;
        delete SEchild; SEchild = 0;
    }
}

bool CTerrain::draw()
{
    if (terrainData==0)
        qFatal("Terrain in tree - terrainData pointer is NULL!");

    CDrawingStateSnapshot *dss = earth->drawingStateSnapshot;
    CPerformance *performance = CPerformance::getInstance();
    int xStart, xStop, yStart, yStop;
    int i;
    bool childDrawed;

    if (!visible) return false;

    for (i=0; i<4; i++) {
        switch (i) {
            case 0:if (NWchild!=0)
                       childDrawed = NWchild->draw(); else
                       childDrawed = false;
                   yStart = 0; yStop = 4;
                   xStart = 0; xStop = 4;
                   break;
            case 1:if (NEchild!=0)
                       childDrawed = NEchild->draw(); else
                       childDrawed = false;
                   yStart = 0; yStop = 4;
                   xStart = 4; xStop = 8;
                   break;
            case 2:if (SWchild!=0)
                       childDrawed = SWchild->draw(); else
                       childDrawed = false;
                   yStart = 4; yStop = 8;
                   xStart = 0; xStop = 4;
                   break;
            case 3:if (SEchild!=0)
                       childDrawed = SEchild->draw(); else
                       childDrawed = false;
                   yStart = 4; yStop = 8;
                   xStart = 4; xStop = 8;
                   break;
        }

        if (!childDrawed && terrainInCameraFOV) {

            if (dss->drawTerrainPoint || dss->drawTerrainWire || dss->drawTerrainSolid || dss->drawTerrainTexture) {
                performance->terrainsQuarterDrawed++;
            }

            if (dss->drawTerrainPoint)              terrainData->drawPoint(xStart, xStop, yStart, yStop, dss);
            if (dss->drawTerrainWire)               terrainData->drawWire(xStart, xStop, yStart, yStop, dss);
            if (dss->drawTerrainBottomPlaneWire)    terrainData->drawBottomPlaneWire(xStart, yStart, dss);
            if (dss->drawTerrainSolid)            { if (dss->drawTerrainSolidStrip)
                                                        terrainData->drawSolidStrip(xStart, yStart, dss); else
                                                        terrainData->drawSolid(xStart, xStop, yStart, yStop, dss); }
            if (dss->drawTerrainBottomPlaneSolid)   terrainData->drawBottomPlaneSolid(xStart, yStart, dss);
            if (dss->drawTerrainTexture)          { if (dss->drawTerrainTextureStrip)
                                                        terrainData->drawTextureStrip(xStart, yStart, dss); else
                                                        terrainData->drawTexture(xStart, xStop, yStart, yStop); }
            if (dss->drawTerrainBottomPlaneTexture) terrainData->drawBottomPlaneTexture(xStart, yStart);
            if (dss->drawTerrainNormals)            terrainData->drawNormals(xStart, xStop, yStart, yStop, dss);
        }
    }

    return true;
}
