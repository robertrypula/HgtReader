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

#include "CDrawingState.h"


CDrawingState::CDrawingState()
{
    drawTerrainPoint = false;
    drawTerrainPointColor = false;
    drawTerrainWire = false;
    drawTerrainWireColor = true;
    drawTerrainSolid = false;
    drawTerrainSolidStrip = true;
    drawTerrainSolidColor = true;
    drawTerrainTexture = true;
    drawTerrainTextureStrip = true;

    drawTerrainBottomPlaneWire = false;
    drawTerrainBottomPlaneWireColor = false;
    drawTerrainBottomPlaneSolid = false;
    drawTerrainBottomPlaneSolidColor = false;
    drawTerrainBottomPlaneTexture = true;

    drawTerrainNormals = false;
    drawEarthPoint = true;
    drawGrid = true;
    drawAxes = true;
    sunEnabled = true;
    treeUpdating = true;

    lodMultiplier = 1.74;
    dontUseCache = false;
    dontUseDiskHgt = false;
    dontUseDiskRaw = false;
}

void CDrawingState::getDrawingStateSnapshot(CDrawingStateSnapshot *dss)
{
    QMutexLocker locker(drawingStateMutex);

    dss->drawTerrainPoint = drawTerrainPoint;
    dss->drawTerrainPointColor = drawTerrainPointColor;
    dss->drawTerrainWire = drawTerrainWire;
    dss->drawTerrainWireColor = drawTerrainWireColor;
    dss->drawTerrainSolid = drawTerrainSolid;
    dss->drawTerrainSolidStrip = drawTerrainSolidStrip;
    dss->drawTerrainSolidColor = drawTerrainSolidColor;
    dss->drawTerrainTexture = drawTerrainTexture;
    dss->drawTerrainTextureStrip = drawTerrainTextureStrip;

    dss->drawTerrainBottomPlaneWire = drawTerrainBottomPlaneWire;
    dss->drawTerrainBottomPlaneWireColor = drawTerrainBottomPlaneWireColor;
    dss->drawTerrainBottomPlaneSolid = drawTerrainBottomPlaneSolid;
    dss->drawTerrainBottomPlaneSolidColor = drawTerrainBottomPlaneSolidColor;
    dss->drawTerrainBottomPlaneTexture = drawTerrainBottomPlaneTexture;

    dss->drawTerrainNormals = drawTerrainNormals;
    dss->drawEarthPoint = drawEarthPoint;
    dss->drawGrid = drawGrid;
    dss->drawAxes = drawAxes;
    dss->sunEnabled = sunEnabled;
    dss->treeUpdating = treeUpdating;

    dss->camPosition = camera.camPosition;
    dss->camLookingDirectionNormal = camera.camLookingDirectionNormal;
    dss->camClippingAngleCosine = camera.camClippingAngleCosine;

    dss->camLinkage = camera.camLinkage;
    dss->camPerspectiveX = camera.camPerspectiveX;
    dss->camPerspectiveY = camera.camPerspectiveY;
    dss->camPerspectiveZ = camera.camPerspectiveZ;
    dss->camPerspectiveLookAtX = camera.camPerspectiveLookAtX;
    dss->camPerspectiveLookAtY = camera.camPerspectiveLookAtY;
    dss->camPerspectiveLookAtZ = camera.camPerspectiveLookAtZ;
    dss->earthPointLon = camera.earthPointLon;
    dss->earthPointLat = camera.earthPointLat;
    dss->earthPointX = camera.earthPointX;
    dss->earthPointY = camera.earthPointY;
    dss->earthPointZ = camera.earthPointZ;
    dss->camDistanceToEarthPoint = camera.camDistanceToEarthPoint;
    dss->camAltGround = camera.camAltGround;
    dss->camFOV = camera.camFOV;

    dss->sunPositionGlobe = camera.sunPositionGlobe;
    dss->sunPositionTerrain = camera.sunPositionTerrain;
    dss->sunLightNormal = camera.sunLightVector;

    dss->lodMultiplier = lodMultiplier;
    dss->dontUseCache = dontUseCache;
    dss->dontUseDiskHgt = dontUseDiskHgt;
    dss->dontUseDiskRaw = dontUseDiskRaw;
}

void CDrawingState::setDrawingStateMutex(QMutex *m)
{
    drawingStateMutex = m;
    camera.setDrawingStateMutex(m);
}

CCamera *CDrawingState::getCamera()
{
    return &camera;
}

void CDrawingState::SLOTdrawTerrainPointChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainPoint = true;
    if (state==Qt::Unchecked) drawTerrainPoint = false;
}

void CDrawingState::SLOTdrawTerrainPointColorChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainPointColor = true;
    if (state==Qt::Unchecked) drawTerrainPointColor = false;
}

void CDrawingState::SLOTdrawTerrainWireChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainWire = true;
    if (state==Qt::Unchecked) drawTerrainWire = false;
}

