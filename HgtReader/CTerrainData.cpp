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

#include "CTerrainData.h"
#include "CCacheManager.h"
#include "CCommons.h"


CTerrainData::CTerrainData()
{
    hN = new QVector3D[9];
    hE = new QVector3D[9];
    hS = new QVector3D[9];
    hW = new QVector3D[9];
    h = new QVector3D[81];
    sphere = new QVector3D[81];
    n = new QVector3D[81];
    c = new QColor[81];
    texture = new uint8_t[3*32*32];
    uv = new QVector2D[81];

    textureID = 0;
    topLeftLon = 0.0;
    topLeftLat = 0.0;
    degreeSize = -1.0;
    LOD = -1;
}

CTerrainData::CTerrainData(CTerrainData *source)
{
    int i;

    hN = new QVector3D[9];
    hE = new QVector3D[9];
    hS = new QVector3D[9];
    hW = new QVector3D[9];
    h = new QVector3D[81];
    sphere = new QVector3D[81];
    n = new QVector3D[81];
    c = new QColor[81];
    texture = new uint8_t[3*32*32];
    uv = new QVector2D[81];

    // below data must be copy from source
    topLeftLon = source->topLeftLon;
    topLeftLat = source->topLeftLat;
    degreeSize = source->degreeSize;
    LOD = source->LOD;
    mustShowDistance = source->mustShowDistance;
    hNW = source->hNW;
    hNE = source->hNE;
    hSW = source->hSW;
    hSE = source->hSE;
    for (i=0; i<9; i++) hN[i] = source->hN[i];
    for (i=0; i<9; i++) hE[i] = source->hE[i];
    for (i=0; i<9; i++) hS[i] = source->hS[i];
    for (i=0; i<9; i++) hW[i] = source->hW[i];
    for (i=0; i<81; i++) h[i] = source->h[i];
    for (i=0; i<81; i++) sphere[i] = source->sphere[i];
    for (i=0; i<81; i++) n[i] = source->n[i];
    for (i=0; i<81; i++) c[i] = source->c[i];
    for (i=0; i<3*32*32; i++) texture[i] = source->texture[i];
    textureID = source->textureID;
    for (i=0; i<81; i++) uv[i] = source->uv[i];
    topLeftPoint = source->topLeftPoint;
    topMiddlePoint = source->topMiddlePoint;
    topRightPoint = source->topRightPoint;
    middleLeftPoint = source->middleLeftPoint;
    middleMiddlePoint = source->middleMiddlePoint;
    middleRightPoint = source->middleRightPoint;
    bottomLeftPoint = source->bottomLeftPoint;
    bottomMiddlePoint = source->bottomMiddlePoint;
    bottomRightPoint = source->bottomRightPoint;
    topLeftPointNormal = source->topLeftPointNormal;
    topMiddlePointNormal = source->topMiddlePointNormal;
    topRightPointNormal = source->topRightPointNormal;
    middleLeftPointNormal = source->middleLeftPointNormal;
    middleMiddlePointNormal = source->middleMiddlePointNormal;
    middleRightPointNormal = source->middleRightPointNormal;
    bottomLeftPointNormal = source->bottomLeftPointNormal;
    bottomMiddlePointNormal = source->bottomMiddlePointNormal;
    bottomRightPointNormal = source->bottomRightPointNormal;
}

CTerrainData::~CTerrainData()
{
    delete []hN;
    delete []hE;
    delete []hS;
    delete []hW;
    delete []h;
    delete []sphere;
    delete []n;
    delete []c;
    delete []texture;
    delete []uv;
}

unsigned char *CTerrainData::getTexturePointer()
{
    return (unsigned char *)texture;
}

void CTerrainData::setTextureID(GLuint texID)
{
    textureID = texID;
}

unsigned int CTerrainData::getTextureID()
{
    return (unsigned int)textureID;
}

