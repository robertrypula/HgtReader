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

#ifndef CCAMERA_H
#define CCAMERA_H

#define CAM_LINKAGE_GLOBE      0
#define CAM_LINKAGE_TERRAIN    1
#define CAM_MODE_ORBIT         0
#define CAM_MODE_FREELOOK      1

#include <QPoint>
#include <QObject>
#include <QVector3D>
#include <QMutex>
#include <QMouseEvent>
#include <QTimer>
#include <QKeyEvent>
#include "CEarthPointsList.h"


class CCamera : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void SIGNALupdateCameraInfo(double camLon, double camLat, double camAltGround,
                                double camDistanceToEarthPoint);                         // thread safe (drawingStateMutex)
    void SIGNALupdateEarthPointInfo(double earthPointLon, double earthPointLat,
                                    double earthPointAltGround, bool fromSelect);        // thread safe (drawingStateMutex)
    void SIGNALupdateSunInfo(double sunLon, double sunLat,
                             double sunAzim, double sunElev);                            // thread safe (emit in mouseMoveHandler)
    void SIGNALforceResize();
    void SIGNALreloadEarthPointSelect(int indexToSelect);                                // thread safe (drawingStateMutex)
    void SIGNALupdateFovAndCamVel(double fov, double vel);                               // thread safe (drawingStateMutex)
    void SIGNALupdateCameraInteractMode(int interactState);
    void SIGNALupdateSunInteractMode(bool sunMoving);
    void SIGNALanimateToEarthPoint(double currEarthPointLon, double currEarthPointLat,
                                   double currEarthPointAlt, double animEarthPointLon,
                                   double animEarthPointLat, double animEarthPointAlt);  // thread safe (drawingStateMutex)

public:
    CCamera();
    ~CCamera();
    friend class CDrawingState;          // for full access from CDrawingState class

    // public functions
    void setDrawingStateMutex(QMutex *m);
    void getEarthPointXYZ(double *x, double *y, double *z);                               // thread safe (drawingStateMutex)
    void getEarthPointLonLatAlt(double *lon, double *lat, double *alt);                   // thread safe (drawingStateMutex)
    void setEarthPointXYZ(const double &x, const double &y,
                          const double &z, const bool &fromAnimation = false);            // thread safe (drawingStateMutex)
    void setEarthPointLonLatAlt(const double &lon, const double &lat,
                                const double &alt, const bool &fromAnimation = false);    // thread safe (drawingStateMutex)
    void getSunLonLatAzimElev(double *slon, double *slat, double *sazim, double *selev);  // thread safe (drawingStateMutex)
    void mousePressEventHandler(QMouseEvent *event);                                      // thread safe (drawingStateMutex)
    void mouseReleaseEventHandler(QMouseEvent *event);                                    // thread safe (drawingStateMutex)
    void mouseMoveEventHandler(QMouseEvent *event);                                       // thread safe (drawingStateMutex)
    bool keyPressEventHandler(QKeyEvent *event);                                          // thread safe (drawingStateMutex)
    bool keyReleaseEventHandler(QKeyEvent *event);                                        // thread safe (drawingStateMutex)
    void switchToGlobalOrbitMode();                                                       // thread safe (drawingStateMutex)
    void switchToGlobalFreeMode();                                                        // thread safe (drawingStateMutex)
    void switchToTerrainOrbitMode();                                                      // thread safe (drawingStateMutex)
    void switchToTerrainFreeMode();                                                       // thread safe (drawingStateMutex)
    void earthPointsListAdd(QString name);                                                // thread safe (drawingStateMutex)
    CEarthPointsList getEarthPointsList();                                                // thread safe (drawingStateMutex)
    void setNewWindowSize(int width, int height, bool withMutex = true);                  // thread safe (drawingStateMutex)
    void checkInteractKeys();                                                             // thread safe (drawingStateMutex)

public slots:
    void SLOTcamVelFromAltChanged(int state);                                             // thread safe (drawingStateMutex)
    void SLOTgetNewEarthPoint();                                                          // thread safe (drawingStateMutex)
    void SLOTearthPointSelectCurrentIndexChanged(int index);                              // thread safe (drawingStateMutex)

