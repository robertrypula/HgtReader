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

#include <QTime>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QImage>
#include <QDebug>
#include <math.h>
#include "CCacheManager.h"
#include "CCommons.h"
#include "CHgtFile.h"


CCacheManager *CCacheManager::instance;

CCacheManager::CCacheManager()
{
    int i;

    // something like singleton :)
    instance = this;

    pathBase = ""; // "E:\\HgtReader_data\\";
    pathL00_L03 = pathBase + "L00-L03\\";
    pathL04_L08 = pathBase + "L04-L08\\";
    pathL09_L13 = pathBase + "L09-L13\\";
    pathSRTM = pathBase + "NASA_SRTM\\";
    pathTexL00_L02 = pathBase + "Textures\\L00_L02\\";
    pathTexL03_L05 = pathBase + "Textures\\L03_L05\\";
    pathTexL06_L08 = pathBase + "Textures\\L06_L08\\";
    pathTexL09_L10 = pathBase + "Textures\\L09_L10\\";
    pathL00_L03_index = pathBase + "L00-L03_index\\";
    pathL04_L08_index = pathBase + "L04-L08_index\\";
    pathL09_L13_index = pathBase + "L09-L13_index\\";
    pathSRTM_index = pathBase + "NASA_SRTM_index\\";

    // generate degree size of tile in each LOD
    LODdegreeSizeLookUp[0] = 60.0;
    for (i=1; i<=13; i++)
        LODdegreeSizeLookUp[i] = LODdegreeSizeLookUp[i-1]/2.0;

    // create look-up tables - source for each LOD
    for (i=0; i<=3; i++)  HGTsourceLookUp[i] = HGT_SOURCE_L00_L03;
    for (i=4; i<=8; i++)  HGTsourceLookUp[i] = HGT_SOURCE_L04_L08;
    for (i=9; i<=13; i++) HGTsourceLookUp[i] = HGT_SOURCE_L09_L13;

    // create look-up tables - source size (n x n) for each LOD
    for (i=0; i<=3; i++)  HGTsourceSizeLookUp[i] = HGT_SOURCE_SIZE_L00_L03;
    for (i=4; i<=8; i++)  HGTsourceSizeLookUp[i] = HGT_SOURCE_SIZE_L04_L08;
    for (i=9; i<=13; i++) HGTsourceSizeLookUp[i] = HGT_SOURCE_SIZE_L09_L13;

    // create look-up tables - source degree size for each LOD
    for (i=0; i<=3; i++)  HGTsourceDegreeSizeLookUp[i] = HGT_SOURCE_DEGREE_SIZE_L00_L03;
    for (i=4; i<=8; i++)  HGTsourceDegreeSizeLookUp[i] = HGT_SOURCE_DEGREE_SIZE_L04_L08;
    for (i=9; i<=13; i++) HGTsourceDegreeSizeLookUp[i] = HGT_SOURCE_DEGREE_SIZE_L09_L13;

    // create look-up tables - source degree size for each LOD
    for (i=0; i<=3; i++)  HGTsourceSkippingLookUp[i] = pow(2, 3-i);
    for (i=4; i<=8; i++)  HGTsourceSkippingLookUp[i] = pow(2, 8-i);
    for (i=9; i<=13; i++) HGTsourceSkippingLookUp[i] = pow(2, 13-i);

    // create look-up tables - texture
    for (i=0; i<=2; i++)  TEXsourceLookUp[i] = TEX_SOURCE_L00_L02;
    for (i=3; i<=5; i++)  TEXsourceLookUp[i] = TEX_SOURCE_L03_L05;
    for (i=6; i<=8; i++)  TEXsourceLookUp[i] = TEX_SOURCE_L06_L08;
    for (i=9; i<=13; i++) TEXsourceLookUp[i] = TEX_SOURCE_L09_L10;

    // create look-up tables - texture pixel skipping
    for (i=0; i<=2; i++)  TEXsourceSkippingLookUp[i] = pow(2, 2-i);
    for (i=3; i<=5; i++)  TEXsourceSkippingLookUp[i] = pow(2, 5-i);
    for (i=6; i<=8; i++)  TEXsourceSkippingLookUp[i] = pow(2, 8-i);
    TEXsourceSkippingLookUp[9]  = 2;
    TEXsourceSkippingLookUp[10] = 1;
    TEXsourceSkippingLookUp[11] = -2;
    TEXsourceSkippingLookUp[12] = -4;
    TEXsourceSkippingLookUp[13] = -8;

    // create look-up tables - texture tile size
    for (i=0; i<=2; i++)  TEXsourcePxSizeLookUp[i] = TEX_SOURCE_PX_SIZE_L00_L02;
    for (i=3; i<=5; i++)  TEXsourcePxSizeLookUp[i] = TEX_SOURCE_PX_SIZE_L03_L05;
    for (i=6; i<=8; i++)  TEXsourcePxSizeLookUp[i] = TEX_SOURCE_PX_SIZE_L06_L08;
    for (i=9; i<=13; i++) TEXsourcePxSizeLookUp[i] = TEX_SOURCE_PX_SIZE_L09_L10;

    // setup avability tables by reading each HGT files directory
    setupAvabilityTables();

    // setup cached terrains tables
    setupCachedTerrainDataTables();
    cacheTime.start();

    // setup texture avability tables
    setupTextureAvalibityTables();

    // setup strip index
    setupStripIndex();
}