void CTerrainData::initTerrainData(double lon, double lat, int lod, const CDrawingStateSnapshot *dss)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    double Palt, Px, Py, Pz;

    degreeSize = cacheManager->LODdegreeSizeLookUp[lod];
    CCommons::findTopLeftCorner(lon, lat, degreeSize, &topLeftLon, &topLeftLat);
    mustShowDistance = ((degreeSize/8.0)/360.0) * CONST_EARTH_CIRCUMFERENCE;
    LOD = lod;

    // build terrain from scaled SRTM data
    getTerrainData(dss);

    // get corners of terrain (-200m below sea level to avoid z-buffer errors)
    Palt = CONST_EARTH_RADIUS - 200.0;
    CCommons::getCartesianFromSpherical(topLeftLon, topLeftLat, Palt, &Px, &Py, &Pz);
    topLeftPoint.setX(Px);  topLeftPoint.setY(Py);  topLeftPoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize/2.0, topLeftLat, Palt, &Px, &Py, &Pz);
    topMiddlePoint.setX(Px);  topMiddlePoint.setY(Py);  topMiddlePoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize, topLeftLat, Palt, &Px, &Py, &Pz);
    topRightPoint.setX(Px);  topRightPoint.setY(Py);  topRightPoint.setZ(Pz);

    CCommons::getCartesianFromSpherical(topLeftLon, topLeftLat-degreeSize/2.0, Palt, &Px, &Py, &Pz);
    middleLeftPoint.setX(Px);  middleLeftPoint.setY(Py);  middleLeftPoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize/2.0, topLeftLat-degreeSize/2.0, Palt, &Px, &Py, &Pz);
    middleMiddlePoint.setX(Px);  middleMiddlePoint.setY(Py);  middleMiddlePoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize, topLeftLat-degreeSize/2.0, Palt, &Px, &Py, &Pz);
    middleRightPoint.setX(Px);  middleRightPoint.setY(Py);  middleRightPoint.setZ(Pz);

    CCommons::getCartesianFromSpherical(topLeftLon, topLeftLat-degreeSize, Palt, &Px, &Py, &Pz);
    bottomLeftPoint.setX(Px);  bottomLeftPoint.setY(Py);  bottomLeftPoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize/2.0, topLeftLat-degreeSize, Palt, &Px, &Py, &Pz);
    bottomMiddlePoint.setX(Px);  bottomMiddlePoint.setY(Py);  bottomMiddlePoint.setZ(Pz);
    CCommons::getCartesianFromSpherical(topLeftLon+degreeSize, topLeftLat-degreeSize, Palt, &Px, &Py, &Pz);
    bottomRightPoint.setX(Px);  bottomRightPoint.setY(Py);  bottomRightPoint.setZ(Pz);

    topLeftPointNormal = topLeftPoint.normalized();
    topMiddlePointNormal = topMiddlePoint.normalized();
    topRightPointNormal = topRightPoint.normalized();
    middleLeftPointNormal = middleLeftPoint.normalized();
    middleMiddlePointNormal = middleMiddlePoint.normalized();
    middleRightPointNormal = middleRightPoint.normalized();
    bottomLeftPointNormal = bottomLeftPoint.normalized();
    bottomMiddlePointNormal = bottomMiddlePoint.normalized();
    bottomRightPointNormal = bottomRightPoint.normalized();
}

void CTerrainData::getNeighborsTerrainData(int *pointNW, int *pointNE, int *pointSW, int *pointSE,
                                       int *pointsN, int *pointsE, int *pointsS, int *pointsW)
{
    double Plon, Plat, Palt, Px, Py, Pz;
    int i;

    // NW point
    Plon = topLeftLon + (-1.0/8.0)*degreeSize;
    Plat = topLeftLat - (-1.0/8.0)*degreeSize;
    Palt = CONST_EARTH_RADIUS + (double)(*pointNW);
    CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
    hNW.setX(Px); hNW.setY(Py); hNW.setZ(Pz);

    // NE point
    Plon = topLeftLon + (+9.0/8.0)*degreeSize;
    Plat = topLeftLat - (-1.0/8.0)*degreeSize;
    Palt = CONST_EARTH_RADIUS + (double)(*pointNE);
    CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
    hNE.setX(Px); hNE.setY(Py); hNE.setZ(Pz);

    // SW point
    Plon = topLeftLon + (-1.0/8.0)*degreeSize;
    Plat = topLeftLat - (+9.0/8.0)*degreeSize;
    Palt = CONST_EARTH_RADIUS + (double)(*pointSW);
    CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
    hSW.setX(Px); hSW.setY(Py); hSW.setZ(Pz);

    // SE point
    Plon = topLeftLon + (+9.0/8.0)*degreeSize;
    Plat = topLeftLat - (+9.0/8.0)*degreeSize;
    Palt = CONST_EARTH_RADIUS + (double)(*pointSE);
    CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
    hSE.setX(Px); hSE.setY(Py); hSE.setZ(Pz);

    // N points line
    for (i=0; i<9; i++) {
        Plon = topLeftLon + (i/8.0)*degreeSize;
        Plat = topLeftLat - (-1.0/8.0)*degreeSize;
        Palt = CONST_EARTH_RADIUS + (double)(pointsN[i]);
        CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
        hN[i].setX(Px); hN[i].setY(Py); hN[i].setZ(Pz);
    }

    // E points line
    for (i=0; i<9; i++) {
        Plon = topLeftLon + (9.0/8.0)*degreeSize;
        Plat = topLeftLat - (i/8.0)*degreeSize;
        Palt = CONST_EARTH_RADIUS + (double)(pointsE[i]);
        CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
        hE[i].setX(Px); hE[i].setY(Py); hE[i].setZ(Pz);
    }

    // S points line
    for (i=0; i<9; i++) {
        Plon = topLeftLon + (i/8.0)*degreeSize;
        Plat = topLeftLat - (9.0/8.0)*degreeSize;
        Palt = CONST_EARTH_RADIUS + (double)(pointsS[i]);
        CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
        hS[i].setX(Px); hS[i].setY(Py); hS[i].setZ(Pz);
    }

    // W points line
    for (i=0; i<9; i++) {
        Plon = topLeftLon + (-1.0/8.0)*degreeSize;
        Plat = topLeftLat - (i/8.0)*degreeSize;
        Palt = CONST_EARTH_RADIUS + (double)(pointsW[i]);
        CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
        hW[i].setX(Px); hW[i].setY(Py); hW[i].setZ(Pz);
    }
}

