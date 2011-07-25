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
#include <math.h>
#include "CCommons.h"
#include "CCacheManager.h"


CCommons::CCommons()
{
}

void CCommons::getCartesianFromSpherical(const double &azlon, const double &ellat, const double &radalt, double *x, double *y, double *z)
{
    (*x) = radalt*sin(azlon*CONST_PIDIV180)*cos(ellat*CONST_PIDIV180);
    (*y) = radalt*sin(ellat*CONST_PIDIV180);
    (*z) = radalt*cos(azlon*CONST_PIDIV180)*cos(ellat*CONST_PIDIV180);
}

void CCommons::getSphericalFromCartesian(const double &x, const double &y, const double &z, double *azlon, double *ellat, double *radalt)
{
    QVector3D v;

    v.setX(x); v.setY(y); v.setZ(z);

    (*radalt) = v.length();
    (*ellat) = asin(y/(*radalt))*CONST_180DIVPI;
    (*azlon) = CCommons::getAngleFromCartesian(z, x);
}

double CCommons::getAngleFromCartesian(const double &x, const double &y)
{
    QVector3D vec;
    double len, angle = 0.0;

    vec.setX(x);
    vec.setY(y);
    vec.setZ(0.0);
    len = vec.length();

    if (len<=0.001)
        len = 0.001;

    if (x>=0 && y>=0) {
        angle = asin(y/len) * CONST_180DIVPI;
    } else
        if (x<0 && y>=0) {
            angle = asin(-x/len) * CONST_180DIVPI;
            angle += 90.0;
        } else
            if (x<0 && y<0) {
                angle = asin(-y/len) * CONST_180DIVPI;
                angle += 180.0;
            } else
                if (x>=0 && y<0) {
                    angle = asin(x/len) * CONST_180DIVPI;
                    angle += 270.0;
                }

    return angle;
}

void CCommons::findTopLeftCorner(const double &lon, const double &lat, const double &degreeSize, double *tlLon, double *tlLat)
{
    double lonX, latY;
    double correctedLon;

    correctedLon = lon;
    if (correctedLon>=360.0) correctedLon -= 360.0;
    if (correctedLon<0.0) correctedLon += 360.0;

    convertLonLatToCartesian(correctedLon, lat, &lonX, &latY);       // convert to more friendly coordinate system

    lonX = floor(lonX / degreeSize) * degreeSize;     // find closest top left corner on 'sourceDegreeSize' Earth grid
    latY = floor(latY / degreeSize) * degreeSize;     // find closest top left corner on 'sourceDegreeSize' Earth grid

    convertCartesianToLonLat(lonX, latY, tlLon, tlLat);     // convert back to lon-lat format
}

void CCommons::findTopLeftCornerOfHgtFile(const double &lon, const double &lat, const int &lod, double *tlLon, double *tlLat)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();

    double sourceDegreeSize = cacheManager->HGTsourceDegreeSizeLookUp[lod];    // get deg size of source terrain files belongs to LOD
    findTopLeftCorner(lon, lat, sourceDegreeSize, tlLon, tlLat);
}

void CCommons::findXYInHgtFile(const double &tlLon, const double &tlLat, const double &lon, const double &lat, const int &lod, int *x, int *y)
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    double deltaLon, deltaLat;
    double tlLonInHgt, tlLatInHgt;
    double HGTsourceDegreeSize = cacheManager->HGTsourceDegreeSizeLookUp[lod];
    int    HGTsourceSize       = cacheManager->HGTsourceSizeLookUp[lod];
    double LODdegreeSize       = cacheManager->LODdegreeSizeLookUp[lod];

    // find top left corner on grid on current LOD
    findTopLeftCorner(lon, lat, LODdegreeSize, &tlLonInHgt, &tlLatInHgt);

    deltaLon = tlLonInHgt - tlLon;
    deltaLat = tlLat - tlLatInHgt;

    // find x/y pos in HGT file
    (*x) = (int)( (deltaLon / HGTsourceDegreeSize) * (HGTsourceSize - 1) );
    (*y) = (int)( (deltaLat / HGTsourceDegreeSize) * (HGTsourceSize - 1) );
}

void CCommons::convertTopLeft2AvabilityIndex(const double &tlLon, const double &tlLat, const double &degreeSize, int *index)
{
    double correctedLon;
    double tlLonX, tlLatY;
    int tlLonIndexX, tlLatIndexY;

    correctedLon = tlLon;
    if (correctedLon>=360.0) correctedLon -= 360.0;
    if (correctedLon<0.0) correctedLon += 360.0;
    convertLonLatToCartesian(correctedLon, tlLat, &tlLonX, &tlLatY);       // convert to more friendly coordinate system

    tlLonIndexX = (int)( (tlLonX / degreeSize) + 0.5 );     // find closest top left corner on 'sourceDegreeSize' Earth grid
    tlLatIndexY = (int)( (tlLatY / degreeSize) + 0.5 );     // find closest top left corner on 'sourceDegreeSize' Earth grid

    (*index) = tlLatIndexY * ((int)( (360.0 / degreeSize) + 0.5 )) + tlLonIndexX;
}

