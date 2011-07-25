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

#include <QVector3D>
#include <QMatrix4x4>
#include <math.h>
#include "CCommons.h"
#include "CCamera.h"


CCamera::CCamera()
{
    double unit = 1.0;

    camLinkage = CAM_LINKAGE_GLOBE;
    camMode = CAM_MODE_ORBIT;

    // INIT - earth point (center of terrain camera coordinate system)
    earthPointLon = 17.038;
    earthPointLat = 51.102;
    earthPointAlt = CONST_EARTH_RADIUS + 118;
    CCommons::getCartesianFromSpherical(earthPointLon, earthPointLat, earthPointAlt,
                                        &earthPointX, &earthPointY, &earthPointZ);

    // INIT - camera all modes settings
    camPix2AngleX = 1.0;         // [deg/pix] must be re-set when window dimension is setup !!!
    camPix2AngleY = 1.0;         // [deg/pix] must be re-set when window dimension is setup !!!
    camVel = 1.0;                // [m/sec] must be re-set when cam altitude is setup !!!
    camVelFromAlt = true;

    // INIT - camera linked to globe center in orbit mode
    camGlobeOrbitAzim = earthPointLon;
    camGlobeOrbitElev = earthPointLat;
    camGlobeOrbitRad = CONST_EARTH_RADIUS + 20.0*CONST_1GM;     // 20 000 km above Earth default camera
    CCommons::getCartesianFromSpherical(camGlobeOrbitAzim, camGlobeOrbitElev, camGlobeOrbitRad,
                                        &camGlobeX, &camGlobeY, &camGlobeZ);

    // INIT - camera linked to globe center in freelook mode
    camGlobeFreeAzim = camGlobeOrbitAzim + 180.0;
    camGlobeFreeElev = -camGlobeOrbitElev;
    CCommons::getCartesianFromSpherical(camGlobeFreeAzim, camGlobeFreeElev, unit,
                                        &camGlobeFreeDirX, &camGlobeFreeDirY, &camGlobeFreeDirZ);

    // INIT - camera linked to terrain coords (earth point) in orbit mode
    camTerrainOrbitAzim = 20.0;
    camTerrainOrbitElev = 30.0;
    camTerrainOrbitRad = 20*CONST_1KM;      // 20 km altitude default
    CCommons::getCartesianFromSpherical(camTerrainOrbitAzim, camTerrainOrbitElev, camTerrainOrbitRad,
                                        &camTerrainX, &camTerrainY, &camTerrainZ);

    // INIT - camera linked to terrain coords (earth point) in freelook mode
    camTerrainFreeAzim = camTerrainOrbitAzim + 180.0;
    camTerrainFreeElev = -camTerrainOrbitElev;
    CCommons::getCartesianFromSpherical(camTerrainFreeAzim, camTerrainFreeElev, unit,
                                        &camTerrainFreeDirX, &camTerrainFreeDirY, &camTerrainFreeDirZ);

    // INIT - sun
    sunLon = 320.0; sunLat = CONST_SUN_MAX_LAT;
    updateSunVectors();
    sunMovingMode = false;


    // set interactive state
    interactMouseLeftButton = false;
    interactMouseRightButton = false;
    interactKeyDownW = false;
    interactKeyDownS = false;
    interactKeyDownA = false;
    interactKeyDownD = false;
    interactKeyDownX = false;
    interactKeyDownZ = false;

    // set force resize flags (for scene z-buffer settings)
    camZBufferRecalculated1000km = false;
    camZBufferRecalculated1000km100km = false;
    camZBufferRecalculated100km10km = false;
    camZBufferRecalculated10km1km = false;
    camZBufferRecalculated1km = false;

    // camera FOV & terrain clipping
    camFOV = 70.0;
    setNewWindowSize(CONST_DEF_WIDTH, CONST_DEF_HEIGHT, false);

    updateCameraWhenInGlobeLinkage();
}

CCamera::~CCamera()
{
}

void CCamera::setDrawingStateMutex(QMutex *m)
{
    drawingStateMutex = m;
}

void CCamera::setCamClippingAngle()
{
    double aspectRatio = (double)windowWidth/(double)windowHeight;

    camClippingAngle = (camFOV * aspectRatio * 1.1) / 2.0;
    camClippingAngleCosine = cos(CONST_PIDIV180 * camClippingAngle);
}

void CCamera::setNewWindowSize(int width, int height, bool withMutex)
{
    if (withMutex)                               // for constructor call - version without mutex :]
        QMutexLocker locker(drawingStateMutex);

    // same for all
    camPix2AngleX = (360.0/(double)width);
    camPix2AngleY = (180.0/(double)height);
    windowWidth = width;
    windowHeight = height;

    setCamClippingAngle();
}

void CCamera::switchToGlobalOrbitMode()
{
    QMutexLocker locker(drawingStateMutex);
    camLinkage = CAM_LINKAGE_GLOBE;
    camMode = CAM_MODE_ORBIT;
    updateCameraWhenInGlobeLinkage();
}

