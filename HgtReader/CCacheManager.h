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

#ifndef CCACHEMANAGER_H
#define CCACHEMANAGER_H

#include <QString>
#include <QTime>
#include "CEarth.h"
#include "CCachedTerrainDataGroup.h"
#include "CAvability.h"
#include "CRawFile.h"

#define HGT_SOURCE_L00_L03                 0
#define HGT_SOURCE_L04_L08                 1
#define HGT_SOURCE_L09_L13                 2
#define HGT_SOURCE_SRTM                   10
#define HGT_SOURCE_SIZE_L00_L03           65
#define HGT_SOURCE_SIZE_L04_L08          513
#define HGT_SOURCE_SIZE_L09_L13         4097
#define HGT_SOURCE_SIZE_SRTM            1201
#define HGT_SOURCE_DEGREE_SIZE_L00_L03    60.00
#define HGT_SOURCE_DEGREE_SIZE_L04_L08    15.00
#define HGT_SOURCE_DEGREE_SIZE_L09_L13     3.75
#define HGT_SOURCE_DEGREE_SIZE_SRTM        1.00
#define HGT_DONT_USE_DISK_HEIGHT         300
#define TEX_SOURCE_MAX_LOD                10
#define TEX_SOURCE_L00_L02                 0
#define TEX_SOURCE_L03_L05                 1
#define TEX_SOURCE_L06_L08                 2
#define TEX_SOURCE_L09_L10                 3
#define TEX_SOURCE_PX_SIZE_L00_L02        96
#define TEX_SOURCE_PX_SIZE_L03_L05       768
#define TEX_SOURCE_PX_SIZE_L06_L08      6144
#define TEX_SOURCE_PX_SIZE_L09_L10     24576
#define TEX_DEGREE_SIZE                   45.00
#define TEX_EMPTY_COLOR             0xEEFFEE
#define TEX_TERRAIN_SIZE                  32
#define CACHE_MAX_UNUSED_TERRAIN_DATA  50000

class CCacheManager
{
public:
    CCacheManager();
    ~CCacheManager();
    static CCacheManager *getInstance();

    QString pathBase;
    QString pathL00_L03;
    QString pathL04_L08;
    QString pathL09_L13;
    QString pathSRTM;
    QString pathTexL00_L02;
    QString pathTexL03_L05;
    QString pathTexL06_L08;
    QString pathTexL09_L10;
    QString pathL00_L03_index;
    QString pathL04_L08_index;
    QString pathL09_L13_index;
    QString pathSRTM_index;
    CAvability *avability_L00_L03;       // tile size = 60.00 deg
    CAvability *avability_L04_L08;       // tile size = 15.00 deg
    CAvability *avability_L09_L13;       // tile size =  3.75 deg
    CAvability *avability_SRTM;          // tile size =  1.00 deg
    CAvability *avabilityTex_L00_L02;
    CAvability *avabilityTex_L03_L05;
    CAvability *avabilityTex_L06_L08;
    CAvability *avabilityTex_L09_L10;
    char *stripIndexListNW;
    char *stripIndexListNE;
    char *stripIndexListSW;
    char *stripIndexListSE;
    CEarth *earthBufferA;
    CEarth *earthBufferB;
    double LODdegreeSizeLookUp[14];
    int HGTsourceLookUp[14];
    double HGTsourceDegreeSizeLookUp[14];
    int HGTsourceSizeLookUp[14];
    int HGTsourceSkippingLookUp[14];
    int TEXsourceLookUp[14];
    int TEXsourcePxSizeLookUp[14];
    int TEXsourceSkippingLookUp[14];
    CCachedTerrainDataGroup *cachedTerrainDataGroup_L00_L03;      // cached terrain data database
    CCachedTerrainDataGroup *cachedTerrainDataGroup_L04_L08;      // cached terrain data database
    CCachedTerrainDataGroup *cachedTerrainDataGroup_L09_L13;      // cached terrain data database
    QTime cacheTime;

    void getTerrainPoints(double lon, double lat, int lod,
                          int *points, int *pointNW, int *pointNE, int *pointSW, int *pointSE,
                          int *pointsN, int *pointsE, int *pointsS, int *pointsW, unsigned char *texture,
                          bool dontUseDiskHgt, bool dontUseDiskRaw);
    void setEarthBuffers(CEarth *eBuffA, CEarth *eBuffB);
    bool cacheTerrainDataFind(const double lon, const double lat, const int lod, const CEarth *earth, CTerrainData **terrainData);
    void cacheTerrainDataRegister(const CEarth *earth, CTerrainData **terrainData);
    void cacheTerrainDataFree(const CEarth *earth, CTerrainData **terrainData, const bool &dontSaveJustDelete);
    void cacheInfo(int *cTDCount, int *cTDInUseCount, int *cTDNotInUseCount, int *cTDEmptyEntryCount, unsigned int *cMinNotInUseTime);
    void cacheClear(CEarth *earth);
    void cacheKeepSize(CEarth *earth);

public:
    static CCacheManager *instance;
    int cachedTerrainDataCount;
    int cachedTerrainDataInUseCount;
    int cachedTerrainDataNotInUseCount;
    int cachedTerrainDataEmptyEntryCount;
    unsigned int cacheMinNotInUseTime;

    bool findRawFiles(const double &tlLon, const double &tlLat, const int &lod, int *RAWfilesIndex, int *pixOffsetLon, int *pixOffsetLat);
    void buildTextureFromRawFiles(const double &tlLon, const double &tlLat, const int &lod, CRawFile *terrainTexture);
    void findHgtFileName(const double &lon, const double &lat, const int &lod, QString *filePath, bool *fileFound, int *x, int *y, int *hgtSkipping, int *hgtSize);
    void setupAvabilityTables();
    void setupCachedTerrainDataTables();
    void setupTextureAvalibityTables();
    void setupStripIndex();
};

#endif // CCACHEMANAGER_H