void CTerrainData::getTerrainData(const CDrawingStateSnapshot *dss)
{
    QVector3D *v, vSum, vN, vNE, vE, vSE, vS, vSW, vW, vNW;
    QVector3D vN_NE, vNE_E, vE_SE, vSE_S, vS_SW, vSW_W, vW_NW, vNW_N;
    CCacheManager *cacheManager = CCacheManager::getInstance();
    double lodMAXTEXtlLon = 0.0, lodMAXTEXtlLat = 0.0;
    double lodMAXTEXdeltaLon = 0.0, lodMAXTEXdeltaLat = 0.0;
    int lodMAXTEXDiff = 0;
    double lodMAXTEXoffsetLon = 0.0, lodMAXTEXoffsetLat = 0.0;
    double lodMAXTEXuvSize = 0.0;
    int *points = new int[81];
    int pointNW, pointNE, pointSW, pointSE;
    int *pointsN = new int[9];
    int *pointsE = new int[9];
    int *pointsS = new int[9];
    int *pointsW = new int[9];
    double Plon, Plat, Palt;
    double Px, Py, Pz;
    double hue, val;
    int r, g, b, a;
    int x, y;
    int i;

    // get height data from cache manager
    cacheManager->getTerrainPoints(topLeftLon, topLeftLat, LOD, points,
                                   &pointNW, &pointNE, &pointSW, &pointSE,
                                   pointsN, pointsE, pointsS, pointsW,
                                   (unsigned char *)texture,
                                   dss->dontUseDiskHgt, dss->dontUseDiskRaw);

    // map points from Neighbors terrains for normal vectors
    getNeighborsTerrainData(&pointNW, &pointNE, &pointSW, &pointSE, pointsN, pointsE, pointsS, pointsW);

    // get LOD10 topLeft corner for texture uv mapping & delta to current LOD
    if (LOD>TEX_SOURCE_MAX_LOD) {
        CCommons::findTopLeftCorner(topLeftLon, topLeftLat, cacheManager->LODdegreeSizeLookUp[TEX_SOURCE_MAX_LOD], &lodMAXTEXtlLon, &lodMAXTEXtlLat);
        lodMAXTEXdeltaLon = topLeftLon - lodMAXTEXtlLon;
        lodMAXTEXdeltaLat = lodMAXTEXtlLat - topLeftLat;
        lodMAXTEXDiff = LOD - TEX_SOURCE_MAX_LOD;

        lodMAXTEXoffsetLon = lodMAXTEXdeltaLon / cacheManager->LODdegreeSizeLookUp[TEX_SOURCE_MAX_LOD];
        lodMAXTEXoffsetLat = lodMAXTEXdeltaLat / cacheManager->LODdegreeSizeLookUp[TEX_SOURCE_MAX_LOD];
        lodMAXTEXuvSize = 1.0 / pow(2.0, (double)(lodMAXTEXDiff));
    }


    // map points to sphere & generate color
    i = 0;
    for (y=0; y<9; y++)
        for (x=0; x<9; x++) {

            // texture uv
            if (LOD>TEX_SOURCE_MAX_LOD) {
                uv[i].setX( (lodMAXTEXoffsetLon + (((double)x)/8.0)*lodMAXTEXuvSize)*0.973 + 0.0135 );
                uv[i].setY( (lodMAXTEXoffsetLat + (((double)y)/8.0)*lodMAXTEXuvSize)*0.973 + 0.0135 );
            } else {
                uv[i].setX( (((double)x)/8.0)*0.973 + 0.0135 );   // (...)*0.973 + 0.0135 to avoid Qt texture border :/
                uv[i].setY( (((double)y)/8.0)*0.973 + 0.0135 );   // (...)*0.973 + 0.0135 to avoid Qt texture border :/
            }

            // SRTM data error marked as very hight altidute
            if (points[i]>9000)
                points[i] = 10;

            Plon = topLeftLon + ((double)x/8.0)*degreeSize;
            Plat = topLeftLat - ((double)y/8.0)*degreeSize;
            Palt = CONST_EARTH_RADIUS + (double)points[i];
            CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);

            h[i].setX(Px);
            h[i].setY(Py);
            h[i].setZ(Pz);

            // get sea level (-500.0 m below)
            Palt = CONST_EARTH_RADIUS - 500.0;
            CCommons::getCartesianFromSpherical(Plon, Plat, Palt, &Px, &Py, &Pz);
            sphere[i].setX(Px);
            sphere[i].setY(Py);
            sphere[i].setZ(Pz);

            if (points[i]==0) {
                c[i].setRedF(0.2784);
                c[i].setGreenF(0.6431);
                c[i].setBlueF(0.7216);
            } else {
                val = 240.0;
                hue = 170.0 - 170.0 * (((double)points[i])/1500.0);
                if (hue<0.0) {
                    hue = 0.0;
                    hue = 360.0 - 100.0 * (((double)(points[i]-1500))/1500.0);
                    if (hue<260.0) {
                        hue = 260.0;
                        val = 240.0 - 200.0 * (((double)(points[i]-3000))/5000.0);
                        if (val<40.0) {
                            val = 40.0 + 215.0 * (((double)(points[i]-8000))/850.0);
                        }
                    }
                }
                c[i].setHsv(hue, 170, val);
                c[i].getRgb(&r, &g, &b, &a);
                c[i].setRedF((double)r/255.0);
                c[i].setGreenF((double)g/255.0);
                c[i].setBlueF((double)b/255.0);
            }
            i++;
        }

    // setup normal vectors
    for (y=0; y<9; y++)
        for (x=0; x<9; x++) {

            v = getHeight(x, y);

            if (x==0 || y==0)
                getNeighborVector(v, x-1, y-1, &vNW); else
                vNW = (*getHeight(x-1, y-1)) - (*v);

            if (y==0)
                getNeighborVector(v, x+0, y-1, &vN); else
                vN = (*getHeight(x, y-1)) - (*v);

            if (x==8 || y==0)
                getNeighborVector(v, x+1, y-1, &vNE); else
                vNE = (*getHeight(x+1, y-1)) - (*v);

            if (x==0)
                getNeighborVector(v, x-1, y+0, &vW); else
                vW = (*getHeight(x-1, y)) - (*v);

            if (x==8)
                getNeighborVector(v, x+1, y+0, &vE); else
                vE = (*getHeight(x+1, y)) - (*v);

            if (x==0 || y==8)
                getNeighborVector(v, x-1, y+1, &vSW); else
                vSW = (*getHeight(x-1, y+1)) - (*v);

            if (y==8)
                getNeighborVector(v, x+0, y+1, &vS); else
                vS = (*getHeight(x, y+1)) - (*v);

            if (x==8 || y==8)
                getNeighborVector(v, x+1, y+1, &vSE); else
                vSE = (*getHeight(x+1, y+1)) - (*v);


            vN_NE = QVector3D::normal(vNE, vN);
            vNE_E = QVector3D::normal(vE, vNE);
            vE_SE = QVector3D::normal(vSE, vE);
            vSE_S = QVector3D::normal(vS, vSE);
            vS_SW = QVector3D::normal(vSW, vS);
            vSW_W = QVector3D::normal(vW, vSW);
            vW_NW = QVector3D::normal(vNW, vW);
            vNW_N = QVector3D::normal(vN, vNW);

            // sum all normal vectors of neighbor planes
            vSum = vN_NE + vNE_E + vE_SE + vSE_S + vS_SW + vSW_W + vW_NW + vNW_N;
            vSum.normalize();

            (*getNormal(x, y)) = vSum;
        }

    delete []points;
    delete []pointsN;
    delete []pointsE;
    delete []pointsS;
    delete []pointsW;
}

