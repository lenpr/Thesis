#include "topological_loops.h"


topological_loops::topological_loops() {}

void topological_loops::init(MyMesh &mesh) {

    mesh.add_property(vPositive);
    mesh.add_property(ePositive);
    mesh.add_property(fPositive);

    mesh.add_property(vPair);
    mesh.add_property(ePair);

    bettiNumber[0] = 0;
    bettiNumber[1] = 0;
    bettiNumber[2] = 0;

    genus = -1;
    boundaries = 0;
    hasBoundary = false;

    vertex_n.clear();
    edge_n.clear();
    face_n.clear();
    boundaryVertices.clear();

    MyMesh::EdgeHandle eUnpaired;
    MyMesh::FaceHandle fUnpaired;

    // put handles of the faces in extra list
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {
        face_n.push_back( f_it.handle() );
    }

    // edges are unpaired at init, extra list as well
    for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) {
        edge_n.push_back( e_it.handle() );
        // set all to unpaired
        mesh.property(ePair, e_it) = fUnpaired;
    }

    for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
        vertex_n.push_back( v_it.handle() );
        // all 0-simplices are trivially positive
        mesh.property(vPositive, v_it) = true;
        // set all to unpaired
        mesh.property(vPair, v_it) = eUnpaired;
        ++bettiNumber[0];

        if ( mesh.is_boundary(v_it.handle()) ) {
            hasBoundary = true;
            boundaryVertices.insert( v_it.handle() );
            mesh.data(v_it.handle()).setLoop(0);
        }
    }

    if (hasBoundary)
        genus = -1;
    else
        genus = -2;
}

int topological_loops::getBetti(int idx) {

    if (idx>=0 && idx<3){
        return (bettiNumber[idx]);
    } else {
        return (-1);
    }
}


// --

