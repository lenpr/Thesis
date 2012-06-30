#include "topstoc.h"
#include <GLUT/glut.h>


TopStoc::TopStoc() :
		mesh(),
		sampledVertexQueue(),
		decimatedMesh(),
		minVertexWeight(1.0),
		maxVertexWeight(0.0),
		meanVertexWeight(0.0),
		meshStatus(0) {}

bool TopStoc::loadMeshFromFile (const string &fileName) {
	minVertexWeight = 1.0;
	maxVertexWeight = 0.0;
	meanVertexWeight = 0.0;
	meshStatus = 0;

	return OpenMesh::IO::read_mesh(mesh, fileName, opt);
}

bool TopStoc::saveMeshToFile (const string &fileName) {

	if ( decimatedMesh.empty() ) {
		return OpenMesh::IO::write_mesh(mesh, fileName);
	} else {

		MyMesh meshToStore;

		typedef std::map<MyMesh::Point, MyMesh::VertexHandle> MapType;
		MapType pointList;
		MapType::iterator it;

		for (int n = 0;
                 n < (int)decimatedMesh.size();
				 ++n) {

					MyMesh::VertexHandle vh = decimatedMesh.at(n);
					MyMesh::Point point = mesh.point(vh);

					it = pointList.find(point);
					if ( it != pointList.end () ) {
						vh = (*it).second;
					} else {
						vh = meshToStore.add_vertex(point);
						pointList.insert (
			std::pair<MyMesh::Point, MyMesh::VertexHandle> (point, vh));
					}
		}

		for (int n = 0;
                 n < (int)decimatedMesh.size()/3;
				 ++n) {

			MyMesh::Point point0 = mesh.point( decimatedMesh.at((3*n)+0) );
			MyMesh::Point point1 = mesh.point( decimatedMesh.at((3*n)+1) );
			MyMesh::Point point2 = mesh.point( decimatedMesh.at((3*n)+2) );


			MyMesh::VertexHandle vh0;
			MyMesh::VertexHandle vh1;
			MyMesh::VertexHandle vh2;

			it = pointList.find(point0);
			if ( it != pointList.end () ) {
				vh0 = (*it).second;
			} else { std::cerr << "error point not found"; }
			it = pointList.find(point1);
			if ( it != pointList.end () ) {
				vh1 = (*it).second;
			} else { std::cerr << "error point not found"; }
			it = pointList.find(point2);
			if ( it != pointList.end () ) {
				vh2 = (*it).second;
			} else { std::cerr << "error point not found"; }

			if ((vh0 == vh1) || (vh1 == vh2) || (vh0 == vh2))
				std::cerr << "error complex edge" << std::endl;
			meshToStore.add_face(vh2, vh1, vh0);
		}

		std::cout << "l: " << pointList.size() << std::endl;
		std::cout << "s: " << meshToStore.n_faces() << std::endl;

		OpenMesh::IO::write_mesh(meshToStore, fileName);
	}

    return true;
}

void TopStoc::setModelBounds () {

	MyMesh::ConstVertexIter v_it=mesh.vertices_begin();

	float xMin = mesh.point(v_it)[0]; float xMax = mesh.point(v_it)[0];
	float yMin = mesh.point(v_it)[1]; float yMax = mesh.point(v_it)[1];
	float zMin = mesh.point(v_it)[2]; float zMax = mesh.point(v_it)[2];
	++v_it;

	for (; v_it!=mesh.vertices_end(); ++v_it){
		float x = mesh.point(v_it)[0];
		float y = mesh.point(v_it)[1];
		float z = mesh.point(v_it)[2];

		if (x < xMin) xMin = x;
		if (x > xMax) xMax = x;

		if (y < yMin) yMin = y;
		if (y > yMax) yMax = y;

		if (z < zMin) zMin = z;
		if (z > zMax) zMax = z;
	}

	bbox.setMinPoint (xMin, yMin, zMin);
	bbox.setMaxPoint (xMax, yMax, zMax);

	float deltaX = fabs(xMax - xMin);
	float deltaY = fabs(yMax - yMin);
	float deltaZ = fabs(zMax - zMin);

	bbox.setCenterPoint ((xMin+(deltaX/2)), yMin+(deltaY/2), zMin+(deltaZ/2));
}



