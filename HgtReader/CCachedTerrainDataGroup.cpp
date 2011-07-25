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
#include <QTime>
#include "CCachedTerrainDataGroup.h"
#include "CCacheManager.h"

#define CACHE_SHOW_DEBUG_INFO   false

CCachedTerrainDataGroup::CCachedTerrainDataGroup()
{
}

void CCachedTerrainDataGroup::deleteNotInUse(CEarth *earth, unsigned int olderThan)
{
    QList<CCachedTerrainData>::iterator i;

    for (i=cachedTerrainDataList.begin(); i!=cachedTerrainDataList.end(); i++) {
        if (!i->terrainAinUse && !i->terrainBinUse && i->time<olderThan) {
            if (i->terrainData!=0) {

                // add textureID to removeFromVRAM list
                if (earth!=0 && i->terrainData->getTextureID()!=0) {
                    earth->textureIDListToRemoveFromVRAM.append( i->terrainData->getTextureID() );
                }

                delete i->terrainData;
                i->terrainData = 0;
            }
            //cachedTerrainDataList.erase(i);
            //continue;
        }
        //i++;
    }
}

bool CCachedTerrainDataGroup::cachedTerrainDataListFind(const double tlLon, const double tlLat, const int lod, const CEarth *earth, CTerrainData **terrainData)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    QList<CCachedTerrainData>::iterator i;
    bool found = false;
    int match;

    // check integrity
    if (earth!=cacheManager->earthBufferA && earth!=cacheManager->earthBufferB) {
        qFatal("FIND - earth pointer != earthA or earthB");
    }

    // search existing entry
    for (i=cachedTerrainDataList.begin(); i!=cachedTerrainDataList.end(); i++) {
        match = 0;
        if (i->terrainData==0) {
            if (CACHE_SHOW_DEBUG_INFO) qDebug("FIND - empty cache entry");
            continue;
        } else {
            if (i->terrainData->topLeftLon == tlLon) match++;
            if (i->terrainData->topLeftLat == tlLat) match++;
            if (i->terrainData->LOD == lod) match++;
            if (match==3) {
                found = true;
                break;
            }
        }
    }

    // register pointer
    if (found) {
        if (earth==cacheManager->earthBufferA)
            i->terrainAinUse = true; else
            i->terrainBinUse = true;
        (*terrainData) = i->terrainData;
        i->time = cacheManager->cacheTime.elapsed();
    } else {
        (*terrainData) = 0;
    }

    return found;
}

void CCachedTerrainDataGroup::cachedTerrainDataListRegister(const CEarth *earth, CTerrainData **terrainData)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    QList<CCachedTerrainData>::iterator i;
    CCachedTerrainData cTDNew;
    bool found = false;
    int match;

    // check integrity
    if (earth!=cacheManager->earthBufferA && earth!=cacheManager->earthBufferB) {
        qFatal("REGISTER - earth pointer != earthA or earthB");
    }

    // search existing entry
    for (i=cachedTerrainDataList.begin(); i!=cachedTerrainDataList.end(); i++) {
        match = 0;
        if (i->terrainData==0) {
            if (CACHE_SHOW_DEBUG_INFO) qDebug("REGISTER - empty cache entry");
            continue;
        } else {
            if (i->terrainData->topLeftLon == (*terrainData)->topLeftLon) match++;
            if (i->terrainData->topLeftLat == (*terrainData)->topLeftLat) match++;
            if (i->terrainData->LOD == (*terrainData)->LOD) match++;
            if (match==3) {
                found = true;
                break;
            }
        }
    }

    // register pointer
    if (found) {
        if (CACHE_SHOW_DEBUG_INFO) qDebug("REGISTER - found existing TerrainData when register new");
        if ((*terrainData)==i->terrainData)
            qFatal("REGISTER - double register same terrain data");
        if (earth==cacheManager->earthBufferA)
            i->terrainAinUse = true; else
            i->terrainBinUse = true;
        delete (*terrainData);
        (*terrainData) = i->terrainData;
        i->time = cacheManager->cacheTime.elapsed();
    } else {
        if (earth==cacheManager->earthBufferA)
            cTDNew.terrainAinUse = true; else
            cTDNew.terrainBinUse = true;
        cTDNew.terrainData = (*terrainData);
        cTDNew.time = cacheManager->cacheTime.elapsed();
        cachedTerrainDataList.append(cTDNew);
    }
}

void CCachedTerrainDataGroup::cachedTerrainDataListFree(const CEarth *earth, CTerrainData **terrainData, const bool &dontSaveJustDelete)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    QList<CCachedTerrainData>::iterator i;
    bool found = false;
    int match;

    // check integrity
    if (earth!=cacheManager->earthBufferA && earth!=cacheManager->earthBufferB) {
        qFatal("FREE - earth pointer != earthA or earthB");
    }

    // search existing entry
    for (i=cachedTerrainDataList.begin(); i!=cachedTerrainDataList.end(); i++) {
        match = 0;
        if (i->terrainData==0) {
            if (CACHE_SHOW_DEBUG_INFO) qDebug("FREE - empty cache entry");
            continue;
        } else {
            if (i->terrainData->topLeftLon == (*terrainData)->topLeftLon) match++;
            if (i->terrainData->topLeftLat == (*terrainData)->topLeftLat) match++;
            if (i->terrainData->LOD == (*terrainData)->LOD) match++;
            if (match==3) {
                found = true;
                break;
            }
        }
    }

    // store TerrainData in cache
    if (found) {
        if (earth==cacheManager->earthBufferA)
            i->terrainAinUse = false; else
            i->terrainBinUse = false;
        i->time = cacheManager->cacheTime.elapsed();
    } else {
        if (CACHE_SHOW_DEBUG_INFO) qDebug("FREE - dataTerrain not found but request to free (zombie :] ?)");
        delete (*terrainData);
    }
}

void CCachedTerrainDataGroup::cachedTerrainDataInfo(int *cachedTerrainCount, int *cachedTerrainInUseCount,
                                                    int *cachedTerrainNotInUseCount, int *cachedTerrainEmptyEntryCount,
                                                    unsigned int *cacheMinNotInUseTime)
{
    const CCachedTerrainData *ctd;
    int i;

    (*cachedTerrainCount) = cachedTerrainDataList.size();
    for (i=0; i<cachedTerrainDataList.size(); i++) {
        ctd = &cachedTerrainDataList.at(i);

        if (ctd->terrainData==0)
            (*cachedTerrainEmptyEntryCount)++;

        if (ctd->terrainAinUse || ctd->terrainBinUse)
            (*cachedTerrainInUseCount)++;

        if (!ctd->terrainAinUse && !ctd->terrainBinUse && ctd->terrainData!=0) {
            (*cachedTerrainNotInUseCount)++;
            if (ctd->time<(*cacheMinNotInUseTime))
                (*cacheMinNotInUseTime) = ctd->time;
        }
    }
}