private:
    char camLinkage;                     // camera linkage
    double camFOV;                       // camera field of view
    double camClippingAngleCosine;       // terrain clipping angle cosine
    QVector3D camPosition;               // [QVector3D] real camera position in global coordinate system vector
    QVector3D camLookingDirectionNormal; // [QVector3D] real camera direction vector in global coordinate system (normalized)
    double camAltGround;                 //             real camera alt (to ground)
    double camDistanceToEarthPoint;      //             real camera distance to earth point
    double camPerspectiveX;              // [gluLookAt] camera X pos
    double camPerspectiveY;              // [gluLookAt] camera Y pos
    double camPerspectiveZ;              // [gluLookAt] camera Z pos
    double camPerspectiveLookAtX;        // [gluLookAt] camera look at X
    double camPerspectiveLookAtY;        // [gluLookAt] camera look at Y
    double camPerspectiveLookAtZ;        // [gluLookAt] camera look at Z
    double earthPointLon;                // point on Earth longitude
    double earthPointLat;                // point on Earth latitude
    double earthPointAlt;                // point on Earth altitude
    double earthPointX;                  // point on Earth X position
    double earthPointY;                  // point on Earth Y position
    double earthPointZ;                  // point on Earth Z position
    QMutex *drawingStateMutex;
    char camMode;                     // camera mode
    double camClippingAngle;          // terrain clipping angle
    double camPix2AngleX;             // [all modes] camera velocity deg/pix X
    double camPix2AngleY;             // [all modes] camera velocity deg/pix Y
    double camVel;                    // [all modes] camera moving velocity m/sek
    bool camVelFromAlt;               // [all modes] camera velocity from ground alt flag
    double camGlobeX;                 // [globe linked] camera X position
    double camGlobeY;                 // [globe linked] camera Y position
    double camGlobeZ;                 // [globe linked] camera Z position
    double camGlobeOrbitAzim;         // [globe linked] camera azimuth in mode 0 (orbit)
    double camGlobeOrbitElev;         // [globe linked] camera elevation in mode 0 (orbit)
    double camGlobeOrbitRad;          // [globe linked] camera radius in mode 0 (orbit)
    double camGlobeFreeAzim;          // [globe linked] camera azimuth in mode 1 (freelook)
    double camGlobeFreeElev;          // [globe linked] camera elevation in mode 1 (freelook)
    double camGlobeFreeDirX;          // [globe linked] camera direction vector X (freelook)
    double camGlobeFreeDirY;          // [globe linked] camera direction vector Y (freelook)
    double camGlobeFreeDirZ;          // [globe linked] camera direction vector Z (freelook)
    double camTerrainX;               // [terrain linked] camera X position
    double camTerrainY;               // [terrain linked] camera Y position
    double camTerrainZ;               // [terrain linked] camera Z position
    double camTerrainOrbitAzim;       // [terrain linked] camera azimuth in mode 0 (orbit)
    double camTerrainOrbitElev;       // [terrain linked] camera elevation in mode 0 (orbit)
    double camTerrainOrbitRad;        // [terrain linked] camera radius in mode 0 (orbit)
    double camTerrainFreeAzim;        // [terrain linked] camera azimuth in mode 1 (freelook)
    double camTerrainFreeElev;        // [terrain linked] camera elevation in mode 1 (freelook)
    double camTerrainFreeDirX;        // [terrain linked] camera direction vector X (freelook)
    double camTerrainFreeDirY;        // [terrain linked] camera direction vector Y (freelook)
    double camTerrainFreeDirZ;        // [terrain linked] camera direction vector Z (freelook)
    double camLon;                    // real camera lon in global coordinate system
    double camLat;                    // real camera lat in global coordinate system
    double camAlt;                    // real camera alt in global coordinate system
    double camX;                      // real camera X in global coordinate system
    double camY;                      // real camera Y in global coordinate system
    double camZ;                      // real camera Z in global coordinate system
    QVector3D sunPositionGlobe;       // Sun position in global coordinate system
    QVector3D sunPositionTerrain;     // Sun position in terrain coordinate system
    QVector3D sunLightVector;         // Sun light vector (normalized)
    double sunLon;                    // Sun longitude
    double sunLat;                    // Sun latidute
    double sunAzim;                   // Sun azimuth in earth point location
    double sunElev;                   // Sun elevation in earth point location
    bool sunMovingMode;               // Sun moveing mode flag
    bool camZBufferRecalculated1000km;
    bool camZBufferRecalculated1000km100km;
    bool camZBufferRecalculated100km10km;
    bool camZBufferRecalculated10km1km;
    bool camZBufferRecalculated1km;
    QPoint interactMouseLastPos;
    bool interactMouseLeftButton;
    bool interactMouseRightButton;
    bool interactKeyDownW;
    bool interactKeyDownS;
    bool interactKeyDownA;
    bool interactKeyDownD;
    bool interactKeyDownZ;
    bool interactKeyDownX;
    CEarthPointsList earthPointsList;    // list of location on Earth
    int windowWidth;
    int windowHeight;

    void setCamClippingAngle();
    void convertTerrainVectorToGlobalVector(QVector3D *vec);
    void convertGlobalVectorToTerrainVector(QVector3D *vec);
    void updateCameraWhenInGlobeLinkage();
    void updateCameraWhenInTerrainLinkage();
    void updateCameraZBuffer();
    void updateSunVectors();
    void freeLookGlobalCameraForward(double dt);
    void freeLookGlobalCameraBackward(double dt);
    void freeLookGlobalCameraLeft(double dt);
    void freeLookGlobalCameraRight(double dt);
    void freeLookTerrainCameraForward(double dt);
    void freeLookTerrainCameraBackward(double dt);
    void freeLookTerrainCameraLeft(double dt);
    void freeLookTerrainCameraRight(double dt);
};

#endif // CCAMERA_H