void TopStoc::drawSamplAndControlPoints (bool sampledVertices, bool controlPoints) {

	glDisable(GL_LIGHTING);
	glPointSize(3.0f);
	glBegin(GL_POINTS);

	for (int i=0; i<(int)sampledVertexQueue.size(); ++i){
		if (controlPoints && (mesh.data( sampledVertexQueue[i] ).getWeight() > 1.0)) {
			glColor3f(0.20, 0.80, 0.20);
			glVertex3f(	mesh.point( sampledVertexQueue[i] )[0],
									mesh.point( sampledVertexQueue[i] )[1],
									mesh.point( sampledVertexQueue[i] )[2]);
		}
		if (sampledVertices && (mesh.data( sampledVertexQueue[i] ).getWeight() <= 1.0)) {
			glColor3f(1.0, 0.25, 0.25);
			glVertex3f(	mesh.point( sampledVertexQueue[i] )[0],
									mesh.point( sampledVertexQueue[i] )[1],
									mesh.point( sampledVertexQueue[i] )[2]);
		}
	}
	glEnd();
	glEnable (GL_LIGHTING);
}


/*
void TopStoc::list_hits(GLint hits, GLuint *names) {
        int i;

        printf("%d hits:\n", hits);

        for (i = 0; i < hits; i++)
            printf(	"Number: %d\n"
                    "Min Z: %d\n"
                    "Max Z: %d\n"
                    "Name on stack: %d\n",
                    (GLubyte)names[i * 4],
                    (GLubyte)names[i * 4 + 1],
                    (GLubyte)names[i * 4 + 2],
                    (GLubyte)names[i * 4 + 3]
                    );

        printf("\n");
}*/

/*
void TopStoc::gl_select(int x, int y)
 {
    GLuint buff[512] = {0};
    GLint hits, view[4];

    glSelectBuffer(512, buff);

    glGetIntegerv(GL_VIEWPORT, view);

    glRenderMode(GL_SELECT);

    glInitNames();

    glPushName(0);


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();

        gluPickMatrix(x, view[3]-y, 5, 5,view);
        gluPerspective(120, 1.0, 0.0001, 1000.0);

        glMatrixMode(GL_MODELVIEW);

        drawTriangles();

        glMatrixMode(GL_PROJECTION);
    glPopMatrix();

glMatrixMode(GL_PROJECTION);

    hits = glRenderMode(GL_RENDER);

    list_hits(hits, buff);

    glMatrixMode(GL_MODELVIEW);
 }

*/

void TopStoc::drawDecimatedMesh (bool vertexWeights) {
	for (int i=0; i< (int)decimatedMesh.size(); ++i){
		if (vertexWeights) {
			glColor3f( 	mesh.color( decimatedMesh[i] )[0],
									mesh.color( decimatedMesh[i] )[1],
									mesh.color( decimatedMesh[i] )[2]);
		} else {
			glColor3f(0.64, 0.70, 0.80);
		}
		glNormal3f(	mesh.normal( decimatedMesh[i] )[0],
								mesh.normal( decimatedMesh[i] )[1],
								mesh.normal( decimatedMesh[i] )[2]);
		glVertex3f(	mesh.point( decimatedMesh[i] )[0],
								mesh.point( decimatedMesh[i] )[1],
								mesh.point( decimatedMesh[i] )[2]);
	}
}

void TopStoc::drawMesh(bool vertexWeights, bool remeshedRegions) {

    //debug later
    remeshedRegions = true;

    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        // run along the face vertices and draw them
        for (MyMesh::FaceVertexIter fv_it=mesh.fv_iter(f_it.handle()); fv_it; ++fv_it) {

            if (mesh.data(f_it.handle()).isSelected()) {
                glColor3f(0.0f, 1.0f, 0.0f);
            } else if (vertexWeights) {
                glColor3f( 	mesh.color(fv_it)[0],
                            mesh.color(fv_it)[1],
                            mesh.color(fv_it)[2]);
            } else {
                glColor3f(0.5, 0.5, 0.5);
            }
            glNormal3f(	mesh.normal(fv_it)[0],
                        mesh.normal(fv_it)[1],
                        mesh.normal(fv_it)[2]);
            glVertex3f(	mesh.point(fv_it)[0],
                        mesh.point(fv_it)[1],
                        mesh.point(fv_it)[2]);
            /*
            glPushMatrix();
            glLoadIdentity();
            glTranslatef(mesh.point(fv_it)[0],
                         mesh.point(fv_it)[1],
                         mesh.point(fv_it)[2]);

           glutWireSphere(0.1, 6, 6);
           glPopMatrix();
           */
        }
    }
}