CCacheManager::~CCacheManager()
{
    delete []avability_L00_L03;
    delete []avability_L04_L08;
    delete []avability_L09_L13;
    delete []avability_SRTM;
    delete []avabilityTex_L00_L02;
    delete []avabilityTex_L03_L05;
    delete []avabilityTex_L06_L08;
    delete []avabilityTex_L09_L10;
    delete []stripIndexListNW;
    delete []stripIndexListNE;
    delete []stripIndexListSW;
    delete []stripIndexListSE;

    cacheClear(0);  // drop all unused terrains

    delete []cachedTerrainDataGroup_L00_L03;
    delete []cachedTerrainDataGroup_L04_L08;
    delete []cachedTerrainDataGroup_L09_L13;

    instance = 0;
}

CCacheManager *CCacheManager::getInstance()
{
    return instance;
}

void CCacheManager::setupStripIndex()
{
    int i;
    char stripIndexListNWtmp[] = {  0,  9,  1, 10,  2, 11,  3, 12,  4, 13,
                                   13, 22, 12, 21, 11, 20, 10, 19,  9, 18,
                                   18, 27, 19, 28, 20, 29, 21, 30, 22, 31,
                                   31, 40, 30, 39, 29, 38, 28, 37, 27, 36 };
    char stripIndexListNEtmp[] = {  4, 13,  5, 14,  6, 15,  7, 16,  8, 17,
                                   17, 26, 16, 25, 15, 24, 14, 23, 13, 22,
                                   22, 31, 23, 32, 24, 33, 25, 34, 26, 35,
                                   35, 44, 34, 43, 33, 42, 32, 41, 31, 40 };
    char stripIndexListSWtmp[] = { 36, 45, 37, 46, 38, 47, 39, 48, 40, 49,
                                   49, 58, 48, 57, 47, 56, 46, 55, 45, 54,
                                   54, 63, 55, 64, 56, 65, 57, 66, 58, 67,
                                   67, 76, 66, 75, 65, 74, 64, 73, 63, 72 };
    char stripIndexListSEtmp[] = { 40, 49, 41, 50, 42, 51, 43, 52, 44, 53,
                                   53, 62, 52, 61, 51, 60, 50, 59, 49, 58,
                                   58, 67, 59, 68, 60, 69, 61, 70, 62, 71,
                                   71, 80, 70, 79, 69, 78, 68, 77, 67, 76 };


    stripIndexListNW = new char[40];
    for (i=0; i<40; i++)
        stripIndexListNW[i] = stripIndexListNWtmp[i];

    stripIndexListNE = new char[40];
    for (i=0; i<40; i++)
        stripIndexListNE[i] = stripIndexListNEtmp[i];

    stripIndexListSW = new char[40];
    for (i=0; i<40; i++)
        stripIndexListSW[i] = stripIndexListSWtmp[i];

    stripIndexListSE = new char[40];
    for (i=0; i<40; i++)
        stripIndexListSE[i] = stripIndexListSEtmp[i];
}

