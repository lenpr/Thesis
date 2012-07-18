#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include "topstoc.h"

using namespace qglviewer;

#define DRAW_MODE_POINT 0
#define DRAW_MODE_WIREFRAME 1
#define DRAW_MODE_FLAT 2
#define DRAW_MODE_SMOOTH 3
#define DRAW_MODE_TEXTURE 4

class Viewer : public QGLViewer {
    Q_OBJECT

public:
    Viewer();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    virtual void init();
    virtual void draw();

private:
    TopStoc topstoc;

    // for visualization settings
    int drawingMode;
    GLuint drawList;
    bool vertexWeights, sampledVertices, controlPoints, remeshedRegions, decimatedMesh, displayUpdate, showHausdorff;
    int showBoundaries;
    int currentTri;
    Vec defaultCameraPosition;
    Quaternion defaultCameraOrientation;

    bool pickingEvent;

    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *k);

    void updateDisplay();
    void selectVertex(int x, int y, int mode);
    void sendCameraPosition ();

public slots:
    void loadModel(const QString& fileName);
    void saveModel(const QString& fileName);
    void visualization(	int drawingMode, bool vertexWeights, bool sampledVertices,
                        bool controlPoints, bool remeshedRegions,	bool decimatedMesh, bool displayUpdate, int showBoundaries, bool showHausdorff);
    void interaction(interactionVariables currentOptions);
    void invertNormals();

    void stocWeights (const QString& mode);
    void stocSampling (const float& adaptivity, const float& subsetTargetSize);

    void topReMeshing (const QString& mode);

    void hausdorff(double sampling_density_user);
    void turntable();
    void turntableSpun();

    void filtrate();
    void findloops();
    void killLoop();

    void passToConsole(const QString& msg, int mode);

    void test();

signals:
    /* mode: 0 -> ALL
             1 -> SAMPLING
             2 -> INDEXING
             3 -> DEBUG
    */
    void writeToConsole(const QString& msg, int mode);
    void updateViewVec(Vec viewVec);
    void meshstatus (int status);
};

#endif // VIEWER_H
