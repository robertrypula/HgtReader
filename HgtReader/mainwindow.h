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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "COpenGl.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void SLOTupdateCameraInfo(double camLon, double camLat, double camAltGround, double camDistanceToEarthPoint);
    void SLOTupdateSunInfo(double sunLon, double sunLat, double sunAzim, double sunElev);
    void SLOTupdateFovAndCamVelInfo(double fov, double vel);
    void SLOTupdateEarthPointInfo(double earthPointLon, double earthPointLat, double earthPointAltGround, bool newEarthPoint);
    void SLOTreloadEarthPointsSelect(int indexToSelect);
    void SLOTearthPointAddButtonClicked();
    void SLOTupdateFrameRenderingInfo(int terrainsQuarterDrawed, double fps);
    void SLOTupdateTerrainTreeUpdatingInfo(int terrainsInTree, int maxLOD, double tups);
    void SLOTupdateCameraInteractMode(int interactState);
    void SLOTupdateSunInteractMode(bool sunMoving);
    void SLOTupdateCacheInfo(int cachedTDCount, int cachedTDInUseCount, int cachedTDNotInUseCount, int cachedTDEmptyEntryCount, unsigned int cacheMinNotInUseTime);

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    COpenGl *openGl;

private slots:
    void SLOTaboutButtonClicked();
    void SLOTkeyMapButtonClicked();
};

#endif // MAINWINDOW_H
