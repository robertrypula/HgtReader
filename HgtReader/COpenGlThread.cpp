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

#include <QDebug>
#include "COpenGlThread.h"
#include "CRawFile.h"
#include "CCommons.h"


COpenGlThread::COpenGlThread(COpenGl *openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    windowWidth = CONST_DEF_WIDTH;
    windowHeight = CONST_DEF_HEIGHT;

    doTerminate = false;
    doResize = false;

    // set earth pointer to earth "A"
    earth = openGl->earthBufferA;

    // set in Earth DrawingStateSnapshot object
    earth->setDrawingStateSnapshot(&dss);
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
    earth->initLOD_0();
}

void COpenGlThread::resizeEvent(int w, int h)
{
    doMutex.lock();
    doResizeWindowWidth = w;
    doResizeWindowHeight = h;
    doResize = true;
    doMutex.unlock();
}

void COpenGlThread::resizeEvent()
{
    doMutex.lock();
    doResizeWindowWidth = windowWidth;
    doResizeWindowHeight = windowHeight;
    doResize = true;
    doMutex.unlock();
}

void COpenGlThread::stop()
{
    doMutex.lock();
    doTerminate = true;
    doMutex.unlock();
}

void COpenGlThread::run()
{
    int i;
    unsigned int texID;
    CEarth *tmp;

    openGl->makeCurrent();
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
    initializeScene();

    while (true) {
        time.start();

        doMutex.lock();
        if (doTerminate) {
            doMutex.unlock();
            return;
        }
        if (doResize) {
            resize(doResizeWindowWidth, doResizeWindowHeight);
            doResize = false;
        }
        doMutex.unlock();

        // ### OpenGL scene
        checkSunLightningAndAtmosphere(); // at 85km sky is black
        drawScene();
        // ### OpenGL scene END

        openGl->swapBuffers();
        openGl->drawingState.getDrawingStateSnapshot(&dss);  // get current scene state

        // check if new earth was loaded from disk in parraler thread
        openGl->earthBufferMutex.lock();
        if (openGl->earthBufferReadyToExchange) {
            // exchange earths :]
            tmp = earth;
            earth = openGl->terrainLoaderThread->earth;
            openGl->terrainLoaderThread->earth = tmp;

            earth->setDrawingStateSnapshot(&dss);       // set drawing state used in >this< thread

            openGl->earthBufferReadyToExchange = false;
            openGl->earthBufferExchange.wakeOne();      // wake terrain loading thread
        }
        openGl->earthBufferMutex.unlock();

        // remove from VRAM textures assigned to terrainData that was deleted from cache in TerrainLoaderThread
        for (i=0; i<earth->textureIDListToRemoveFromVRAM.size(); i++) {
            texID = earth->textureIDListToRemoveFromVRAM.at(i);
            glDeleteTextures(1, &texID);
        }
        earth->textureIDListToRemoveFromVRAM.clear();

        // update performance info
        msleep(1);
        openGl->performance.setFrameRenderingTime(time.elapsed());
        openGl->performance.updateFrameRenderingInfo();
    }
}

void COpenGlThread::checkSunLightningAndAtmosphere()
{
    double sunHorizonCosine;
    double sunBelowHorizonFadeAltStoper;
    double sunBelowHorizonFade;
    double atmosphereAltFade;
    double atmosphereNightFade;

    sunHorizonCosine = QVector3D::dotProduct(dss.camPosition.normalized(), dss.sunLightNormal);

    sunBelowHorizonFadeAltStoper = dss.camAltGround/100000.0 + 0.0;
    if (sunBelowHorizonFadeAltStoper>1.0) sunBelowHorizonFadeAltStoper = 1.0;

    sunBelowHorizonFade = (sunHorizonCosine + 0.02)*25.0;
    if (sunBelowHorizonFade>1.0) sunBelowHorizonFade = 1.0;
    if (sunBelowHorizonFade<0.0) sunBelowHorizonFade = 0.0;

    if (sunBelowHorizonFade<sunBelowHorizonFadeAltStoper) sunBelowHorizonFade = sunBelowHorizonFadeAltStoper;

    GLfloat light_diffuse0[]  = {1.0*sunBelowHorizonFade, 1.0*sunBelowHorizonFade, 1.0*sunBelowHorizonFade, 0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);


    // night & space atmosphere fading (to black)
    atmosphereNightFade = (sunHorizonCosine + 0.1)*10.0;
    if (atmosphereNightFade>1.0) atmosphereNightFade = 1.0;
    if (atmosphereNightFade<0.0) atmosphereNightFade = 0.0;

    atmosphereAltFade = 1.0 - dss.camAltGround/85000.0;
    if (atmosphereAltFade<0.0) atmosphereAltFade = 0.0;
    if (atmosphereAltFade>1.0) atmosphereAltFade = 1.0;

    glClearColor(0.447*atmosphereAltFade*atmosphereNightFade,
                 0.812*atmosphereAltFade*atmosphereNightFade,
                 1.000*atmosphereAltFade*atmosphereNightFade,
                 1.0);
}

