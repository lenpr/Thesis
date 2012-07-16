#ifndef TOPOLOGICAL_LOOPS_H
#define TOPOLOGICAL_LOOPS_H

#include <map>

#include "triangulate.h"

#include "mymesh.h"
#include "tetgen.h"

typedef std::map<MyMesh::EdgeHandle, int> eh_OMtoTG;
typedef std::map<MyMesh::FaceHandle, int> fh_OMtoTG;

// for the filtration of the inside and outside volume Meshes
struct tgF{
    tgF() : isPaired(false), positive(), surfaceFace(false), fh_tg(-1), fh_om(-1), eh0_tg(-1), eh1_tg(-1), eh2_tg(-1) {}

    bool isPaired;
    bool positive;
    bool surfaceFace;
    int fh_tg;
    int fh_om;
    int eh0_tg;
    int eh1_tg;
    int eh2_tg;
};

struct tgE{
    tgE() : age(-1), isPaired(false), positive(false), surfaceEdge(false), fh_tg(-1), fh_om(), eh_tg(-1), vh0_tg(-1), vh1_tg(-1), eh_om() {}

    int age;
    bool isPaired;
    bool positive;
    bool surfaceEdge;
    int fh_tg;
    MyMesh::FaceHandle fh_om;
    int eh_tg;
    int vh0_tg;
    int vh1_tg;
    MyMesh::EdgeHandle eh_om;
};

struct tgV{
    tgV() : isPaired(false), positive(true), eh_tg(-1), eh_om(), vh_tg(-1), vh_om() {}

    bool isPaired;
    bool positive;
    int eh_tg;
    MyMesh::EdgeHandle eh_om;
    int vh_tg;
    MyMesh::VertexHandle vh_om;
};

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

    void meshInside(MyMesh &mesh);

    int getGenus() { return genus; }
    int n_boundaries() { return boundaries; }
    int getBetti(int idx);

    void test(MyMesh &mesh);
    std::vector< std::set<int> > test2(MyMesh &mesh);
//    int getLoops;

private:
    int bettiNumber[3];
    int bettiNumberInside[3];
    int genus;
    int boundaries;
    bool hasBoundary;
    std::vector<loops> bdLoops;
//    std::vector<MyMesh::VertexHandle> vertex_n;
//    std::vector<MyMesh::EdgeHandle> edge_n;
//    std::vector<MyMesh::FaceHandle> face_n;
    void cout_tg(tgV &vertex);
    void cout_tg(tgE &edge);
    void cout_tg(tgF &face);

    OpenMesh::VPropHandleT<bool> vPositive;
    OpenMesh::EPropHandleT<bool> ePositive;
    OpenMesh::FPropHandleT<bool> fPositive;

    OpenMesh::VPropHandleT<MyMesh::EdgeHandle> vPair;
    OpenMesh::EPropHandleT<MyMesh::FaceHandle> ePair;

    std::set<MyMesh::VertexHandle> boundaryVertices;

    tetgenio meshI, meshO;
    std::vector<tgV> vertices, verticesUnpaired, verticesNew;
    std::vector<tgE> edges, edgesUnpaired, edgesNew;
    std::vector<tgF> faces, facesUnpaired, facesNew;
};

#endif // TOPOLOGICAL_LOOPS_H
