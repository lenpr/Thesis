#include "boundingbox.h"

BoundingBox::BoundingBox() {}

// setter & getter

void BoundingBox::setMinPoint (const float& vecX, const float& vecY, const float& vecZ) {
	minPoint = Vec(vecX, vecY, vecZ);
}

void BoundingBox::setMaxPoint (const float& vecX, const float& vecY, const float& vecZ) {
	maxPoint = Vec(vecX, vecY, vecZ);
}

void BoundingBox::setCenterPoint (const float& vecX, const float& vecY, const float& vecZ) {
	centerPoint = Vec(vecX, vecY, vecZ);
}

Vec BoundingBox::getMinPoint () const {
	return minPoint;
}

Vec BoundingBox::getMaxPoint () const {
	return maxPoint;
}

Vec BoundingBox::getCenterPoint () const {
	return centerPoint;
}