void CCamera::switchToGlobalFreeMode()
{
    QMutexLocker locker(drawingStateMutex);
    double unit = 1.0;
    camLinkage = CAM_LINKAGE_GLOBE;
    camMode = CAM_MODE_FREELOOK;
    // update direction vector (look_at)
    CCommons::getCartesianFromSpherical(camGlobeFreeAzim, camGlobeFreeElev, unit,
                                        &camGlobeFreeDirX, &camGlobeFreeDirY, &camGlobeFreeDirZ);
    updateCameraWhenInGlobeLinkage();
}

void CCamera::switchToTerrainOrbitMode()
{
    QMutexLocker locker(drawingStateMutex);
    camLinkage = CAM_LINKAGE_TERRAIN;
    camMode = CAM_MODE_ORBIT;
    updateCameraWhenInTerrainLinkage();
}

void CCamera::switchToTerrainFreeMode()
{
    QMutexLocker locker(drawingStateMutex);
    double unit = 1.0;
    camLinkage = CAM_LINKAGE_TERRAIN;
    camMode = CAM_MODE_FREELOOK;
    // update direction vector (look_at)
    CCommons::getCartesianFromSpherical(camTerrainFreeAzim, camTerrainFreeElev, unit,
                                        &camTerrainFreeDirX, &camTerrainFreeDirY, &camTerrainFreeDirZ);
    updateCameraWhenInTerrainLinkage();
}

void CCamera::updateCameraZBuffer()
{
    bool sendSignal = false;

    if (camAltGround<=1.0*CONST_1KM) {
        if (!camZBufferRecalculated1km) {
            sendSignal = true;
            camZBufferRecalculated1000km = false;
            camZBufferRecalculated1000km100km = false;
            camZBufferRecalculated100km10km = false;
            camZBufferRecalculated10km1km = false;
            camZBufferRecalculated1km = true;
        }
    } else
        if (camAltGround<=10.0*CONST_1KM && camAltGround>1.0*CONST_1KM) {
            if (!camZBufferRecalculated10km1km) {
                sendSignal = true;
                camZBufferRecalculated1000km = false;
                camZBufferRecalculated1000km100km = false;
                camZBufferRecalculated100km10km = false;
                camZBufferRecalculated10km1km = true;
                camZBufferRecalculated1km = false;
            }
        } else
            if (camAltGround<=100.0*CONST_1KM && camAltGround>10.0*CONST_1KM) {
                if (!camZBufferRecalculated100km10km) {
                    sendSignal = true;
                    camZBufferRecalculated1000km = false;
                    camZBufferRecalculated1000km100km = false;
                    camZBufferRecalculated100km10km = true;
                    camZBufferRecalculated10km1km = false;
                    camZBufferRecalculated1km = false;
                }
            } else
                if (camAltGround<=1000.0*CONST_1KM && camAltGround>100.0*CONST_1KM) {
                    if (!camZBufferRecalculated1000km100km) {
                        sendSignal = true;
                        camZBufferRecalculated1000km = false;
                        camZBufferRecalculated1000km100km = true;
                        camZBufferRecalculated100km10km = false;
                        camZBufferRecalculated10km1km = false;
                        camZBufferRecalculated1km = false;
                    }
                } else
                    if (camAltGround>1000.0*CONST_1KM) {
                        if (!camZBufferRecalculated1000km) {
                            sendSignal = true;
                            camZBufferRecalculated1000km = true;
                            camZBufferRecalculated1000km100km = false;
                            camZBufferRecalculated100km10km = false;
                            camZBufferRecalculated10km1km = false;
                            camZBufferRecalculated1km = false;
                        }
                    }

    if (sendSignal)
        emit SIGNALforceResize();
}

void CCamera::updateCameraWhenInGlobeLinkage()
{
    QVector3D earthPoint;

    // camera location in global coordinate system
    camLon = camGlobeOrbitAzim;
    camLat = camGlobeOrbitElev;
    camAlt = camGlobeOrbitRad;
    camX = camGlobeX;
    camY = camGlobeY;
    camZ = camGlobeZ;

    // get camera position vector (QVector3D)
    camPosition.setX(camX);
    camPosition.setY(camY);
    camPosition.setZ(camZ);

    camPerspectiveX = camGlobeX;
    camPerspectiveY = camGlobeY;
    camPerspectiveZ = camGlobeZ;
    if (camMode==CAM_MODE_ORBIT) {
        camPerspectiveLookAtX = 0.0;
        camPerspectiveLookAtY = 0.0;
        camPerspectiveLookAtZ = 0.0;
    } else {
        camPerspectiveLookAtX = camPerspectiveX + camGlobeFreeDirX*1000000.0;
        camPerspectiveLookAtY = camPerspectiveY + camGlobeFreeDirY*1000000.0;
        camPerspectiveLookAtZ = camPerspectiveZ + camGlobeFreeDirZ*1000000.0;
    }

    // get looking direction vector
    camLookingDirectionNormal.setX(camPerspectiveLookAtX - camGlobeX);
    camLookingDirectionNormal.setY(camPerspectiveLookAtY - camGlobeY);
    camLookingDirectionNormal.setZ(camPerspectiveLookAtZ - camGlobeZ);
    camLookingDirectionNormal.normalize();

    // distance to earth point
    earthPoint.setX(earthPointX); earthPoint.setY(earthPointY); earthPoint.setZ(earthPointZ);
    camDistanceToEarthPoint = (camPosition-earthPoint).length();

    // camera altidute relative to earth surface
    camAltGround = camAlt - CONST_EARTH_RADIUS;

    // camera velocity in free look (depends on altitude)
    if (camVelFromAlt) {
        camVel = camAltGround/10.0;
        if (camVel<0.0) camVel *= -1.0;
    }

    emit SIGNALupdateFovAndCamVel(camFOV, camVel);
    emit SIGNALupdateCameraInfo(camLon, camLat, camAltGround, camDistanceToEarthPoint);
    updateCameraZBuffer();
}

