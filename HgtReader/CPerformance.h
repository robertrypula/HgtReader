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

#ifndef CPERFORMANCE_H
#define CPERFORMANCE_H

#include <QObject>
#include <QMutex>

class CPerformance : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void SIGNALupdateFrameRenderingInfo(int terrainsQuarterDrawed, double fps);
    void SIGNALupdateTerrainTreeUpdatingInfo(int terrainsInTree, int maxLOD, double tups);

public:
    CPerformance();
    ~CPerformance();

    int terrainsInTree;
    int terrainsQuarterDrawed;
    int maxLOD;

    static CPerformance *getInstance();
    void updateFrameRenderingInfo();
    void updateTerrainTreeUpdatingInfo();
    void setFrameRenderingTime(int fms);
    void setTerrainTreeUpdatingTime(int tums);
    void addEventToHistory(QString evName);
    void resetHistory();
    void disableSavingToHistory();
    void enableSavingToHistory();

private:
    static CPerformance *instance;

    QMutex mutex;
    double fps;
    double tups;
    double *fpsHistory;
    double *tupsHistory;
    double *fpsGlobalTimeHistory;
    double *tupsGlobalTimeHistory;
    int fpsSavedMeasurements;
    int tupsSavedMeasurements;
    double fpsGlobalTime;
    double tupsGlobalTime;
    QString *eventsName;
    double *eventsTime;
    int eventsCount;
    bool saveToHistory;

    void saveLog();
};

#endif // CPERFORMANCE_H
