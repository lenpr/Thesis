#ifndef TOPSTOC_H
#define TOPSTOC_H

#include <cmath>
#include <OpenGL/gl.h> // change include for unix systems (ifdef)

#include <deque>		// double ended queue
#include <time.h>		// as random seed
#include <map>
#include <set>
#include <fstream>

// used to force precission output of float
//#include <iomanip>

#include "boundingbox.h"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>


struct MyTraits : public OpenMesh::DefaultTraits {
    FaceTraits {
    private:
        // face color RGB
        float fcR, fcG, fcB;
        bool selected;


    public:
        FaceT() : fcR(0.0), fcG(0.0), fcB(0.0), selected(false) {}

        void setColor(const float& R, const float& G, const float& B) {
            fcR = R; fcG = G; fcB = B;
        }
        const float getColorR() { return fcR; }
        const float getColorG() { return fcG; }
        const float getColorB() { return fcB; }

        bool isSelected() { return selected;}
        void setSelected(bool newSelected) { selected = newSelected; }
    };

    VertexTraits {
        // changing the std property does not work for me
        //typedef OpenMesh::Vec3f Color;

    private:
        float  weightValue;
        OpenMesh::VertexHandle seedVector;
        bool conquered;

    public:
        VertexT() : weightValue(0.0), seedVector(), conquered(false) { }

        const float& getWeight() const { return weightValue; }
        void setWeight(const float& passedValue) { weightValue=passedValue;}

        const OpenMesh::VertexHandle& getSeedVector() const { return seedVector; }
        void setSeedVector(const OpenMesh::VertexHandle& passedVertex) { seedVector=passedVertex; }

        bool isConquered() const { return conquered;}
        void setToConquered() { conquered = true; }
        void setToUnconquered() { conquered = false; }
    };
};

//typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits> MyPolyMesh;
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;


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

	void setModelBounds();
	void invertNormals();

    void drawMesh(bool vertexWeights, bool remeshedRegions);
	void drawDecimatedMesh(bool vertexWeights);
	void drawSamplAndControlPoints (bool sampledVertices, bool controlPoints);
	void test();

	// encapsulated in their classes
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