void CCamera::updateCameraWhenInTerrainLinkage()
{
    QVector3D cam, earthPoint;

    // get earth point & camera position [QVector3D]
    earthPoint.setX(earthPointX); earthPoint.setY(earthPointY); earthPoint.setZ(earthPointZ);
    cam.setX(camTerrainX); cam.setY(camTerrainY); cam.setZ(camTerrainZ);

    // camera is in terrain coordinate system - we need global xyz
    convertTerrainVectorToGlobalVector(&cam);
    camX = cam.x();               // real coords
    camY = cam.y();               // real coords
    camZ = cam.z();               // real coords
    CCommons::getSphericalFromCartesian(camX, camY, camZ, &camLon, &camLat, &camAlt);  // // real geo coords

    // get real camera position vector (QVector3D)
    camPosition.setX(camX);
    camPosition.setY(camY);
    camPosition.setZ(camZ);

    camPerspectiveX = camTerrainX;
    camPerspectiveY = camTerrainY;
    camPerspectiveZ = camTerrainZ;
    if (camMode==CAM_MODE_ORBIT) {
        camPerspectiveLookAtX = 0.0;
        camPerspectiveLookAtY = 0.0;
        camPerspectiveLookAtZ = 0.0;
    } else {
        camPerspectiveLookAtX = camPerspectiveX + camTerrainFreeDirX*1000000.0;
        camPerspectiveLookAtY = camPerspectiveY + camTerrainFreeDirY*1000000.0;
        camPerspectiveLookAtZ = camPerspectiveZ + camTerrainFreeDirZ*1000000.0;
    }                                                            // ^^^^^^^^^^ to fix precision bug when
                                                                 //            converting camLookingDirectionNormal
    // get looking direction vector
    camLookingDirectionNormal.setX(camPerspectiveLookAtX);
    camLookingDirectionNormal.setY(camPerspectiveLookAtY);
    camLookingDirectionNormal.setZ(camPerspectiveLookAtZ);
    convertTerrainVectorToGlobalVector(&camLookingDirectionNormal);
    camLookingDirectionNormal = camLookingDirectionNormal - camPosition;
    camLookingDirectionNormal.normalize();

    // distance to earth point
    camDistanceToEarthPoint = (cam-earthPoint).length();

    // camera altidute relative to earth surface
    camAltGround = camAlt - CONST_EARTH_RADIUS;

    // camera velocity in free look (depends on altitude)
    if (camVelFromAlt) {
        camVel = camAltGround/10.0;
        if (camVel<0.0) camVel *= -1.0;
    }

    emit SIGNALupdateFovAndCamVel(camFOV, camVel);
    emit SIGNALupdateCameraInfo(camLon, camLat, camAltGround, camDistanceToEarthPoint);
    updateCameraZBuffer();
}

void CCamera::convertTerrainVectorToGlobalVector(QVector3D *vec)
{
    QMatrix4x4 transform;
    QVector3D earthPoint;

    earthPoint.setX(earthPointX); earthPoint.setY(earthPointY); earthPoint.setZ(earthPointZ);
    transform.rotate(earthPointLat-90.0, 1.0, 0.0, 0.0);
    (*vec) = (*vec) * transform;
    transform.setToIdentity();
    transform.rotate(-earthPointLon, 0.0, 1.0, 0.0);
    (*vec) = (*vec) * transform;
    (*vec) += earthPoint;
}

void CCamera::convertGlobalVectorToTerrainVector(QVector3D *vec)
{
    QMatrix4x4 transform;
    QVector3D earthPoint;

    earthPoint.setX(earthPointX); earthPoint.setY(earthPointY); earthPoint.setZ(earthPointZ);
    (*vec) -= earthPoint;
    transform.rotate(earthPointLon, 0.0, 1.0, 0.0);
    (*vec) = (*vec) * transform;
    transform.setToIdentity();
    transform.rotate(-earthPointLat+90.0, 1.0, 0.0, 0.0);
    (*vec) = (*vec) * transform;
}

