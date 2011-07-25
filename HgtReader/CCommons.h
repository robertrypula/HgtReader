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

#ifndef CCOMMONS_H
#define CCOMMONS_H

#include <QString>

#define CONST_EARTH_RADIUS           6378100
#define CONST_EARTH_CIRCUMFERENCE    40074784.208
#define CONST_SUN_RADIUS             696000000.0
#define CONST_SUN_DISTANCE           149600000000.0       // 149.6 milions km
#define CONST_SUN_MAX_LAT            23.45
#define CONST_PI                     3.1415926535897932384626433832795
#define CONST_PIDIV180               0.01745329251994329576923690768489
#define CONST_180DIVPI               57.295779513082320876798154814105
#define CONST_1KM                    1000.0
#define CONST_1GM                    1000000.0
#define CONST_DEF_WIDTH              800
#define CONST_DEF_HEIGHT             600
#define ANIMATION_SPEED_MS           15.0
#define ANIMATION_SPEED_SEK          ANIMATION_SPEED_MS/1000.0
#define ANIMATION_EP_DURATION_MS     1500.0
#define ANIMATION_EP_ALT             10000000.0
#define CAM_FOV                      70.0

class CCommons
{
public:
    CCommons();

    static void getCartesianFromSpherical(const double &azlon, const double &ellat, const double &radalt, double *x, double *y, double *z);
    static void getSphericalFromCartesian(const double &x, const double &y, const double &z, double *azlon, double *ellat, double *radalt);
    static double getAngleFromCartesian(const double &x, const double &y);
    static void findTopLeftCorner(const double &lon, const double &lat, const double &degreeSize, double *tlLon, double *tlLat);
    static void findTopLeftCornerOfHgtFile(const double &lon, const double &lat, const int &lod, double *tlLon, double *tlLat);
    static void findXYInHgtFile(const double &tlLon, const double &tlLat, const double &lon, const double &lat, const int &lod, int *x, int *y);
    static void convertTopLeft2AvabilityIndex(const double &tlLon, const double &tlLat, const double &degreeSize, int *index);
    static void convertAvabilityIndex2TopLeft(const int &index, const double &degreeSize, double *tlLon, double *tlLat);
    static void convertSRTMfileNameToLonLat(const QString &name, double *lon, double *lat);
    static void convertLonLatToSRTMfileName(const double &lon, const double &lat, QString *name);
    static void convertFileNameToLonLat(const QString &name, double *lon, double *lat);
    static void convertLonLatToFileName(const double &lon, const double &lat, QString *name);
    static void convertLonLatToCartesian(const double &lon, const double &lat, double *lonX, double *latY);
    static void convertCartesianToLonLat(const double &lonX, const double &latY, double *lon, double *lat);
    static int getNeighborAvabilityIndex(const int &baseIndex, const double &degreeSize, const int &dx, const int &dy);
};

#endif // CCOMMONS_H
