#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include "topstoc.h"


class Viewer : public QGLViewer {
    Q_OBJECT

public:
    Viewer();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    virtual void init();
    virtual void draw();
    //test
    //    virtual void fastDraw();

private:
    TopStoc topstoc;

    // for visualization settings
    int drawingMode;
    bool vertexWeights, sampledVertices, controlPoints, remeshedRegions, decimatedMesh;

    bool pickingEvent;
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent *k);

    void selectVertex(int x, int y);
    void sendCameraPosition ();

public slots:
    void loadModel(const QString& fileName);
    void saveModel(const QString& fileName);
    void visualization(	int drawingMode, bool vertexWeights, bool sampledVertices,
                        bool controlPoints, bool remeshedRegions,	bool decimatedMesh );
    void invertNormals();

    void stocWeights (const QString& mode);
    void stocSampling (const float& adaptivity, const float& subsetTargetSize);

    void topReMeshing (const QString& mode);

    void hausdorff(double sampling_density_user);

    void passToConsole(const QString& msg, int mode);
    void test();

signals:
    /* mode: 0 -> ALL
                     1 -> SAMPLING
                     2 -> INDEXING
                     3 -> DEBUG */
    void writeToConsole(const QString& msg, int mode);
    void updateViewVec(Vec viewVec);
    void meshstatus (int status);
};

#endif // VIEWER_H
