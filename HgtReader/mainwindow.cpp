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

#include <QtGui>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CDrawingState.h"
#include "CCommons.h"
#include "CCamera.h"

/*
  sizeof(QVector3D) = 12 bytes
  sizeof(QColor)    = 16 bytes
  sizeof(CTerrain)  = 168 bytes
  CTerrain object size:            7*ptr + 2*bool + 1*double + 1*QVector3D =
                                 = 7*4   + 2*1    + 1*8      + 1*12 =
                                 = 28    + 2      + 8        + 12 =
                                 = 50 bytes
  real CTerrainData object size:   4*double + 1*int + 22*QVector3D + 1*GLuint + 10*ptr + [32*32*3]*uint8_t + 8*81*QVector3D + 81*QColor =
                                 = 4*8      + 1*4   + 22*12        + 1*4      + 10*4   + 3072*1            + 648*12       + 81*16 =
                                 = 32       + 4     + 264          + 4        + 40     + 3072              + 7776         + 1296 =
                                 = 12 488 bytes = 0,011909485 MB
  real CTerrain object size:     50 bytes + 12 488 bytes = 12 538 bytes = 0,011957169 MB
*/

#define REAL_SIZEOF_CTERRAINDATA_CLASS   0.011909485
#define REAL_SIZEOF_CTERRAIN_CLASS       0.011957169

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    double epLon, epLat, epAlt;
    double sunLon, sunLat, sunAzim, sunElev;
    CDrawingState *drawingState;
    CCamera *camera;
    ui->setupUi(this);

    QGLFormat glFormat;
    glFormat.setDoubleBuffer(true);
    glFormat.setSampleBuffers(true);
    glFormat.setAlpha(true);
    glFormat.setDepthBufferSize(24);
    openGl = new COpenGl(glFormat, centralWidget());
    centralWidget()->layout()->addWidget(openGl);

    // shortcuts for objects :]
    drawingState = &openGl->drawingState;
    camera = openGl->drawingState.getCamera();

    // load earth point to select
    SLOTreloadEarthPointsSelect(0);

    QObject::connect(ui->cacheClearButton, SIGNAL(clicked()), openGl->terrainLoaderThread, SLOT(SLOTclearCache()));
    QObject::connect(openGl->terrainLoaderThread, SIGNAL(SIGNALupdateCacheInfo(int,int,int,int,unsigned int)), this, SLOT(SLOTupdateCacheInfo(int,int,int,int,unsigned int)));
    QObject::connect(ui->benchmarkButton, SIGNAL(clicked()), openGl->animationThread, SLOT(SLOTstartBenchmark()));
    QObject::connect(camera, SIGNAL(SIGNALanimateToEarthPoint(double,double,double,double,double,double)), openGl->animationThread,
                             SLOT(SLOTanimateToEarthPoint(double,double,double,double,double,double)));
    QObject::connect(camera, SIGNAL(SIGNALupdateCameraInfo(double,double,double,double)), this, SLOT(SLOTupdateCameraInfo(double, double, double, double)));
    QObject::connect(camera, SIGNAL(SIGNALupdateSunInfo(double,double,double,double)), this, SLOT(SLOTupdateSunInfo(double,double,double,double)));
    QObject::connect(camera, SIGNAL(SIGNALupdateFovAndCamVel(double,double)), this, SLOT(SLOTupdateFovAndCamVelInfo(double,double)));
    QObject::connect(camera, SIGNAL(SIGNALupdateEarthPointInfo(double,double,double,bool)), this, SLOT(SLOTupdateEarthPointInfo(double,double,double,bool)));
    QObject::connect(camera, SIGNAL(SIGNALupdateCameraInteractMode(int)), this, SLOT(SLOTupdateCameraInteractMode(int)));
    QObject::connect(camera, SIGNAL(SIGNALupdateSunInteractMode(bool)), this, SLOT(SLOTupdateSunInteractMode(bool)));
    QObject::connect(ui->lodMultiplierSelect, SIGNAL(currentIndexChanged(int)), drawingState, SLOT(SLOTlodMultiplierIndexChanged(int)));
    QObject::connect(ui->camVelFromAltitudeCheckbox, SIGNAL(stateChanged(int)), camera, SLOT(SLOTcamVelFromAltChanged(int)));
    QObject::connect(ui->getEarthPointButton, SIGNAL(clicked()), camera, SLOT(SLOTgetNewEarthPoint()));
    QObject::connect(ui->earthPointSelect, SIGNAL(currentIndexChanged(int)), camera, SLOT(SLOTearthPointSelectCurrentIndexChanged(int)));
    QObject::connect(ui->earthPointAddButton, SIGNAL(clicked()), this, SLOT(SLOTearthPointAddButtonClicked()));
    QObject::connect(camera, SIGNAL(SIGNALreloadEarthPointSelect(int)), this, SLOT(SLOTreloadEarthPointsSelect(int)));
    QObject::connect(&(openGl->performance), SIGNAL(SIGNALupdateFrameRenderingInfo(int,double)), this, SLOT(SLOTupdateFrameRenderingInfo(int,double)));
    QObject::connect(&(openGl->performance), SIGNAL(SIGNALupdateTerrainTreeUpdatingInfo(int,int,double)), this, SLOT(SLOTupdateTerrainTreeUpdatingInfo(int,int,double)));
    QObject::connect(ui->drawTerrainPointCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainPointChanged(int)));
    QObject::connect(ui->drawTerrainPointColorCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainPointColorChanged(int)));
    QObject::connect(ui->drawTerrainWireCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainWireChanged(int)));
    QObject::connect(ui->drawTerrainWireColorCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainWireColorChanged(int)));
    QObject::connect(ui->drawTerrainSolidCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainSolidChanged(int)));
    QObject::connect(ui->drawTerrainSolidNormalRadioButton, SIGNAL(clicked()), drawingState, SLOT(SLOTdrawTerrainSolidNormalClicked()));
    QObject::connect(ui->drawTerrainSolidStripRadioButton, SIGNAL(clicked()), drawingState, SLOT(SLOTdrawTerrainSolidStripClicked()));
    QObject::connect(ui->drawTerrainSolidColorCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainSolidColorChanged(int)));
    QObject::connect(ui->drawTerrainTextureCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainTextureChanged(int)));
    QObject::connect(ui->drawTerrainTextureNormalRadioButton, SIGNAL(clicked()), drawingState, SLOT(SLOTdrawTerrainTextureNormalClicked()));
    QObject::connect(ui->drawTerrainTextureStripRadioButton, SIGNAL(clicked()), drawingState, SLOT(SLOTdrawTerrainTextureStripClicked()));
    QObject::connect(ui->drawTerrainBottomPlaneWireCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainBottomPlaneWireChanged(int)));
    QObject::connect(ui->drawTerrainBottomPlaneWireColorCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainBottomPlaneWireColorChanged(int)));
    QObject::connect(ui->drawTerrainBottomPlaneSolidCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainBottomPlaneSolidChanged(int)));
    QObject::connect(ui->drawTerrainBottomPlaneSolidColorCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainBottomPlaneSolidColorChanged(int)));
    QObject::connect(ui->drawTerrainBottomPlaneTextureCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainBottomPlaneTextureChanged(int)));
    QObject::connect(ui->drawTerrainNormalsCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawTerrainNormalsChanged(int)));
    QObject::connect(ui->drawEarthPointCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawEarthPointChanged(int)));
    QObject::connect(ui->drawGridCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawGridChanged(int)));
    QObject::connect(ui->drawAxesCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdrawAxesChanged(int)));
    QObject::connect(ui->sunEnabledCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTsunEnabledChanged(int)));
    QObject::connect(ui->treeUpdatingCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTtreeUpdatingChanged(int)));
    QObject::connect(ui->dontUseDiskHgtCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdontUseDiskHgtChanged(int)));
    QObject::connect(ui->dontUseDiskRawCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdontUseDiskRawChanged(int)));
    QObject::connect(ui->dontUseCacheCheckBox, SIGNAL(stateChanged(int)), drawingState, SLOT(SLOTdontUseCacheChanged(int)));
    QObject::connect(ui->aboutButton, SIGNAL(clicked()), this, SLOT(SLOTaboutButtonClicked()));
    QObject::connect(ui->keyMapButton, SIGNAL(clicked()), this, SLOT(SLOTkeyMapButtonClicked()));

    // set infos for the first time
    camera->getEarthPointLonLatAlt(&epLon, &epLat, &epAlt);
    SLOTupdateEarthPointInfo(epLon, epLat, epAlt-CONST_EARTH_RADIUS, false);
    camera->getSunLonLatAzimElev(&sunLon, &sunLat, &sunAzim, &sunElev);
    SLOTupdateSunInfo(sunLon, sunLat, sunAzim, sunElev);
    camera->switchToGlobalOrbitMode();

    // set icon
    //setWindowIcon(QIcon(":/images/icon.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (ui->tabWidget->isHidden())
            ui->tabWidget->show(); else
            ui->tabWidget->hide();
    }
}

void MainWindow::SLOTearthPointAddButtonClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add EarthPoint"),
                                               tr("Location text:"), QLineEdit::Normal,
                                               tr("New location"), &ok);
    if (ok && !text.isEmpty())
        openGl->drawingState.getCamera()->earthPointsListAdd(text);

    openGl->setFocus();
}

void MainWindow::SLOTreloadEarthPointsSelect(int indexToSelect)
{
    CEarthPointsList epList = openGl->drawingState.getCamera()->getEarthPointsList();
    int i;

    QObject::disconnect(ui->earthPointSelect, SIGNAL(currentIndexChanged(int)), openGl->drawingState.getCamera(), SLOT(SLOTearthPointSelectCurrentIndexChanged(int)));
    ui->earthPointSelect->clear();
    ui->earthPointSelect->addItem("");
    for (i=0; i<epList.earthPoints.size(); i++) {
        ui->earthPointSelect->addItem(epList.earthPoints.at(i).name);
    }
    QObject::connect(ui->earthPointSelect, SIGNAL(currentIndexChanged(int)), openGl->drawingState.getCamera(), SLOT(SLOTearthPointSelectCurrentIndexChanged(int)));
    ui->earthPointSelect->setCurrentIndex(indexToSelect);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void MainWindow::SLOTupdateCacheInfo(int cachedTDCount, int cachedTDInUseCount, int cachedTDNotInUseCount, int cachedTDEmptyEntryCount, unsigned int cacheMinNotInUseTime)
{
    int usedunused = cachedTDInUseCount + cachedTDNotInUseCount;

    ui->usedTerrainsLabel->setText( QString::number(cachedTDInUseCount) + QString(" (") +
                                    QString::number(cachedTDInUseCount*REAL_SIZEOF_CTERRAINDATA_CLASS, 'f', 2) + QString(" MB)") );
    ui->unusedTerrainLabel->setText( QString::number(cachedTDNotInUseCount) + QString(" (") +
                                    QString::number(cachedTDNotInUseCount*REAL_SIZEOF_CTERRAINDATA_CLASS, 'f', 2) + QString(" MB)") );
    ui->usedUnusedTerrainLabel->setText( QString::number(usedunused) + QString(" (") +
                                         QString::number(usedunused*REAL_SIZEOF_CTERRAINDATA_CLASS, 'f', 2) + QString(" MB)") );
    ui->erasedTerrainLabel->setText( QString::number(cachedTDEmptyEntryCount) + QString(" (") +
                                     QString::number(cachedTDEmptyEntryCount*REAL_SIZEOF_CTERRAINDATA_CLASS, 'f', 2) + QString(" MB)") );
}

void MainWindow::SLOTupdateCameraInteractMode(int interactState)
{
    switch (interactState)
    {
        case 1:ui->cameraInteractModeInfoLabel->setText( QString("Globe-Orbit") );
               break;
        case 2:ui->cameraInteractModeInfoLabel->setText( QString("Globe-Free") );
               break;
        case 3:ui->cameraInteractModeInfoLabel->setText( QString("Terrain-Orbit") );
               break;
        case 4:ui->cameraInteractModeInfoLabel->setText( QString("Terrain-Free") );
               break;
    }
}

void MainWindow::SLOTupdateSunInteractMode(bool sunMoving)
{
    if (!sunMoving)
        ui->sunInteractModeInfoLabel->setText( QString("inactive") ); else
        ui->sunInteractModeInfoLabel->setText( QString("active") );
}

void MainWindow::SLOTupdateCameraInfo(double camLon, double camLat, double camAltGround, double camDistanceToEarthPoint)
{
    ui->camInfoLonLabel->setText( QString::number(camLon, 'f', 6) );
    ui->camInfoLatLabel->setText( QString::number(camLat, 'f', 6) );
    ui->camInfoAltLabel->setText( QString::number(camAltGround/CONST_1KM, 'f', 3) + QString(" km") );
    ui->camInfoEpInfoLabel->setText( QString::number(camDistanceToEarthPoint/CONST_1KM, 'f', 3) + QString(" km") );
}

void MainWindow::SLOTupdateSunInfo(double sunLon, double sunLat, double sunAzim, double sunElev)
{
    double sA;

    sA = 180.0 - sunAzim;
    if (sA>=360.0) sA -= 360.0;
    if (sA<0.0) sA += 360.0;

    ui->sunInfoLonLabel->setText( QString::number(sunLon, 'f', 2) );
    ui->sunInfoLatLabel->setText( QString::number(sunLat, 'f', 2) );
    ui->sunEpAzimLabel->setText( QString::number(sA, 'f', 2) );
    ui->sunEpElevLabel->setText( QString::number(sunElev, 'f', 2) );
}

void MainWindow::SLOTupdateFovAndCamVelInfo(double fov, double vel)
{
    ui->camFOVLabel->setText( QString::number(fov, 'f', 1) );
    ui->camVelMsLabel->setText( QString::number(vel, 'f', 1) + QString(" m/s") );
    ui->camVelKmHLabel->setText( QString::number(vel*3.6, 'f', 0) + QString(" km/h") );
}

void MainWindow::SLOTupdateEarthPointInfo(double earthPointLon, double earthPointLat, double earthPointAltGround, bool fromSelect)
{
    ui->earthPointLonLabel->setText( QString::number(earthPointLon, 'f', 6) );
    ui->earthPointLatLabel->setText( QString::number(earthPointLat, 'f', 6) );
    ui->earthPointAltLabel->setText( QString::number(earthPointAltGround/CONST_1KM, 'f', 3) + QString(" km") );

    if (!fromSelect) {
        ui->earthPointSelect->setCurrentIndex(0);
        ui->earthPointAddButton->setEnabled(true);
    } else {
        ui->earthPointAddButton->setEnabled(false);
    }

    openGl->setFocus();
}

void MainWindow::SLOTupdateFrameRenderingInfo(int terrainsQuarterDrawed, double fps)
{
    ui->fpsInfoLabel->setText( QString::number(fps, 'f', 2) );
    ui->quatersCountLabel->setText( QString::number(terrainsQuarterDrawed) );
    ui->trianglesLabel->setText( QString::number(terrainsQuarterDrawed*32) );
}

void MainWindow::SLOTupdateTerrainTreeUpdatingInfo(int terrainsInTree, int maxLOD, double tups)
{
    ui->tupsInfoLabel->setText( QString::number(tups, 'f', 2) );
    ui->terrainsInTreeLabel->setText( QString::number(terrainsInTree) );
    ui->treeSizeLabel->setText( QString::number((terrainsInTree*REAL_SIZEOF_CTERRAIN_CLASS), 'f', 2) + QString(" MB") );
    ui->maxLODLabel->setText( QString::number(maxLOD) );
}

void MainWindow::SLOTkeyMapButtonClicked()
{
    QMessageBox::about(this, tr("HgtReader - Key map"),
             tr("<b>Key map:</b><br/><br/>"
                "F1 - switch camera mode to Globe-Orbit<br/>"
                "F2 - switch camera mode to Globe-Free<br/>"
                "F3 - switch camera mode to Terrain-Orbit<br/>"
                "F4 - switch camera mode to Terrain-Free<br/>"
                "F5 - turn on/off sun moving<br/>"
                "WSAD - walking in 'Free' camera mode<br/>"
                "Z - camera FOV +<br/>"
                "X - camera FOV -<br/>"
                "Esc - show/hide top tabs<br/>"
                "left mouse button - orbiting/looking around/changing sun's position<br/>"
                "right mouse button - zooming in 'Orbit' camera mode<br/>"
                ));
}

void MainWindow::SLOTaboutButtonClicked()
{
    QMessageBox::about(this, tr("HgtReader - about"),
             tr("<style> ul { margin: 0; } </style>"
                "<div style='font-size: 11px;'>"
                    "<b>HgtReader v1.0</b><br/><br/>"
                    "(c) Robert Rypula 156520<br/>"
                    "Wroclaw University of Technology - Poland<br/>"
                    "<a href='http://www.pwr.wroc.pl'>http://www.pwr.wroc.pl</a><br/>"
                    "2011.01 - 2011.06<br/><br/>"
                    "<b>What is this:</b>"
                    "<ul>"
                        "<li>graphic system based on OpenGL to visualize entire Earth including<br/>terrain topography & satellite images</li>"
                        "<li>part of my thesis 'Rendering of complex 3D scenes'</li>"
                    "</ul><br/>"
                    "<b>What it use:</b>"
                    "<ul>"
                        "<li>Nokia Qt cross-platform C++ application framework</li>"
                        "<li>OpenGL graphic library</li>"
                        "<li>"
                            "NASA SRTM terrain elevation data"
                            "<ul>"
                                "<li>"
                                    "oryginal dataset:<br/>"
                                    "<a href='http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/'>http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/</a>"
                                "</li>"
                                "<li>"
                                    "corrected part of earth:<br/>"
                                    "<a href='http://www.viewfinderpanoramas.org/dem3.html'>http://www.viewfinderpanoramas.org/dem3.html</a>"
                                "</li>"
                                "<li>"
                                    "SRTM v4 highest quality SRTM dataset avaiable:<br/>"
                                    "<a href='http://srtm.csi.cgiar.org/'>http://srtm.csi.cgiar.org/</a>"
                                "</li>"
                            "</ul>"
                        "</li>"
                        "<li>"
                            "TrueMarble satellite images"
                            "<ul>"
                                "<li>"
                                    "free version from Unearthed Outdoors (250m/pix):<br/>"
                                    "<a href='http://www.unearthedoutdoors.net/global_data/true_marble/download'>http://www.unearthedoutdoors.net/global_data/true_marble/download</a>"
                                "</li>"
                            "</ul>"
                        "</li>"
                        "<li>"
                            "ALGLIB cross-platform numerical analysis and data processing library for SRTM<br/>dataset bicubic interpolation from 90m to 103m (more flexible LOD division)"
                            "<ul>"
                                "<li>"
                                    "ALGLIB website:<br/>"
                                    "<a href='http://www.alglib.net/'>http://www.alglib.net/</a>"
                                "</li>"
                            "</ul>"
                        "</li>"
                    "</ul><br/>"
                    "<b>Contact to author:</b>"
                    "<ul>"
                        "<li>phone: +48 505-363-331</li>"
                        "<li>e-mail: <a href='mailto:robert.rypula@gmail.com'>robert.rypula@gmail.com</a></li>"
                        "<li>GG: 1578139</li>"
                    "</ul><br/><br/>"
                    "program under GNU licence"
                "</div>"
               ));
}
