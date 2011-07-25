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

#ifndef CRAWFILE_H
#define CRAWFILE_H

#include <QString>
#include <fstream>

using namespace std;

class CRawPixel
{
public:
    CRawPixel() { }
    CRawPixel(unsigned char red, unsigned char green, unsigned char blue)
    {
        r = red;
        g = green;
        b = blue;
    }
    CRawPixel(unsigned int rgb)
    {
        r = (rgb & 0xFF0000) >> 16;
        g = (rgb & 0x00FF00) >> 8;
        b = rgb & 0x0000FF;
    }

    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class CRawFile
{
public:
    CRawFile();
    ~CRawFile();

    void init(int sX, int sY);
    void saveFile(QString name);
    void loadFile(QString name, int x, int y);
    CRawPixel getPixel(int x, int y) { return pixel[y*sizeX + x]; }
    void setPixel(int x, int y, CRawPixel pix) { pixel[y*sizeX + x] = pix; }
    void getPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip);
    void setPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip);
    void fileOpen(QString name, int sX, int sY);
    void fileClose();
    void fileSetPixel(int x, int y, CRawPixel pix);
    CRawPixel fileGetPixel(int x, int y);
    void fileGetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip);
    void fileSetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip);
    void savePGM(QString name);
    unsigned char *getPixelsPointer();
    void setPixelsPointer(int sx, int sy, CRawPixel *p);

private:
    fstream file;
    CRawPixel *pixel;
    int sizeX;
    int sizeY;
    bool externalPixelPointer;
};

#endif // CRAWFILE_H
