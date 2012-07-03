#ifndef TOPSTOC_H
#define TOPSTOC_H

#include <cmath>
#ifdef LINUX
    #include <GL/gl.h>
#else
    #include <OpenGL/gl.h>
#endif

#include <deque>		// double ended queue
#include <time.h>		// as random seed
#include <map>
#include <set>
#include <fstream>



#include "boundingbox.h"

// Housdorff distance
#include "compute_error.h"
#include "xalloc.h"



using std::string;


class TopStoc: public QObject {
	Q_OBJECT

public:
    TopStoc();

    bool loadMeshFromFile(const string& fileName);
    bool saveMeshToFile(const string& fileName);

    void initMesh();
    bool calculateWeights(const QString& mode);
    bool runStocSampling(const float& adaptivity, const float& subsetTargetSize);
    bool rayIntersectsTriangle(OpenMesh::Vec3f rayP1, OpenMesh::Vec3f rayP2);
    bool runTopReMeshing(const QString& mode);
    void calculateHausdorff(double sampling_density_user);

    void setModelBounds();
    void invertNormals();

    void drawMesh(bool vertexWeights, bool remeshedRegions);
    void drawDecimatedMesh(bool vertexWeights);
    void drawSamplAndControlPoints (bool sampledVertices, bool controlPoints);
    void test();

    BoundingBox bbox;

    void gl_select(int x, int y);
    void drawTriangles();

signals:
    void writeToConsole(const QString, int mode);

private:
	void colorizeMesh(int mode);
	float adaptivityCalculation();

	MyMesh mesh;
	OpenMesh::IO::Options opt;
	std::deque<MyMesh::VertexHandle> sampledVertexQueue;
	std::deque<MyMesh::VertexHandle> decimatedMesh;

	float minVertexWeight, maxVertexWeight, meanVertexWeight;
	int meshStatus;
};

#endif // TOPSTOC_H
