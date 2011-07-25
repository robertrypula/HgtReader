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

#ifndef CDRAWINGSTATESNAPSHOT_H
#define CDRAWINGSTATESNAPSHOT_H

#include <QVector3D>

class CDrawingStateSnapshot
{
public:
    CDrawingStateSnapshot();

    bool drawTerrainPoint;
    bool drawTerrainPointColor;
    bool drawTerrainWire;
    bool drawTerrainWireColor;
    bool drawTerrainSolid;
    bool drawTerrainSolidStrip;
    bool drawTerrainSolidColor;
    bool drawTerrainTexture;
    bool drawTerrainTextureStrip;
    bool drawTerrainBottomPlaneWire;
    bool drawTerrainBottomPlaneWireColor;
    bool drawTerrainBottomPlaneSolid;
    bool drawTerrainBottomPlaneSolidColor;
    bool drawTerrainBottomPlaneTexture;
    bool drawTerrainNormals;
    bool drawEarthPoint;
    bool drawGrid;
    bool drawAxes;
    bool sunEnabled;
    bool treeUpdating;
    QVector3D camPosition;
    QVector3D camLookingDirectionNormal;
    double camClippingAngleCosine;
    char camLinkage;
    double camPerspectiveX;
    double camPerspectiveY;
    double camPerspectiveZ;
    double camPerspectiveLookAtX;
    double camPerspectiveLookAtY;
    double camPerspectiveLookAtZ;
    double earthPointLon;
    double earthPointLat;
    double earthPointX;
    double earthPointY;
    double earthPointZ;
    double camDistanceToEarthPoint;
    double camAltGround;
    double camFOV;
    QVector3D sunPositionGlobe;
    QVector3D sunPositionTerrain;
    QVector3D sunLightNormal;
    double lodMultiplier;
    bool dontUseDiskHgt;
    bool dontUseDiskRaw;
    bool dontUseCache;
};

#endif // CDRAWINGSTATESNAPSHOT_H