void CCacheManager::getTerrainPoints(double lon, double lat, int lod,
                                     int *points, int *pointNW, int *pointNE, int *pointSW, int *pointSE,
                                     int *pointsN, int *pointsE, int *pointsS, int *pointsW, unsigned char *texture,
                                     bool dontUseDiskHgt, bool dontUseDiskRaw)
{
    QString filePath;
    CRawFile terrainTexture;
    CHgtFile hgtFile;
    double lodDegreeSize;
    double neighborLon, neighborLat;
    int i, x, y, hgtSkipping, hgtSize;
    bool fileFound;

    // pixel buffer comes from TerrainData object
    terrainTexture.setPixelsPointer(TEX_TERRAIN_SIZE, TEX_TERRAIN_SIZE, (CRawPixel *)texture);

    if (dontUseDiskRaw) {
        for (y=0; y<TEX_TERRAIN_SIZE; y++)
            for (x=0; x<TEX_TERRAIN_SIZE; x++) {
                terrainTexture.setPixel(x, y, CRawPixel(TEX_EMPTY_COLOR));
            }
    } else {
        buildTextureFromRawFiles(lon, lat, lod, &terrainTexture);    // get texture data
    }

    if (dontUseDiskHgt) {

        for (i=0; i<81; i++) points[i] = HGT_DONT_USE_DISK_HEIGHT;
        (*pointNW) = HGT_DONT_USE_DISK_HEIGHT;
        (*pointNE) = HGT_DONT_USE_DISK_HEIGHT;
        (*pointSW) = HGT_DONT_USE_DISK_HEIGHT;
        (*pointSE) = HGT_DONT_USE_DISK_HEIGHT;
        for (i=0; i<9; i++) pointsN[i] = HGT_DONT_USE_DISK_HEIGHT;
        for (i=0; i<9; i++) pointsE[i] = HGT_DONT_USE_DISK_HEIGHT;
        for (i=0; i<9; i++) pointsS[i] = HGT_DONT_USE_DISK_HEIGHT;
        for (i=0; i<9; i++) pointsW[i] = HGT_DONT_USE_DISK_HEIGHT;

    } else {

        lodDegreeSize = LODdegreeSizeLookUp[lod];

        // main terrain tile
        findHgtFileName(lon, lat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
        if (fileFound) {
            hgtFile.fileOpen(filePath, hgtSize, hgtSize);
            hgtFile.fileGetHeightBlock(points, x, y, 9, 9, hgtSkipping);
            hgtFile.fileClose();
        } else {
            for (i=0; i<81; i++)
                points[i] = 0;
        }

        // Neighbor NW
        neighborLon = lon - lodDegreeSize;
        neighborLat = lat + lodDegreeSize;
        if (neighborLon<0.0) neighborLon += 360.0;
        if (neighborLat>90.0) {
            (*pointNW) = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                (*pointNW) = hgtFile.fileGetHeight(x + 7*hgtSkipping, y + 7*hgtSkipping);
                hgtFile.fileClose();
            } else {
                (*pointNW) = 0;
            }
        }

        // Neighbor NE
        neighborLon = lon + lodDegreeSize;
        neighborLat = lat + lodDegreeSize;
        if (neighborLon>=360.0) neighborLon -= 360.0;
        if (neighborLat>90.0) {
            (*pointNE) = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                (*pointNE) = hgtFile.fileGetHeight(x + 1*hgtSkipping, y + 7*hgtSkipping);
                hgtFile.fileClose();
            } else {
                (*pointNE) = 0;
            }
        }

        // Neighbor SE
        neighborLon = lon + lodDegreeSize;
        neighborLat = lat - lodDegreeSize;
        if (neighborLon>=360.0) neighborLon -= 360.0;
        if (neighborLat<=-90.0) {
            (*pointSE) = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                (*pointSE) = hgtFile.fileGetHeight(x + 1*hgtSkipping, y + 1*hgtSkipping);
                hgtFile.fileClose();
            } else {
                (*pointSE) = 0;
            }
        }

        // Neighbor SW
        neighborLon = lon - lodDegreeSize;
        neighborLat = lat - lodDegreeSize;
        if (neighborLon<0.0) neighborLon += 360.0;
        if (neighborLat<=-90.0) {
            (*pointSW) = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                (*pointSW) = hgtFile.fileGetHeight(x + 7*hgtSkipping, y + 1*hgtSkipping);
                hgtFile.fileClose();
            } else {
                (*pointSW) = 0;
            }
        }

        // Neighbor N
        neighborLon = lon;
        neighborLat = lat + lodDegreeSize;
        if (neighborLat>90.0) {
            for (i=0; i<9; i++) pointsN[i] = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                for (i=0; i<9; i++)
                    pointsN[i] = hgtFile.fileGetHeight(x + i*hgtSkipping, y + 7*hgtSkipping);
                hgtFile.fileClose();
            } else {
                for (i=0; i<9; i++) pointsN[i] = 0;
            }
        }

        // Neighbor E
        neighborLon = lon + lodDegreeSize;
        neighborLat = lat;
        if (neighborLon>=360.0) neighborLon -= 360.0;
        findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
        if (fileFound) {
            hgtFile.fileOpen(filePath, hgtSize, hgtSize);
            for (i=0; i<9; i++)
                pointsE[i] = hgtFile.fileGetHeight(x + 1*hgtSkipping, y + i*hgtSkipping);
            hgtFile.fileClose();
        } else {
            for (i=0; i<9; i++) pointsE[i] = 0;
        }

        // Neighbor S
        neighborLon = lon;
        neighborLat = lat - lodDegreeSize;
        if (neighborLat<=-90.0) {
            for (i=0; i<9; i++) pointsS[i] = 0;
        } else {
            findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
            if (fileFound) {
                hgtFile.fileOpen(filePath, hgtSize, hgtSize);
                for (i=0; i<9; i++)
                    pointsS[i] = hgtFile.fileGetHeight(x + i*hgtSkipping, y + 1*hgtSkipping);
                hgtFile.fileClose();
            } else {
                for (i=0; i<9; i++) pointsS[i] = 0;
            }
        }

        // Neighbor W
        neighborLon = lon - lodDegreeSize;
        neighborLat = lat;
        if (neighborLon<0.0) neighborLon += 360.0;
        findHgtFileName(neighborLon, neighborLat, lod, &filePath, &fileFound, &x, &y, &hgtSkipping, &hgtSize);
        if (fileFound) {
            hgtFile.fileOpen(filePath, hgtSize, hgtSize);
            for (i=0; i<9; i++)
                pointsW[i] = hgtFile.fileGetHeight(x + 7*hgtSkipping, y + i*hgtSkipping);
            hgtFile.fileClose();
        } else {
            for (i=0; i<9; i++) pointsW[i] = 0;
        }

    }
}

void CCacheManager::findHgtFileName(const double &lon, const double &lat, const int &lod,
                                    QString *filePath, bool *fileFound, int *x, int *y,
                                    int *hgtSkipping, int *hgtSize)
{
    double tlLon, tlLat;
    int index;

    // find hgt LOD directory, filename, position in file
    CCommons::findTopLeftCornerOfHgtFile(lon, lat, lod, &tlLon, &tlLat);
    CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, HGTsourceDegreeSizeLookUp[lod], &index);
    CCommons::findXYInHgtFile(tlLon, tlLat, lon, lat, lod, x, y);
    (*hgtSkipping) = HGTsourceSkippingLookUp[lod];
    (*hgtSize) = HGTsourceSizeLookUp[lod];

    // check that file exists in HGT directory
    (*fileFound) = false;
    switch (HGTsourceLookUp[lod]) {
        case HGT_SOURCE_L00_L03:if (avability_L00_L03[index].available) {
                                    (*filePath) = pathL00_L03 + (*avability_L00_L03[index].name);
                                    (*fileFound) = true;
                                }
                                break;
        case HGT_SOURCE_L04_L08:if (avability_L04_L08[index].available) {
                                    (*filePath) = pathL04_L08 + (*avability_L04_L08[index].name);
                                    (*fileFound) = true;
                                }
                                break;
        case HGT_SOURCE_L09_L13:if (avability_L09_L13[index].available) {
                                    (*filePath) = pathL09_L13 + (*avability_L09_L13[index].name);
                                    (*fileFound) = true;
                                }
                                break;
    }
}