void CDrawingState::SLOTdrawTerrainWireColorChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainWireColor = true;
    if (state==Qt::Unchecked) drawTerrainWireColor = false;
}

void CDrawingState::SLOTdrawTerrainSolidChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainSolid = true;
    if (state==Qt::Unchecked) drawTerrainSolid = false;
}

void CDrawingState::SLOTdrawTerrainSolidNormalClicked()
{
    QMutexLocker locker(drawingStateMutex);
    drawTerrainSolidStrip = false;
}

void CDrawingState::SLOTdrawTerrainSolidStripClicked()
{
    QMutexLocker locker(drawingStateMutex);
    drawTerrainSolidStrip = true;
}

void CDrawingState::SLOTdrawTerrainSolidColorChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainSolidColor = true;
    if (state==Qt::Unchecked) drawTerrainSolidColor = false;
}

void CDrawingState::SLOTdrawTerrainTextureChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainTexture = true;
    if (state==Qt::Unchecked) drawTerrainTexture = false;
}

void CDrawingState::SLOTdrawTerrainTextureNormalClicked()
{
    QMutexLocker locker(drawingStateMutex);
    drawTerrainTextureStrip = false;
}

void CDrawingState::SLOTdrawTerrainTextureStripClicked()
{
    QMutexLocker locker(drawingStateMutex);
    drawTerrainTextureStrip = true;
}

void CDrawingState::SLOTdrawTerrainBottomPlaneWireChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainBottomPlaneWire = true;
    if (state==Qt::Unchecked) drawTerrainBottomPlaneWire = false;
}

void CDrawingState::SLOTdrawTerrainBottomPlaneWireColorChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainBottomPlaneWireColor = true;
    if (state==Qt::Unchecked) drawTerrainBottomPlaneWireColor = false;
}

void CDrawingState::SLOTdrawTerrainBottomPlaneSolidChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainBottomPlaneSolid = true;
    if (state==Qt::Unchecked) drawTerrainBottomPlaneSolid = false;
}

void CDrawingState::SLOTdrawTerrainBottomPlaneSolidColorChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainBottomPlaneSolidColor = true;
    if (state==Qt::Unchecked) drawTerrainBottomPlaneSolidColor = false;
}

void CDrawingState::SLOTdrawTerrainBottomPlaneTextureChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainBottomPlaneTexture = true;
    if (state==Qt::Unchecked) drawTerrainBottomPlaneTexture = false;
}

void CDrawingState::SLOTdrawTerrainNormalsChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawTerrainNormals = true;
    if (state==Qt::Unchecked) drawTerrainNormals = false;
}

void CDrawingState::SLOTdrawEarthPointChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawEarthPoint = true;
    if (state==Qt::Unchecked) drawEarthPoint = false;
}

void CDrawingState::SLOTdrawGridChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawGrid = true;
    if (state==Qt::Unchecked) drawGrid = false;
}

void CDrawingState::SLOTdrawAxesChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) drawAxes = true;
    if (state==Qt::Unchecked) drawAxes = false;
}

void CDrawingState::SLOTsunEnabledChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) sunEnabled = true;
    if (state==Qt::Unchecked) sunEnabled = false;
}

void CDrawingState::SLOTtreeUpdatingChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) treeUpdating = true;
    if (state==Qt::Unchecked) treeUpdating = false;
}

void CDrawingState::SLOTdontUseDiskHgtChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) dontUseDiskHgt = true;
    if (state==Qt::Unchecked) dontUseDiskHgt = false;
}

void CDrawingState::SLOTdontUseDiskRawChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) dontUseDiskRaw = true;
    if (state==Qt::Unchecked) dontUseDiskRaw = false;
}

void CDrawingState::SLOTdontUseCacheChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) dontUseCache = true;
    if (state==Qt::Unchecked) dontUseCache = false;

    if (dontUseCache) {
        emit SIGNALclearCache();
    }
}

void CDrawingState::SLOTlodMultiplierIndexChanged(int index)
{
    QMutexLocker locker(drawingStateMutex);

    switch (index) {
        case 0:lodMultiplier = 1.74 * 0.1;  break;
        case 1:lodMultiplier = 1.74 * 0.4;  break;
        case 2:lodMultiplier = 1.74 * 0.7;  break;
        case 3:lodMultiplier = 1.74 * 1.0;  break;
        case 4:lodMultiplier = 1.74 * 1.3;  break;
        case 5:lodMultiplier = 1.74 * 1.6;  break;
        case 6:lodMultiplier = 1.74 * 1.9;  break;
        case 7:lodMultiplier = 1.74 * 2.2;  break;
        case 8:lodMultiplier = 1.74 * 2.5;  break;
        case 9:lodMultiplier = 1.74 * 2.8;  break;
    }
}
