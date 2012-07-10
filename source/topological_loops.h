#ifndef TOPOLOGICAL_LOOPS_H
#define TOPOLOGICAL_LOOPS_H

#include "triangulate.h"

#include "mymesh.h"

struct loops {
    std::vector<MyMesh::VertexHandle> bdVertices;
};


class topological_loops
{
public:
    topological_loops();
    void init(MyMesh &mesh);
    void pairing(MyMesh &mesh);
    void findBoundaries(MyMesh &mesh);
    void triangulateBd(MyMesh &mesh, int bdIdx);

    int getGenus() { return genus; }
    int n_boundaries() { return boundaries; }
    int getBetti(int idx);

    void test();
//    int getLoops;

private:

    int bettiNumber[3];
    int genus;
    int boundaries;
    bool hasBoundary;
    std::vector<loops> bdLoops;
    std::vector<MyMesh::VertexHandle> vertex_n;
    std::vector<MyMesh::EdgeHandle> edge_n;
    std::vector<MyMesh::FaceHandle> face_n;

    OpenMesh::VPropHandleT<bool> vPositive;
    OpenMesh::EPropHandleT<bool> ePositive;
    OpenMesh::FPropHandleT<bool> fPositive;

    OpenMesh::VPropHandleT<MyMesh::EdgeHandle> vPair;
    OpenMesh::EPropHandleT<MyMesh::FaceHandle> ePair;

    std::set<MyMesh::VertexHandle> boundaryVertices;
};

#endif // TOPOLOGICAL_LOOPS_H
