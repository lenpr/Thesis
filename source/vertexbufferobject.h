#ifndef VERTEXBUFFEROBJECT_H
#define VERTEXBUFFEROBJECT_H

#include <OpenGL/gl.h> // change include for unix systems (ifdef)

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


struct VertexArray {
	float x, y, z;				// coord. vertex
	float nx, ny, nz;			// normal
	float cR, cG, cB;			// color
	float padding[7];
};

class VertexBufferObject {
public:
	VertexBufferObject();
	VertexBufferObject(const int& size);
	// produces crash if pointer being freed without being set before
	~VertexBufferObject();

	void makeVertexStructure (const MyMesh& mesh);
	void makeVBO ();
	void drawVBO ();

private:
	VertexArray *va;
};

#endif // VERTEXBUFFEROBJECT_H
