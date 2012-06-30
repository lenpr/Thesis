#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QGLViewer/vec.h>

using qglviewer::Vec;

class BoundingBox {
public:
    BoundingBox();
		void setMinPoint(const float& vecX, const float& vecY, const float& vecZ);
		void setMaxPoint(const float& vecX, const float& vecY, const float& vecZ);
		void setCenterPoint(const float& vecX, const float& vecY, const float& vecZ);
		Vec getMinPoint() const;
		Vec getMaxPoint() const;
		Vec getCenterPoint() const;

private:
		Vec minPoint;
		Vec maxPoint;
		Vec centerPoint;
};

#endif // BOUNDINGBOX_H
