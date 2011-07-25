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
#include "CHgtFile.h"

using namespace std;

CHgtFile::CHgtFile()
{
    sizeX = 0;
    sizeY = 0;
    height = 0;
}

CHgtFile::~CHgtFile()
{
    if (height!=0)
        delete []height;
}

void CHgtFile::init(int sX, int sY)
{
    if (height!=0)
        delete []height;

    sizeX = sX;
    sizeY = sY;

    height = new quint16[sizeX*sizeY];
}

void CHgtFile::exchangeEndian()
{
    unsigned char byte1, byte0;
    int i;

    for (i=0; i<sizeX*sizeY; i++) {
        byte1 = (height[i] & 0xFF00) >> 8;
        byte0 = height[i] & 0xFF;

        height[i] = (byte0 << 8) + byte1;
    }
}

void CHgtFile::savePGM(QString name)
{
    if (height==0) return;
    fstream filePGM;
    int height;
    int nr, x, y;

    // save PGM file to disk
    filePGM.open(name.toAscii(), fstream::out);

    filePGM << "P2" << endl;
    filePGM << sizeX << " " << sizeY << endl << 255;
    nr = 0;
    for (y=0; y<sizeY; y++)
      for (x=0; x<sizeX; x++) {
        if (nr%15==0) filePGM << endl;

        height = getHeight(x, y);
        if (height!=0) {
            height = (int)(((double)height / 3000.0) * 200.0) + 55;
            if (height>255) height = 255;
        }
        filePGM << height << " ";
        nr++;
      }

    filePGM.close();
}

void CHgtFile::saveFile(QString name)
{
    if (height==0) return;
    fstream fileHgt;

    // save HGT file to disk
    fileHgt.open(name.toAscii(), fstream::out | fstream::binary);
    exchangeEndian();
    fileHgt.write((char *)height, sizeX*sizeY*2);
    fileHgt.close();
}

void CHgtFile::loadFile(QString name, int x, int y)
{
    init(x, y);
    fstream fileHgt;

    // load HGT file to memory
    fileHgt.open(name.toAscii(), fstream::in | fstream::binary);
    fileHgt.read((char *)height, sizeX*sizeY*2);
    exchangeEndian();
    fileHgt.close();
}

void CHgtFile::getHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = getHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::getHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = (quint16)getHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::setHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setHeight(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CHgtFile::setHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setHeight(x + X*skip, y + Y*skip, (int)buffer[i]);
            i++;
        }
}

void CHgtFile::fileOpen(QString name, int sX, int sY)
{
    sizeX = sX;
    sizeY = sY;
    file.open(name.toAscii(), fstream::in | fstream::out | fstream::binary);
}

void CHgtFile::fileClose()
{
    file.close();
}

void CHgtFile::fileSetHeight(int x, int y, int hgt)
{
    unsigned char byte[2];

    byte[0] = (hgt & 0xFF00) >> 8;
    byte[1] = hgt & 0xFF;

    file.seekp((y*sizeX + x)*2);
    file.write((char *)byte, 2);
}

int CHgtFile::fileGetHeight(int x, int y)
{
    char byte[2];

    file.seekg((y*sizeX + x)*2);
    file.read(byte, 2);

    return (int)( (((unsigned char)byte[0]) << 8) + ((unsigned char)byte[1]) );
}

void CHgtFile::fileGetHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = fileGetHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::fileGetHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = (quint16)fileGetHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::fileSetHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetHeight(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CHgtFile::fileSetHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetHeight(x + X*skip, y + Y*skip, (int)buffer[i]);
            i++;
        }
}