void TopStoc::drawTriangles() {

    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {

        bool selected = mesh.data(f_it.handle()).isSelected();

        if (selected)
            glColor3f(0.0f, 1.0f, 0.0f);
        else
            glColor3f(0.5f, 0.5f, 0.5f);

        glBegin(GL_TRIANGLES);

        for (MyMesh::FaceVertexIter fv_it=mesh.fv_iter(f_it.handle()); fv_it; ++fv_it) {
            glNormal3f(	mesh.normal(fv_it)[0], mesh.normal(fv_it)[1], mesh.normal(fv_it)[2]);
            glVertex3f(	mesh.point(fv_it)[0], mesh.point(fv_it)[1], mesh.point(fv_it)[2]);
        }

        glEnd();
    }
}

void TopStoc::initMesh (){
	// we want to use the following attributes of OpenMesh
	mesh.request_vertex_normals();
	mesh.request_vertex_colors();

	// if the file did not provide vertex normals, then calculate them
	if ( !opt.check( OpenMesh::IO::Options::VertexNormal ) ) {
		writeToConsole ("generating new normals",2);

		// we need face normals to update the vertex normals
		mesh.request_face_normals();

		// let the mesh update the normals
		mesh.update_normals();

		// dispose the face normals, as we don't need them anymore
		//mesh.release_face_normals();
	} else {
		writeToConsole ("skipped normals generation",2);
	}
	// if more than one model it is important to reset
	meshStatus=0;

	// send infos to console
	// number of vertices base mesh
	writeToConsole (QString::number( mesh.n_vertices() ), 4);
	// set sampled and ratio to "-"
	writeToConsole ("-", 5);
	writeToConsole ("-", 6);
}


bool TopStoc::calculateWeights (const QString& mode) {
	// make the compiler happy, TODO: use it
	mode.toStdString();

	// for counting problematic vertex normals
	int skipped = 0;

	// (linearly) iterate over all vertices, collect 1-ring weights
	for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {

		// to normalize the result
		//int valence = 0; // maybe a bug hides here!
		float weight = 0.0;

		// circulate around the vertex
		for (MyMesh::VertexVertexIter vv_it=mesh.vv_iter(v_it.handle()); vv_it; ++vv_it) {

			// vector product (scalar)
			float loc_weight = (mesh.normal(v_it) | mesh.normal(vv_it));

			// if normals are not accurate enough

			if (loc_weight > 1.0) {
				++skipped;
				loc_weight = 1.0;
			}
			if (loc_weight < -1.0) {
				++skipped;
				loc_weight = -1.0;
			}

			// transpose from [-1,1]->[0,1]
			weight += (1-loc_weight)/2;
			//++valence;
		}
		// calculate result and save to custom vertex variable
		weight = (weight/mesh.valence(v_it));

		meanVertexWeight += weight;

		mesh.data(v_it).setWeight(weight);


		if (minVertexWeight > weight) minVertexWeight = weight;
		if (maxVertexWeight < weight) maxVertexWeight = weight;
	}
	meanVertexWeight /= mesh.n_vertices();

	// debugoutput about badly calculated normals
	if (skipped > 0)
		writeToConsole ("Problematic normals: "+ QString::number(skipped) ,3);

	// histogramm and colors, for output set mode=1
	colorizeMesh(1);


	QString consoleMessage = "- model mean weight value: " + QString::number(meanVertexWeight)
													 + "\n min.: " + QString::number(minVertexWeight)
													 + " / " + QString::number(minVertexWeight/meanVertexWeight)
													 + "(to mean ratio)"
													 + "\n max.: " + QString::number( (maxVertexWeight) )
													 + " / " + QString::number( (maxVertexWeight/meanVertexWeight) )
													 + "(to mean ratio)";
	writeToConsole (consoleMessage, 1);

	return (true);
}