void topological_loops::pairing(MyMesh &mesh) {

    // Filtration of the edges, vertices have been set in the initialization
    for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) {
        //        std::cout << "Edge: " << e_it << std::endl;

        MyMesh::HalfedgeHandle heh0 = mesh.halfedge_handle(e_it.handle(), 0);
        MyMesh::HalfedgeHandle heh1 = mesh.halfedge_handle(e_it.handle(), 1);

        MyMesh::VertexHandle vh0;
        MyMesh::VertexHandle vh1;
        if ( heh0.is_valid() )
            vh0 = mesh.to_vertex_handle(heh0);
        else
            std::cout << " - Halfedge handle invalid, edge[0]" << std::endl;
        if ( heh1.is_valid() )
            vh1 = mesh.to_vertex_handle(heh1);
        else
            std::cout << " - Halfedge handle invalid, edge[1]" << std::endl;

        MyMesh::VertexHandle youngestVertex;

        if ( vh0.idx() > vh1.idx() )
            youngestVertex = vh0;
        else if ( vh0.idx() < vh1.idx() )
            youngestVertex = vh1;
        else
            std::cout << "Pöser Fehler" << std::endl;

        std::set<MyMesh::VertexHandle> boundaryVerices;
        boundaryVerices.clear();
        boundaryVerices.insert(vh0);
        boundaryVerices.insert(vh1);

        int maxCountVertices = mesh.n_vertices();

        while ( mesh.property(vPair, youngestVertex).is_valid() && !boundaryVerices.empty() ) {

            MyMesh::EdgeHandle pairedEdge = mesh.property(vPair, youngestVertex);
            if ( !pairedEdge.is_valid() )
                std::cout << " - Paired edge handle invalid" << std::endl;

            MyMesh::VertexHandle vhPair0 = mesh.to_vertex_handle(mesh.halfedge_handle(pairedEdge, 0));
            MyMesh::VertexHandle vhPair1 = mesh.to_vertex_handle(mesh.halfedge_handle(pairedEdge, 1));

            //mod2 addition
            std::set<MyMesh::VertexHandle>::iterator set_it;
            //            std::cout << "New: " << vhPair0 << " " << vhPair1 << std::endl;
            //            std::cout << "Set (" << boundaryVerices.size() << "): ";
            //            for ( set_it = boundaryVerices.begin(); set_it != boundaryVerices.end(); ++set_it) {
            //                std::cout <<  (*set_it) << " ";
            //            } std::cout << std::endl;

            if (boundaryVerices.find(vhPair0) != boundaryVerices.end()) {
                boundaryVerices.erase(vhPair0);
            } else {
                boundaryVerices.insert(vhPair0);
            }
            if (boundaryVerices.find(vhPair1) != boundaryVerices.end()) {
                boundaryVerices.erase(vhPair1);
            } else {
                boundaryVerices.insert(vhPair1);
            }
            if (!boundaryVerices.empty()) {

                //                std::cout << "Set (" << boundaryVerices.size() << "): ";
                //                for ( set_it = boundaryVerices.begin(); set_it != boundaryVerices.end(); ++set_it) {
                //                    std::cout <<  (*set_it) << " ";
                //                } std::cout << std::endl;

                MyMesh::VertexHandle reset;
                youngestVertex = reset;
                for ( set_it = boundaryVerices.begin(); set_it != boundaryVerices.end(); ++set_it) {

                    MyMesh::VertexHandle ageTest = (*set_it);
                    if ( ageTest.idx() > youngestVertex.idx() )
                        youngestVertex = ageTest;
                }
                --maxCountVertices;
                if (maxCountVertices == 0) {
                    std::cout << "Endlosschleife (Edges)" << std::endl;
                    return;
                }
            }
        }
        if ( !boundaryVerices.empty() ) {
            //            std::cout << "Edge: " << e_it << " is - and paired with vertex: " << youngestVertex << std::endl;
            mesh.property(ePositive, e_it) = false;
            mesh.property(vPair, youngestVertex) = e_it;
            --bettiNumber[0];
        } else {
            //            std::cout << "Edge: " << e_it << " is +" << std::endl;
            mesh.property(ePositive, e_it) = true;
            ++bettiNumber[1];
        }
    }

    // Filtration of the faces
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) {

        MyMesh::FaceEdgeIter fe_it0 = mesh.fe_iter(f_it);
        MyMesh::EdgeHandle eh0 = fe_it0;
        MyMesh::EdgeHandle eh1 = (++fe_it0);
        MyMesh::EdgeHandle eh2 = (++fe_it0);
        // if ((fe_it.handle()).is_valid()){}

        MyMesh::EdgeHandle youngestEdge;
        if (eh0.idx() > eh1.idx() && eh0.idx() > eh2.idx())
            youngestEdge = eh0;
        else if (eh1.idx() > eh2.idx())
            youngestEdge = eh1;
        else
            youngestEdge = eh2;

        std::set<MyMesh::EdgeHandle> boundaryEdges;
        boundaryEdges.clear();
        boundaryEdges.insert(eh0);
        boundaryEdges.insert(eh1);
        boundaryEdges.insert(eh2);

        int maxCountEdges = mesh.n_edges();

        while ( mesh.property(ePair, youngestEdge).is_valid() && !boundaryEdges.empty() ) {

            MyMesh::FaceHandle pairedFace = mesh.property(ePair, youngestEdge);
            if ( !pairedFace.is_valid() )
                std::cout << " - Paired face handle invalid" << std::endl;

            MyMesh::FaceEdgeIter fe_it1 = mesh.fe_iter(pairedFace);
            MyMesh::EdgeHandle fhPair0 = fe_it1;
            MyMesh::EdgeHandle fhPair1 = (++fe_it1);
            MyMesh::EdgeHandle fhPair2 = (++fe_it1);

            // mod2 addition
            if ( boundaryEdges.find(fhPair0) != boundaryEdges.end() ) {
                boundaryEdges.erase(fhPair0);
            } else {
                boundaryEdges.insert(fhPair0);
            }
            if ( boundaryEdges.find(fhPair1) != boundaryEdges.end() ) {
                boundaryEdges.erase(fhPair1);
            } else {
                boundaryEdges.insert(fhPair1);
            }
            if ( boundaryEdges.find(fhPair2) != boundaryEdges.end() ) {
                boundaryEdges.erase(fhPair2);
            } else {
                boundaryEdges.insert(fhPair2);
            }
            if (!boundaryEdges.empty()) {
                MyMesh::EdgeHandle reset;
                youngestEdge = reset;
                std::set<MyMesh::EdgeHandle>::iterator set_it;
                for ( set_it = boundaryEdges.begin(); set_it != boundaryEdges.end(); ++set_it) {

                    MyMesh::EdgeHandle ageTest = (*set_it);
                    if ( ageTest.idx() > youngestEdge.idx() )
                        youngestEdge = ageTest;
                }
                --maxCountEdges;
                if (maxCountEdges == 0) {
                    std::cout << "Endlosschleife (Faces)" << std::endl;
                }
            }
        }
        if ( !boundaryEdges.empty() ) {
            mesh.property(fPositive, f_it) = false;
            mesh.property(ePair, youngestEdge) = f_it;
            --bettiNumber[1];
        } else {
            mesh.property(fPositive, f_it) = true;
            ++bettiNumber[2];
        }
    }

    //    std::cout << "Betti 0: " << bettiNumber[0] << std::endl;
    //    std::cout << "Betti 1: " << bettiNumber[1] << std::endl;
    //    std::cout << "Betti 2: " << bettiNumber[2] << std::endl;
    //    std::cout << "Euler  : " << bettiNumber[0]-bettiNumber[1]+bettiNumber[2] << std::endl;
    //    std::cout << "V: " << mesh.n_vertices() << std::endl;
    //    std::cout << "E: " << mesh.n_edges() << std::endl;
    //    std::cout << "F: " << mesh.n_faces() << std::endl;

    if ( !hasBoundary )
        genus = (2-(bettiNumber[0] - bettiNumber[1] + bettiNumber[2]))/2;
    else
        genus = -1;

    return;
}

