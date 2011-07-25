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

#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include "CPerformance.h"
#include "CCacheManager.h"

CPerformance *CPerformance::instance;

CPerformance::CPerformance()
{
    // something like singleton :)
    instance = this;

    terrainsInTree = 0;
    terrainsQuarterDrawed = 0;
    maxLOD = 0;
    fps = 0.0;
    tups = 0.0;
    fpsGlobalTime = 0.0;
    tupsGlobalTime = 0.0;

    // create history arrays
    fpsHistory = new double[3600];            // at least one hour buffer
    tupsHistory = new double[3600];           // at least one hour buffer
    fpsGlobalTimeHistory = new double[3600];
    tupsGlobalTimeHistory = new double[3600];
    fpsSavedMeasurements = 0;
    tupsSavedMeasurements = 0;
    eventsName = new QString[300];
    eventsTime = new double[300];
    eventsCount = 0;

    saveToHistory = true;
}

CPerformance::~CPerformance()
{
    saveLog();

    delete []fpsGlobalTimeHistory;
    delete []tupsGlobalTimeHistory;
    delete []fpsHistory;
    delete []tupsHistory;
    delete []eventsName;
    delete []eventsTime;

    instance = 0;
}

void CPerformance::resetHistory()
{
    QMutexLocker locker(&mutex);
    eventsCount = 0;
    fpsSavedMeasurements = 0;
    tupsSavedMeasurements = 0;
    fpsGlobalTime = 0.0;
    tupsGlobalTime = 0.0;
}

void CPerformance::disableSavingToHistory()
{
    QMutexLocker locker(&mutex);
    saveToHistory = false;
}

void CPerformance::enableSavingToHistory()
{
    QMutexLocker locker(&mutex);
    saveToHistory = true;
}

void CPerformance::saveLog()
{
    QLocale locale;
    CCacheManager *cacheManager = CCacheManager::getInstance();
    QFile file(cacheManager->pathBase + "log.txt");
    int i;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "Events:" << endl;
    for (i=0; i<eventsCount; i++) {
        out << locale.toString(eventsTime[i], 'f', 3) << "     - " << eventsName[i] << endl;
    }
    out << endl;
    out << "--------------------------------------------------------" << endl << endl;
    out << "FPS history:" << endl;
    for (i=0; i<fpsSavedMeasurements; i++) {
        out << locale.toString(fpsGlobalTimeHistory[i], 'f', 3) << ";" << locale.toString(fpsHistory[i], 'f', 1) << endl;
    }
    out << endl;
    out << "--------------------------------------------------------" << endl << endl;
    out << "TUPS history:" << endl;
    for (i=0; i<tupsSavedMeasurements; i++) {
        out << locale.toString(tupsGlobalTimeHistory[i], 'f', 3) << ";" << locale.toString(tupsHistory[i], 'f', 1) << endl;
    }
    file.close();
}

CPerformance *CPerformance::getInstance()
{
    return instance;
}

void CPerformance::updateFrameRenderingInfo()
{
    emit SIGNALupdateFrameRenderingInfo(terrainsQuarterDrawed, fps);
}

void CPerformance::updateTerrainTreeUpdatingInfo()
{
    emit SIGNALupdateTerrainTreeUpdatingInfo(terrainsInTree, maxLOD, tups);
}

void CPerformance::addEventToHistory(QString evName)
{
    QMutexLocker locker(&mutex);

    if (saveToHistory && eventsCount<300) {
        eventsName[eventsCount] = evName;
        eventsTime[eventsCount] = fpsGlobalTime;
        eventsCount++;
    }
}

void CPerformance::setFrameRenderingTime(int fms)
{
    QMutexLocker locker(&mutex);

    fps = 1000.0 / (double)(fms);
    if (saveToHistory && fpsSavedMeasurements<3600) {
        fpsHistory[fpsSavedMeasurements] = fps;
        fpsGlobalTimeHistory[fpsSavedMeasurements] = fpsGlobalTime;
        fpsGlobalTime = fpsGlobalTime + 1.0/fps;
        fpsSavedMeasurements++;
    }
}

void CPerformance::setTerrainTreeUpdatingTime(int tums)
{
    QMutexLocker locker(&mutex);

    tups = 1000.0 / (double)(tums);
    if (saveToHistory && tupsSavedMeasurements<3600) {
        tupsHistory[tupsSavedMeasurements] = tups;
        tupsGlobalTimeHistory[tupsSavedMeasurements] = tupsGlobalTime;
        tupsGlobalTime = tupsGlobalTime + 1.0/tups;
        tupsSavedMeasurements++;
    }
}