bool TopStoc::runStocSampling (const float& adaptivity, const float& subsetTargetSize) {

	// clear DOH
	// sampledVertexQueue.clear();

	QString consoleMessage = "passed target size: 0." + QString::number(subsetTargetSize)
													 + ", adaptivity: " + QString::number(adaptivity);
	writeToConsole (consoleMessage, 1);

	float alpha = adaptivity;
	float k = subsetTargetSize/100.0;
	srand( (unsigned)time(0) );

	MyMesh::VertexIter v_it = mesh.vertices_begin();
	int sampled_vertices = 0, sampled_smalls = 0;
	float probability = 0.0;
	sampledVertexQueue.clear();

	for (; 	v_it != mesh.vertices_end(); ++v_it){
		if (mesh.data(v_it).getWeight() >= 1.0) {
			// add to queue
			sampledVertexQueue.push_front( v_it );
			++sampled_vertices;
		} else {
			probability = k*(1+alpha*((mesh.data(v_it).getWeight()/meanVertexWeight)-1));

            /* View dependency
            float x = 0.5774;
            MyMesh::Point viewDirection = MyMesh::Point(x, 0.5774, 0.5774);
            if ( (mesh.normal(v_it) | viewDirection) < -0.1) {
                probability /= 20;
            }
            if ( (mesh.normal(v_it) | viewDirection) > -0.01 &&
                     (mesh.normal(v_it) | viewDirection) < +0.01) {
                probability *= 20;
            }*/

			if (probability > ((float)rand()/(float)RAND_MAX) ) {
				++sampled_vertices;
				// add to queue
				sampledVertexQueue.push_front( v_it );
				if (mesh.data(v_it).getWeight() < meanVertexWeight)
					++sampled_smalls;
			}
		}
	}


	/* set a few "user contol points" ... dangerous code
	for (int i=31; i<357; ++i) {
		mesh.data( mesh.vertex_handle(i*2) ).setWeight( 1.1 );
	} */

	consoleMessage = "sample ratio: " + QString::number((float)sampled_vertices/mesh.n_vertices())
									 + ", smaller than mean weight vertices of the set in %: "
									 + QString::number(((float)sampled_smalls/(float)sampled_vertices));
	writeToConsole (consoleMessage, 1);

	// send infos to console
	// number of vertices base mesh
	writeToConsole (QString::number( sampled_vertices ), 5);
	writeToConsole (QString::number( (int)(100*((float)sampled_vertices/(float)mesh.n_vertices())) ) + " [in %]", 6);

	meshStatus = 3;
	return (true);
}