bool CCacheManager::findRawFiles(const double &tlLon, const double &tlLat, const int &lod,
                                 int *RAWfilesIndex, int *pixOffsetLon, int *pixOffsetLat)
{
    double degreeDeltaLon, degreeDeltaLat;
    double TEXtlLon, TEXtlLat;
    double HGTtlLon, HGTtlLat;
    int x, y;
    int index;
    bool hasAtLeastOneRAWFile;

    CCommons::findTopLeftCorner(tlLon, tlLat, LODdegreeSizeLookUp[lod<=TEX_SOURCE_MAX_LOD ? lod : 10], &HGTtlLon, &HGTtlLat);
    CCommons::findTopLeftCorner(HGTtlLon, HGTtlLat, TEX_DEGREE_SIZE, &TEXtlLon, &TEXtlLat);

    degreeDeltaLon = HGTtlLon - TEXtlLon;
    degreeDeltaLat = HGTtlLat - TEXtlLat;
    if (degreeDeltaLon<0.0) degreeDeltaLon *= -1.0;
    if (degreeDeltaLat<0.0) degreeDeltaLat *= -1.0;

    (*pixOffsetLon) = (int)( (degreeDeltaLon/TEX_DEGREE_SIZE) * ((double)TEXsourcePxSizeLookUp[lod]) + 0.5 );
    (*pixOffsetLat) = (int)( (degreeDeltaLat/TEX_DEGREE_SIZE) * ((double)TEXsourcePxSizeLookUp[lod]) + 0.5 );

    hasAtLeastOneRAWFile = false;
    for (y=0; y<2; y++)
        for (x=0; x<2; x++) {
            CCommons::convertTopLeft2AvabilityIndex(TEXtlLon + x*TEX_DEGREE_SIZE,
                                                    TEXtlLat - y*TEX_DEGREE_SIZE,
                                                    TEX_DEGREE_SIZE,
                                                    &index);
            if (index>=8*4)
                index = -1;

            if (index!=-1) {
                switch (TEXsourceLookUp[lod]) {
                    case TEX_SOURCE_L00_L02:if (avabilityTex_L00_L02[index].available)
                                                hasAtLeastOneRAWFile = true; else
                                                index = -1;
                                            break;
                    case TEX_SOURCE_L03_L05:if (avabilityTex_L03_L05[index].available)
                                                hasAtLeastOneRAWFile = true; else
                                                index = -1;
                                            break;
                    case TEX_SOURCE_L06_L08:if (avabilityTex_L06_L08[index].available)
                                                hasAtLeastOneRAWFile = true; else
                                                index = -1;
                                            break;
                    case TEX_SOURCE_L09_L10:if (avabilityTex_L09_L10[index].available)
                                                hasAtLeastOneRAWFile = true; else
                                                index = -1;
                                            break;
                }
            }

            RAWfilesIndex[y*2 + x] = index;
        }

    return hasAtLeastOneRAWFile;
}