void CCamera::getSunLonLatAzimElev(double *slon, double *slat, double *sazim, double *selev)
{
    QMutexLocker locker(drawingStateMutex);
    (*slon) = sunLon;
    (*slat) = sunLat;
    (*sazim) = sunAzim;
    (*selev) = sunElev;
}

void CCamera::getEarthPointLonLatAlt(double *lon, double *lat, double *alt)
{
    QMutexLocker locker(drawingStateMutex);
    (*lon) = earthPointLon;
    (*lat) = earthPointLat;
    (*alt) = earthPointAlt;
}

void CCamera::getEarthPointXYZ(double *x, double *y, double *z)
{
    QMutexLocker locker(drawingStateMutex);
    (*x) = earthPointX;
    (*y) = earthPointY;
    (*z) = earthPointZ;
}


void CCamera::setEarthPointXYZ(const double &x, const double &y, const double &z, const bool &fromAnimation)
{
    QMutexLocker locker(drawingStateMutex);
    earthPointX = x;
    earthPointY = y;
    earthPointX = z;
    CCommons::getSphericalFromCartesian(x, y, z, &earthPointLon, &earthPointLat, &earthPointAlt);

    switch (camLinkage)
    {
        case CAM_LINKAGE_GLOBE:  updateCameraWhenInGlobeLinkage();
                                 break;
        case CAM_LINKAGE_TERRAIN:updateCameraWhenInTerrainLinkage();
                                 break;
    }
    updateSunVectors();
    emit SIGNALupdateSunInfo(sunLon, sunLat, sunAzim, sunElev);
    emit SIGNALupdateEarthPointInfo(earthPointLon, earthPointLat, earthPointAlt-CONST_EARTH_RADIUS, fromAnimation ? true : false);
}

void CCamera::setEarthPointLonLatAlt(const double &lon, const double &lat, const double &alt, const bool &fromAnimation)
{
    QMutexLocker locker(drawingStateMutex);
    earthPointLon = lon;
    earthPointLat = lat;
    earthPointAlt = alt;
    CCommons::getCartesianFromSpherical(lon, lat, alt, &earthPointX, &earthPointY, &earthPointZ);

    switch (camLinkage)
    {
        case CAM_LINKAGE_GLOBE:  updateCameraWhenInGlobeLinkage();
                                 break;
        case CAM_LINKAGE_TERRAIN:updateCameraWhenInTerrainLinkage();
                                 break;
    }
    updateSunVectors();
    emit SIGNALupdateSunInfo(sunLon, sunLat, sunAzim, sunElev);
    emit SIGNALupdateEarthPointInfo(earthPointLon, earthPointLat, earthPointAlt-CONST_EARTH_RADIUS, fromAnimation ? true : false);
}

void CCamera::SLOTcamVelFromAltChanged(int state)
{
    QMutexLocker locker(drawingStateMutex);
    if (state==Qt::Checked) camVelFromAlt = true;
    if (state==Qt::Unchecked) camVelFromAlt = false;
}

void CCamera::mousePressEventHandler(QMouseEvent *event)
{
    QMutexLocker locker(drawingStateMutex);
    interactMouseLastPos = event->pos();
    if (event->buttons() & Qt::LeftButton) {
        interactMouseLeftButton = true;
    }
    if (event->buttons() & Qt::RightButton) {
        interactMouseRightButton = true;
    }
}

void CCamera::mouseReleaseEventHandler(QMouseEvent *event)
{
    QMutexLocker locker(drawingStateMutex);
    if (!(event->buttons() & Qt::LeftButton)) {
        interactMouseLeftButton = false;
    }
    if (!(event->buttons() & Qt::RightButton)) {
        interactMouseRightButton = false;
    }
}