bool TopStoc::rayIntersectsTriangle(OpenMesh::Vec3f rayP1, OpenMesh::Vec3f rayP2) {


    // camera pos berechnen

    double mvmatrix[16];
    double projmatrix[16];
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

    double camX, camY, camZ;
    gluUnProject (((double) viewport[2] - viewport[0]) / 2.0, ((double) viewport[3] - viewport[1]) / 2.0, 0.0, mvmatrix, projmatrix, viewport, &camX, &camY, &camZ);

    OpenMesh::Vec3f camPos(camX, camY, camZ);

    //


     bool firstIntersection = true;
     //OpenMesh::Vec3f pp;

     float intersectingFaceCamDist = 0.0f;
     OpenMesh::FaceHandle intersectionFace;

    int hits = 0;
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        //f_it.handle();



        MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(f_it);
        MyMesh::VertexHandle vh1 = cfv_it;
        MyMesh::VertexHandle vh2 = (++cfv_it);
        MyMesh::VertexHandle vh3 = (++cfv_it);

        //f_it.normal;
        //mesh.normal(f_it);

        //OpenMesh::Vec3f dist1 = (mesh.point(vh1) - mesh.point(vh2));
        //dist1 = dist1 * (mesh.normal(f_it));
        //float dist1 = (mesh.point(vh1))[0];

        float dist1 = OpenMesh::dot(rayP1 - mesh.point(vh1), mesh.normal(f_it));
        float dist2 = OpenMesh::dot(rayP2 - mesh.point(vh1), mesh.normal(f_it));

        if (dist1 * dist2 >= 0.0f || dist1 == dist2) continue;


        OpenMesh::Vec3f intersectionPoint = rayP1 + (rayP2 - rayP1) * (-dist1/(dist2 - dist1));


        OpenMesh::Vec3f test;

        test = OpenMesh::cross(mesh.normal(f_it), (mesh.point(vh2) - mesh.point(vh1)));
        if (OpenMesh::dot(test, intersectionPoint - mesh.point(vh1)) < 0.0f) continue;

        test = OpenMesh::cross(mesh.normal(f_it), (mesh.point(vh3) - mesh.point(vh2)));
        if (OpenMesh::dot(test, intersectionPoint - mesh.point(vh2)) < 0.0f) continue;

        test = OpenMesh::cross(mesh.normal(f_it), (mesh.point(vh1) - mesh.point(vh3)));
        if (OpenMesh::dot(test, intersectionPoint - mesh.point(vh1)) < 0.0f) continue;

        float currentIntersectionPointSQ = pow(intersectionPoint[0] - camPos[0], 2) +
                                         pow(intersectionPoint[1] - camPos[1], 2) +
                                         pow(intersectionPoint[2] - camPos[2], 2);



        if (firstIntersection || currentIntersectionPointSQ < intersectingFaceCamDist) {
            intersectingFaceCamDist = currentIntersectionPointSQ;
            intersectionFace = f_it.handle();
            firstIntersection = false;
        }
        hits++;
    }


    if (hits > 0) {

        MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(intersectionFace);
        MyMesh::VertexHandle vha = cfv_it;
        MyMesh::VertexHandle vhb = (++cfv_it);
        MyMesh::VertexHandle vhc = (++cfv_it);

        if ( mesh.data(intersectionFace).isSelected() ){

            std::vector<MyMesh::VertexHandle> vh;
            vh.push_back(vha);
            vh.push_back(vhb);
            vh.push_back(vhc);

            while ( !vh.empty() ) {
                // circulate around the vertex
                MyMesh::VertexVertexIter vv_it=mesh.vv_iter(vh.back());
                MyMesh::VertexHandle v_it = vh.back();
                vh.pop_back();
                float weight = 0.0f;

                for (; vv_it; ++vv_it) {
                    // vector product (scalar)
                    float loc_weight = (mesh.normal(v_it) | mesh.normal(vv_it));

                    // if normals are not accurate enough
                    if (loc_weight > 1.0)
                        loc_weight = 1.0;
                    if (loc_weight < -1.0)
                        loc_weight = -1.0;

                    // transpose from [-1,1]->[0,1]
                    weight += (1-loc_weight)/2;
                }
                // calculate result and save to custom vertex variable
                weight = (weight/mesh.valence(v_it));
                mesh.data(v_it).setWeight(weight);
            }

            mesh.data(intersectionFace).setSelected(false);
        }
        else {
            mesh.data(vha).setWeight(1.0f);
            mesh.data(vhb).setWeight(1.0f);
            mesh.data(vhc).setWeight(1.0f);

            mesh.data(intersectionFace).setSelected(true);
        }
    }

    std::cout << "hits: " << hits << std::endl;

    // color triangle

    //mesh.color()
    //mesh.color(fv_it)[0], mesh.color(fv_it)[1],  mesh.color(fv_it)[2]
      /*
    MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(f_it);
            MyMesh::VertexHandle vh1 = cfv_it;
            MyMesh::VertexHandle vh2 = (++cfv_it);
            MyMesh::VertexHandle vh3 = (++cfv_it);*/
    return true;
}