void CTerrainData::getNeighborVector(QVector3D *vecBase, int x, int y, QVector3D *vecNeighbor)
{
    QVector3D *tmp;

    if (y==-1) {
        if (x==-1) {
            tmp = &hNW;
        } else
            if (x==9) {
                tmp = &hNE;
            } else
                tmp = &hN[x];
    } else
        if (y==9) {
            if (x==-1) {
                tmp = &hSW;
            } else
                if (x==9) {
                    tmp = &hSE;
                } else
                    tmp = &hS[x];
        } else {
            if (x==-1)
                tmp = &hW[y]; else
                tmp = &hE[y];
        }

    (*vecNeighbor) = (*tmp) - (*vecBase);
}

void CTerrainData::drawPoint(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss)
{
    QVector3D *vec;
    QColor *col;
    int x, y;

    glBegin(GL_POINTS);
        for (y=yStart; y<yStop; y++) {
            for (x=xStart; x<xStop; x++) {

                vec = getHeight(x, y);
                col = getColor(x, y);

                if (dss->drawTerrainPointColor)
                    glColor3f(col->redF(), col->greenF(), col->blueF()); else
                    glColor3f(1.0, 1.0, 1.0);

                glVertex3d(vec->x(), vec->y(), vec->z());
            }
        }
    glEnd();
}