void CCamera::mouseMoveEventHandler(QMouseEvent *event)
{
    QMutexLocker locker(drawingStateMutex);
    int dx = event->x() - interactMouseLastPos.x();
    int dy = event->y() - interactMouseLastPos.y();

    double unit = 1.0;
    double orbitSlowing = 1.0;

    if (!sunMovingMode) {

        if (camMode==CAM_MODE_FREELOOK) {

            // free look camera
            if (interactMouseLeftButton && !interactMouseRightButton) {
                switch (camLinkage) {
                    case CAM_LINKAGE_GLOBE:     camGlobeFreeAzim -= dx*camPix2AngleX;
                                                camGlobeFreeElev -= dy*camPix2AngleY;

                                                if (camGlobeFreeElev>89.5) camGlobeFreeElev = 89.5;
                                                if (camGlobeFreeElev<-89.5) camGlobeFreeElev = -89.5;
                                                if (camGlobeFreeAzim>360.0) camGlobeFreeAzim -= 360.0;
                                                if (camGlobeFreeAzim<0.0) camGlobeFreeAzim += 360.0;

                                                // update direction vector (look_at)
                                                CCommons::getCartesianFromSpherical(camGlobeFreeAzim, camGlobeFreeElev, unit,
                                                                                    &camGlobeFreeDirX, &camGlobeFreeDirY, &camGlobeFreeDirZ);
                                                updateCameraWhenInGlobeLinkage();
                                                break;

                    case CAM_LINKAGE_TERRAIN:   camTerrainFreeAzim -= dx*camPix2AngleX;
                                                camTerrainFreeElev -= dy*camPix2AngleY;

                                                if (camTerrainFreeElev>89.5) camTerrainFreeElev = 89.5;
                                                if (camTerrainFreeElev<-89.5) camTerrainFreeElev = -89.5;
                                                if (camTerrainFreeAzim>360.0) camTerrainFreeAzim -= 360.0;
                                                if (camTerrainFreeAzim<0.0) camTerrainFreeAzim += 360.0;

                                                // update direction vector (look_at)
                                                CCommons::getCartesianFromSpherical(camTerrainFreeAzim, camTerrainFreeElev, unit,
                                                                                    &camTerrainFreeDirX, &camTerrainFreeDirY, &camTerrainFreeDirZ);
                                                updateCameraWhenInTerrainLinkage();
                                                break;
                }
            }

        } else {

            // orbit camera
            if (interactMouseLeftButton ^ interactMouseRightButton) {

                // change azim/elev
                if (interactMouseLeftButton) {
                    switch(camLinkage) {
                        case CAM_LINKAGE_GLOBE:     if (camAltGround<(CONST_1GM*3.0)) {
                                                        orbitSlowing = sin((CONST_PI/2.0)*(camAltGround/(CONST_1GM*3.0)));  // slow down orbiting when close to earth
                                                    }

                                                    camGlobeOrbitAzim -= dx*camPix2AngleX*orbitSlowing;
                                                    camGlobeOrbitElev += dy*camPix2AngleY*orbitSlowing;

                                                    if (camGlobeOrbitElev>89.5) camGlobeOrbitElev = 89.5;
                                                    if (camGlobeOrbitElev<-89.5) camGlobeOrbitElev = -89.5;
                                                    if (camGlobeOrbitAzim>360.0) camGlobeOrbitAzim -= 360.0;
                                                    if (camGlobeOrbitAzim<0.0) camGlobeOrbitAzim += 360.0;

                                                    camGlobeFreeAzim = camGlobeOrbitAzim + 180.0; // to look at center of coordinate system in freelook mode
                                                    camGlobeFreeElev = -camGlobeOrbitElev;        // to look at center of coordinate system in freelook mode

                                                    // update position vector
                                                    CCommons::getCartesianFromSpherical(camGlobeOrbitAzim, camGlobeOrbitElev, camGlobeOrbitRad,
                                                                                        &camGlobeX, &camGlobeY, &camGlobeZ);
                                                    updateCameraWhenInGlobeLinkage();
                                                    break;

                        case CAM_LINKAGE_TERRAIN:   camTerrainOrbitAzim -= dx*camPix2AngleX;
                                                    camTerrainOrbitElev += dy*camPix2AngleY;

                                                    if (camTerrainOrbitElev>89.5) camTerrainOrbitElev = 89.5;
                                                    if (camTerrainOrbitElev<-89.5) camTerrainOrbitElev = -89.5;
                                                    if (camTerrainOrbitAzim>360.0) camTerrainOrbitAzim -= 360.0;
                                                    if (camTerrainOrbitAzim<0.0) camTerrainOrbitAzim += 360.0;

                                                    camTerrainFreeAzim = camTerrainOrbitAzim + 180.0; // to look at center of coordinate system in freelook mode
                                                    camTerrainFreeElev = -camTerrainOrbitElev;        // to look at center of coordinate system in freelook mode

                                                    // update position vector
                                                    CCommons::getCartesianFromSpherical(camTerrainOrbitAzim, camTerrainOrbitElev, camTerrainOrbitRad,
                                                                                        &camTerrainX, &camTerrainY, &camTerrainZ);
                                                    updateCameraWhenInTerrainLinkage();
                                                    break;
                    }
                }

                // zooming
                if (interactMouseRightButton) {
                    switch(camLinkage) {
                        case CAM_LINKAGE_GLOBE:     if (dy<0) camGlobeOrbitRad = (camGlobeOrbitRad-CONST_EARTH_RADIUS)*0.95 + CONST_EARTH_RADIUS;
                                                    if (dy>0) camGlobeOrbitRad = (camGlobeOrbitRad-CONST_EARTH_RADIUS)*1.05 + CONST_EARTH_RADIUS;

                                                    // block camera position > 1 milion km
                                                    if (camGlobeOrbitRad>1000.0*CONST_1GM) camGlobeOrbitRad = 1000.0*CONST_1GM;

                                                    CCommons::getCartesianFromSpherical(camGlobeOrbitAzim, camGlobeOrbitElev, camGlobeOrbitRad,
                                                                                        &camGlobeX, &camGlobeY, &camGlobeZ);
                                                    updateCameraWhenInGlobeLinkage();
                                                    break;

                        case CAM_LINKAGE_TERRAIN:   if (dy<0) camTerrainOrbitRad *= 0.95;
                                                    if (dy>0) camTerrainOrbitRad *= 1.05;

                                                    // block camera position > 1 milion km
                                                    if (camTerrainOrbitRad>1000.0*CONST_1GM) camTerrainOrbitRad = 1000.0*CONST_1GM;

                                                    CCommons::getCartesianFromSpherical(camTerrainOrbitAzim, camTerrainOrbitElev, camTerrainOrbitRad,
                                                                                        &camTerrainX, &camTerrainY, &camTerrainZ);
                                                    updateCameraWhenInTerrainLinkage();
                                                    break;
                    }
                }

            }
        }

    } else {

        if (interactMouseLeftButton ^ interactMouseRightButton)
            if (interactMouseLeftButton) {
                sunLon -= dx*camPix2AngleX;
                sunLat += dy*camPix2AngleY;

                if (sunLat>CONST_SUN_MAX_LAT) sunLat = CONST_SUN_MAX_LAT;
                if (sunLat<-CONST_SUN_MAX_LAT) sunLat = -CONST_SUN_MAX_LAT;
                if (sunLon>360.0) sunLon -= 360.0;
                if (sunLon<0.0) sunLon += 360.0;

                // update sun vector
                updateSunVectors();
                emit SIGNALupdateSunInfo(sunLon, sunLat, sunAzim, sunElev);
            }

    }

    interactMouseLastPos = event->pos();
}