bool TopStoc::runTopReMeshing (const QString &mode) {

	// make the compiler happy, TODO: use it
	writeToConsole ("start remeshing §" + mode, 2);

	decimatedMesh.clear ();
	std::deque<MyMesh::VertexHandle> sampledVertexQueueCopy;
	sampledVertexQueueCopy = sampledVertexQueue;
	MyMesh::VertexHandle mark;

	// init mesh
	for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
		mesh.data(v_it).setToUnconquered();
	}
	// set all seeds
	for (int i=0; i<(int)sampledVertexQueueCopy.size(); ++i) {
		mark = sampledVertexQueueCopy.at(i);
		mesh.data(mark).setSeedVector(mark);
		mesh.data(mark).setToConquered();

		// to see the set marks
		//std::cout << i << ". VH: " << mark << std::endl;
	}

	// all vertices get assigned to a seed vertex
	int conquered = (int)sampledVertexQueueCopy.size();

	//test
	std::set<MyMesh::FaceHandle> facesToCheck;
	std::set<MyMesh::FaceHandle>::iterator it;

	while ( !sampledVertexQueueCopy.empty() ) {
		// get front
		MyMesh::VertexHandle vh = sampledVertexQueueCopy.front ();
		mark = mesh.data(vh).getSeedVector();

		for (MyMesh::ConstVertexOHalfedgeIter vhe_it=mesh.cvoh_iter(vh); vhe_it; ++ vhe_it) {
			MyMesh::VertexHandle vh = mesh.to_vertex_handle(vhe_it);
			// if not taken set mark
			if ( !mesh.data( vh ).isConquered() ) {
				mesh.data( vh ).setSeedVector(mark);
				mesh.data( vh ).setToConquered();
				++conquered;
				sampledVertexQueueCopy.push_back (vh);
			}  else {
				// check if vertex is our own
				// if not put it in the face queue
				if ( !(mesh.data(vh).getSeedVector() == mark) ) {

					MyMesh::FaceHandle f1 = mesh.face_handle(vhe_it.handle());
					MyMesh::FaceHandle f2 = mesh.face_handle(mesh.opposite_halfedge_handle(vhe_it.handle()));

					facesToCheck.insert (f1);
					facesToCheck.insert (f2);
				}
			}
			//---
		}
		/*
		for (MyMesh::ConstVertexVertexIter vv_it=mesh.cvv_iter(vh); vv_it; ++vv_it) {
			// if not taken set mark
			if ( !mesh.data(vv_it).isConquered() ) {
				mesh.data( vv_it ).setSeedVector(mark);
				mesh.data( vv_it ).setToConquered();
				++conquered;
				sampledVertexQueueCopy.push_back (vv_it);
			}  else {
				// check if vertex is our own
				// if not put it in the face queue
				if ( !(mesh.data(vv_it).getSeedVector() == mark) ) {
					MyMesh::HalfedgeHandle e12 = mesh.find_halfedge(vh, vv_it);
					MyMesh::FaceHandle f1 = mesh.face_handle(e12);
					MyMesh::FaceHandle f2 = mesh.face_handle(mesh.opposite_halfedge_handle(e12));

					facesToCheck.insert (f1);
					facesToCheck.insert (f2);
				}
			}
			//---
		} */
		sampledVertexQueueCopy.pop_front ();
	}
/*
	std::set<MyMesh::VertexHandle> difMarks;
	MyMesh::VertexIter v_it = mesh.vertices_begin();
	for (; 	v_it != mesh.vertices_end(); ++v_it){
			difMarks.insert (v_it);
	}
		std::cout << "difMarks: " << difMarks.size () << std::endl;
*/
	std::cout << "Flood filled: " << conquered << " of " << mesh.n_vertices() << std::endl;


	MyMesh::VertexHandle seedA, seedB, seedC;
	MyMesh::VertexHandle vhA, vhB, vhC;

	std::cout << "Savings: " << facesToCheck.size() << "/" << mesh.n_faces() << std::endl;

	int count1 = 0;
	int count2 = 0;
	int count3 = 0;

	// changed to queue
	//for (	it=facesToCheck.begin (); it!=facesToCheck.end (); ++it) {
	// not so fast but stable!
	for (MyMesh::ConstFaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {

		//MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(*it);
		MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(f_it);
		vhA = cfv_it;
		vhB = (++cfv_it);
		vhC = (++cfv_it);
		seedA = mesh.data( vhA ).getSeedVector();
		seedB = mesh.data( vhB ).getSeedVector();
		seedC = mesh.data( vhC ).getSeedVector();


		if (		(seedA != seedB) && (seedB != seedC) && (seedA != seedC) ) {

			decimatedMesh.push_front ( seedA );
			decimatedMesh.push_front ( seedB );
			decimatedMesh.push_front ( seedC );

			mesh.set_color(vhA, MyMesh::Color(1.0,0.0,0.0));
			mesh.set_color(vhB, MyMesh::Color(1.0,0.0,0.0));
			mesh.set_color(vhC, MyMesh::Color(1.0,0.0,0.0));
			++count1;
		}
		else if (		((seedA == seedB) && (seedB == seedC)) ) {
			// transitive so no need to check for (A == C)
			mesh.set_color(vhA, MyMesh::Color(0.0,1.0,0.0));
			mesh.set_color(vhB, MyMesh::Color(0.0,1.0,0.0));
			mesh.set_color(vhC, MyMesh::Color(0.0,1.0,0.0));
			++count2;
		}
		else {
//			case:
//			(	((seedA != seedB) && (seedB != seedC) && (seedA == seedC)) ||
//			((seedA != seedB) && (seedB == seedC) && (seedA != seedC)) ||
//			((seedA == seedB) && (seedB != seedC) && (seedA != seedC)) ) {

			mesh.set_color(vhA, MyMesh::Color(0.0,0.0,1.0));
			mesh.set_color(vhB, MyMesh::Color(0.0,0.0,1.0));
			mesh.set_color(vhC, MyMesh::Color(0.0,0.0,1.0));
			++count3;
		}
	}

	std::cout << "blau: " << count1 << std::endl;
	std::cout << "grün: " << count2 << std::endl;
	std::cout << "grau: " << count3 << std::endl;
	std::cout << "check: " << count1+count2+count3 << std::endl;
	std::cout << "faces: " << mesh.n_faces() << std::endl;

	QString consoleMessage = "finished remashing, new mesh is "
													 + QString::number((float)((float)decimatedMesh.size()/3)/mesh.n_faces())
													 + "% of base the mesh";
	writeToConsole (consoleMessage, 2);
	meshStatus = 1;

	return (true);
}