void CCacheManager::buildTextureFromRawFiles(const double &tlLon, const double &tlLat, const int &lod, CRawFile *terrainTexture)
{
    QImage texture(TEX_TERRAIN_SIZE, TEX_TERRAIN_SIZE, QImage::Format_RGB32);
    QString fileName;
    int index;
    CRawFile rawFile;
    CRawPixel pixel;
    int x, y;
    int pixInBaseTileLon;
    int pixInBaseTileLat;
    int pixInBaseStopLon;
    int pixInBaseStopLat;
    int pixInNeighborStopLon;
    int pixInNeighborStopLat;
    int TEXskipping, TEXpxSize;
    double TEXmult;
    int pixOffsetLon, pixOffsetLat;
    bool hasAtLeastOneRawFile;
    int RAWfilesIndex[4];

    hasAtLeastOneRawFile = findRawFiles(tlLon, tlLat, lod, RAWfilesIndex, &pixOffsetLon, &pixOffsetLat);
    if ( ! hasAtLeastOneRawFile) {
        for (y=0; y<TEX_TERRAIN_SIZE; y++)
            for (x=0; x<TEX_TERRAIN_SIZE; x++) {
                terrainTexture->setPixel(x, y, CRawPixel(TEX_EMPTY_COLOR));
            }
        return;
    }

    TEXskipping = TEXsourceSkippingLookUp[lod];
    TEXpxSize = TEXsourcePxSizeLookUp[lod];

    TEXmult = TEXskipping;
    if (TEXmult<0.0) {
        TEXmult = 1.0;
//        TEXmult *= -1;
//        TEXmult = 1.0/TEXmult;
    }

    // texture 32x32 window in base tile if:
    //    pixOffsetLon + TEXskipping*x  < TEXpxSize
    //    TEXskipping*x  < TEXpxSize - pixOffsetLon
    //    x < (TEXpxSize - pixOffsetLon) / TEXskipping
    pixInBaseTileLon = (TEXpxSize - pixOffsetLon) / (TEXskipping<0 ? 1 : TEXskipping);
    pixInBaseTileLat = (TEXpxSize - pixOffsetLat) / (TEXskipping<0 ? 1 : TEXskipping);

    if (pixInBaseTileLon>=TEX_TERRAIN_SIZE) {    // all in base
        pixInBaseStopLon = TEX_TERRAIN_SIZE;
        pixInNeighborStopLon = 0;
    } else {                                     // part in base
        pixInBaseStopLon = pixInBaseTileLon;
        pixInNeighborStopLon = TEX_TERRAIN_SIZE - pixInBaseTileLon;
    }
    if (pixInBaseTileLat>=TEX_TERRAIN_SIZE) {    // all in base
        pixInBaseStopLat = TEX_TERRAIN_SIZE;
        pixInNeighborStopLat = 0;
    } else {                                     // part in base
        pixInBaseStopLat = pixInBaseTileLat;
        pixInNeighborStopLat = TEX_TERRAIN_SIZE - pixInBaseTileLat;
    }

    // copy from base tile
    index = RAWfilesIndex[0 + 2*0];
    pixel = CRawPixel(TEX_EMPTY_COLOR);
    fileName = "";
    if (index!=-1) {
        switch (TEXsourceLookUp[lod]) {
            case TEX_SOURCE_L00_L02:fileName = pathTexL00_L02 + (*avabilityTex_L00_L02[index].name); break;
            case TEX_SOURCE_L03_L05:fileName = pathTexL03_L05 + (*avabilityTex_L03_L05[index].name); break;
            case TEX_SOURCE_L06_L08:fileName = pathTexL06_L08 + (*avabilityTex_L06_L08[index].name); break;
            case TEX_SOURCE_L09_L10:fileName = pathTexL09_L10 + (*avabilityTex_L09_L10[index].name); break;
        }
        rawFile.fileOpen(fileName, TEXpxSize, TEXpxSize);
    }
    for (y=0; y<pixInBaseStopLat; y++)
        for (x=0; x<pixInBaseStopLon; x++) {
            if (index!=-1)
                pixel = rawFile.fileGetPixel(pixOffsetLon + (int)(x*TEXmult), pixOffsetLat + (int)(y*TEXmult));

            texture.setPixel(x, y, qRgb(pixel.r, pixel.g, pixel.b));
            //terrainTexture->setPixel(x, y, pixel);
        }
    if (index!=-1)
        rawFile.fileClose();

    // copy from right tile
    index = RAWfilesIndex[1 + 2*0];
    pixel = CRawPixel(TEX_EMPTY_COLOR);
    fileName = "";
    if (index!=-1) {
        switch (TEXsourceLookUp[lod]) {
            case TEX_SOURCE_L00_L02:fileName = pathTexL00_L02 + (*avabilityTex_L00_L02[index].name); break;
            case TEX_SOURCE_L03_L05:fileName = pathTexL03_L05 + (*avabilityTex_L03_L05[index].name); break;
            case TEX_SOURCE_L06_L08:fileName = pathTexL06_L08 + (*avabilityTex_L06_L08[index].name); break;
            case TEX_SOURCE_L09_L10:fileName = pathTexL09_L10 + (*avabilityTex_L09_L10[index].name); break;
        }
        rawFile.fileOpen(fileName, TEXpxSize, TEXpxSize);
    }
    for (y=0; y<pixInBaseStopLat; y++)
        for (x=0; x<pixInNeighborStopLon; x++) {
            if (index!=-1)
                pixel = rawFile.fileGetPixel(0 + (int)(x*TEXmult), pixOffsetLat + (int)(y*TEXmult));

            texture.setPixel(pixInBaseStopLon + x, y, qRgb(pixel.r, pixel.g, pixel.b));
            //terrainTexture->setPixel(pixInBaseStopLon + x, y, pixel);
        }
    if (index!=-1)
        rawFile.fileClose();

    // copy from left-bottom tile
    index = RAWfilesIndex[0 + 2*1];
    pixel = CRawPixel(TEX_EMPTY_COLOR);
    fileName = "";
    if (index!=-1) {
        switch (TEXsourceLookUp[lod]) {
            case TEX_SOURCE_L00_L02:fileName = pathTexL00_L02 + (*avabilityTex_L00_L02[index].name); break;
            case TEX_SOURCE_L03_L05:fileName = pathTexL03_L05 + (*avabilityTex_L03_L05[index].name); break;
            case TEX_SOURCE_L06_L08:fileName = pathTexL06_L08 + (*avabilityTex_L06_L08[index].name); break;
            case TEX_SOURCE_L09_L10:fileName = pathTexL09_L10 + (*avabilityTex_L09_L10[index].name); break;
        }
        rawFile.fileOpen(fileName, TEXpxSize, TEXpxSize);
    }
    for (y=0; y<pixInNeighborStopLat; y++)
        for (x=0; x<pixInBaseStopLon; x++) {
            if (index!=-1)
                pixel = rawFile.fileGetPixel(pixOffsetLon + (int)(x*TEXmult), 0 + (int)(y*TEXmult));

            texture.setPixel(0 + x, pixInBaseStopLat + y, qRgb(pixel.r, pixel.g, pixel.b));
            //terrainTexture->setPixel(0 + x, pixInBaseStopLat + y, pixel);
        }
    if (index!=-1)
        rawFile.fileClose();

    // copy from right-bottom tile
    index = RAWfilesIndex[1 + 2*1];
    pixel = CRawPixel(TEX_EMPTY_COLOR);
    fileName = "";
    if (index!=-1) {
        switch (TEXsourceLookUp[lod]) {
            case TEX_SOURCE_L00_L02:fileName = pathTexL00_L02 + (*avabilityTex_L00_L02[index].name); break;
            case TEX_SOURCE_L03_L05:fileName = pathTexL03_L05 + (*avabilityTex_L03_L05[index].name); break;
            case TEX_SOURCE_L06_L08:fileName = pathTexL06_L08 + (*avabilityTex_L06_L08[index].name); break;
            case TEX_SOURCE_L09_L10:fileName = pathTexL09_L10 + (*avabilityTex_L09_L10[index].name); break;
        }
        rawFile.fileOpen(fileName, TEXpxSize, TEXpxSize);
    }
    for (y=0; y<pixInNeighborStopLat; y++)
        for (x=0; x<pixInNeighborStopLon; x++) {
            if (index!=-1)
                pixel = rawFile.fileGetPixel(0 + (int)(x*TEXmult), 0 + (int)(y*TEXmult));

            texture.setPixel(pixInBaseStopLon + x, pixInBaseStopLat + y, qRgb(pixel.r, pixel.g, pixel.b));
            //terrainTexture->setPixel(pixInBaseStopLon + x, pixInBaseStopLat + y, pixel);
        }
    if (index!=-1)
        rawFile.fileClose();


    // smooth texture - now filtered by graphic card (faster)
//    if (TEXskipping!=1) {
//        texture = texture.scaledToWidth(40, Qt::SmoothTransformation);
//        texture = texture.scaledToWidth(TEX_TERRAIN_SIZE, Qt::SmoothTransformation);
//    }

    // copy terrain texture
    for (y=0; y<TEX_TERRAIN_SIZE; y++)
        for (x=0; x<TEX_TERRAIN_SIZE; x++) {
            terrainTexture->setPixel(x, y, CRawPixel(texture.pixel(x, y)));
        }
}

