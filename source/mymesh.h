#ifndef MYMESH_H
#define MYMESH_H

// Open Mesh

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

//

#endif // MYMESH_H
