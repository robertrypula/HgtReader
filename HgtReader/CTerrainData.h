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

#ifndef CTERRAINDATA_H
#define CTERRAINDATA_H

#include <QVector2D>
#include <QVector3D>
#include <QColor>
#include <QtOpenGL>
#include "CDrawingStateSnapshot.h"

class CTerrainData
{
public:
    CTerrainData();
    CTerrainData(CTerrainData *source);
    ~CTerrainData();
    friend class CTerrain;          // for full access from CTerrain class

    double topLeftLon;   // top left is general to localize terrain on Earth
    double topLeftLat;   // top left is general to localize terrain on Earth
    int LOD;             // LevelOfDetails is general to localize terrain on Earth

    void initTerrainData(double lon, double lat, int lod, const CDrawingStateSnapshot *dss);
    void drawPoint(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss);
    void drawWire(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss);
    void drawNormals(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss);
    void drawSolid(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss);
    void drawSolidStrip(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss);
    void drawTexture(const int &xStart, const int &xStop, const int &yStart, const int &yStop);
    void drawTextureStrip(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss);
    void drawBottomPlaneWire(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss);
    void drawBottomPlaneSolid(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss);
    void drawBottomPlaneTexture(const int &xStart, const int &yStart);
    unsigned char *getTexturePointer();
    void setTextureID(GLuint texID);
    unsigned int getTextureID();

private:
    double mustShowDistance;    // when camera is closer that this value tile must be show
    double degreeSize;
    QVector3D hNW;              // point in NW neighbor
    QVector3D hNE;              // point in NE neighbor
    QVector3D hSW;              // point in SW neighbor
    QVector3D hSE;              // point in SE neighbor
    QVector3D *hN;              // points line in N neighbor
    QVector3D *hE;              // points line in E neighbor
    QVector3D *hS;              // points line in S neighbor
    QVector3D *hW;              // points line in W neighbor
    QVector3D *h;               // terrain data
    QVector3D *sphere;          // sea level for terrainPointClosestToCam searching
    QVector3D *n;               // terrain data normals
    QColor *c;                  // color data
    uint8_t *texture;           // texture data
    GLuint textureID;           // OpenGL texture ID
    QVector2D *uv;              // texture coordinate
    QVector3D topLeftPoint;
    QVector3D topMiddlePoint;
    QVector3D topRightPoint;
    QVector3D middleLeftPoint;
    QVector3D middleMiddlePoint;
    QVector3D middleRightPoint;
    QVector3D bottomLeftPoint;
    QVector3D bottomMiddlePoint;
    QVector3D bottomRightPoint;
    QVector3D topLeftPointNormal;
    QVector3D topMiddlePointNormal;
    QVector3D topRightPointNormal;
    QVector3D middleLeftPointNormal;
    QVector3D middleMiddlePointNormal;
    QVector3D middleRightPointNormal;
    QVector3D bottomLeftPointNormal;
    QVector3D bottomMiddlePointNormal;
    QVector3D bottomRightPointNormal;

    void bindTexture(CTerrainData *terrainData);
    void getTerrainData(const CDrawingStateSnapshot *dss);
    QVector3D *getHeight(int x, int y) { return &h[y*9+x]; }                   // inline func
    QVector3D *getNormal(int x, int y) { return &n[y*9+x]; }                   // inline func
    QVector2D *getUv(int x, int y) { return &uv[y*9+x]; }                      // inline func
    QColor *getColor(int x, int y) { return &c[y*9+x]; }                       // inline func
    void getNeighborVector(QVector3D *vecBase, int x, int y, QVector3D *vecNeighborFake);
    void getNeighborsTerrainData(int *pointNW, int *pointNE, int *pointSW, int *pointSE,
                                 int *pointsN, int *pointsE, int *pointsS, int *pointsW);

};

#endif // CTERRAINDATA_H
