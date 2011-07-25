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

#include <fstream>
#include <iostream>
#include "CRawFile.h"

using namespace std;

CRawFile::CRawFile()
{
    sizeX = 0;
    sizeY = 0;
    pixel = 0;
    externalPixelPointer = false;
}

CRawFile::~CRawFile()
{
    if (!externalPixelPointer && pixel!=0)
        delete []pixel;
}

void CRawFile::init(int sX, int sY)
{
    if (pixel!=0)
        delete []pixel;

    sizeX = sX;
    sizeY = sY;

    pixel = new CRawPixel[sizeX*sizeY];
}

unsigned char *CRawFile::getPixelsPointer()
{
    return (unsigned char *)pixel;
}

void CRawFile::setPixelsPointer(int sx, int sy, CRawPixel *p)
{
    externalPixelPointer = true;
    sizeX = sx;
    sizeY = sy;
    pixel = p;
}

void CRawFile::savePGM(QString name)
{
    if (pixel==0) return;
    fstream filePGM;
    CRawPixel pixel;
    int pixelPGM;
    int nr, x, y;

    // save PGM file to disk
    filePGM.open(name.toAscii(), fstream::out);

    filePGM << "P2" << endl;
    filePGM << sizeX << " " << sizeY << endl << 255;
    nr = 0;
    for (y=0; y<sizeY; y++)
      for (x=0; x<sizeX; x++) {
        if (nr%15==0) filePGM << endl;

        pixel = getPixel(x, y);
        pixelPGM = (int)( ((double)(pixel.r + pixel.g + pixel.b))/3.0 );
        if (pixelPGM>255)
            pixelPGM = 255;
        filePGM << pixelPGM << " ";
        nr++;
      }

    filePGM.close();
}

void CRawFile::saveFile(QString name)
{
    if (pixel==0) return;
    fstream fileRaw;

    // save RAW file to disk
    fileRaw.open(name.toAscii(), fstream::out | fstream::binary);
    fileRaw.write((char *)pixel, sizeX*sizeY*3);
    fileRaw.close();
}

void CRawFile::loadFile(QString name, int x, int y)
{
    init(x, y);
    fstream fileRaw;

    // load RAW file to memory
    fileRaw.open(name.toAscii(), fstream::in | fstream::binary);
    fileRaw.read((char *)pixel, sizeX*sizeY*3);
    fileRaw.close();
}

void CRawFile::getPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = getPixel(x + X*skip, y + Y*skip);
            i++;
        }
}

void CRawFile::setPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setPixel(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CRawFile::fileOpen(QString name, int sX, int sY)
{
    sizeX = sX;
    sizeY = sY;
    file.open(name.toAscii(), fstream::in | fstream::out | fstream::binary);
}

void CRawFile::fileClose()
{
    file.close();
}

void CRawFile::fileSetPixel(int x, int y, CRawPixel pix)
{
    file.seekp((y*sizeX + x)*3);
    file.write((char *)(&pix), 3);
}

CRawPixel CRawFile::fileGetPixel(int x, int y)
{
    CRawPixel pix;

    file.seekg((y*sizeX + x)*3);
    file.read((char *)(&pix), 3);

    return pix;
}

void CRawFile::fileGetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = fileGetPixel(x + X*skip, y + Y*skip);
            i++;
        }
}

void CRawFile::fileSetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetPixel(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}