void CCamera::updateSunVectors()
{
    double tmp;
    double sunX, sunY, sunZ;
    CCommons::getCartesianFromSpherical(sunLon, sunLat, CONST_SUN_DISTANCE,
                                        &sunX, &sunY, &sunZ);
    sunPositionGlobe.setX(sunX);
    sunPositionGlobe.setY(sunY);
    sunPositionGlobe.setZ(sunZ);
    sunLightVector = sunPositionGlobe;
    sunLightVector.normalize();
    sunLightVector = sunLightVector;
    sunPositionTerrain = sunPositionGlobe;
    convertGlobalVectorToTerrainVector(&sunPositionTerrain);
    CCommons::getSphericalFromCartesian(sunPositionTerrain.x(), sunPositionTerrain.y(), sunPositionTerrain.z(),
                                        &sunAzim, &sunElev, &tmp);
}

bool CCamera::keyPressEventHandler(QKeyEvent *event)
{
    drawingStateMutex->lock();
    bool catched = false;

    if (event->key() == Qt::Key_F1) {
        drawingStateMutex->unlock();
        switchToGlobalOrbitMode();     // in function mutex is locked too
        drawingStateMutex->lock();
        emit SIGNALupdateCameraInteractMode(1);
        catched = true;
    } else
    if (event->key() == Qt::Key_F2) {
        drawingStateMutex->unlock();
        switchToGlobalFreeMode();      // in function mutex is locked too
        drawingStateMutex->lock();
        emit SIGNALupdateCameraInteractMode(2);
        catched = true;
    } else
    if (event->key() == Qt::Key_F3) {
        drawingStateMutex->unlock();
        switchToTerrainOrbitMode();    // in function mutex is locked too
        drawingStateMutex->lock();
        emit SIGNALupdateCameraInteractMode(3);
        catched = true;
    } else
    if (event->key() == Qt::Key_F4) {
        drawingStateMutex->unlock();
        switchToTerrainFreeMode();    // in function mutex is locked too
        drawingStateMutex->lock();
        emit SIGNALupdateCameraInteractMode(4);
        catched = true;
    } else
    if (event->key() == Qt::Key_F5) {
        sunMovingMode = sunMovingMode ? false : true;
        emit SIGNALupdateSunInteractMode(sunMovingMode);
        catched = true;
    } else
    if (event->key() == Qt::Key_W) {
        interactKeyDownW = true;
        catched = true;
    } else
    if (event->key() == Qt::Key_S) {
        interactKeyDownS = true;
        catched = true;
    } else
    if (event->key() == Qt::Key_D) {
        interactKeyDownD = true;
        catched = true;
    } else
    if (event->key() == Qt::Key_A) {
        interactKeyDownA = true;
        catched = true;
    }
    if (event->key() == Qt::Key_Z) {
        interactKeyDownZ = true;
        catched = true;
    }
    if (event->key() == Qt::Key_X) {
        interactKeyDownX = true;
        catched = true;
    }

    drawingStateMutex->unlock();
    return catched;
}

bool CCamera::keyReleaseEventHandler(QKeyEvent *event)
{
    QMutexLocker locker(drawingStateMutex);
    bool catched = false;

    if (event->key() == Qt::Key_W) {
        interactKeyDownW = false;
        catched = true;
    } else
    if (event->key() == Qt::Key_S) {
        interactKeyDownS = false;
        catched = true;
    } else
    if (event->key() == Qt::Key_D) {
        interactKeyDownD = false;
        catched = true;
    } else
    if (event->key() == Qt::Key_A) {
        interactKeyDownA = false;
        catched = true;
    }
    if (event->key() == Qt::Key_Z) {
        interactKeyDownZ = false;
        catched = true;
    }
    if (event->key() == Qt::Key_X) {
        interactKeyDownX = false;
        catched = true;
    }

    return catched;
}