void CCommons::convertAvabilityIndex2TopLeft(const int &index, const double &degreeSize, double *tlLon, double *tlLat)
{
    double tlLonX, tlLatY;
    int tmpX, tmpY;

    tmpX = index % ((int)( (360.0 / degreeSize) + 0.5 ));
    tmpY = index / ((int)( (360.0 / degreeSize) + 0.5 ));

    tlLonX = tmpX * degreeSize;
    tlLatY = tmpY * degreeSize;

    convertCartesianToLonLat(tlLonX, tlLatY, tlLon, tlLat);
}

void CCommons::convertSRTMfileNameToLonLat(const QString &name, double *lon, double *lat)
{
    QString tmp;
    tmp = name.right(11).left(7);
    QChar fnLatSide = tmp.at(0);
    QChar fnLonSide = tmp.at(3);
    QString fnLat = tmp.right(6).left(2);
    QString fnLon = tmp.right(3);

    (*lat) = fnLat.toInt();
    (*lon) = fnLon.toInt();

    if (fnLonSide==QChar('W')) {
        (*lon) = 360.0 - (*lon);
    }
    if (fnLatSide==QChar('S')) {
        (*lat) *= -1.0;
    }

    (*lat) = (*lat) + 1.0;          // lat+1.0   -> my mistake, SRTM lonlat in filename is >lower< left corner
                                    //              not upper left... :(
}

void CCommons::convertLonLatToSRTMfileName(const double &lon, const double &lat, QString *name)
{
    double tmpLon, tmpLat;
    QChar fnLonSide, fnLatSide;
    QString fnLon, fnLat;

    if (lon>=180.0) {
        tmpLon = 360.0 - lon;
        fnLonSide = 'W';
    } else {
        tmpLon = lon;
        fnLonSide = 'E';
    }

    if ((lat-1.0)>=0.0) {          // lat-1.0   -> my mistake, SRTM lonlat in filename is >lower< left corner
        tmpLat = (lat-1.0);        //              not upper left... :(
        fnLatSide = 'N';
    } else {
        tmpLat = -1.0 * (lat-1.0);
        fnLatSide = 'S';
    }

    fnLon = QString::number(tmpLon, 'f', 0).rightJustified(3, QChar('0'));
    fnLat = QString::number(tmpLat, 'f', 0).rightJustified(2, QChar('0'));

    (*name) = fnLatSide + fnLat + fnLonSide + fnLon + ".hgt";
}

void CCommons::convertFileNameToLonLat(const QString &name, double *lon, double *lat)
{
    QString tmp;
    tmp = name.right(18).left(14);
    QChar fnLatSide = tmp.at(0);
    QChar fnLonSide = tmp.at(7);
    QString fnLat = tmp.right(13).left(5);
    QString fnLon = tmp.right(6);

    (*lat) = fnLat.toDouble();
    (*lon) = fnLon.toDouble();

    if (fnLonSide==QChar('W')) {
        (*lon) = 360.0 - (*lon);
    }
    if (fnLatSide==QChar('S')) {
        (*lat) *= -1.0;
    }
}

void CCommons::convertLonLatToFileName(const double &lon, const double &lat, QString *name)
{
    double tmpLon, tmpLat;
    QChar fnLonSide, fnLatSide;
    QString fnLon, fnLat;

    if (lon>=180.0) {
        tmpLon = 360.0 - lon;
        fnLonSide = 'W';
    } else {
        tmpLon = lon;
        fnLonSide = 'E';
    }

    if (lat>=0.0) {
        tmpLat = lat;
        fnLatSide = 'N';
    } else {
        tmpLat = -1.0 * lat;
        fnLatSide = 'S';
    }

    fnLon = QString::number(tmpLon, 'f', 2).rightJustified(6, QChar('0'));
    fnLat = QString::number(tmpLat, 'f', 2).rightJustified(5, QChar('0'));

    (*name) = fnLatSide + fnLat + '_' + fnLonSide + fnLon + ".hgt";
    (*name)[3] = ',';
    (*name)[11] = ',';
}

void CCommons::convertLonLatToCartesian(const double &lon, const double &lat, double *lonX, double *latY)
{
    (*lonX) = lon;
    (*latY) = 90.0 - lat;
}

void CCommons::convertCartesianToLonLat(const double &lonX, const double &latY, double *lon, double *lat)
{
    (*lon) = lonX;
    (*lat) = 90.0 - latY;
}

int CCommons::getNeighborAvabilityIndex(const int &baseIndex, const double &degreeSize, const int &dx, const int &dy)
{
    int baseX, baseY;
    int newX, newY;
    int maxX, maxY;

    maxX = ((int)( (360.0 / degreeSize) + 0.5 )) - 1;
    maxY = ((int)( (180.0 / degreeSize) + 0.5 )) - 1;
    baseX = baseIndex % ((int)( (360.0 / degreeSize) + 0.5 ));
    baseY = baseIndex / ((int)( (360.0 / degreeSize) + 0.5 ));

    newX = baseX + dx;
    newY = baseY + dy;

    if (newX<0) newX = newX + (maxX + 1);
    if (newX>maxX) newX = newX - (maxX + 1);

    if (newY<0) return -1;
    if (newY>maxY) return -1;

    return newY*(maxX+1) + newX;
}