void topological_loops::findBoundaries(MyMesh &mesh) {

    bdLoops.clear();
    int loopNr = 0;

    while ( !boundaryVertices.empty() ) {

        MyMesh::VertexHandle bdV = *(boundaryVertices.begin());
        boundaryVertices.erase(bdV);
        loops currentLoop;
        ++loopNr;

        currentLoop.bdVertices.push_back(bdV);
        mesh.data(bdV).setLoop(loopNr);

        MyMesh::VertexHandle currentVertex = bdV;
        //        std::cout << "\nBoundary: " << bdv;
        do {
            for (MyMesh::ConstVertexOHalfedgeIter vhe_it=mesh.cvoh_iter(currentVertex); vhe_it; ++vhe_it) {
                currentVertex = mesh.to_vertex_handle(vhe_it);

                if ( boundaryVertices.find(currentVertex) != boundaryVertices.end()
                     && mesh.is_boundary( vhe_it.handle() ) ) {
                    currentLoop.bdVertices.push_back( currentVertex );
                    mesh.data(currentVertex).setLoop(loopNr);
                    boundaryVertices.erase( currentVertex );
                    //                    std::cout << ", " << currentVertex;
                }
            }
        } while (currentVertex != bdV);

        // because of the design of the algorithm the irst twi get swapt, the rest is ok
        std::swap( (currentLoop.bdVertices.at(1)), (currentLoop.bdVertices.at(0)) );

        bdLoops.push_back( currentLoop );
    }

    boundaries = bdLoops.size();
    int temp = 0;

    if (bettiNumber[0] != 0 || bettiNumber[1] != 0 || bettiNumber[2] != 0 )
        temp = 2-(bettiNumber[0]-bettiNumber[1]+bettiNumber[2])-boundaries;
    else
        temp = 2-(mesh.n_vertices()-mesh.n_edges()+mesh.n_faces())-boundaries;

    if (temp%2 == 0) {
        temp /= 2;
        genus = temp;
    } else {
        std::cout << "Fehler bei Genus Berechnung: " << genus << std::endl;
    }
    //    std::cout << "Boundaries: " << bdLoops.size() << std::endl;
}