void CTerrainData::drawNormals(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss)
{
    QVector3D *vec, *vecN;
    int x, y;

    glBegin(GL_LINES);
        glColor3f(0.0, 1.0, 0.0);
        for (y=yStart; y<=yStop; y++) {
            for (x=xStart; x<=xStop; x++) {
                vec = getHeight(x, y);
                vecN = getNormal(x, y);

                glVertex3d(vec->x(), vec->y(), vec->z());
                glVertex3d(vec->x()+vecN->x()*100.0, vec->y()+vecN->y()*100.0, vec->z()+vecN->z()*100.0);
            }
        }
    glEnd();
}

void CTerrainData::drawWire(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss)
{
    QVector3D *vec, *vecS, *vecE;
    QColor *col, *colS, *colE;
    int x, y;

    glBegin(GL_LINES);
        for (y=yStart; y<yStop; y++) {
            for (x=xStart; x<xStop; x++) {

                vec = getHeight(x, y);
                vecS = getHeight(x, y+1);
                vecE = getHeight(x+1, y);

                col = getColor(x, y);
                colS = getColor(x, y+1);
                colE = getColor(x+1, y);

                if (dss->drawTerrainWireColor) {

                    glColor3f(col->redF(), col->greenF(), col->blueF());
                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glColor3f(colE->redF(), colE->greenF(), colE->blueF());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                    glColor3f(col->redF(), col->greenF(), col->blueF());
                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glColor3f(colS->redF(), colS->greenF(), colS->blueF());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());

                    //glColor3f(colS->redF(), colS->greenF(), colS->blueF());
                    //glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    //glColor3f(colE->redF(), colE->greenF(), colE->blueF());
                    //glVertex3d(vecE->x(), vecE->y(), vecE->z());

                } else {

                    glColor3f(0.0, 0.0, 0.0);

                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());

                    //glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    //glVertex3d(vecE->x(), vecE->y(), vecE->z());

                }

            }
        }
    glEnd();

}

void CTerrainData::drawBottomPlaneWire(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss)
{
    QVector3D *hgt = 0, *hgtS = 0, *hgtE = 0, *hgtSE = 0;
    QColor *col = 0, *colS = 0, *colE = 0, *colSE = 0;

    // get data
    if (xStart==0 && yStart==0) {
        hgt = &topLeftPoint;         hgtS = &middleLeftPoint;        hgtE = &topMiddlePoint;        hgtSE = &middleMiddlePoint;
    } else
        if (xStart==4 && yStart==0) {
            hgt = &topMiddlePoint;        hgtS = &middleMiddlePoint;        hgtE = &topRightPoint;        hgtSE = &middleRightPoint;
        } else
            if (xStart==0 && yStart==4) {
                hgt = &middleLeftPoint;        hgtS = &bottomLeftPoint;        hgtE = &middleMiddlePoint;        hgtSE = &bottomMiddlePoint;
            } else
                if (xStart==4 && yStart==4) {
                    hgt = &middleMiddlePoint;        hgtS = &bottomMiddlePoint;        hgtE = &middleRightPoint;        hgtSE = &bottomRightPoint;
                } else
                    qFatal("Wrong xStart or yStart parameter passed to drawBottomPlaneWire function");

    col = getColor(xStart, yStart); colS = getColor(xStart, yStart+4);  colE = getColor(xStart+4, yStart); colSE = getColor(xStart+4, yStart+4);

    // draw terrain bottom plane
    glBegin(GL_LINES);
        if (dss->drawTerrainBottomPlaneWireColor) {

            glColor3f(col->redF(), col->greenF(), col->blueF());
            glVertex3d(hgt->x(), hgt->y(), hgt->z());
            glColor3f(colE->redF(), colE->greenF(), colE->blueF());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

            glColor3f(col->redF(), col->greenF(), col->blueF());
            glVertex3d(hgt->x(), hgt->y(), hgt->z());
            glColor3f(colS->redF(), colS->greenF(), colS->blueF());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());

            glColor3f(colS->redF(), colS->greenF(), colS->blueF());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());
            glColor3f(colE->redF(), colE->greenF(), colE->blueF());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

        } else {

            glColor3f(0.4, 1.0, 0.4);

            glVertex3d(hgt->x(), hgt->y(), hgt->z());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

            glVertex3d(hgt->x(), hgt->y(), hgt->z());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());

            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

        }
    glEnd();
}

