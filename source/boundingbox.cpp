#include "boundingbox.h"

BoundingBox::BoundingBox() :
    minPoint(0.0, 0.0, 0.0),
    maxPoint(0.0, 0.0, 0.0),
    centerPoint(0.0, 0.0, 0.0),
    diagonal(-1.0f),
    centerPointStatus(false) {}

void BoundingBox::calculateAll(const MyMesh &mesh) {
    MyMesh::ConstVertexIter v_it=mesh.vertices_begin();
    minPoint.x = maxPoint.x = mesh.point(v_it)[0];
    minPoint.y = maxPoint.y = mesh.point(v_it)[1];
    minPoint.z = maxPoint.z = mesh.point(v_it)[2];
    ++v_it;

    for (; v_it!=mesh.vertices_end(); ++v_it){
        float x = mesh.point(v_it)[0];
        float y = mesh.point(v_it)[1];
        float z = mesh.point(v_it)[2];

        if (x < minPoint.x) minPoint.x = x;
        if (x > maxPoint.x) maxPoint.x = x;

        if (y < minPoint.y) minPoint.y = y;
        if (y > maxPoint.y) maxPoint.y = y;

        if (z < minPoint.z) minPoint.z = z;
        if (z > maxPoint.z) maxPoint.z = z;
    }

    calcDiagonal();
    calcCenterPoint();
}

void BoundingBox::calcDiagonal() {

    diagonal = (maxPoint-minPoint).norm();
}

void BoundingBox::calcCenterPoint() {

    centerPoint.x = minPoint.x + (fabs(maxPoint.x-minPoint.x))/2;
    centerPoint.y = minPoint.y + (fabs(maxPoint.y-minPoint.y))/2;
    centerPoint.z = minPoint.z + (fabs(maxPoint.z-minPoint.z))/2;
    centerPointStatus = true;
}

float BoundingBox::getDiagonal() {

    if ( diagonal < 0.0f )
        calcDiagonal();

    return diagonal;
}

Vec BoundingBox::getCenterPoint() {

    if (!centerPointStatus)
        calcCenterPoint();

    return centerPoint;
}
