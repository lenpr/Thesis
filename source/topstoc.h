#ifndef TOPSTOC_H
#define TOPSTOC_H

#include <cmath>
#ifdef LINUX
    #include <GL/gl.h>
    #include <GL/glut.h>
#else
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#endif

#include <deque>		// double ended queue
#include <time.h>		// as random seed
#include <map>
#include <set>
#include <fstream>


#include "controlpanel.h"
#include "boundingbox.h"

// Tetrahedral creation
#include "tetgen.h"

// Housdorff distance
#include "compute_error.h"
#include "xalloc.h"

// topology
#include "topological_loops.h"

using std::string;


class TopStoc: public QObject {
	Q_OBJECT

public:
    TopStoc();

    bool loadMeshFromFile(const string& fileName);
    bool saveMeshToFile(const string& fileName);
    int getMeshStatus() {return meshStatus;}

    void initMesh();
    bool calculateWeights(const QString& mode);
    bool runStocSampling(const float& adaptivity, const float& subsetTargetSize);
    MyMesh::FaceHandle rayIntersectsTriangle(int x, int y);
    bool setUserWeights(MyMesh::FaceHandle selectedFace, float value, int mode);
    bool setUserWeights(float value); // for all selected faces
    void clearSelection();

    bool runTopReMeshing(const QString& mode);
    void calculateHausdorff(double sampling_density_user);

    void setModelBounds();
    void invertNormals();

    void drawMesh(bool vertexWeights, bool remeshedRegions);
    void drawDecimatedMesh(bool vertexWeights);
    void drawSamplAndControlPoints (bool sampledVertices, bool controlPoints, int boundaries);

    void filtrate();
    void findLoops();
    void killLoop(int loopIdx);
    void deleteFaces();

    void test();

    BoundingBox bbox;
    topological_loops loops;
    interactionVariables options;

    void gl_select(int x, int y);
    void drawTriangles();

signals:
    void writeToConsole(const QString, int mode);

private:
    void colorizeMesh();
	float adaptivityCalculation();

	MyMesh mesh;
	OpenMesh::IO::Options opt;
	std::deque<MyMesh::VertexHandle> sampledVertexQueue;
	std::deque<MyMesh::VertexHandle> decimatedMesh;

	float minVertexWeight, maxVertexWeight, meanVertexWeight;
	int meshStatus;
    int numberSelectedTriangles;
};

#endif // TOPSTOC_H