void CTerrainData::drawSolid(const int &xStart, const int &xStop, const int &yStart, const int &yStop, const CDrawingStateSnapshot *dss)
{
    QVector3D *vec, *vecS, *vecE, *vecSE;
    QVector3D *nVec, *nVecS, *nVecE, *nVecSE;
    QColor *col, *colS, *colE, *colSE;
    int x, y;

    glBegin(GL_TRIANGLES);
        for (y=yStart; y<yStop; y++) {
            for (x=xStart; x<xStop; x++) {

                vec = getHeight(x, y);
                vecS = getHeight(x, y+1);
                vecE = getHeight(x+1, y);
                vecSE = getHeight(x+1, y+1);

                nVec = getNormal(x, y);
                nVecS = getNormal(x, y+1);
                nVecE = getNormal(x+1, y);
                nVecSE = getNormal(x+1, y+1);

                if (dss->drawTerrainSolidColor) {

                    col = getColor(x, y);
                    colS = getColor(x, y+1);
                    colE = getColor(x+1, y);
                    colSE = getColor(x+1, y+1);

                    glColor3f(col->redF(), col->greenF(), col->blueF());
                    glNormal3d(nVec->x(), nVec->y(), nVec->z());
                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glColor3f(colS->redF(), colS->greenF(), colS->blueF());
                    glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    glColor3f(colE->redF(), colE->greenF(), colE->blueF());
                    glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                    glColor3f(colS->redF(), colS->greenF(), colS->blueF());
                    glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    glColor3f(colSE->redF(), colSE->greenF(), colSE->blueF());
                    glNormal3d(nVecSE->x(), nVecSE->y(), nVecSE->z());
                    glVertex3d(vecSE->x(), vecSE->y(), vecSE->z());
                    glColor3f(colE->redF(), colE->greenF(), colE->blueF());
                    glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                } else {

                    glColor3f(1.0, 1.0, 1.0);

                    glNormal3d(nVec->x(), nVec->y(), nVec->z());
                    glVertex3d(vec->x(), vec->y(), vec->z());
                    glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                    glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                    glVertex3d(vecS->x(), vecS->y(), vecS->z());
                    glNormal3d(nVecSE->x(), nVecSE->y(), nVecSE->z());
                    glVertex3d(vecSE->x(), vecSE->y(), vecSE->z());
                    glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                    glVertex3d(vecE->x(), vecE->y(), vecE->z());

                }

            }
        }
    glEnd();
}

void CTerrainData::drawSolidStrip(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    char *stripIndex = 0;
    QVector3D *vec;
    QVector3D *nVec;
    QColor *col;
    int i;

    // get strip index pointer
    if (xStart==0 && yStart==0) stripIndex = cacheManager->stripIndexListNW; else
        if (xStart==4 && yStart==0) stripIndex = cacheManager->stripIndexListNE; else
            if (xStart==0 && yStart==4) stripIndex = cacheManager->stripIndexListSW; else
                if (xStart==4 && yStart==4) stripIndex = cacheManager->stripIndexListSE; else
                    qFatal("Wrong xStart or yStart parameter passed to drawSolidStrip function");

    glBegin(GL_TRIANGLE_STRIP);
        if (!dss->drawTerrainSolidColor) {
            glColor3f(1.0, 1.0, 1.0);
        }
        for (i=0; i<40; i++) {
            vec = &h[(int)stripIndex[i]];
            nVec = &n[(int)stripIndex[i]];

            if (dss->drawTerrainSolidColor) {
                col = &c[(int)stripIndex[i]];
                glColor3f(col->redF(), col->greenF(), col->blueF());
                glNormal3d(nVec->x(), nVec->y(), nVec->z());
                glVertex3d(vec->x(), vec->y(), vec->z());
            } else {
                glNormal3d(nVec->x(), nVec->y(), nVec->z());
                glVertex3d(vec->x(), vec->y(), vec->z());
            }
        }
    glEnd();
}

