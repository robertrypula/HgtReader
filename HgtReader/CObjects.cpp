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

#include "CObjects.h"
#include "CCommons.h"

CObjects::CObjects()
{
    sphere = gluNewQuadric();
}

CObjects::~CObjects()
{
    gluDeleteQuadric(sphere);
}

void CObjects::drawAxes(bool sunLight)
{
    if (sunLight) glDisable(GL_LIGHTING);

    // x
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(10.0*CONST_1GM, 0.0, 0.0);
    glEnd();

    // y
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 10.0*CONST_1GM, 0.0);
    glEnd();

    // z
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 10.0*CONST_1GM);
    glEnd();

    if (sunLight) glEnable(GL_LIGHTING);
}

void CObjects::drawGrid(bool sunLight)
{
    int i;
    int sizeZ = 10;
    int sizeX = 10;

    if (sunLight) glDisable(GL_LIGHTING);

    glColor3f(0.3, 0.3, 0.3);
    for (i=-sizeX; i<=sizeX; i++) {
        glBegin(GL_LINES);
            glVertex3f(i*CONST_1GM, 0.0, -sizeZ*CONST_1GM);
            glVertex3f(i*CONST_1GM, 0.0, sizeZ*CONST_1GM);
        glEnd();
    }
    for (i=-sizeZ; i<=sizeZ; i++) {
        glBegin(GL_LINES);
            glVertex3f(-sizeX*CONST_1GM, 0.0, i*CONST_1GM);
            glVertex3f(sizeX*CONST_1GM, 0.0, i*CONST_1GM);
        glEnd();
    }

    if (sunLight) glEnable(GL_LIGHTING);
}

void CObjects::drawSun(const double &sunX, const double &sunY, const double &sunZ, bool sunLight)
{
    glPushMatrix();
    glTranslated(sunX, sunY, sunZ);

    glColor3f(1.0, 1.0, 1.0);
    if (sunLight) glDisable(GL_LIGHTING);
    gluSphere(sphere, CONST_SUN_RADIUS, 15, 15);
    if (sunLight) glEnable(GL_LIGHTING);

    glPopMatrix();
}

void CObjects::drawEarthPoint(const double &epX, const double &epY, const double &epZ, const double &epDist, bool sunLight)
{
    double size = epDist/100.0;
    double sizeLine = size*1.04;

    if (sunLight) glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3, 1.0, 0.3);
        glVertex3d(epX, epY+size, epZ);
        glColor3f(0.3, 0.3, 1.0);
        glVertex3d(epX, epY, epZ+size);
        glColor3f(1.0, 0.3, 0.3);
        glVertex3d(epX+size, epY, epZ);
        glColor3f(1.0, 0.682, 0.0);
        glVertex3d(epX, epY, epZ-size);
        glColor3f(1.0, 0.682, 0.0);
        glVertex3d(epX-size, epY, epZ);
        glColor3f(0.3, 0.3, 1.0);
        glVertex3d(epX, epY, epZ+size);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0, 0.682, 0.0);
        glVertex3d(epX, epY-size, epZ);
        glColor3f(0.3, 0.3, 1.0);
        glVertex3d(epX, epY, epZ+size);
        glColor3f(1.0, 0.682, 0.0);
        glVertex3d(epX-size, epY, epZ);
        glColor3f(1.0, 0.682, 0.0);
        glVertex3d(epX, epY, epZ-size);
        glColor3f(1.0, 0.3, 0.3);
        glVertex3d(epX+size, epY, epZ);
        glColor3f(0.3, 0.3, 1.0);
        glVertex3d(epX, epY, epZ+size);
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
        // belt
        glVertex3d(epX, epY, epZ+sizeLine);
        glVertex3d(epX+sizeLine, epY, epZ);
        glVertex3d(epX+sizeLine, epY, epZ);
        glVertex3d(epX, epY, epZ-sizeLine);
        glVertex3d(epX, epY, epZ-sizeLine);
        glVertex3d(epX-sizeLine, epY, epZ);
        glVertex3d(epX-sizeLine, epY, epZ);
        glVertex3d(epX, epY, epZ+sizeLine);
        // up cone
        glVertex3d(epX, epY+sizeLine, epZ);
        glVertex3d(epX, epY, epZ+sizeLine);
        glVertex3d(epX, epY+sizeLine, epZ);
        glVertex3d(epX+sizeLine, epY, epZ);
        glVertex3d(epX, epY+sizeLine, epZ);
        glVertex3d(epX, epY, epZ-sizeLine);
        glVertex3d(epX, epY+sizeLine, epZ);
        glVertex3d(epX-sizeLine, epY, epZ);
        // down cone
        glVertex3d(epX, epY-sizeLine, epZ);
        glVertex3d(epX, epY, epZ+sizeLine);
        glVertex3d(epX, epY-sizeLine, epZ);
        glVertex3d(epX+sizeLine, epY, epZ);
        glVertex3d(epX, epY-sizeLine, epZ);
        glVertex3d(epX, epY, epZ-sizeLine);
        glVertex3d(epX, epY-sizeLine, epZ);
        glVertex3d(epX-sizeLine, epY, epZ);
    glEnd();

    if (sunLight) glEnable(GL_LIGHTING);
}
