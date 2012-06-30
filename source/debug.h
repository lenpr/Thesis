#ifndef DEBUG_H
#define DEBUG_H


// for testing - include galore - todo get rid of the douplets
#include <QLineEdit>
#include <QtGui>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyPolyMesh;
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;


MyMesh debug_loadMesh () {
	MyMesh mesh;
	QString filePath = QFileDialog::getOpenFileName(
			0,
			"Choose a file to open:",
			QString::null,
			"Meshes (*.off *.obj *.stla *om .stl	*.stlb *.stl);;Text Files (*.txt)", 0,
			QFileDialog::ReadOnly);
	OpenMesh::IO::read_mesh(mesh, filePath.toStdString ());
	return mesh;
}

void debug_drawMesh (MyMesh mesh) {
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
		float x = mesh.point(v_it)[0];
		float y = mesh.point(v_it)[1];
		float z = mesh.point(v_it)[2];

		glVertex3f(x,y,z);
	}
	glEnd();
	glPointSize(1.0f);
}


/*
MyMesh makeMesh () {
	MyMesh mesh;

	// generate vertices

	MyMesh::VertexHandle vhandle[8];

	vhandle[0] = mesh.add_vertex(MyMesh::Point(-1, -1,  1));
	vhandle[1] = mesh.add_vertex(MyMesh::Point( 1, -1,  1));
	vhandle[2] = mesh.add_vertex(MyMesh::Point( 1,  1,  1));
	vhandle[3] = mesh.add_vertex(MyMesh::Point(-1,  1,  1));
	vhandle[4] = mesh.add_vertex(MyMesh::Point(-1, -1, -1));
	vhandle[5] = mesh.add_vertex(MyMesh::Point( 1, -1, -1));
	vhandle[6] = mesh.add_vertex(MyMesh::Point( 1,  1, -1));
	vhandle[7] = mesh.add_vertex(MyMesh::Point(-1,  1, -1));


	// generate (quadrilateral) faces

	std::vector<MyMesh::VertexHandle>  face_vhandles;

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[0]);
	face_vhandles.push_back(vhandle[1]);
	face_vhandles.push_back(vhandle[2]);
	face_vhandles.push_back(vhandle[3]);
	mesh.add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[7]);
	face_vhandles.push_back(vhandle[6]);
	face_vhandles.push_back(vhandle[5]);
	face_vhandles.push_back(vhandle[4]);
	mesh.add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[1]);
	face_vhandles.push_back(vhandle[0]);
	face_vhandles.push_back(vhandle[4]);
	face_vhandles.push_back(vhandle[5]);
	mesh.add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[2]);
	face_vhandles.push_back(vhandle[1]);
	face_vhandles.push_back(vhandle[5]);
	face_vhandles.push_back(vhandle[6]);
	mesh.add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[3]);
	face_vhandles.push_back(vhandle[2]);
	face_vhandles.push_back(vhandle[6]);
	face_vhandles.push_back(vhandle[7]);
	mesh.add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhandle[0]);
	face_vhandles.push_back(vhandle[3]);
	face_vhandles.push_back(vhandle[7]);
	face_vhandles.push_back(vhandle[4]);
	mesh.add_face(face_vhandles);

	return mesh;
}

void saveMeshToDisk (MyMesh mesh){
	// write mesh to output.obj
	if ( !OpenMesh::IO::write_mesh(mesh, "../../../output.off") )
		std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
}*/


#endif // DEBUG_H