void COpenGlThread::drawScene()
{
    // clear scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set camera
    glLoadIdentity();
    gluLookAt(dss.camPerspectiveX, dss.camPerspectiveY, dss.camPerspectiveZ,
              dss.camPerspectiveLookAtX, dss.camPerspectiveLookAtY, dss.camPerspectiveLookAtZ,
              0.0, 1.0, 0.0);
    glPushMatrix();

    // move all world to terrain coordinates if in TerrainLinkage
    if (dss.camLinkage==CAM_LINKAGE_TERRAIN) {
        glRotated(dss.earthPointLat-90.0, 1.0, 0.0, 0.0);
        glRotated(-dss.earthPointLon, 0.0, 1.0, 0.0);
        glTranslated(-dss.earthPointX, -dss.earthPointY, -dss.earthPointZ);
    }

    // draw sun (its far so z-buffer trick must be aplied)
    resize(windowWidth, windowHeight, true);
    glClear(GL_DEPTH_BUFFER_BIT);
    objects.drawSun(dss.sunPositionGlobe.x(), dss.sunPositionGlobe.y(), dss.sunPositionGlobe.z(), dss.sunEnabled);
    resize(windowWidth, windowHeight, false);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (dss.sunEnabled)        enableSunLight(); else disableSunLight();
    if (dss.drawGrid)          objects.drawGrid(dss.sunEnabled);
    if (dss.drawEarthPoint)    objects.drawEarthPoint(dss.earthPointX, dss.earthPointY, dss.earthPointZ, dss.camDistanceToEarthPoint, dss.sunEnabled);
    earth->draw();

    glPopMatrix();
    if (dss.drawAxes)    objects.drawAxes(dss.sunEnabled);
}

void COpenGlThread::resize(int w, int h, bool forSunRendering)
{
    double windowAspectRatio;
    double zNear = 1.0, zFar = 10.0;

    windowWidth = w;
    windowHeight = h;
    windowAspectRatio = (double)w/(double)h;

    // recalculate OpenGL projection matrix
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (forSunRendering) {
        zNear = 1000.0 * CONST_1GM;
        zFar = CONST_SUN_DISTANCE + CONST_SUN_RADIUS*10.0;
    } else {
        if (dss.camAltGround<=1.0*CONST_1KM)                                        { zNear =    0.015*CONST_1KM; zFar =     200.0*CONST_1KM; } else
        if (dss.camAltGround<=10.0*CONST_1KM && dss.camAltGround>1.0*CONST_1KM)     { zNear =    0.015*CONST_1KM; zFar =     300.0*CONST_1KM; } else
        if (dss.camAltGround<=100.0*CONST_1KM && dss.camAltGround>10.0*CONST_1KM)   { zNear =    0.150*CONST_1KM; zFar =    3000.0*CONST_1KM; } else
        if (dss.camAltGround<=1000.0*CONST_1KM && dss.camAltGround>100.0*CONST_1KM) { zNear =   15.000*CONST_1KM; zFar =  300000.0*CONST_1KM; } else
        if (dss.camAltGround>1000.0*CONST_1KM)                                      { zNear =  150.000*CONST_1KM; zFar = 3000000.0*CONST_1KM; }
    }

    gluPerspective(dss.camFOV, windowAspectRatio, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
}

void COpenGlThread::initializeScene()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // zbuffor settings
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glEnable(GL_COLOR_MATERIAL);                        // enable color tracking
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);  // set material properties which will be assigned by glColor
    glShadeModel(GL_SMOOTH);                            // smooth shadows

    // set lightning
    GLfloat light_ambient0[]  = {0.00, 0.00, 0.00, 0.0};
    GLfloat light_diffuse0[]  = {1.0, 1.0, 1.0, 0.0};
    GLfloat light_specular0[] = {0.0, 0.0, 0.0, 0.0};
    GLfloat att_constant0     = {0.0};
    GLfloat att_linear0       = {0.00};
    GLfloat att_quadratic0    = {0.000};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear0);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic0);
    glEnable(GL_LIGHT0);

    resize(windowWidth, windowHeight);
}

void COpenGlThread::enableSunLight()
{
    GLfloat light_position0[] = {dss.sunLightNormal.x(),
                                 dss.sunLightNormal.y(),
                                 dss.sunLightNormal.z(),
                                 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glEnable(GL_LIGHTING);
}

void COpenGlThread::disableSunLight()
{
    glDisable(GL_LIGHTING);
}
