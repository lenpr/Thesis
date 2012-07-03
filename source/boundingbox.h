#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QGLViewer/vec.h>
#include "mymesh.h"

using qglviewer::Vec;

class BoundingBox {
public:
    BoundingBox();

    // setter & getter
    void setMinPoint(const float& vecX, const float& vecY, const float& vecZ) { minPoint = Vec(vecX, vecY, vecZ); }
    void setMaxPoint(const float& vecX, const float& vecY, const float& vecZ) { maxPoint = Vec(vecX, vecY, vecZ); }
    void setCenterPoint(const float& vecX, const float& vecY, const float& vecZ) { centerPoint = Vec(vecX, vecY, vecZ); }
    Vec getMinPoint() const { return minPoint; }
    Vec getMaxPoint() const { return maxPoint; }
    Vec getCenterPoint() const { return centerPoint; }
    float getDiagonal();
    Vec getCenterPoint();
    void calculateAll(const MyMesh& mesh);

private:
    Vec minPoint;
    Vec maxPoint;
    Vec centerPoint;
    float diagonal;
    bool centerPointStatus;

    void calcDiagonal();
    void calcCenterPoint();

    // Alternative to bool variables
    //std::bitset<3> calculationAssert;
    //calculationAssert[0-2]
};

#endif // BOUNDINGBOX_H