void CCamera::SLOTgetNewEarthPoint()
{
    drawingStateMutex->lock();

    double tmpSunLon, tmpSunLat, tmpSunAzim, tmpSunElev;
    double tmpEarthPointLon, tmpEarthPointLat, tmpEarthPointAltGround;

    earthPointLon = camLon;
    earthPointLat = camLat;
    earthPointAlt = camAlt;
    earthPointX = camX;
    earthPointY = camY;
    earthPointZ = camZ;

    switch (camLinkage)
    {
        case CAM_LINKAGE_GLOBE:  updateCameraWhenInGlobeLinkage();
                                 break;
        case CAM_LINKAGE_TERRAIN:updateCameraWhenInTerrainLinkage();
                                 break;
    }
    updateSunVectors();

    // copy variables for emit signal (unlock mutex BEFORE emit signal)
    tmpSunLon = sunLon;
    tmpSunLat = sunLat;
    tmpSunAzim = sunAzim;
    tmpSunElev = sunElev;
    tmpEarthPointLon = earthPointLon;
    tmpEarthPointLat = earthPointLat;
    tmpEarthPointAltGround = earthPointAlt-CONST_EARTH_RADIUS;

    drawingStateMutex->unlock();

    emit SIGNALupdateSunInfo(tmpSunLon, tmpSunLat, tmpSunAzim, tmpSunElev);
    emit SIGNALupdateEarthPointInfo(tmpEarthPointLon, tmpEarthPointLat, tmpEarthPointAltGround, false);
}

void CCamera::SLOTearthPointSelectCurrentIndexChanged(int index)
{
    if (index==0) return;

    double currEarthPointLon, currEarthPointLat, currEarthPointAlt;
    double animEarthPointLon, animEarthPointLat, animEarthPointAlt;

    drawingStateMutex->lock();
    index--;                 // first on list in select is empty
    currEarthPointLon = earthPointLon;
    currEarthPointLat = earthPointLat;
    currEarthPointAlt = earthPointAlt;
    animEarthPointLon = earthPointsList.earthPoints.at(index).lon;
    animEarthPointLat = earthPointsList.earthPoints.at(index).lat;
    animEarthPointAlt = earthPointsList.earthPoints.at(index).alt;
    drawingStateMutex->unlock();

    emit SIGNALanimateToEarthPoint(currEarthPointLon, currEarthPointLat, currEarthPointAlt,
                                   animEarthPointLon, animEarthPointLat, animEarthPointAlt);
}

void CCamera::earthPointsListAdd(QString name)
{
    drawingStateMutex->lock();
    int index;
    index = earthPointsList.addAndSort(name,
                                       earthPointLon, earthPointLat, earthPointAlt,
                                       earthPointX, earthPointY, earthPointZ);
    drawingStateMutex->unlock();
    emit SIGNALreloadEarthPointSelect(index+1);
}

void CCamera::checkInteractKeys()
{
    QMutexLocker locker(drawingStateMutex);
    bool buttonDown = false;

    if (interactKeyDownZ) {
        camFOV -= 25.0 * ANIMATION_SPEED_SEK;
    }
    if (interactKeyDownX)
        camFOV += 25.0 * ANIMATION_SPEED_SEK;

    if (interactKeyDownX || interactKeyDownZ) {
        if (camFOV>170.0) camFOV = 170.0;
        if (camFOV<5.0) camFOV = 5.0;
        setCamClippingAngle();
        emit SIGNALforceResize();
        emit SIGNALupdateFovAndCamVel(camFOV, camVel);
    }


    if (camMode!=CAM_MODE_FREELOOK)
        return;

    if (camLinkage==CAM_LINKAGE_GLOBE) {
        if (interactKeyDownW) {
            freeLookGlobalCameraForward(ANIMATION_SPEED_SEK);
            buttonDown = true;
        }
        if (interactKeyDownS) {
            freeLookGlobalCameraBackward(ANIMATION_SPEED_SEK);
            buttonDown = true;
        }
        if (interactKeyDownA) {
            freeLookGlobalCameraLeft(ANIMATION_SPEED_SEK);
            buttonDown = true;
        }
        if (interactKeyDownD) {
            freeLookGlobalCameraRight(ANIMATION_SPEED_SEK);
            buttonDown = true;
        }
        if (buttonDown) {
            CCommons::getSphericalFromCartesian(camGlobeX, camGlobeY, camGlobeZ, &camGlobeOrbitAzim, &camGlobeOrbitElev, &camGlobeOrbitRad);
            updateCameraWhenInGlobeLinkage();
        }
    } else
        if (camLinkage==CAM_LINKAGE_TERRAIN) {
            if (interactKeyDownW) {
                freeLookTerrainCameraForward(ANIMATION_SPEED_SEK);
                buttonDown = true;
            }
            if (interactKeyDownS) {
                freeLookTerrainCameraBackward(ANIMATION_SPEED_SEK);
                buttonDown = true;
            }
            if (interactKeyDownA) {
                freeLookTerrainCameraLeft(ANIMATION_SPEED_SEK);
                buttonDown = true;
            }
            if (interactKeyDownD) {
                freeLookTerrainCameraRight(ANIMATION_SPEED_SEK);
                buttonDown = true;
            }

            if (buttonDown) {
                CCommons::getSphericalFromCartesian(camTerrainX, camTerrainY, camTerrainZ, &camTerrainOrbitAzim, &camTerrainOrbitElev, &camTerrainOrbitRad);
                updateCameraWhenInTerrainLinkage();
            }
        }
}