void CTerrainData::drawBottomPlaneSolid(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss)
{
    QVector3D *hgt = 0, *hgtS = 0, *hgtE = 0, *hgtSE = 0;
    QVector3D *nor = 0, *norS = 0, *norE = 0, *norSE = 0;
    QColor *col = 0, *colS = 0, *colE = 0, *colSE = 0;

    // get data
    if (xStart==0 && yStart==0) {
        hgt = &topLeftPoint;        hgtS = &middleLeftPoint;        hgtE = &topMiddlePoint;        hgtSE = &middleMiddlePoint;
        nor = &topLeftPointNormal;  norS = &middleLeftPointNormal;  norE = &topMiddlePointNormal;  norSE = &middleMiddlePointNormal;
    } else
        if (xStart==4 && yStart==0) {
            hgt = &topMiddlePoint;        hgtS = &middleMiddlePoint;        hgtE = &topRightPoint;        hgtSE = &middleRightPoint;
            nor = &topMiddlePointNormal;  norS = &middleMiddlePointNormal;  norE = &topRightPointNormal;  norSE = &middleRightPointNormal;
        } else
            if (xStart==0 && yStart==4) {
                hgt = &middleLeftPoint;        hgtS = &bottomLeftPoint;        hgtE = &middleMiddlePoint;        hgtSE = &bottomMiddlePoint;
                nor = &middleLeftPointNormal;  norS = &bottomLeftPointNormal;  norE = &middleMiddlePointNormal;  norSE = &bottomMiddlePointNormal;
            } else
                if (xStart==4 && yStart==4) {
                    hgt = &middleMiddlePoint;        hgtS = &bottomMiddlePoint;        hgtE = &middleRightPoint;        hgtSE = &bottomRightPoint;
                    nor = &middleMiddlePointNormal;  norS = &bottomMiddlePointNormal;  norE = &middleRightPointNormal;  norSE = &bottomRightPointNormal;
                } else
                    qFatal("Wrong xStart or yStart parameter passed to drawBottomPlaneSolid function");

    col = getColor(xStart, yStart); colS = getColor(xStart, yStart+4); colE = getColor(xStart+4, yStart); colSE = getColor(xStart+4, yStart+4);


    // draw terrain bottom plane
    glBegin(GL_TRIANGLE_STRIP);
        if (dss->drawTerrainBottomPlaneSolidColor) {

            glColor3f(col->redF(), col->greenF(), col->blueF());
            glNormal3d(nor->x(), nor->y(), nor->z());
            glVertex3d(hgt->x(), hgt->y(), hgt->z());

            glColor3f(colS->redF(), colS->greenF(), colS->blueF());
            glNormal3d(norS->x(), norS->y(), norS->z());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());

            glColor3f(colE->redF(), colE->greenF(), colE->blueF());
            glNormal3d(norE->x(), norE->y(), norE->z());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

            glColor3f(colSE->redF(), colSE->greenF(), colSE->blueF());
            glNormal3d(norSE->x(), norSE->y(), norSE->z());
            glVertex3d(hgtSE->x(), hgtSE->y(), hgtSE->z());

        } else {

            glColor3f(0.3, 0.3, 1.0);

            glNormal3d(nor->x(), nor->y(), nor->z());
            glVertex3d(hgt->x(), hgt->y(), hgt->z());
            glNormal3d(norS->x(), norS->y(), norS->z());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());
            glNormal3d(norE->x(), norE->y(), norE->z());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());
            glNormal3d(norSE->x(), norSE->y(), norSE->z());
            glVertex3d(hgtSE->x(), hgtSE->y(), hgtSE->z());

        }
    glEnd();
}