void TopStoc::colorizeMesh (int mode) {

	// calculate histogramm and color the vertices
	int quantile[6] = {0,0,0,0,0,0};

	float upperDelta = (maxVertexWeight-meanVertexWeight)/4;
	float lowerDelta = (meanVertexWeight-minVertexWeight)/4;
	// sheme of the spliting [-1-|-2-|-3-|-4-ǁ-5-| ---6--- ]

	MyMesh::VertexIter v_it = mesh.vertices_begin();
	for (; 	v_it != mesh.vertices_end(); ++v_it){
		float value = mesh.data(v_it).getWeight();
		if(value <= (minVertexWeight+lowerDelta)) {
			mesh.set_color(v_it, MyMesh::Color(0.5,0.5,0.5));
			++quantile[0];
		}
		if(value >= (minVertexWeight+lowerDelta) && value < (minVertexWeight+lowerDelta)*2) {
			mesh.set_color(v_it, MyMesh::Color(255,0,255));
			++quantile[1];
		}
		if(value >= (minVertexWeight+lowerDelta)*2 && value < (meanVertexWeight-lowerDelta)) {
			mesh.set_color(v_it, MyMesh::Color(0,255,0));
			++quantile[2];
		}
		if(value >= (meanVertexWeight-lowerDelta) && value < meanVertexWeight) {
			mesh.set_color(v_it, MyMesh::Color(0,255,255));
			++quantile[3];
		}
		if(value >= meanVertexWeight && value < (meanVertexWeight+upperDelta)) {
			mesh.set_color(v_it, MyMesh::Color(255,255,0));
			++quantile[4];
		}
		if(value >= (meanVertexWeight+upperDelta) && value <= maxVertexWeight) {
			mesh.set_color(v_it, MyMesh::Color(255,0,0));
			++quantile[5];
		}
	}

	// enable drawing the new vertices
	meshStatus = 1;

	// debug information
	if(mode == 1) {
		float sum = mesh.n_vertices();
		std::cout	<< "   1Q [ 0.0-12.5): " << quantile[0]
				<< " - " << (quantile[0]/sum) << std::endl;
		std::cout	<< "   2Q [12.5-25.0): " << quantile[1]
				<< " - " << (quantile[1]/sum) << std::endl;
		std::cout	<< "   3Q [25.0-37.5): " << quantile[2]
				<< " - " << (quantile[2]/sum) << std::endl;
		std::cout	<< "   4Q [37.5-50.0): " << quantile[3]
				<< " - " << (quantile[3]/sum) << std::endl;
		std::cout	<< "   5Q [50.0-62.5): " << quantile[4]
				<< " - " << (quantile[4]/sum) << std::endl;
		std::cout	<< "   6Q [62.5-100.]: " << quantile[5]
				<< " - " << (quantile[5]/sum) << std::endl;
		std::cout << "   total vertices: " << sum << std::endl;
	}
}

void TopStoc::invertNormals () {
	for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
		mesh.set_normal(v_it, (-mesh.normal(v_it))) ;
	}
}


float TopStoc::adaptivityCalculation () {
	// calculate ratio of vertices smaller respectively bigger than mean vertex weight
	int biggerThanMean = 0;

	MyMesh::VertexIter v_it = mesh.vertices_begin();
	for (; 	v_it != mesh.vertices_end(); ++v_it){
		float value = mesh.data(v_it).getWeight();
		if(value >= meanVertexWeight)
			++biggerThanMean;
	}

	float result = 1-((float)biggerThanMean/(mesh.n_vertices()-biggerThanMean));
	return (result);
}


void TopStoc::test () {

    std::cout << "Testbutton " << std::endl;
}