void CCacheManager::setEarthBuffers(CEarth *eBuffA, CEarth *eBuffB)
{
    earthBufferA = eBuffA;
    earthBufferB = eBuffB;
}

bool CCacheManager::cacheTerrainDataFind(const double lon, const double lat, const int lod, const CEarth *earth, CTerrainData **terrainData)
{
    int index;
    double tlLonSource, tlLatSource;
    double tlLon, tlLat;
    bool result = false;

    // find cache region on Earth where to search for terrain
    CCommons::findTopLeftCorner(lon, lat, LODdegreeSizeLookUp[lod], &tlLon, &tlLat);
    CCommons::findTopLeftCornerOfHgtFile(lon, lat, lod, &tlLonSource, &tlLatSource);
    CCommons::convertTopLeft2AvabilityIndex(tlLonSource, tlLatSource, HGTsourceDegreeSizeLookUp[lod], &index);

    // search in region for cached terrain
    switch (HGTsourceLookUp[lod]) {
        case HGT_SOURCE_L00_L03:result = cachedTerrainDataGroup_L00_L03[index].cachedTerrainDataListFind(tlLon, tlLat, lod, earth, terrainData);
                                break;
        case HGT_SOURCE_L04_L08:result = cachedTerrainDataGroup_L04_L08[index].cachedTerrainDataListFind(tlLon, tlLat, lod, earth, terrainData);
                                break;
        case HGT_SOURCE_L09_L13:result = cachedTerrainDataGroup_L09_L13[index].cachedTerrainDataListFind(tlLon, tlLat, lod, earth, terrainData);
                                break;
    }

    return result;
}

void CCacheManager::cacheTerrainDataRegister(const CEarth *earth, CTerrainData **terrainData)
{
    double tlLonSource, tlLatSource;
    int index;

    // find cache region on Earth where to register terrain
    CCommons::findTopLeftCornerOfHgtFile((*terrainData)->topLeftLon, (*terrainData)->topLeftLat,
                                         (*terrainData)->LOD, &tlLonSource, &tlLatSource);
    CCommons::convertTopLeft2AvabilityIndex(tlLonSource, tlLatSource, HGTsourceDegreeSizeLookUp[(*terrainData)->LOD], &index);

    // regiter terrain in cache region
    switch (HGTsourceLookUp[(*terrainData)->LOD]) {
        case HGT_SOURCE_L00_L03:cachedTerrainDataGroup_L00_L03[index].cachedTerrainDataListRegister(earth, terrainData);
                                break;
        case HGT_SOURCE_L04_L08:cachedTerrainDataGroup_L04_L08[index].cachedTerrainDataListRegister(earth, terrainData);
                                break;
        case HGT_SOURCE_L09_L13:cachedTerrainDataGroup_L09_L13[index].cachedTerrainDataListRegister(earth, terrainData);
                                break;
    }
}

void CCacheManager::cacheTerrainDataFree(const CEarth *earth, CTerrainData **terrainData, const bool &dontSaveJustDelete)
{
    double tlLonSource, tlLatSource;
    int index;

    // find cache region on Earth where to free terrain
    CCommons::findTopLeftCornerOfHgtFile((*terrainData)->topLeftLon, (*terrainData)->topLeftLat, (*terrainData)->LOD, &tlLonSource, &tlLatSource);
    CCommons::convertTopLeft2AvabilityIndex(tlLonSource, tlLatSource, HGTsourceDegreeSizeLookUp[(*terrainData)->LOD], &index);

    // free terrain in cache region
    switch (HGTsourceLookUp[(*terrainData)->LOD]) {
        case HGT_SOURCE_L00_L03:cachedTerrainDataGroup_L00_L03[index].cachedTerrainDataListFree(earth, terrainData, dontSaveJustDelete);
                                break;
        case HGT_SOURCE_L04_L08:cachedTerrainDataGroup_L04_L08[index].cachedTerrainDataListFree(earth, terrainData, dontSaveJustDelete);
                                break;
        case HGT_SOURCE_L09_L13:cachedTerrainDataGroup_L09_L13[index].cachedTerrainDataListFree(earth, terrainData, dontSaveJustDelete);
                                break;
    }
}

