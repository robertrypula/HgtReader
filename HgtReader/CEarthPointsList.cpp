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
#include <QStringList>
#include <QTextStream>
#include "CEarthPointsList.h"
#include "CCacheManager.h"
#include "CCommons.h"

CEarthPointsList::CEarthPointsList()
{
    load();
}

void CEarthPointsList::load()
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    CEarthPoint *earthPoint;
    QFile file(cacheManager->pathBase + "earthPoints.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList lineList;

        lineList = line.split(';');

        earthPoint = new CEarthPoint();
        earthPoint->lon = ((QString)lineList.at(0)).toDouble();
        earthPoint->lat = ((QString)lineList.at(1)).toDouble();
        earthPoint->alt = CONST_EARTH_RADIUS + ((QString)lineList.at(2)).toDouble();
        CCommons::getCartesianFromSpherical(earthPoint->lon, earthPoint->lat, earthPoint->alt, &(earthPoint->x), &(earthPoint->y), &(earthPoint->z));
        earthPoint->name = lineList.at(3);

        earthPoints.append(*earthPoint);
    }
}

void CEarthPointsList::save()
{
    CCacheManager *cacheManager = CCacheManager::getInstance();
    int i;
    CEarthPoint earthPoint;
    QFile file(cacheManager->pathBase + "earthPoints.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (i=0; i<earthPoints.size(); i++) {
        earthPoint = earthPoints.at(i);
        QStringList lineList;
        QString line;

        lineList.append( QString::number(earthPoint.lon, 'f', 6) );
        lineList.append( QString::number(earthPoint.lat, 'f', 6) );
        lineList.append( QString::number(earthPoint.alt - CONST_EARTH_RADIUS, 'f', 3) );
        lineList.append( earthPoint.name );

        line = lineList.join(";");
        out << line << endl;
    }
    file.close();
}

int CEarthPointsList::addAndSort(QString name, double earthPointLon, double earthPointLat, double earthPointAlt, double earthPointX, double earthPointY, double earthPointZ)
{
    CEarthPoint *earthPoint = new CEarthPoint;
    earthPoint->lon = earthPointLon;
    earthPoint->lat = earthPointLat;
    earthPoint->alt = earthPointAlt;
    earthPoint->x = earthPointX;
    earthPoint->y = earthPointY;
    earthPoint->z = earthPointZ;
    earthPoint->name = name;

    earthPoints.append(*earthPoint);
    qSort(earthPoints.begin(), earthPoints.end());
    save();

    return earthPoints.indexOf(*earthPoint);
}
