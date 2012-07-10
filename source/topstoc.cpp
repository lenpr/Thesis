#include "topstoc.h"

TopStoc::TopStoc() :
    bbox(),
    loops(),
    options(),
    mesh(),
    sampledVertexQueue(),
    decimatedMesh(),
    minVertexWeight(1.0),
    maxVertexWeight(0.0),
    meanVertexWeight(0.0),
    meshStatus(0),
    numberSelectedTriangles(0) {}

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

    bbox.calculateAll(mesh);
}



void TopStoc::drawSamplAndControlPoints (bool sampledVertices, bool controlPoints, int boundaries) {

    if (controlPoints || (boundaries>0) ) {
        for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
            if (boundaries>0) {
                if ( mesh.is_boundary( v_it.handle() ) ) {
                    if ( mesh.data(v_it.handle()).isLoop()==boundaries-1 )
                        glColor3f(1.0f, 1.0f, 0.0f);
                    else
                        glColor3f(1.0f, 0.0f, 0.0f);
                } else {
                    continue;
                }
            }
            if (controlPoints) {
                if ( mesh.data(v_it).getUserWeight() == 0.0)
                    continue;
                if ( mesh.data(v_it).getUserWeight() >= +1.0)
                    glColor3f(1.0f, 0.5f, 0.0f);
                if ( mesh.data(v_it).getUserWeight() <= -1.0)
                    glColor3f(0.0f, 0.5f, 1.0f);
            }
            glVertex3f(	mesh.point( v_it )[0],
                        mesh.point( v_it )[1],
                        mesh.point( v_it )[2]);
        }
    }
    if (sampledVertices) {
        for (int i=0; i<(int)sampledVertexQueue.size(); ++i){
            glColor3f(0.80f, 0.6f, 0.80f);
            glVertex3f(	mesh.point( sampledVertexQueue[i] )[0],
                        mesh.point( sampledVertexQueue[i] )[1],
                        mesh.point( sampledVertexQueue[i] )[2]);
        }
    }

    for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
        if ( mesh.is_boundary( v_it.handle() ) ) {

        glVertex3f(	mesh.point( v_it )[0],
                    mesh.point( v_it )[1],
                    mesh.point( v_it )[2]);
        }
    }
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

    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        // run along the face vertices and draw them
        for (MyMesh::FaceVertexIter fv_it=mesh.fv_iter(f_it.handle()); fv_it; ++fv_it) {

            if (remeshedRegions && mesh.data(f_it.handle()).isSelected() && !vertexWeights) {
                glColor3f(0.0f, 1.0f, 0.0f);
            } else if (mesh.data(fv_it.handle()).getUserWeight() != 0.0f
                       && remeshedRegions && !vertexWeights) {
                float cv = (mesh.data(fv_it.handle()).getUserWeight())/2.0f;
                glColor3f((0.5+cv),(0.5),(0.5-cv));
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

    //                glPushMatrix();
    //                glLoadIdentity();
    //                glTranslatef(mesh.point(fv_it)[0],
    //                             mesh.point(fv_it)[1],
    //                             mesh.point(fv_it)[2]);
    //                glColor3f(1.0, 0.0, 1.0);
    //                glutWireSphere(0.1, 6, 6);
    //                glutSolidSphere( 0.1f, 6, 6);
    //                glPopMatrix();
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

    QString msg =   QString::number(mesh.n_vertices()) + "," +
                    QString::number(mesh.n_edges()) + "," +
                    QString::number(mesh.n_faces());
    writeToConsole ( msg, 14);
    msg = "-,-,-";
    writeToConsole ( msg, 13);
    msg = "-";
    writeToConsole ( msg, 15);
    writeToConsole ( "-", 16);
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
    colorizeMesh();


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
    float k = subsetTargetSize/100.0f;
	srand( (unsigned)time(0) );

	MyMesh::VertexIter v_it = mesh.vertices_begin();
	int sampled_vertices = 0, sampled_smalls = 0;
    float probability = 0.0f;
    float currentWeight = 0.0f;
	sampledVertexQueue.clear();

	for (; 	v_it != mesh.vertices_end(); ++v_it){

        currentWeight = mesh.data(v_it).getWeight() + mesh.data(v_it).getUserWeight();

        if (currentWeight >= 1.0f) {
            // add to queue
            sampledVertexQueue.push_front( v_it );
            ++sampled_vertices;
        } else if (currentWeight <= 0.0f) {
            continue;
        } else {
            probability = k*(1+alpha*((currentWeight/meanVertexWeight)-1));
            MyMesh::Point viewDirection(options.cameraView[0],
                                        options.cameraView[1],
                                        options.cameraView[2]);

            if (options.povDecimation) {
                if ( (mesh.normal(v_it) | viewDirection) < 0.0) {
                    probability *= options.povRatio;
                }
            }
            if (options.keepSilhouette) {
                float val = (1-cos((options.silhouetteAngle*M_PI)/(180.0f)))/2;
                if ( (mesh.normal(v_it) | viewDirection) >= (-val) &&
                     (mesh.normal(v_it) | viewDirection) <= (+val) ) {
                    probability = 1.0f;
                }
            }

            if (probability >= ((float)rand()/(float)RAND_MAX) ) {
                // add to queue
                sampledVertexQueue.push_front( v_it );
                ++sampled_vertices;
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


// This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
OpenMesh::FaceHandle TopStoc::rayIntersectsTriangle(int x, int y) {

    double mvmatrix[16];
    double projmatrix[16];
    int viewport[4];
    double dX1, dY1, dZ1, dX2, dY2, dZ2, dClickY; // glUnProject uses doubles, but I'm using floats for these 3D vectors

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
    dClickY = double (viewport[3] - y); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

    gluUnProject ((double) x, dClickY, 0.0, mvmatrix, projmatrix, viewport, &dX1, &dY1, &dZ1);
    gluUnProject ((double) x, dClickY, 1.0, mvmatrix, projmatrix, viewport, &dX2, &dY2, &dZ2);
    //qDebug() << "x " << dX1 << " - y " << dY1 << " - z " << dZ1 << endl;

    OpenMesh::Vec3f rayP1(dX1, dY1, dZ1);
    OpenMesh::Vec3f rayP2(dX2, dY2, dZ2);

    // camera pos berechnen
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

        MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(f_it);
        MyMesh::VertexHandle vh1 = cfv_it;
        MyMesh::VertexHandle vh2 = (++cfv_it);
        MyMesh::VertexHandle vh3 = (++cfv_it);

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
    //std::cout << "hits: " << hits << std::endl;
    MyMesh::FaceHandle noHit; //somewhere the intersF. gets set, weird?!
    if (hits > 0)
        return intersectionFace;
    else
        return noHit;
}

bool TopStoc::setUserWeights(MyMesh::FaceHandle selectedFace, float value, int mode) {

    if (!selectedFace.is_valid())
        return false;
    else {

        if (mode == 1) {
            bool selected = mesh.data(selectedFace).isSelected();
            mesh.data(selectedFace).setSelected(!selected);
            if (selected) {
                --numberSelectedTriangles;
            } else {
                ++numberSelectedTriangles;
            }
            writeToConsole(QString::number(numberSelectedTriangles),9);
            return true;
        } else {
            MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(selectedFace);

            if ((value >= +1.0f) || (value <= -1.0f) || (value == 0.0f)) {
                mesh.data(cfv_it).setUserWeight(value);
                mesh.data(++cfv_it).setUserWeight(value);
                mesh.data(++cfv_it).setUserWeight(value);
            } else {
                float setValue;
                int i = 0;
                do{
                    setValue = value + mesh.data(cfv_it).getUserWeight();
                    if (setValue > 1.0f) { setValue = +1.0f; }
                    if (setValue < -1.0f) { setValue = -1.0f; }
                    mesh.data(cfv_it).setUserWeight(setValue);
                    ++cfv_it;
                    ++i;
                }while(i<3);
            }
            colorizeMesh();
            return true;
        }
    }
}

bool TopStoc::setUserWeights(float value) {

    //das ginge alles besser mit einem set in der viewer Klasse aber wäre viel zum umschreiben
    std::vector<MyMesh::FaceHandle> selectedFaces;
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        bool selected = mesh.data(f_it.handle()).isSelected();
        if (selected)
            selectedFaces.push_back(f_it.handle());
    }

    if (selectedFaces.empty())
        return false;

    while (!selectedFaces.empty()) {

        MyMesh::FaceHandle current = selectedFaces.back();
        selectedFaces.pop_back();
        MyMesh::ConstFaceVertexIter cfv_it = mesh.cfv_iter(current);

        if ((value == +1.0) || (value == -1.0)) {
            mesh.data(cfv_it).setUserWeight(value);
            mesh.data(++cfv_it).setUserWeight(value);
            mesh.data(++cfv_it).setUserWeight(value);
        } else {
            float setValue;
            int i = 0;
            do{
                setValue = value + mesh.data(cfv_it).getUserWeight();
                if (setValue > 1.0f) { setValue = +1.0f; }
                if (setValue < -1.0f) { setValue = -1.0f; }
                mesh.data(cfv_it).setUserWeight(setValue);
                ++cfv_it;
                ++i;
            }while(i<3);
        }
    }
    colorizeMesh();
    return true;
}

void TopStoc::clearSelection() {

    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        mesh.data(f_it).setSelected(false);
    }
    numberSelectedTriangles = 0;
    writeToConsole("0", 9);
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
    // if the meshes changes, updated panel
    writeToConsole ("-", 7);
    writeToConsole ("-", 8);
	meshStatus = 1;

	return (true);
}


void TopStoc::colorizeMesh () {

	// calculate histogramm and color the vertices
	int quantile[6] = {0,0,0,0,0,0};

	float upperDelta = (maxVertexWeight-meanVertexWeight)/4;
	float lowerDelta = (meanVertexWeight-minVertexWeight)/4;
	// sheme of the spliting [-1-|-2-|-3-|-4-ǁ-5-| ---6--- ]

	MyMesh::VertexIter v_it = mesh.vertices_begin();
	for (; 	v_it != mesh.vertices_end(); ++v_it){
        float value = mesh.data(v_it).getWeight() + mesh.data(v_it).getUserWeight();
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
//    float sum = mesh.n_vertices();
//    std::cout	<< "   1Q [ 0.0-12.5): " << quantile[0]
//                << " - " << (quantile[0]/sum) << std::endl;
//    std::cout	<< "   2Q [12.5-25.0): " << quantile[1]
//                << " - " << (quantile[1]/sum) << std::endl;
//    std::cout	<< "   3Q [25.0-37.5): " << quantile[2]
//                << " - " << (quantile[2]/sum) << std::endl;
//    std::cout	<< "   4Q [37.5-50.0): " << quantile[3]
//                << " - " << (quantile[3]/sum) << std::endl;
//    std::cout	<< "   5Q [50.0-62.5): " << quantile[4]
//                << " - " << (quantile[4]/sum) << std::endl;
//    std::cout	<< "   6Q [62.5-100.]: " << quantile[5]
//                << " - " << (quantile[5]/sum) << std::endl;
//    std::cout << "   total vertices: " << sum << std::endl;
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

/*
  Computes the Hausdorff distance between the original and the decimated mesh
  of the TopStoc object. Thereby sampling_density is the average number of test
  points per face and min_sample_freq the minimum number of test points per face
  regardless of sampling_density. The content of stats is cleared beforehand.
*/
void TopStoc::calculateHausdorff(double sampling_density_user) {

    int min_sample_freq = 3;
    double sampling_density;

    if (sampling_density_user < min_sample_freq)
        sampling_density_user = min_sample_freq;
    else
        sampling_density = sampling_density_user;

    struct dist_surf_surf_stats stats;

    int i;
    struct model m1, m2;
    struct model_error me1;

    memset(&m1, 0, sizeof(m1));
    memset(&m2, 0, sizeof(m2));
    memset(&me1, 0, sizeof(me1));
    me1.mesh = &m1;
    m2.builtin_normals = 1;

    i = 0;
    m2.num_vert = mesh.n_vertices();
    m2.vertices = (vertex_t *) xa_malloc(m2.num_vert * sizeof(vertex_t));
    m2.normals = (vertex_t *) xa_malloc(m2.num_vert * sizeof(vertex_t));
    for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
        MyMesh::Point p = mesh.point(v_it.handle());
        m2.vertices[i].x = p[0];
        m2.vertices[i].y = p[1];
        m2.vertices[i].z = p[2];
        MyMesh::Normal n = mesh.normal(v_it.handle());
        m2.normals[i].x = n[0];
        m2.normals[i].y = n[1];
        m2.normals[i].z = n[2];
        i++;
    }

    i = 0;
    m2.num_faces = mesh.n_faces();
    m2.faces = (face_t *) xa_malloc(m2.num_faces * sizeof(face_t));
    m2.face_normals = (vertex_t *) xa_malloc(m2.num_faces * sizeof(vertex_t));
    for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
        MyMesh::FaceVertexIter fv_it = mesh.fv_iter(f_it.handle());
        m2.faces[i].f0 = fv_it.handle().idx();
        ++fv_it;
        m2.faces[i].f1 = fv_it.handle().idx();
        ++fv_it;
        m2.faces[i].f2 = fv_it.handle().idx();
        MyMesh::Normal n = mesh.normal(f_it.handle());
        m2.face_normals[i].x = n[0];
        m2.face_normals[i].y = n[1];
        m2.face_normals[i].z = n[2];
        ++i;
    }

    Vec boxMin = bbox.getMinPoint();
    m2.bBox[0].x = (float) boxMin.x;
    m2.bBox[0].y = (float) boxMin.y;
    m2.bBox[0].z = (float) boxMin.z;
    Vec boxMax = bbox.getMaxPoint();
    m2.bBox[1].x = (float) boxMax.x;
    m2.bBox[1].y = (float) boxMax.y;
    m2.bBox[1].z = (float) boxMax.z;

    m1.bBox[0] = m2.bBox[0];
    m1.bBox[1] = m2.bBox[1];

    m1.num_vert = decimatedMesh.size();
    m1.vertices = (vertex_t *) xa_malloc(m1.num_vert * sizeof(vertex_t));
    m1.normals = (vertex_t *) xa_malloc(m1.num_vert * sizeof(vertex_t));
    m1.num_faces = (decimatedMesh.size()/3);
    m1.faces = (face_t *) xa_malloc(m1.num_faces * sizeof(face_t));
    m1.face_normals = (vertex_t *) xa_malloc(m1.num_faces * sizeof(vertex_t));

    for (i = 0; i < m1.num_vert; ++i) {
        MyMesh::Point p = mesh.point(decimatedMesh[i]);
        m1.vertices[i].x = p[0];
        m1.vertices[i].y = p[1];
        m1.vertices[i].z = p[2];
        MyMesh::Normal n1 = mesh.normal(decimatedMesh[i]);
        m1.normals[i].x = n1[0];
        m1.normals[i].y = n1[1];
        m1.normals[i].z = n1[2];

        if (i%3 == 0) {
            int face_number = (i/3);
            m1.faces[face_number].f0 = decimatedMesh[(i+0)].idx();
            m1.faces[face_number].f1 = decimatedMesh[(i+1)].idx();
            m1.faces[face_number].f2 = decimatedMesh[(i+2)].idx();

            MyMesh::Normal n2 = mesh.normal(decimatedMesh[(i+1)]);
            MyMesh::Normal n3 = mesh.normal(decimatedMesh[(i+2)]);
            MyMesh::Normal n = n1 + n2 + n3;
            n.normalize();
            m1.face_normals[face_number].x = n[0];
            m1.face_normals[face_number].y = n[1];
            m1.face_normals[face_number].z = n[2];
        }
    }

    //timer anschalten Hausdorff
    dist_surf_surf(&me1, &m2, sampling_density, min_sample_freq, &stats, m2.builtin_normals, NULL);
    //timer abschalten Hausdorff

    writeToConsole (QString::number( stats.mean_dist ), 7);
    writeToConsole (QString::number( stats.max_dist ), 8);

    free(m1.vertices);
    free(m1.normals);
    free(m1.faces);
    free(m1.face_normals);

    free(m2.vertices);
    free(m2.normals);
    free(m2.faces);
    free(m2.face_normals);

    // ToDo Output the results
}

void TopStoc::filtrate() {
    loops.init(mesh);
    loops.pairing(this->mesh);

    QString msg =   QString::number(loops.getBetti(0)) + ","
                  + QString::number(loops.getBetti(1)) + ","
                  + QString::number(loops.getBetti(2));
    writeToConsole(msg, 13);
    writeToConsole( QString::number(loops.getGenus()), 15);
}

void TopStoc::findLoops() {
    loops.findBoundaries(this->mesh);
    writeToConsole( QString::number(loops.getGenus()), 15);
    writeToConsole( QString::number(loops.n_boundaries()), 16);

}

void TopStoc::killLoop(int loopIdx) {

//    std::cout << "idx:" << (loopIdx) << std::endl;
    if( loopIdx < loops.n_boundaries() ) {
        loops.triangulateBd(this->mesh, loopIdx);

        QString msg =   QString::number(mesh.n_vertices()) + "," +
                QString::number(mesh.n_edges()) + "," +
                QString::number(mesh.n_faces());
        writeToConsole ( msg, 14);
        msg = "-,-,-";
        writeToConsole ( msg, 13);
        msg = "-";
        writeToConsole ( msg, 15);
        writeToConsole ( msg, 16);
        loops.init(mesh);
    }
}

void TopStoc::deleteFaces() {

    //das ginge alles besser mit einem set in der viewer Klasse aber wäre viel zum umschreiben
    std::vector<MyMesh::FaceHandle> selectedFaces;
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        bool selected = mesh.data(f_it.handle()).isSelected();
        if (selected)
            selectedFaces.push_back(f_it.handle());
    }

    if (selectedFaces.empty())
        return;

    while (!selectedFaces.empty()) {
        MyMesh::FaceHandle currentFace = selectedFaces.back();
        selectedFaces.pop_back();
        std::cout << "deleting Face: " << currentFace << std::endl;
        mesh.delete_face(currentFace, true);

    }
    mesh.garbage_collection();

    QString msg =   QString::number(mesh.n_vertices()) + "," +
            QString::number(mesh.n_edges()) + "," +
            QString::number(mesh.n_faces());
    writeToConsole ( msg, 14);
    msg = "-,-,-";
    writeToConsole ( msg, 13);
    msg = "-";
    writeToConsole ( msg, 15);
    writeToConsole ( msg, 16);
    loops.init(this->mesh);
}

void TopStoc::test () {

    std::cout << "\n-Test Button-" << std::endl;

//    BoundingBox bb;
//    bb.setMinPoint( 0.0f, 0.0f, 0.0f );
//    bb.setMaxPoint( 1.1f, 1.2f, 1.3f );
//    std::cout << "Dinstance: " << bb.getDiagonal() << std::endl;
//    std::cout << "Center: " << bb.getCenterPoint() << std::endl;

//    time_t timeToken0, timeToken1;
//    double difTime;

//    timeToken0 = clock();
//    // function
//    timeToken1 = clock();
//    difTime = difftime(timeToken0, timeToken1)/CLOCKS_PER_SEC;
//    std::cout << "t3: " << difTime << "\n" << std::endl;

    OpenMesh::IO::write_mesh(this->mesh, "tmp.off");

    tetgenio tetin;
    bool tetin_okay = tetin.load_off((char *)"tmp.off");

    std::cout << "loading mesh into tetgen: " << tetin_okay << std::endl;

    tetgenbehavior b;
    b.parse_commandline((char *)"p");
    strcpy(b.outfilename, "tet");
    b.geomview = 1;
    b.quiet = 0;
    b.verbose = 5;

//    tetgenio tetout;
//    tetrahedralize(&b, &tetin, &tetout);
}
