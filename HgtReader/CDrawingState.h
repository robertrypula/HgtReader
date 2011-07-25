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

#ifndef CDRAWINGSTATE_H
#define CDRAWINGSTATE_H

#include <QObject>
#include <QMutex>
#include "CCamera.h"
#include "CDrawingStateSnapshot.h"


class CDrawingState : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void SIGNALclearCache();

public:
    CDrawingState();

    CCamera *getCamera();
    void setDrawingStateMutex(QMutex *m);
    void getDrawingStateSnapshot(CDrawingStateSnapshot *dss);       // thread safe (drawingStateMutex)

public slots:
    void SLOTdrawTerrainPointChanged(int state);                    // thread safe (drawingStateMutex)
    void SLOTdrawTerrainPointColorChanged(int state);               // thread safe (drawingStateMutex)
    void SLOTdrawTerrainWireChanged(int state);                     // thread safe (drawingStateMutex)
    void SLOTdrawTerrainWireColorChanged(int state);                // thread safe (drawingStateMutex)
    void SLOTdrawTerrainSolidChanged(int state);                    // thread safe (drawingStateMutex)
    void SLOTdrawTerrainSolidNormalClicked();                       // thread safe (drawingStateMutex)
    void SLOTdrawTerrainSolidStripClicked();                        // thread safe (drawingStateMutex)
    void SLOTdrawTerrainSolidColorChanged(int state);               // thread safe (drawingStateMutex)
    void SLOTdrawTerrainTextureChanged(int state);                  // thread safe (drawingStateMutex)
    void SLOTdrawTerrainTextureNormalClicked();                     // thread safe (drawingStateMutex)
    void SLOTdrawTerrainTextureStripClicked();                      // thread safe (drawingStateMutex)
    void SLOTdrawTerrainBottomPlaneWireChanged(int state);          // thread safe (drawingStateMutex)
    void SLOTdrawTerrainBottomPlaneWireColorChanged(int state);     // thread safe (drawingStateMutex)
    void SLOTdrawTerrainBottomPlaneSolidChanged(int state);         // thread safe (drawingStateMutex)
    void SLOTdrawTerrainBottomPlaneSolidColorChanged(int state);    // thread safe (drawingStateMutex)
    void SLOTdrawTerrainBottomPlaneTextureChanged(int state);         // thread safe (drawingStateMutex)
    void SLOTdrawTerrainNormalsChanged(int state);                  // thread safe (drawingStateMutex)
    void SLOTdrawEarthPointChanged(int state);                      // thread safe (drawingStateMutex)
    void SLOTdrawGridChanged(int state);                            // thread safe (drawingStateMutex)
    void SLOTdrawAxesChanged(int state);                            // thread safe (drawingStateMutex)
    void SLOTsunEnabledChanged(int state);                          // thread safe (drawingStateMutex)
    void SLOTtreeUpdatingChanged(int state);                        // thread safe (drawingStateMutex)
    void SLOTlodMultiplierIndexChanged(int index);                  // thread safe (drawingStateMutex)
    void SLOTdontUseDiskHgtChanged(int state);                      // thread safe (drawingStateMutex)
    void SLOTdontUseDiskRawChanged(int state);                      // thread safe (drawingStateMutex)
    void SLOTdontUseCacheChanged(int state);                        // thread safe (drawingStateMutex)

private:
    CCamera camera;
    QMutex *drawingStateMutex;
    bool drawTerrainPoint;
    bool drawTerrainPointColor;
    bool drawTerrainWire;
    bool drawTerrainWireColor;
    bool drawTerrainSolid;
    bool drawTerrainSolidStrip;
    bool drawTerrainSolidColor;
    bool drawTerrainTexture;
    bool drawTerrainTextureStrip;
    bool drawTerrainBottomPlaneWire;
    bool drawTerrainBottomPlaneWireColor;
    bool drawTerrainBottomPlaneSolid;
    bool drawTerrainBottomPlaneSolidColor;
    bool drawTerrainBottomPlaneTexture;
    bool drawTerrainNormals;
    bool drawEarthPoint;
    bool drawGrid;
    bool drawAxes;
    bool sunEnabled;
    bool treeUpdating;
    double lodMultiplier;
    bool dontUseDiskHgt;
    bool dontUseDiskRaw;
    bool dontUseCache;
};

#endif // CDRAWINGSTATE_H