void CCacheManager::cacheInfo(int *cTDCount, int *cTDInUseCount, int *cTDNotInUseCount, int *cTDEmptyEntryCount, unsigned int *cMinNotInUseTime)
{
    int L00_L03_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L00_L03_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L04_L08_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L04_L08_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L09_L13_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int L09_L13_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int i;
    int tmpCount, tmpInUseCount, tmpNotInUseCount, tmpEmptyEntryCount;

    cachedTerrainDataCount = 0;
    cachedTerrainDataInUseCount = 0;
    cachedTerrainDataNotInUseCount = 0;
    cachedTerrainDataEmptyEntryCount = 0;
    cacheMinNotInUseTime = 25*3600*1000;

    // L00-L03
    for (i=0; i<L00_L03_width*L00_L03_height; i++) {
        tmpCount = 0;
        tmpInUseCount = 0;
        tmpNotInUseCount = 0;
        tmpEmptyEntryCount = 0;
        cachedTerrainDataGroup_L00_L03[i].cachedTerrainDataInfo(&tmpCount, &tmpInUseCount, &tmpNotInUseCount, &tmpEmptyEntryCount, &cacheMinNotInUseTime);

        cachedTerrainDataCount += tmpCount;
        cachedTerrainDataInUseCount += tmpInUseCount;
        cachedTerrainDataNotInUseCount += tmpNotInUseCount;
        cachedTerrainDataEmptyEntryCount += tmpEmptyEntryCount;
    }

    // L04-L08
    for (i=0; i<L04_L08_width*L04_L08_height; i++) {
        tmpCount = 0;
        tmpInUseCount = 0;
        tmpNotInUseCount = 0;
        tmpEmptyEntryCount = 0;
        cachedTerrainDataGroup_L04_L08[i].cachedTerrainDataInfo(&tmpCount, &tmpInUseCount, &tmpNotInUseCount, &tmpEmptyEntryCount, &cacheMinNotInUseTime);

        cachedTerrainDataCount += tmpCount;
        cachedTerrainDataInUseCount += tmpInUseCount;
        cachedTerrainDataNotInUseCount += tmpNotInUseCount;
        cachedTerrainDataEmptyEntryCount += tmpEmptyEntryCount;
    }

    // L09-L13
    for (i=0; i<L09_L13_width*L09_L13_height; i++) {
        tmpCount = 0;
        tmpInUseCount = 0;
        tmpNotInUseCount = 0;
        tmpEmptyEntryCount = 0;
        cachedTerrainDataGroup_L09_L13[i].cachedTerrainDataInfo(&tmpCount, &tmpInUseCount, &tmpNotInUseCount, &tmpEmptyEntryCount, &cacheMinNotInUseTime);

        cachedTerrainDataCount += tmpCount;
        cachedTerrainDataInUseCount += tmpInUseCount;
        cachedTerrainDataNotInUseCount += tmpNotInUseCount;
        cachedTerrainDataEmptyEntryCount += tmpEmptyEntryCount;
    }

    (*cTDCount) = cachedTerrainDataCount;
    (*cTDInUseCount) = cachedTerrainDataInUseCount;
    (*cTDNotInUseCount) = cachedTerrainDataNotInUseCount;
    (*cTDEmptyEntryCount) = cachedTerrainDataEmptyEntryCount;
    (*cMinNotInUseTime) = cacheMinNotInUseTime;
}

void CCacheManager::cacheClear(CEarth *earth)
{
    int L00_L03_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L00_L03_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L04_L08_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L04_L08_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L09_L13_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int L09_L13_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int i;

    // L00-L03
    for (i=0; i<L00_L03_width*L00_L03_height; i++) {
        cachedTerrainDataGroup_L00_L03[i].deleteNotInUse(earth, 30*3600*1000);
    }

    // L04-L08
    for (i=0; i<L04_L08_width*L04_L08_height; i++) {
        cachedTerrainDataGroup_L04_L08[i].deleteNotInUse(earth, 30*3600*1000);
    }

    // L09-L13
    for (i=0; i<L09_L13_width*L09_L13_height; i++) {
        cachedTerrainDataGroup_L09_L13[i].deleteNotInUse(earth, 30*3600*1000);
    }
}

void CCacheManager::cacheKeepSize(CEarth *earth)
{
    int L00_L03_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L00_L03_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L04_L08_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L04_L08_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L09_L13_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int L09_L13_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int i;

    if (cacheMinNotInUseTime>24*3600*1000)
        return;

    if (cachedTerrainDataNotInUseCount>CACHE_MAX_UNUSED_TERRAIN_DATA) {
        // L00-L03
        for (i=0; i<L00_L03_width*L00_L03_height; i++) {
            cachedTerrainDataGroup_L00_L03[i].deleteNotInUse(earth, cacheMinNotInUseTime + 5000);
        }

        // L04-L08
        for (i=0; i<L04_L08_width*L04_L08_height; i++) {
            cachedTerrainDataGroup_L04_L08[i].deleteNotInUse(earth, cacheMinNotInUseTime + 5000);
        }

        // L09-L13
        for (i=0; i<L09_L13_width*L09_L13_height; i++) {
            cachedTerrainDataGroup_L09_L13[i].deleteNotInUse(earth, cacheMinNotInUseTime + 5000);
        }
    }
}