void CCamera::freeLookGlobalCameraForward(double dt)
{
    camGlobeX = camGlobeX + camGlobeFreeDirX*camVel*dt;
    camGlobeY = camGlobeY + camGlobeFreeDirY*camVel*dt;
    camGlobeZ = camGlobeZ + camGlobeFreeDirZ*camVel*dt;
}

void CCamera::freeLookGlobalCameraBackward(double dt)
{
    camGlobeX = camGlobeX - camGlobeFreeDirX*camVel*dt;
    camGlobeY = camGlobeY - camGlobeFreeDirY*camVel*dt;
    camGlobeZ = camGlobeZ - camGlobeFreeDirZ*camVel*dt;
}

void CCamera::freeLookGlobalCameraRight(double dt)
{
    QVector3D upVec, camDirVec, rightVec;

    upVec.setX(0.0);
    upVec.setY(1.0);
    upVec.setZ(0.0);
    camDirVec.setX(camGlobeFreeDirX);
    camDirVec.setY(camGlobeFreeDirY);
    camDirVec.setZ(camGlobeFreeDirZ);

    rightVec = QVector3D::crossProduct(camDirVec, upVec);
    rightVec.normalize();

    camGlobeX = camGlobeX + rightVec.x()*camVel*dt;
    camGlobeY = camGlobeY + rightVec.y()*camVel*dt;
    camGlobeZ = camGlobeZ + rightVec.z()*camVel*dt;
}

void CCamera::freeLookGlobalCameraLeft(double dt)
{
    QVector3D upVec, camDirVec, rightVec;

    upVec.setX(0.0);
    upVec.setY(1.0);
    upVec.setZ(0.0);
    camDirVec.setX(camGlobeFreeDirX);
    camDirVec.setY(camGlobeFreeDirY);
    camDirVec.setZ(camGlobeFreeDirZ);

    rightVec = QVector3D::crossProduct(camDirVec, upVec);
    rightVec.normalize();

    camGlobeX = camGlobeX - rightVec.x()*camVel*dt;
    camGlobeY = camGlobeY - rightVec.y()*camVel*dt;
    camGlobeZ = camGlobeZ - rightVec.z()*camVel*dt;
}

void CCamera::freeLookTerrainCameraForward(double dt)
{
    camTerrainX = camTerrainX + camTerrainFreeDirX*camVel*dt;
    camTerrainY = camTerrainY + camTerrainFreeDirY*camVel*dt;
    camTerrainZ = camTerrainZ + camTerrainFreeDirZ*camVel*dt;
}

void CCamera::freeLookTerrainCameraBackward(double dt)
{
    camTerrainX = camTerrainX - camTerrainFreeDirX*camVel*dt;
    camTerrainY = camTerrainY - camTerrainFreeDirY*camVel*dt;
    camTerrainZ = camTerrainZ - camTerrainFreeDirZ*camVel*dt;
}

void CCamera::freeLookTerrainCameraRight(double dt)
{
    QVector3D upVec, camDirVec, rightVec;

    upVec.setX(0.0);
    upVec.setY(1.0);
    upVec.setZ(0.0);
    camDirVec.setX(camTerrainFreeDirX);
    camDirVec.setY(camTerrainFreeDirY);
    camDirVec.setZ(camTerrainFreeDirZ);

    rightVec = QVector3D::crossProduct(camDirVec, upVec);
    rightVec.normalize();

    camTerrainX = camTerrainX + rightVec.x()*camVel*dt;
    camTerrainY = camTerrainY + rightVec.y()*camVel*dt;
    camTerrainZ = camTerrainZ + rightVec.z()*camVel*dt;
}

void CCamera::freeLookTerrainCameraLeft(double dt)
{
    QVector3D upVec, camDirVec, rightVec;

    upVec.setX(0.0);
    upVec.setY(1.0);
    upVec.setZ(0.0);
    camDirVec.setX(camTerrainFreeDirX);
    camDirVec.setY(camTerrainFreeDirY);
    camDirVec.setZ(camTerrainFreeDirZ);

    rightVec = QVector3D::crossProduct(camDirVec, upVec);
    rightVec.normalize();

    camTerrainX = camTerrainX - rightVec.x()*camVel*dt;
    camTerrainY = camTerrainY - rightVec.y()*camVel*dt;
    camTerrainZ = camTerrainZ - rightVec.z()*camVel*dt;
}

CEarthPointsList CCamera::getEarthPointsList()
{
    QMutexLocker locker(drawingStateMutex);
    CEarthPointsList epl = earthPointsList;
    return epl;
}