void topological_loops::triangulateBd(MyMesh &mesh, int bdIdx) {

//    std::cout << "- " << bdLoops.size() << std::endl;

    if ( bdIdx < (int)bdLoops.size() ) {

        loops currentLoop = bdLoops.at(bdIdx);
        MyMesh::VertexHandle currentVertex;
        if (currentLoop.bdVertices.empty())
            return;

        MyMesh::Point mean(0.0f,0.0f,0.0f), variance(0.0f,0.0f,0.0f);
        float min = 0;

        for (int n=0; n < (int)currentLoop.bdVertices.size(); ++n) {
            currentVertex = currentLoop.bdVertices.at(n);
            mean += mesh.point(currentVertex);
            if ( mesh.point(currentVertex)[0] < min )
                min = mesh.point(currentVertex)[0];
            if ( mesh.point(currentVertex)[1] < min )
                min = mesh.point(currentVertex)[1];
            if ( mesh.point(currentVertex)[2] < min )
                min = mesh.point(currentVertex)[2];
        }
        mean /= currentLoop.bdVertices.size();

        for (int n=0; n < (int)currentLoop.bdVertices.size(); ++n) {
            currentVertex = currentLoop.bdVertices.at(n);
            variance[0] += (mesh.point(currentVertex)[0]*mesh.point(currentVertex)[0] - (n+1)*mean[0]*mean[0]);
            variance[1] += (mesh.point(currentVertex)[1]*mesh.point(currentVertex)[1] - (n+1)*mean[1]*mean[1]);
            variance[2] += (mesh.point(currentVertex)[2]*mesh.point(currentVertex)[2] - (n+1)*mean[2]*mean[2]);
        }
        variance /= (currentLoop.bdVertices.size()-1);

        Vector2dVector hole;
        min *= -1;

        if ( variance[0] < variance[1] && variance[0] < variance[2] ) {
            for (int n=0; n < (int)currentLoop.bdVertices.size(); ++n) {
                currentVertex = currentLoop.bdVertices.at(n);
                float y = mesh.point(currentVertex)[1];
                float z = mesh.point(currentVertex)[2];
                y += min;
                z += min;
                hole.push_back( Vector2d( y, z, currentVertex));
//                std::cout << "- " << y << ", " << z << " @ " << currentVertex << std::endl;
            }
        } else {
            if ( variance[1] < variance[2] ) {
                for (int n=0; n < (int)currentLoop.bdVertices.size(); ++n) {
                    currentVertex = currentLoop.bdVertices.at(n);
                    float x = mesh.point(currentVertex)[0];
                    float z = mesh.point(currentVertex)[2];
                    x += min;
                    z += min;
                    hole.push_back( Vector2d(x,z, currentVertex));
//                    std::cout << "- " << x << ", " << z << " @ " << currentVertex << std::endl;
                }
            } else {
                for (int n=0; n < (int)currentLoop.bdVertices.size(); ++n) {
                    currentVertex = currentLoop.bdVertices.at(n);
                    float x = mesh.point(currentVertex)[0];
                    float y = mesh.point(currentVertex)[1];
                    x += min;
                    y += min;
                    hole.push_back( Vector2d(x,y, currentVertex));
//                    std::cout << "- " << x << ", " << y << " @ " << currentVertex << std::endl;
                }
            }
        }

        Vector2dVector result;
        //  Invoke the triangulator to triangulate this polygon.
        Triangulate::Process(hole,result);

        //        std::cout << "H:" << hole.size() << std::endl;
        //        std::cout << "R:" << result.size() << std::endl;

        for (int i=0; i < (int)(result.size()/3); ++i) {
            const Vector2d &p1 = result[i*3+0];
            const Vector2d &p2 = result[i*3+1];
            const Vector2d &p3 = result[i*3+2];
//            printf("Triangle %d => (%0.0f,%0.0f) (%0.0f,%0.0f) (%0.0f,%0.0f) - ",i+1,p1.GetX(),p1.GetY(),p2.GetX(),p2.GetY(),p3.GetX(),p3.GetY());
//            std::cout << "VH: " << p1.getVH() << ", " << p2.getVH() << ", " << p3.getVH() << std::endl;

            std::vector<MyMesh::VertexHandle> face_vhandles;
            face_vhandles.clear();
            face_vhandles.push_back(p1.getVH());
            face_vhandles.push_back(p2.getVH());
            face_vhandles.push_back(p3.getVH());

            MyMesh::FaceHandle newFace = mesh.add_face(face_vhandles);
            // ich hasse OpenMesh manchmal, total sinnloser Fehler, super picky das Ding
            if ( !newFace.is_valid() ){
                face_vhandles.clear();
                face_vhandles.push_back(p1.getVH());
                face_vhandles.push_back(p3.getVH());
                face_vhandles.push_back(p2.getVH());
                newFace = mesh.add_face(face_vhandles);

                if ( !newFace.is_valid() )
                    std::cout << "Couldn't Add Triangle" << std::endl;
            }
        }

//        findBoundaries(mesh);
    }
}

void topological_loops::test() {

    std::cout << "Loops: " << bdLoops.size();

    for ( int n = 0; n < (int)bdLoops.size(); ++n) {
        std::cout << "\nl" << (n+1) << "," << (bdLoops.at(n)).bdVertices.size() <<": ";

        for ( int i = 0; i < (int)(bdLoops.at(n)).bdVertices.size(); ++i) {
            std::cout << (bdLoops.at(n)).bdVertices.at(i) << " ";
        }
    }
    std::cout << std::endl;
}