void CCacheManager::setupCachedTerrainDataTables()
{
    int L00_L03_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L00_L03_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L04_L08_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L04_L08_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L09_L13_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int L09_L13_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);

    cachedTerrainDataGroup_L00_L03 = new CCachedTerrainDataGroup[L00_L03_width * L00_L03_height];
    cachedTerrainDataGroup_L04_L08 = new CCachedTerrainDataGroup[L04_L08_width * L04_L08_height];
    cachedTerrainDataGroup_L09_L13 = new CCachedTerrainDataGroup[L09_L13_width * L09_L13_height];
}

void CCacheManager::setupAvabilityTables()
{
    int i, index;
    double tlLon, tlLat;
    QDir dir;
    QFileInfo fileInfo;
    QFileInfoList list;
    int L00_L03_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L00_L03_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L00_L03);
    int L04_L08_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L04_L08_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L04_L08);
    int L09_L13_width  = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int L09_L13_height = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_L09_L13);
    int SRTM_width     = (int)(360.0 / HGT_SOURCE_DEGREE_SIZE_SRTM);
    int SRTM_height    = (int)(180.0 / HGT_SOURCE_DEGREE_SIZE_SRTM);

    avability_L00_L03 = new CAvability[L00_L03_width * L00_L03_height];
    avability_L04_L08 = new CAvability[L04_L08_width * L04_L08_height];
    avability_L09_L13 = new CAvability[L09_L13_width * L09_L13_height];
    avability_SRTM    = new CAvability[SRTM_width * SRTM_height];

    // L00-L03 levels
    dir.setPath(pathL00_L03);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();

    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==8450 && fileInfo.suffix()=="hgt") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, HGT_SOURCE_DEGREE_SIZE_L00_L03, &index);
            avability_L00_L03[index].setAvailable(fileInfo.fileName());
        }
    }

    // L04-L08 levels
    dir.setPath(pathL04_L08);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();

    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==526338 && fileInfo.suffix()=="hgt") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, HGT_SOURCE_DEGREE_SIZE_L04_L08, &index);
            avability_L04_L08[index].setAvailable(fileInfo.fileName());
        }
    }

    // L09-L13 levels
    dir.setPath(pathL09_L13);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();

    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==33570818 && fileInfo.suffix()=="hgt") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, HGT_SOURCE_DEGREE_SIZE_L09_L13, &index);
            avability_L09_L13[index].setAvailable(fileInfo.fileName());
        }
    }

    // SRTM files
    dir.setPath(pathSRTM);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();

    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==2884802 && fileInfo.suffix()=="hgt") {
            CCommons::convertSRTMfileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, HGT_SOURCE_DEGREE_SIZE_SRTM, &index);
            avability_SRTM[index].setAvailable(fileInfo.fileName());
        }
    }
}

void CCacheManager::setupTextureAvalibityTables()
{
    int i, index;
    double tlLon, tlLat;
    QDir dir;
    QFileInfo fileInfo;
    QFileInfoList list;
    int TEX_width   = (int)(360.0 / TEX_DEGREE_SIZE);
    int TEX_height  = (int)(180.0 / TEX_DEGREE_SIZE);

    avabilityTex_L00_L02 = new CAvability[TEX_width * TEX_height];
    avabilityTex_L03_L05 = new CAvability[TEX_width * TEX_height];
    avabilityTex_L06_L08 = new CAvability[TEX_width * TEX_height];
    avabilityTex_L09_L10 = new CAvability[TEX_width * TEX_height];

    // L00-L02 levels
    dir.setPath(pathTexL00_L02);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();
    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==27648 && fileInfo.suffix()=="raw") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, TEX_DEGREE_SIZE, &index);
            avabilityTex_L00_L02[index].setAvailable(fileInfo.fileName());
        }
    }

    // L03-L05 levels
    dir.setPath(pathTexL03_L05);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();
    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==1769472 && fileInfo.suffix()=="raw") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, TEX_DEGREE_SIZE, &index);
            avabilityTex_L03_L05[index].setAvailable(fileInfo.fileName());
        }
    }

    // L06-L08 levels
    dir.setPath(pathTexL06_L08);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();
    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==113246208 && fileInfo.suffix()=="raw") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, TEX_DEGREE_SIZE, &index);
            avabilityTex_L06_L08[index].setAvailable(fileInfo.fileName());
        }
    }

    // L09-L10 levels
    dir.setPath(pathTexL09_L10);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    list = dir.entryInfoList();
    for (i=0; i<list.size(); i++) {
        fileInfo = list.at(i);
        if (fileInfo.size()==1811939328 && fileInfo.suffix()=="raw") {
            CCommons::convertFileNameToLonLat(fileInfo.fileName(), &tlLon, &tlLat);
            CCommons::convertTopLeft2AvabilityIndex(tlLon, tlLat, TEX_DEGREE_SIZE, &index);
            avabilityTex_L09_L10[index].setAvailable(fileInfo.fileName());
        }
    }
}