void CTerrainData::drawTexture(const int &xStart, const int &xStop, const int &yStart, const int &yStop)
{
    QVector3D *vec, *vecS, *vecE, *vecSE;
    QVector3D *nVec, *nVecS, *nVecE, *nVecSE;
    QVector2D *uvVec, *uvVecS, *uvVecE, *uvVecSE;
    int x, y;

    glEnable(GL_TEXTURE_2D);
    if (textureID==0)
        bindTexture(this);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_TRIANGLES);
        glColor3f(1.0, 1.0, 1.0);
        for (y=yStart; y<yStop; y++) {
            for (x=xStart; x<xStop; x++) {

                vec = getHeight(x, y);
                vecS = getHeight(x, y+1);
                vecE = getHeight(x+1, y);
                vecSE = getHeight(x+1, y+1);

                nVec = getNormal(x, y);
                nVecS = getNormal(x, y+1);
                nVecE = getNormal(x+1, y);
                nVecSE = getNormal(x+1, y+1);

                uvVec = getUv(x, y);
                uvVecS = getUv(x, y+1);
                uvVecE = getUv(x+1, y);
                uvVecSE = getUv(x+1, y+1);

                glNormal3d(nVec->x(), nVec->y(), nVec->z());
                glTexCoord2d(uvVec->x(), uvVec->y());
                glVertex3d(vec->x(), vec->y(), vec->z());

                glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                glTexCoord2d(uvVecS->x(), uvVecS->y());
                glVertex3d(vecS->x(), vecS->y(), vecS->z());

                glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                glTexCoord2d(uvVecE->x(), uvVecE->y());
                glVertex3d(vecE->x(), vecE->y(), vecE->z());


                glNormal3d(nVecS->x(), nVecS->y(), nVecS->z());
                glTexCoord2d(uvVecS->x(), uvVecS->y());
                glVertex3d(vecS->x(), vecS->y(), vecS->z());

                glNormal3d(nVecSE->x(), nVecSE->y(), nVecSE->z());
                glTexCoord2d(uvVecSE->x(), uvVecSE->y());
                glVertex3d(vecSE->x(), vecSE->y(), vecSE->z());

                glNormal3d(nVecE->x(), nVecE->y(), nVecE->z());
                glTexCoord2d(uvVecE->x(), uvVecE->y());
                glVertex3d(vecE->x(), vecE->y(), vecE->z());

            }
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CTerrainData::drawTextureStrip(const int &xStart, const int &yStart, const CDrawingStateSnapshot *dss)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    char *stripIndex = 0;
    QVector3D *vec;
    QVector3D *nVec;
    QVector2D *uvVec;
    int i;

    // get strip index pointer
    if (xStart==0 && yStart==0) stripIndex = cacheManager->stripIndexListNW; else
        if (xStart==4 && yStart==0) stripIndex = cacheManager->stripIndexListNE; else
            if (xStart==0 && yStart==4) stripIndex = cacheManager->stripIndexListSW; else
                if (xStart==4 && yStart==4) stripIndex = cacheManager->stripIndexListSE; else
                    qFatal("Wrong xStart or yStart parameter passed to drawTextureStrip function");

    glEnable(GL_TEXTURE_2D);
        if (textureID==0) bindTexture(this);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_TRIANGLE_STRIP);
            glColor3f(1.0, 1.0, 1.0);
            for (i=0; i<40; i++) {
                vec = &h[(int)stripIndex[i]];
                nVec = &n[(int)stripIndex[i]];
                uvVec = &uv[(int)stripIndex[i]];

                glNormal3d(nVec->x(), nVec->y(), nVec->z());
                glTexCoord2d(uvVec->x(), uvVec->y());
                glVertex3d(vec->x(), vec->y(), vec->z());
            }
        glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CTerrainData::drawBottomPlaneTexture(const int &xStart, const int &yStart)
{
    QVector3D *hgt = 0, *hgtS = 0, *hgtE = 0, *hgtSE = 0;
    QVector3D *nor = 0, *norS = 0, *norE = 0, *norSE = 0;
    QVector2D *Tuv = 0, *TuvS = 0, *TuvE = 0, *TuvSE = 0;

    // get data
    if (xStart==0 && yStart==0) {
        hgt = &topLeftPoint;         hgtS = &middleLeftPoint;        hgtE = &topMiddlePoint;        hgtSE = &middleMiddlePoint;
        nor = &topLeftPointNormal;   norS = &middleLeftPointNormal;  norE = &topMiddlePointNormal;  norSE = &middleMiddlePointNormal;
    } else
        if (xStart==4 && yStart==0) {
            hgt = &topMiddlePoint;        hgtS = &middleMiddlePoint;        hgtE = &topRightPoint;        hgtSE = &middleRightPoint;
            nor = &topMiddlePointNormal;  norS = &middleMiddlePointNormal;  norE = &topRightPointNormal;  norSE = &middleRightPointNormal;
        } else
            if (xStart==0 && yStart==4) {
                hgt = &middleLeftPoint;        hgtS = &bottomLeftPoint;        hgtE = &middleMiddlePoint;        hgtSE = &bottomMiddlePoint;
                nor = &middleLeftPointNormal;  norS = &bottomLeftPointNormal;  norE = &middleMiddlePointNormal;  norSE = &bottomMiddlePointNormal;
            } else
                if (xStart==4 && yStart==4) {
                    hgt = &middleMiddlePoint;        hgtS = &bottomMiddlePoint;        hgtE = &middleRightPoint;        hgtSE = &bottomRightPoint;
                    nor = &middleMiddlePointNormal;  norS = &bottomMiddlePointNormal;  norE = &middleRightPointNormal;  norSE = &bottomRightPointNormal;
                } else
                    qFatal("Wrong xStart or yStart parameter passed to drawBottomPlaneTexture function");

    Tuv = getUv(xStart, yStart); TuvS = getUv(xStart, yStart+4);  TuvE = getUv(xStart+4, yStart); TuvSE = getUv(xStart+4, yStart+4);


    // draw terrain bottom plane
    glEnable(GL_TEXTURE_2D);
        if (textureID==0) bindTexture(this);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_TRIANGLE_STRIP);

            glColor3f(1.0, 1.0, 1.0);

            glNormal3d(nor->x(), nor->y(), nor->z());
            glTexCoord2d(Tuv->x(), Tuv->y());
            glVertex3d(hgt->x(), hgt->y(), hgt->z());

            glNormal3d(norS->x(), norS->y(), norS->z());
            glTexCoord2d(TuvS->x(), TuvS->y());
            glVertex3d(hgtS->x(), hgtS->y(), hgtS->z());

            glNormal3d(norE->x(), norE->y(), norE->z());
            glTexCoord2d(TuvE->x(), TuvE->y());
            glVertex3d(hgtE->x(), hgtE->y(), hgtE->z());

            glNormal3d(norSE->x(), norSE->y(), norSE->z());
            glTexCoord2d(TuvSE->x(), TuvSE->y());
            glVertex3d(hgtSE->x(), hgtSE->y(), hgtSE->z());

        glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CTerrainData::bindTexture(CTerrainData *terrainData)
{
    GLfloat color[4] = { 0.0, 0.0, 0.0, 0.0 };
    bool wrap = false;
    GLuint textureID;
    unsigned char *textureData;

    textureData = terrainData->getTexturePointer();

    // uploading texture to VRAM
    // very nice tutorial here http://www.nullterminator.net/gltexture.html :)
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP ); // no GL_CLAMP_TO_BORDER_EXT in Qt :/
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP ); // no GL_CLAMP_TO_BORDER_EXT in Qt :/
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color );
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TEX_TERRAIN_SIZE, TEX_TERRAIN_SIZE, GL_RGB, GL_UNSIGNED_BYTE, textureData );

    // below sample from http://glprogramming.com/red/chapter09.html without mipmaping
    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_TERRAIN_SIZE, TEX_TERRAIN_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    */

    terrainData->setTextureID(textureID);
}
