#include "vertexbufferobject.h"

VertexBufferObject::VertexBufferObject() : va(NULL) {}

VertexBufferObject::VertexBufferObject(const int& size) {
	va = new VertexArray[size];
}

VertexBufferObject::~VertexBufferObject() {
	if (va != 0)
		delete[] va;
}

void VertexBufferObject::makeVBO () {
	GLuint verticesVBO, indicesVBO;
	glGenBuffers (1, &verticesVBO);

	glBindBuffer (GL_ARRAY_BUFFER, verticesVBO);
	//glBufferData (GL_ARRAY_BUFFER, mesh.n_vertices()*16*sizeof(float), va, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, 0);

	glGenBuffers (1, &indicesVBO);
	//glBufferData (GL_ARRAY_BUFFER, 3*sizeof(unsigned));
	//std::cout<<"Info: "<<sizeof(va)<<","<<sizeof(*va)<<","<<sizeof(&va)<<std::endl;

	// VBO for the indices
	//glGenBuffers (1, &indexVBOid);
	//glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexVBOid);
	//glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(int), NULL, GL_STATIC_DRAW);
}

void VertexBufferObject::makeVertexStructure (const MyMesh& mesh) {
	va = new VertexArray[mesh.n_vertices()];

	int i=0;
	for (MyMesh::ConstVertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
		va[i].x  = mesh.point(v_it)[0];
		va[i].y  = mesh.point(v_it)[1];
		va[i].z  = mesh.point(v_it)[2];

		va[i].nx = mesh.normal(v_it)[0];
		va[i].ny = mesh.normal(v_it)[1];
		va[i].nz = mesh.normal(v_it)[2];

		va[i].cR = mesh.color(v_it)[0];
		va[i].cG = mesh.color(v_it)[1];
		va[i].cB = mesh.color(v_it)[2];

		++i;
	}
}

void VertexBufferObject::drawVBO () {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	//glVertexPointer(3, GL_FLOAT, 0, &g_vbo_vertices[0][0]);
	//glNormalPointer(GL_FLOAT, 0, &g_vbo_normals[0][0]);
	//glDrawElements(GL_TRIANGLES, 3*g_vbo_indices.size(), GL_UNSIGNED_INT, &g_vbo_indices[0][0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}
