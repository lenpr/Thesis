#include "topological_loops.h"


topological_loops::topological_loops() {}

void topological_loops::init(MyMesh &mesh) {

    mesh.add_property(vPositive);
    mesh.add_property(ePositive);
    mesh.add_property(fPositive);

    mesh.add_property(vPair);
    mesh.add_property(ePair);

    mesh.add_property(killedVertices);
    mesh.add_property(killedEdges);

    bettiNumber[0] = 0;
    bettiNumber[1] = 0;
    bettiNumber[2] = 0;

    genus = -1;
    boundaries = 0;
    hasBoundary = false;

    boundaryVertices.clear();

    MyMesh::EdgeHandle eUnpaired;
    MyMesh::FaceHandle fUnpaired;

    // edges are unpaired at init, extra list as well
    for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) {
        // set all to unpaired
        mesh.property(ePair, e_it) = fUnpaired;
    }

    for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {
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

    // to set the initial output, see controlpanel for details
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

// -- Filtration of the surface simplicials
void topological_loops::pairing(MyMesh &mesh) {

    time_t timeToken0, timeToken1;
    double difTime0, difTime1;
    timeToken0 = clock();

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

        std::set< int > boundaryChain;
        boundaryChain.clear();
        boundaryChain.insert( vh0.idx() );
        boundaryChain.insert( vh1.idx() );

        int maxCountVertices = mesh.n_vertices();

        while ( mesh.property(vPair, youngestVertex).is_valid() && !boundaryChain.empty() ) {

            MyMesh::EdgeHandle pairedEdge = mesh.property(vPair, youngestVertex);
            if ( !pairedEdge.is_valid() )
                std::cout << " - Paired edge handle invalid" << std::endl;

            std::set< int > killedChain = mesh.property(killedVertices, pairedEdge);
            std::set< int >::iterator kc_it;

            //mod2 addition
            for ( kc_it = killedChain.begin(); kc_it != killedChain.end(); ++kc_it ) {

                if ( boundaryChain.find( *kc_it ) != boundaryChain.end() ) {
                    boundaryChain.erase( *kc_it );
                } else {
                    boundaryChain.insert( *kc_it );
                }
            }

            if (!boundaryChain.empty()) {

                //                std::cout << "Set (" << boundaryVerices.size() << "): ";
                //                for ( set_it = boundaryVerices.begin(); set_it != boundaryVerices.end(); ++set_it) {
                //                    std::cout <<  (*set_it) << " ";
                //                } std::cout << std::endl;

                MyMesh::VertexHandle reset;
                youngestVertex = reset;
                std::set< int >::iterator set_it;
                for ( set_it = boundaryChain.begin(); set_it != boundaryChain.end(); ++set_it) {

                    MyMesh::VertexHandle ageTest = MyMesh::VertexHandle(*set_it);
                    if ( ageTest.idx() > youngestVertex.idx() )
                        youngestVertex = ageTest;
                }
                if ( !youngestVertex.is_valid() )
                    std::cout << "Couldn't find youngest simplex in chain, at: " << e_it.handle() << std::endl;

                --maxCountVertices;
                if (maxCountVertices == 0) {
                    std::cout << "Endlosschleife (Edges)" << std::endl;
                    return;
                }
            }
        }
        if ( !boundaryChain.empty() ) {
            //            std::cout << "Edge: " << e_it << " is - and paired with vertex: " << youngestVertex << std::endl;
            mesh.property(ePositive, e_it) = false;
            mesh.property(killedVertices, e_it) = boundaryChain;
            mesh.property(vPair, youngestVertex) = e_it;
            --bettiNumber[0];
        } else {
            //            std::cout << "Edge: " << e_it << " is +" << std::endl;
            mesh.property(ePositive, e_it) = true;
            ++bettiNumber[1];
        }
    }
    timeToken1 = clock();
    difTime0 = difftime(timeToken1, timeToken0)/CLOCKS_PER_SEC;
    std::cout << "time : " << difTime0 << std::endl;
    timeToken0 = clock();

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

        std::set< int > boundaryChain;
        boundaryChain.clear();
        boundaryChain.insert( eh0.idx() );
        boundaryChain.insert( eh1.idx() );
        boundaryChain.insert( eh2.idx() );

        int maxCountEdges = mesh.n_edges();

        while ( mesh.property(ePair, youngestEdge).is_valid() && !boundaryChain.empty() ) {

            MyMesh::FaceHandle pairedFace = mesh.property(ePair, youngestEdge);
            if ( !pairedFace.is_valid() )
                std::cout << " - Paired face handle invalid" << std::endl;


            std::set< int > killedChain = mesh.property(killedEdges, pairedFace);
            std::set< int >::iterator kc_it;

            // mod2 addition
            for ( kc_it = killedChain.begin(); kc_it != killedChain.end(); ++kc_it ) {

                if ( boundaryChain.find( *kc_it ) != boundaryChain.end() ) {
                    boundaryChain.erase( *kc_it );
                } else {
                    boundaryChain.insert( *kc_it );
                }
            }

            if (!boundaryChain.empty()) {

                MyMesh::EdgeHandle reset;
                youngestEdge = reset;
                std::set< int >::iterator set_it;
                for ( set_it = boundaryChain.begin(); set_it != boundaryChain.end(); ++set_it) {

                    MyMesh::EdgeHandle ageTest = MyMesh::EdgeHandle(*set_it);
                    if ( ageTest.idx() > youngestEdge.idx() )
                        youngestEdge = ageTest;
                }           
                if ( !youngestEdge.is_valid() )
                    std::cout << "Couldn't find youngest simplex in chain, at: " << f_it.handle() << std::endl;

                --maxCountEdges;
                if (maxCountEdges == 0) {
                    std::cout << "Endlosschleife (Faces)" << std::endl;
                }
            }
        }
        if ( !boundaryChain.empty() ) {
            mesh.property(fPositive, f_it) = false;
            mesh.property(killedEdges, f_it) = boundaryChain;
            mesh.property(ePair, youngestEdge) = f_it;
            --bettiNumber[1];
        } else {
            mesh.property(fPositive, f_it) = true;
            ++bettiNumber[2];
        }
    }
    timeToken1 = clock();
    difTime1 = difftime(timeToken1, timeToken0)/CLOCKS_PER_SEC;
    std::cout << "time : " << difTime1 << std::endl;
    std::cout << "difference: " << difTime0/difTime1 << std::endl;
    std::cout << "------" << std::endl;

    for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) {

        MyMesh::FaceHandle fh = mesh.property(ePair, e_it.handle());
        bool paired = fh.is_valid();
        bool positive = mesh.property(ePositive, e_it);
        if (positive && !paired)
            mesh.data(e_it.handle()).setEdgeCircle(-1);
    }

    // Debug
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

        // because of the design of the algorithm the first two get swapted, the rest is ok
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

void topological_loops::meshInsideAndConvexHull(MyMesh &mesh) {

    OpenMesh::IO::write_mesh(mesh, "tetrahedra_surface.off");

    tetgenio tetin;
    bool tetin_okay = tetin.load_off((char *)"tetrahedra_surface.off");

    if (!tetin_okay)
        std::cout << "Couldn't load surface mesh into tetgen!" << std::endl;


    // the Y forces the programm to respect the original edges, p specifies input as PLC
    // the M prohibits coplanar merging of facets
    tetgenbehavior bInside, bOutside;
    bInside.parse_commandline((char *)"pYM");
    strcpy(bInside.outfilename, "tetrahedra_inside");
    bInside.quiet = 1;      // b.quiet = 0;
    bInside.verbose = 0;    // b.verbose = 5;
    bInside.facesout = 1;
    bInside.edgesout = 2;   // 0: don't output edges, 1: output subsegments only, >1: output all edges
    tetrahedralize(&bInside, &tetin, &meshI);
//    tetrahedralize(&bInside, &tetin, NULL); // for writing to disk

    bOutside.parse_commandline((char *)"Y");
    strcpy(bOutside.outfilename, "tetrahedra_convexhull");
    bOutside.quiet = 1;
    bOutside.verbose = 0;
    bOutside.facesout = 1;
    tetrahedralize(&bOutside, &tetin, &meshO);
//    tetrahedralize(&bOutside, &tetin, NULL); // for writing to disk

    // if the surface is not ready, put it through the gears
    if ( bettiNumber[1] == 0 && bettiNumber[2] == 0) {
        std::cout << "Mesh not ready... processing.\n" << std::endl;
        init(mesh);
        pairing(mesh);
        findBoundaries(mesh);
        meshInsideAndConvexHull(mesh);
        return;
    } else if ( bettiNumber[1] == 0 ) {
        std::cout << "Mesh has no handles or tunnels!" << std::endl;
        return;
    } else if ( hasBoundary ) {
        std::cout << "Mesh still has Boundaries!" << std::endl;
        return;
    }

    // Sanity check, although I am not sure this has to hold for every simplicial complex.
//    int eulerChNew, eulerChOld, eulerChDiff;
//    eulerChOld = mesh.n_vertices() - mesh.n_edges() + mesh.n_faces();
//    eulerChNew = meshI.numberofpoints - meshI.numberofedges + meshI.numberoftrifaces - meshI.numberoftetrahedra;
//    eulerChDiff = eulerChNew - eulerChOld;
//    if ( eulerChNew != 0 ) {
//        std::cout << "\n---\nEuler Characteristic Changed: " << eulerChDiff << std::endl;
//        std::cout << "|V| = " << meshI.numberofpoints << " - " << mesh.n_vertices() << std::endl
//                  << "|E| = " << meshI.numberofedges << " - " << mesh.n_edges() << std::endl
//                  << "|F| = " << meshI.numberoftrifaces << " - " << mesh.n_faces() << std::endl
//                  << "|T| = " << meshI.numberoftetrahedra << std::endl;
//        std::cout << "Euler old:" << eulerChOld << std::endl;
//        std::cout << "Euler new:" << eulerChNew << std::endl;
//        std::cout << "---\n" << std::endl;
//    }

    bettiNumberInside[0] = bettiNumber[0];
    bettiNumberInside[1] = bettiNumber[1];
    bettiNumberInside[2] = bettiNumber[2];

    const int surfaceVertices = mesh.n_vertices();
    const int surfaceEdges = mesh.n_edges();
    const int surfaceFaces = mesh.n_faces();
    const int tetrahedraVertices = meshI.numberofpoints;
    const int tetrahedraEdges = meshI.numberofedges;
    const int tetrahedraFaces = meshI.numberoftrifaces;
    const MyMesh::VertexHandle vhNone;
    const MyMesh::EdgeHandle ehNone;
    const MyMesh::FaceHandle fhNone;

    // init inside mesh
    // set vertices in accordance to OpenMesh
    verticesIn.clear();
    verticesUnpairedIn.clear();
    verticesNewIn.clear();

    for (int i = 0; i < surfaceVertices; ++i) {
        tgV currentVertex;

        currentVertex.positive = true;
        currentVertex.surfaceVertex = true;
        currentVertex.vh_tg = i;
        currentVertex.vh_om = MyMesh::VertexHandle(i);
        MyMesh::EdgeHandle eh = mesh.property(vPair, MyMesh::VertexHandle(i));
        currentVertex.eh_om = eh;

        if ( !eh.is_valid() ) {
            //  these ones will not get paired
            currentVertex.isPaired = false;
            verticesUnpairedIn.push_back( currentVertex );
        } else {
            currentVertex.isPaired = true;
            currentVertex.eh_om = eh;
        }
        verticesIn.push_back( currentVertex );
    }
    for (int i = surfaceVertices; i < tetrahedraVertices; ++i) {
        tgV currentVertex;

        currentVertex.positive = true;
        currentVertex.surfaceVertex = false;
        currentVertex.isPaired = false;
        ++bettiNumberInside[0];
        currentVertex.eh_om = ehNone;
        currentVertex.vh_tg = i;
        currentVertex.vh_om = vhNone;

//        cout_tg( currentVertex );
        verticesIn.push_back( currentVertex );
        verticesUnpairedIn.push_back( currentVertex );
        verticesNewIn.push_back( currentVertex );
    }
    // sanity check
    if ( (int)verticesUnpairedIn.size()-bettiNumber[0] != tetrahedraVertices-surfaceVertices ) {
        std::cout << "Vertices got lost, difference: " << verticesUnpairedIn.size()-bettiNumber[0]-tetrahedraVertices-surfaceVertices << std::endl;
        std::cout << "Found Vertices: " << verticesIn.size() << std::endl;
        std::cout << "Tetrahedra |V|: " << meshI.numberofpoints << std::endl;
        std::cout << "Old vertices  : " << surfaceVertices << std::endl;
        std::cout << "New vertices  : " << verticesUnpairedIn.size() << " - " << bettiNumber[0] << std::endl;
    }
    // Debug
//    for (int i=0; i<mesh.n_vertices(); ++i) {
//        tgV cV = vertices.at(i);
//        MyMesh::VertexHandle vh = cV.vh_om;
//        int tgIdx = cV.vh_tg;
//        std::cout << "idx: " << cV.vh_tg << " "<< cV.vh_om << std::endl;
//        std::cout << i <<": " << mesh.point( MyMesh::VertexHandle(i) ) << std::endl;
//        std::cout << i <<": " << meshI.pointlist[i*3+0] << " " << meshI.pointlist[i*3+1] << " " << meshI.pointlist[i*3+2] << std::endl;
//        // although they are the same points, they have a dif. because of the conversion (IO)
//        std::cout << i << ".0: " << (meshI.pointlist[tgIdx*3+0]-mesh.point(vh)[0]) << std::endl;
//        std::cout << i << ".1: " << (meshI.pointlist[tgIdx*3+1]-mesh.point(vh)[1]) << std::endl;
//        std::cout << i << ".2: " << (meshI.pointlist[tgIdx*3+2]-mesh.point(vh)[2]) << std::endl;
//        std::cout << "---" << std::endl;
//    }

    // edges
    // set vertices in accordance to OpenMesh
    int edgesInside = 0;
    int edgesSurface = 0;
    edgesIn.clear();
    edgesUnpaired.clear();
    edgesNewIn.clear();
    // to find edges later on and translate them
    eh_OMtoTG ehDictionary;
    int oldEdgesIdx = 0;
    int newEdgesIdx = tetrahedraEdges;

    for (int i = 0; i < tetrahedraEdges; ++i) {

        int vh0_tg = meshI.edgelist[2*i];
        int vh1_tg = meshI.edgelist[(2*i)+1];
        tgE currentEdge;

        // the same for all
        currentEdge.eh_tg = i;
        currentEdge.vh0_tg = vh0_tg;
        currentEdge.vh1_tg = vh1_tg;

        if ( vh0_tg >= surfaceVertices || vh1_tg >= surfaceVertices ) {
            // must be a new edge
            currentEdge.isPaired = false;
            currentEdge.surfaceEdge = false;
            currentEdge.fh_om = fhNone;
            currentEdge.eh_om = ehNone;
            currentEdge.age = newEdgesIdx;
            ++newEdgesIdx;

            edgesUnpaired.push_back( currentEdge );
            edgesNewIn.push_back( currentEdge );
            ++edgesInside;
        } else {
            // check if old or new
            MyMesh::VertexHandle vh0_om = MyMesh::VertexHandle( vh0_tg );
            MyMesh::VertexHandle vh1_om = MyMesh::VertexHandle( vh1_tg );

            // this is symmetrical (a,b)=(b,a) since the mesh is closed
            MyMesh::HalfedgeHandle heh;
            heh = mesh.find_halfedge( vh0_om, vh1_om );

            if ( !heh.is_valid() ) {
                // inside edge that connects two existing vertices
                currentEdge.isPaired = false;
                currentEdge.surfaceEdge = false;
                currentEdge.fh_om = fhNone;
                currentEdge.eh_om = ehNone;
                currentEdge.age = newEdgesIdx;
                ++newEdgesIdx;

                edgesUnpaired.push_back( currentEdge );
                edgesNewIn.push_back( currentEdge );
                ++edgesInside;
            } else {
                // old edge on the surface
                ++edgesSurface;
                currentEdge.surfaceEdge = true;
                currentEdge.age = oldEdgesIdx;
                ++oldEdgesIdx;

                MyMesh::EdgeHandle eh = mesh.edge_handle(heh);
                currentEdge.eh_om = eh;
                MyMesh::FaceHandle fh = mesh.property(ePair, eh);
                bool isPositive = mesh.property(ePositive, eh);
                currentEdge.killedVertices = mesh.property(killedVertices, eh);

                // add into the dictionary
                ehDictionary.insert( std::pair<MyMesh::EdgeHandle,int>(eh,i) );

                // some of the "old" edges are still unpaired -> 2g precisely
                if ( !fh.is_valid() && isPositive ) {
                    currentEdge.positive = true;
                    currentEdge.isPaired = false;
                    currentEdge.fh_om = fhNone;
                    edgesUnpaired.push_back( currentEdge );
//                    std::cout << "#";
                } else if (!isPositive) {
                    currentEdge.positive = false;
                    currentEdge.isPaired = true;
                    currentEdge.fh_om = fhNone;
//                    std::cout << "-";
                } else {
                    currentEdge.positive = true;
                    currentEdge.isPaired = true;
                    currentEdge.fh_om = fh;
//                    std::cout << "+";
//                    std::cout << i << ". fh:" << fh << std::endl;
                }
            }
        }
        edgesIn.push_back( currentEdge );
    }
    // Sanity check
    if ((oldEdgesIdx!=surfaceEdges) || (newEdgesIdx-tetrahedraEdges!=tetrahedraEdges-surfaceEdges) ) {
        std::cout << "Some age tokens weren't set properly:" << std::endl;
        std::cout << "Old edges: " << oldEdgesIdx << " / " << surfaceEdges << std::endl;
        std::cout << "New edges: " << newEdgesIdx << " / " << tetrahedraEdges-surfaceEdges << std::endl;
    }
    if ( edgesSurface != surfaceEdges ) {
        std::cout << "\n Not all surface edges were found, dif.: " << edgesSurface-surfaceEdges << std::endl;
        std::cout << "Edges in base mesh : " << surfaceEdges << std::endl;
        std::cout << "Found surface edges: " << edgesSurface << std::endl;
    }
    if ( (edgesSurface+edgesInside-meshI.numberofedges) != 0) {
        std::cout << "Edges don't line up, diff: " << edgesSurface + edgesInside -  meshI.numberofedges << std::endl;
        std::cout << "Edges OpenMesh  : " << surfaceEdges << std::endl;
        std::cout << "Edges inside TG : " << edgesInside << std::endl;
        std::cout << "Edges surface TG: " << edgesSurface << std::endl;
    }
    if ( (edgesSurface-ehDictionary.size()) != 0 ) {
        std::cout << "Dictionary is not complete, dif.: " << (edgesSurface-ehDictionary.size()) << std::endl;
        std::cout << "Edges in base mesh : " << surfaceEdges << std::endl;
        std::cout << "Edges in dictionary: " << ehDictionary.size() << std::endl;
    }
    if ( edgesUnpaired.size() != (edgesNewIn.size()+bettiNumber[1]) ) {
        std::cout << "Unpaired edges got lost, dif.: " << edgesUnpaired.size()-(edgesNewIn.size()+bettiNumber[1]) << std::endl;
        std::cout << "Found unpaired edges: " << edgesUnpaired.size() << std::endl;
        std::cout << "New (inside) edges  : " << edgesNewIn.size() << std::endl;
        std::cout << "Old unpaired edges  : " << bettiNumber[1] << std::endl;
    }
    // Debug
//    for (int i = 0; i < meshI.numberofedges; ++i) {
//        std::cout << "e " << i << ": from=" << meshI.edgelist[2*i] << " to=" << meshI.edgelist[2*i+1] << std::endl;
//    }
//    for (eh_OMtoTG::iterator it = ehDictionary.begin(); it!=ehDictionary.end(); ++it) {
//        std::cout << "om: " << it->first << " - tg: " << it->second << std::endl;
//    }

    // assign paired partners to the vertices
    const eh_OMtoTG::iterator ehNotFound = ehDictionary.end();
    int pairSet = 0;
    int missed = 0;
    int notPaired = 0;

    for (int i=0; i<(int)verticesIn.size(); ++i) {

        tgV currentVertex = verticesIn.at(i);
        if (currentVertex.isPaired == true) {

            MyMesh::EdgeHandle partner = currentVertex.eh_om;
            int tgIdx = -1;
            eh_OMtoTG::iterator dictionary_it = ehDictionary.find(partner);

            if ( dictionary_it != ehNotFound ) {
                tgIdx = dictionary_it->second;
                (verticesIn.at(i)).eh_tg = tgIdx;
                ++pairSet;
            } else {
                ++missed;
                std::cout << "The edge couldn't be found in the dictionary: " << partner << std::endl;
            }
        } else {
            ++notPaired;
        }
    }
    // Sanity Check
    if ( (missed!=0) || (notPaired-bettiNumber[0]!=tetrahedraVertices-surfaceVertices) ) {
        std::cout << "Paired      : " << pairSet << std::endl;
        std::cout << "Not paired  : " << notPaired << std::endl;
        std::cout << "Missed      : " << missed << std::endl;
        std::cout << "Betti[1]    : " << bettiNumber[0] << std::endl;
        std::cout << "New Vertices: " << tetrahedraVertices-surfaceVertices << std::endl;
    }

    // faces
    // set faces in accordance to OpenMesh
    // edgesNew would be way faster but leads to problems with some meshes
    std::vector<tgE> edgeList = edgesNewIn;
    bool noMoreFallBack = false;
    FallBackCase:
    int facesInside = 0;
    int facesSurface = 0;
    int oldUnpaired = 0;
    facesIn.clear();
    facesUnpaired.clear();
    facesNewIn.clear();
    fh_OMtoTG fhDictionary;

    for (int i = 0; i < tetrahedraFaces; ++i) {

        int vh0_tg = meshI.trifacelist[3*i+0];
        int vh1_tg = meshI.trifacelist[3*i+1];
        int vh2_tg = meshI.trifacelist[3*i+2];
        tgF currentFace;

        // the same for all
        currentFace.fh_tg = i;

        if ( vh0_tg >= surfaceVertices || vh1_tg >= surfaceVertices || vh2_tg >= surfaceVertices) {
            // trivially new faces
            currentFace.isPaired = false;
            currentFace.surfaceFace = false;

            // is it already set
            bool e0 = false;
            bool e1 = false;
            bool e2 = false;

            for (int n=0; n<(int)edgeList.size(); ++n) {
                tgE currentEdge = edgeList.at(n);
                int cvh0 = currentEdge.vh0_tg;
                int cvh1 = currentEdge.vh1_tg;

                if (!e0) {
                    if ( ((cvh0==vh1_tg)&&(cvh1==vh0_tg)) ||
                         ((cvh0==vh0_tg)&&(cvh1==vh1_tg)) ) {
                        currentFace.eh0_tg = currentEdge.eh_tg;
                        e0 = true;
                        continue;
                    }
                }
                if (!e1) {
                    if ( ((cvh0==vh2_tg)&&(cvh1==vh1_tg)) ||
                         ((cvh0==vh1_tg)&&(cvh1==vh2_tg)) ) {
                        currentFace.eh1_tg = currentEdge.eh_tg;
                        e1 = true;
                        continue;
                    }
                }
                if (!e2) {
                    if ( ((cvh0==vh0_tg)&&(cvh1==vh2_tg)) ||
                         ((cvh0==vh2_tg)&&(cvh1==vh0_tg)) ) {
                        currentFace.eh2_tg = currentEdge.eh_tg;
                        e2 = true;
                        continue;
                    }
                }
                if (e0 && e1 && e2)
                    n = edgeList.size();
            }
            if ( !(e0 && e1 && e2) && !noMoreFallBack ) {
                std::cout << "Bad geometry, fallback (will take longer)!" << std::endl;
                edgeList = edgesIn;
                noMoreFallBack = true;
                goto FallBackCase;
            }

            facesUnpaired.push_back( currentFace );
            facesNewIn.push_back( currentFace );
            ++facesInside;
        } else {
            // could be a new face with existing vertices or partially inside
            MyMesh::VertexHandle vh0_om = MyMesh::VertexHandle( vh0_tg );
            MyMesh::VertexHandle vh1_om = MyMesh::VertexHandle( vh1_tg );
            MyMesh::VertexHandle vh2_om = MyMesh::VertexHandle( vh2_tg );

            // this is not symmetrical (a,b)!=(b,a) since the next if statement depends on it!
            MyMesh::HalfedgeHandle heh0, heh1, heh2;
            heh0 = mesh.find_halfedge( vh1_om, vh0_om );
            heh1 = mesh.find_halfedge( vh2_om, vh1_om );
            heh2 = mesh.find_halfedge( vh0_om, vh2_om );

            bool heHandlesCheck = false;
            bool fhHandlesCheck = false;

            MyMesh::FaceHandle fh0;
            MyMesh::FaceHandle fh1;
            MyMesh::FaceHandle fh2;

            // we check whether all edges are really on the surface and also a very(!) tricky case at 2nd if
            // all edges are legit and they form a triangle but still it is not on the surface (look at the
            // small_torus.obj with the vertices: 4, 14, 16 colored)
            // note: if the geometry is safed with inverted normals the algorithm will break!
            if ( heh0.is_valid() && heh1.is_valid() && heh2.is_valid() ) {
                heHandlesCheck = true;
                fh0 = mesh.face_handle(heh0);
                fh1 = mesh.face_handle(heh1);
                fh2 = mesh.face_handle(heh2);

                if ( ((heh0.idx()-heh1.idx())==0) || ((heh0.idx()-heh2.idx())==0) || ((heh1.idx()-heh2.idx())==0) )
                    std::cout << "To an complex edge degenerated face encountered" << std::endl;

                if (fh0==fh1 && fh1==fh2 && fh2==fh0) {
                    fhHandlesCheck = true;
                }
            }
            // Debug
//            std::cout << i << ": "<< mesh.from_vertex_handle(heh0) << " " << mesh.from_vertex_handle(heh1) << " " << mesh.from_vertex_handle(heh2) << std::endl;
//            std::cout << i << ": " << heh0 << ", " << heh1 << ", " << heh2 << std::endl;
//            if (fh0!=fh1 || fh1!=fh2 || fh2!=fh0) {
//                    std::cout << i << ": "<< mesh.from_vertex_handle(heh0) << " " << mesh.from_vertex_handle(heh1) << " " << mesh.from_vertex_handle(heh2) << std::endl;
//                    std::cout << "fh: "<< fh0 << " " << fh1 << " " << fh2 << std::endl;
//                    std::cout << "---" << std::endl;
//            }

            if ( heHandlesCheck && fhHandlesCheck ) {
                // thus it is in fact an "old" surface triangle
                currentFace.surfaceFace = true;
                // add into the dictionary
                fhDictionary.insert( std::pair<MyMesh::FaceHandle,int>(fh0,i) );

                MyMesh::EdgeHandle eh0 = mesh.edge_handle(heh0);
                MyMesh::EdgeHandle eh1 = mesh.edge_handle(heh1);
                MyMesh::EdgeHandle eh2 = mesh.edge_handle(heh2);
                eh_OMtoTG::iterator dictionary_it0 = ehDictionary.find(eh0);
                eh_OMtoTG::iterator dictionary_it1 = ehDictionary.find(eh1);
                eh_OMtoTG::iterator dictionary_it2 = ehDictionary.find(eh2);

                if ( dictionary_it0 != ehNotFound )
                    currentFace.eh0_tg = (*dictionary_it0).second;
                else
                    std::cout << "The edge couldn't be found in the dictionary: " << eh0 << std::endl;
                if ( dictionary_it1 != ehNotFound )
                    currentFace.eh1_tg = (*dictionary_it1).second;
                else
                    std::cout << "The edge couldn't be found in the dictionary: " << eh1 << std::endl;
                if ( dictionary_it2 != ehNotFound )
                    currentFace.eh2_tg = (*dictionary_it2).second;
                else
                    std::cout << "The edge couldn't be found in the dictionary: " << eh2 << std::endl;

                bool positive = mesh.property(fPositive, fh0);
                currentFace.positive = positive;
                currentFace.killedEdges = mesh.property(killedEdges, fh0);

                if (!positive) {
                    currentFace.isPaired = true;
                    currentFace.fh_om = fh0.idx();
                } else {
                    currentFace.isPaired = false;
                    facesUnpaired.push_back( currentFace );
                    ++oldUnpaired;
                    // should match betti[2] and inside faces should match new faces
                }
                ++facesSurface;
            } else {
                currentFace.isPaired = false;
                currentFace.surfaceFace = false;

                bool e0 = false;
                bool e1 = false;
                bool e2 = false;

                if (heh0.is_valid()) {
                    MyMesh::EdgeHandle eh = mesh.edge_handle(heh0);
                    eh_OMtoTG::iterator dictionary_it = ehDictionary.find(eh);
                    if ( dictionary_it != ehNotFound ) {
                        currentFace.eh0_tg = (*dictionary_it).second;
                        e0 = true;
                    }
                }
                if (heh1.is_valid()) {
                    MyMesh::EdgeHandle eh = mesh.edge_handle(heh1);
                    eh_OMtoTG::iterator dictionary_it = ehDictionary.find(eh);
                    if ( dictionary_it != ehNotFound ) {
                        currentFace.eh1_tg = (*dictionary_it).second;
                        e1 = true;
                    }
                }
                if (heh2.is_valid()) {
                    MyMesh::EdgeHandle eh = mesh.edge_handle(heh2);
                    eh_OMtoTG::iterator dictionary_it = ehDictionary.find(eh);
                    if ( dictionary_it != ehNotFound ) {
                        currentFace.eh2_tg = (*dictionary_it).second;
                        e2 = true;
                    }
                }
                if ( !(e0 && e1 && e2) ) {
                    for (int n=0; n<(int)edgeList.size(); ++n) {
                        tgE currentEdge = edgeList.at(n);
                        int cvh0 = currentEdge.vh0_tg;
                        int cvh1 = currentEdge.vh1_tg;

                        if (!e0) {
                            if ( ((cvh0==vh1_tg)&&(cvh1==vh0_tg)) ||
                                 ((cvh0==vh0_tg)&&(cvh1==vh1_tg)) ) {
                                currentFace.eh0_tg = currentEdge.eh_tg;
                                e0 = true;
                                continue;
                            }
                        }
                        if (!e1) {
                            if ( ((cvh0==vh2_tg)&&(cvh1==vh1_tg)) ||
                                 ((cvh0==vh1_tg)&&(cvh1==vh2_tg)) ) {
                                currentFace.eh1_tg = currentEdge.eh_tg;
                                e1 = true;
                                continue;
                            }
                        }
                        if (!e2) {
                            if ( ((cvh0==vh0_tg)&&(cvh1==vh2_tg)) ||
                                 ((cvh0==vh2_tg)&&(cvh1==vh0_tg)) ) {
                                currentFace.eh2_tg = currentEdge.eh_tg;
                                e2 = true;
                                continue;
                            }
                        }
                        if (e0 && e1 && e2)
                            n = edgeList.size();
                    }
                }
                if ( !(e0 && e1 && e2) && !noMoreFallBack ) {
                    std::cout << "Bad geometry, fallback (will take longer)!" << std::endl;
                    edgeList = edgesIn;
                    noMoreFallBack = true;
                    goto FallBackCase;
                }

                facesUnpaired.push_back( currentFace );
                facesNewIn.push_back( currentFace );
                ++facesInside;
            }
        }
        facesIn.push_back( currentFace );
    }
    // Debug
//    for (int i=0; i<surfaceFaces; ++i) {
//        MyMesh::FaceHandle fh = MyMesh::FaceHandle(i);
//        MyMesh::ConstFaceVertexIter fvIter = mesh.cfv_iter(fh);
//        MyMesh::VertexHandle a = fvIter.handle(); ++fvIter;
//        MyMesh::VertexHandle b = fvIter.handle(); ++fvIter;
//        MyMesh::VertexHandle c = fvIter.handle();
//        std::cout << i << ": " << a << " " << b << " " << c << std::endl;
//    }
//    for (fh_OMtoTG::iterator it = fhDictionary.begin(); it!=fhDictionary.end(); ++it) {
//        std::cout << "om: " << it->first << " - tg: " << it->second << std::endl;
//    }
    // Sanity check
    if ( (facesSurface-surfaceFaces != 0) || (facesInside+facesSurface-tetrahedraFaces != 0) ) {
        std::cout << "Missed faces   : " << facesInside+facesSurface-tetrahedraFaces << std::endl;
        std::cout << "Faces inside   : " << facesInside << std::endl;
        std::cout << "Faces surface  : " << facesSurface << std::endl;
        std::cout << "Base mesh faces: " << surfaceFaces << std::endl;
        std::cout << "Faces tetraheda: " << tetrahedraFaces << std::endl;
    }
    if ( oldUnpaired != bettiNumber[2] ) {
        std::cout << "Old unpaired: " << oldUnpaired << std::endl;
        std::cout << "Betti[2]    : " << bettiNumber[2] << std::endl;
    }
    if ( facesInside-facesNewIn.size() != 0 ) {
        std::cout << "Faces inside   : " << facesInside << std::endl;
        std::cout << "New faces found: " << facesNewIn.size() << std::endl;
    }

    // assign paired partners to the edges
    const fh_OMtoTG::iterator fhNotFound = fhDictionary.end();
    pairSet = 0;
    notPaired = 0;
    missed = 0;
    int negEdgesOld = 0;

    for (int i=0; i<(int)edgesIn.size(); ++i) {

        tgE currentEdge = edgesIn.at(i);
        if ( (currentEdge.isPaired == true) && currentEdge.positive ) {

            MyMesh::FaceHandle partner = currentEdge.fh_om;
            int tgIdx = -1;
            fh_OMtoTG::iterator dictionary_it = fhDictionary.find(partner);

            if ( dictionary_it != fhNotFound ) {
                tgIdx = (*dictionary_it).second;
                (edgesIn.at(i)).fh_tg = tgIdx;
                ++pairSet;
            } else {
                ++missed;
                std::cout << "Check: " << currentEdge.eh_tg-surfaceEdges << std::endl;
                if (partner.is_valid())
                    std::cout << "The face couldn't be found in the dictionary: " << partner << std::endl;
            }
        } else if ( (currentEdge.isPaired == true) && !currentEdge.positive ) {
            ++negEdgesOld;
        } else {
            ++notPaired;
        }
    }
    // Sanity check
    if ((missed!=0) || (bettiNumber[1]+(int)edgesNewIn.size() != notPaired)) {
        std::cout << "Unable to pair edges: " << missed << std::endl;
        std::cout << "Paired edges found  : " << pairSet << std::endl;
        std::cout << "Betti[1] + new edges: " << bettiNumber[1]+edgesNewIn.size() << std::endl;
        std::cout << "Not paired          : " << notPaired << std::endl;
        std::cout << "Negative edges: " << negEdgesOld << std::endl;
        std::cout << "Tetrahedra: " << tetrahedraEdges << std::endl;
        std::cout << "Base mesh : " << surfaceEdges << std::endl;
    }

    // --- match the elements of the convex hull
    // update, the convex hull has flipped edges and can't be used "YM" does not respect the "Y" parameter
    /*
    std::cout << "... creating convex hull data." << std::endl;
    const int convexHullFaces = meshO.numberoftrifaces;
    std::set< std::set<int> > setInsideFaces;
    const int tetrahedraElements = meshI.numberoftetrahedra;
    for (int i = 0; i < tetrahedraElements; ++i) {
        std::set<int> tetFace;
        int face[3];
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+1];
        face[2] = meshI.tetrahedronlist[4*i+2];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+1];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+2];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+1];
        face[1] = meshI.tetrahedronlist[4*i+2];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
    }
    const std::set< std::set<int> >::iterator fhInsideNotFound = setInsideFaces.end();
    std::set< std::set<int> >::iterator tetFaces_it;
    int newface = 0;
    int oldface = 0;
    std::vector< std::set<int> > tris;
    tris.clear();

    for (int i = 0; i < convexHullFaces; ++i) {
        std::set<int> chFace;
        int face[3];
        face[0] = meshO.trifacelist[3*i+0];
        face[1] = meshO.trifacelist[3*i+1];
        face[2] = meshO.trifacelist[3*i+2];
        chFace.insert(face, face+3);

        tetFaces_it = setInsideFaces.find( chFace );
        if (tetFaces_it != fhInsideNotFound) {
            ++newface;
        } else {
            ++oldface;
        }
        tris.push_back( chFace );
        chFace.clear();
    }

    std::cout << "Tetrahedra faces: " << setInsideFaces.size() << std::endl;
    std::cout << "Faces surface   : " << surfaceFaces << std::endl;
    std::cout << "Faces inside    : " << facesNewIn.size()  << std::endl;
    std::cout << "Old faces: " << oldface << std::endl;
    std::cout << "New faces: " << newface << std::endl;
    */
    return;
}

void topological_loops::findHandleLoops(MyMesh &mesh) {

    std::cout << "-> start searching handle loops" << std::endl;

    // Sanity check
    if (bettiNumber[1]%2 != 0) {
        std::cout << "Error, the number of unpaired edges is odd!" << std::endl;
        std::cout << "B0: " << bettiNumber[0] << std::endl;
        std::cout << "B1: " << bettiNumber[1] << std::endl;
        std::cout << "B2: " << bettiNumber[2] << std::endl;
        std::cout << "new Vertices: " << verticesNewIn.size() << " / "<< verticesIn.size()-mesh.n_vertices() << std::endl;
        std::cout << "new Edges   : " << edgesNewIn.size() << " / "<< edgesIn.size()-mesh.n_edges() << std::endl;
        std::cout << "new Faces   : " << facesNewIn.size() << " / "<< facesIn.size()-mesh.n_faces() << std::endl;
    }

    // Filtration of the inside edges, the vertices have been set in the tetrahedrization phase
    for (int i=0; i<(int)edgesNewIn.size(); ++i) {
        tgE currentEdge = edgesNewIn.at(i);
        int vh0 = currentEdge.vh0_tg;
        int vh1 = currentEdge.vh1_tg;

        if ((vh0<0) || (vh1<0))
            std::cout << "Edge is invalid, couldn't find one of its vertices: " << currentEdge.eh_tg << std::endl;

        int youngest;
        if ( vh0 > vh1 )
            youngest = vh0;
        else if ( vh0 < vh1 )
            youngest = vh1;
        else
            std::cout << "Bad Problem" << std::endl;

//        std::cout << "vh0: " << vh0 << std::endl;
//        std::cout << "vh1: " << vh1 << std::endl;
        std::set<int> boundaryChain;
        boundaryChain.clear();
        boundaryChain.insert(vh0);
        boundaryChain.insert(vh1);

        int maxCountVertices = mesh.n_vertices()+verticesNewIn.size();
        tgV youngestVertex = verticesIn.at(youngest);
//        cout_tg(youngestVertex);

        while ( (youngestVertex.isPaired) && !boundaryChain.empty() ) {

            int eh = youngestVertex.eh_tg;
            if (eh<0){
                std::cout << " - Paired edge handle invalid." << std::endl;
                return;
            }

            tgE pairedEdge = edgesIn.at(eh);

            std::set< int > killedChain = pairedEdge.killedVertices;
            std::set< int >::iterator kc_it;

            //mod2 addition
            for ( kc_it = killedChain.begin(); kc_it != killedChain.end(); ++kc_it ) {

                if ( boundaryChain.find( *kc_it ) != boundaryChain.end() ) {
                    boundaryChain.erase( *kc_it );
                } else {
                    boundaryChain.insert( *kc_it );
                }
            }

            if (!boundaryChain.empty()) {

                youngest = -1;
                std::set<int>::iterator set_it;
                for (set_it = boundaryChain.begin(); set_it != boundaryChain.end(); ++set_it) {
                    if ( (*set_it) > youngest )
                        youngest = (*set_it);
                }
                if (youngest == -1)
                    std::cout << "Couldn't find a younger simplex in the chain." << std::endl;
                else
                    youngestVertex = verticesIn.at(youngest);

                --maxCountVertices;
                if (maxCountVertices==0) {                      
                    std::cout << "Endlosschleife (Edges)" << std::endl;
                    return;
                }
            }
        }
        if ( !boundaryChain.empty() ) {
            (edgesIn.at(currentEdge.eh_tg)).positive = false;
            (edgesIn.at(currentEdge.eh_tg)).killedVertices = boundaryChain;
            (edgesIn.at(currentEdge.eh_tg)).isPaired = false;

            (verticesIn.at(youngestVertex.vh_tg)).eh_tg = currentEdge.eh_tg;
            (verticesIn.at(youngestVertex.vh_tg)).isPaired = true;
            --bettiNumberInside[0];
        } else {
            (edgesIn.at(currentEdge.eh_tg)).positive = true;
            (edgesIn.at(currentEdge.eh_tg)).isPaired = false;
            ++bettiNumberInside[1];
        }
    }

    // Debug
    //    std::cout << "B0: " << bettiNumber[0] << " - " << bettiNumberInside[0] << std::endl;
    //    std::cout << "B1: " << bettiNumber[1] << " - " << bettiNumberInside[1] << std::endl;
    //    std::cout << "B2: " << bettiNumber[2] << " - " << bettiNumberInside[2] << std::endl;
    //
    //    std::cout << "\n--- Edges:" << std::endl;
    //    for (int i=0; i<(int)edges.size(); ++i) {
    //        tgE current = edges.at(i);
    ////        std::cout << i << " ";
    //        cout_tg(current);
    //    }
    //    std::cout << "\n--- Faces:" << std::endl;
    //    for (int i=0; i<(int)faces.size(); ++i) {
    //        tgF current = faces.at(i);
    ////        std::cout << i << " ";
    //        cout_tg(current);
    //    }
    //    std::cout << "\n" << std::endl;

    // Filtration of the faces
    int maxHandles = bettiNumber[1]/2;
    std::cout << "Max. number of handle loops: " << maxHandles << std::endl;

//    cout_tg( edgesIn.at(33) );
//    cout_tg( edgesIn.at(34) );
//    cout_tg( edgesIn.at(35) );
//    cout_tg( facesIn.at(34) );

    for (int i=0; i<(int)facesNewIn.size(); ++i) {
        // fallback for problems due to the mesh
        prob:

        tgF currentFace = facesNewIn.at(i);
        int eh0 = currentFace.eh0_tg;
        int eh1 = currentFace.eh1_tg;
        int eh2 = currentFace.eh2_tg;

        if ((eh0==eh1) || (eh0==eh2) || (eh1==eh2))
            std::cout << "Identical edges: " << eh0 << " " << eh1 << " " << eh2 << std::endl;

        if ((eh0<0) || (eh1<0) || (eh2<0))
            std::cout << "Face invalid, couldn't find all its edges: " << currentFace.fh_tg << std::endl;

        tgE youngestEdge;
        int youngest = -1;
        // the offset tackles a really bad bug were I didn't realize that because of the mix-up
        // edge order (some new before old) the filtration pairs surface edges too soon!
        int eh0Age = (edgesIn.at(eh0)).age;
        int eh1Age = (edgesIn.at(eh1)).age;
        int eh2Age = (edgesIn.at(eh2)).age;

        if ( (eh0Age>eh1Age) && (eh0Age>eh2Age) && (edgesIn.at(eh0)).positive)
            youngest = eh0;
        else if ( (eh1Age>eh2Age) && (edgesIn.at(eh1)).positive )
            youngest = eh1;
        else if (edgesIn.at(eh2).positive)
            youngest = eh2;
        else {
            std::cout << "No youngest positive edge at face: "<< currentFace.fh_tg << std::endl;
            ++i; goto prob;
        }

        youngestEdge = edgesIn.at(youngest);

        std::set<int> boundaryChain;
        std::set<int> handleLoops;
        std::set<int> handleLoopsShortest;
        bool firsthandle = true;
        boundaryChain.clear();
        boundaryChain.insert(eh0);
        boundaryChain.insert(eh1);
        boundaryChain.insert(eh2);

        int maxCountEdges = mesh.n_edges()+edgesNewIn.size();

        // Debug
//        int unpaired = 0;
//        int unpairedSurface = 0;
//        for (int j=0; j<(int)edgesIn.size(); ++j) {
//            tgE current = edgesIn.at(j);
//            if (current.positive && !current.isPaired) {
//                ++unpaired;
//                if (current.surfaceEdge)
//                    ++unpairedSurface;
//            }
//        }
//        std::cout << "> "; int ch = std::cin.get();
//        std::cout << "->Face "<< i << ". unpaired surface edges: " << unpairedSurface << ", in total: " << unpaired << std::endl;
//        std::cout << "  edge: " << youngestEdge.eh_tg << ", age: " << youngestEdge.age << ", pos.: " << youngestEdge.positive
//                  << ", paired: " << youngestEdge.isPaired << ", bd: "<< eh0 << ", "<< eh1 << ", "<< eh2 << std::endl;

        while ( (youngestEdge.positive) && (youngestEdge.isPaired) && !boundaryChain.empty() ) {

            int fh = youngestEdge.fh_tg;
            if (fh<0) {
                std::cout << " - Paired face handle invalid." << std::endl;
                cout_tg(youngestEdge);
                return;
            }

            tgF pairedFace = facesIn.at(fh);

            std::set< int > killedChain = pairedFace.killedEdges;
            std::set< int >::iterator kc_it;

            //mod2 addition
            // Debug
            std::set<int>::iterator set_it;
//            std::cout << "  boundary chain:";
//            for (set_it = boundaryChain.begin(); set_it != boundaryChain.end(); ++set_it) {
//                std::cout << " " << *set_it;
//            }
//            std::cout << std::endl;
//            std::cout << "  paired killed chain:";
            for ( kc_it = killedChain.begin(); kc_it != killedChain.end(); ++kc_it ) {
//                std::cout << " " << *kc_it;
                if ( boundaryChain.find( *kc_it ) != boundaryChain.end() ) {
                    boundaryChain.erase( *kc_it );
                } else {
                    boundaryChain.insert( *kc_it );
                }
            }
//            std::cout << std::endl;
//            std::cout << "> "; ch = std::cin.get();

            if (!boundaryChain.empty()) {

                bool allSurfaceEdges = true;
                youngest = -1;
                int youngestAge = -1;
                for (set_it = boundaryChain.begin(); set_it != boundaryChain.end(); ++set_it) {
                    if ( (( (edgesIn.at(*set_it)).age )>youngestAge) && (edgesIn.at(*set_it)).positive ) {
                        youngestAge = (edgesIn.at(*set_it)).age;
                        youngest = (*set_it);
                    }
                    tgE e = edgesIn.at( (*set_it) );
                    if (!e.surfaceEdge)
                        allSurfaceEdges = false;
                }
                if (firsthandle && allSurfaceEdges) {
                    handleLoopsShortest = boundaryChain;
                    firsthandle = false;
                } else if (allSurfaceEdges){
                    if (boundaryChain.size() < handleLoopsShortest.size())
                        handleLoopsShortest = boundaryChain;
                }
                if (youngest == -1)
                    std::cout << "Couldn't find a younger simplex in the chain." << std::endl;
                else
                    youngestEdge = edgesIn.at(youngest);

                // Debug, if there are cyclic dependencies in the filtration
//                if (youngestEdge.fh_tg >= 0) {
//                    tgF f = facesIn.at(youngestEdge.fh_tg);
//                    std::cout << "  "<< youngestEdge.eh_tg << " is paired: " << f.isPaired <<" with: " << f.fh_tg << std::endl;
//                    std::cout << "  chain: ";
//                    for (set_it = chainBoundary.begin(); set_it != chainBoundary.end(); ++set_it) {
//                        std::cout << " " << (*set_it);
//                    }
//                    std::cout << std::endl;
//                    int e0 = f.eh0_tg;
//                    int e1 = f.eh1_tg;
//                    int e2 = f.eh2_tg;
//                    std::cout << "  new edges: " << e0 << " " << e1 << " " << e2 << std::endl;
//                    std::cout << "  youngest edge: " << youngestEdge.eh_tg << ", age:" << youngestEdge.age << std::endl;
//                    for (set_it = chainBoundary.begin(); set_it != chainBoundary.end(); ++set_it) {
//                        cout_tg( edgesIn.at(*set_it) );
//                    }
//                } else {
//                    std::cout << "  invalid face handle:" << std::endl;
//                    cout_tg(youngestEdge);
//                }
                --maxCountEdges;
                if (maxCountEdges == 0) {
//                    std::cout << "Cyclic pairing of edges and faces -> skip face: " << currentFace.fh_tg << std::endl;
                    ++i; goto prob;
                }
            }
        }
        if ( !boundaryChain.empty() ) {

            (facesNewIn.at(i)).positive = false;
//            boundaryChain.clear();
//            boundaryChain.insert( currentFace.eh0_tg );
//            boundaryChain.insert( currentFace.eh1_tg );
//            boundaryChain.insert( currentFace.eh2_tg );
            (facesNewIn.at(i)).killedEdges = boundaryChain;
            (facesNewIn.at(i)).isPaired = false;
            facesIn.at(currentFace.fh_tg).positive = false;
            facesIn.at(currentFace.fh_tg).killedEdges = boundaryChain;
            facesIn.at(currentFace.fh_tg).isPaired = false;

            (edgesIn.at(youngestEdge.eh_tg)).fh_tg = currentFace.fh_tg;
            (edgesIn.at(youngestEdge.eh_tg)).isPaired = true;
            --bettiNumberInside[1];

            if ((edgesIn.at(youngestEdge.eh_tg)).surfaceEdge) {
                std::cout << "--> Found a surface loop: "<< maxHandles <<", of size: " << handleLoopsShortest.size() << std::endl;
                std::set<int>::iterator set_it;
                // set the edge to handle loop generating
                mesh.data(youngestEdge.eh_om).setEdgeCircle(-2);
                for (set_it = handleLoopsShortest.begin(); set_it != handleLoopsShortest.end(); ++set_it) {
                    // Visualize them
                    tgE ce = edgesIn.at((*set_it));
                    MyMesh::EdgeHandle eh = MyMesh::EdgeHandle(ce.eh_om);
                    if ( eh.is_valid() ) {
                        mesh.data(eh).setEdgeCircle(2);
                    }
                }
                --maxHandles;
                if (maxHandles == 0) {
                    i = facesNewIn.size();
                    std::cout << "All handles found!" << std::endl;
                }
            }
            // Debug
//            std::cout << "  face: " << currentFace.fh_tg << ", gets paired with edge: " << youngestEdge.eh_tg << std::endl;;
//            cout_tg( currentFace );
//            cout_tg( edgesIn.at(youngestEdge.eh_tg) );

        } else {
            // positive face
            (facesNewIn.at(i)).positive = true;
            (facesNewIn.at(i)).isPaired = false;
            facesIn.at(currentFace.fh_tg).positive = true;
            facesIn.at(currentFace.fh_tg).isPaired = false;
            ++bettiNumberInside[2];
        }
    }
    // end of filtration
}

void topological_loops::findTunnelLoops(MyMesh &mesh) {

    // Output the found generating edges
    for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) {
        if (mesh.data(e_it.handle()).getEdgeCircle() == -1) {

            MyMesh::HalfedgeHandle heh = mesh.halfedge_handle(e_it.handle(),0);
            MyMesh::VertexHandle vh0 = mesh.to_vertex_handle(heh);
            MyMesh::VertexHandle vh1 = mesh.from_vertex_handle(heh);

            std::cout << "Generating edge: " << vh0 << " - " << vh1 << std::endl;
        }
    }
}

std::vector< std::set<int> > topological_loops::test_function(MyMesh &mesh){

    mesh.n_faces();
    std::set< std::set<int> > setInsideFaces;
    std::vector< std::set<int> > tris;

    // Debug - only output if necessary
    std::cout << "- Tetrahedra faces -" << std::endl;
    for (int i = 0; i < meshI.numberoftetrahedra; i++) {
        // Debug
//        std::cout << i << ".tet: ";
//        for (int k = 0; k < 4; k++) std::cout << meshI.tetrahedronlist[4*i+k] << " ";
//        std::cout << std::endl;

        std::set<int> tetFace;
        int face[3];
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+1];
        face[2] = meshI.tetrahedronlist[4*i+2];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+1];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+0];
        face[1] = meshI.tetrahedronlist[4*i+2];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
        tetFace.clear();
        face[0] = meshI.tetrahedronlist[4*i+1];
        face[1] = meshI.tetrahedronlist[4*i+2];
        face[2] = meshI.tetrahedronlist[4*i+3];
        tetFace.insert(face, face+3);
        setInsideFaces.insert( tetFace );
    }
//    std::cout << "Total: " << setInsideFaces.size() << std::endl;
//    std::cout << "---\nSorted:" << std::endl;

    // int idx=0;
    for (std::set< std::set<int> >::iterator itIF = setInsideFaces.begin(); itIF!=setInsideFaces.end(); ++itIF) {
        // std::set<int> currentFace = *itIF;
        // std::cout << idx << ".f:";
        // for(std::set<int>::iterator it=currentFace.begin(); it!=currentFace.end(); ++it){
        //    std::cout << " " << *it;
        // }
        // std::cout << std::endl;
        // ++idx;

        tris.push_back( *itIF );
    }

    return tris;
}

// helper functions to output the vertices, edges and faces with all relevant information
void topological_loops::cout_tg(tgV &vertex) {

    std::cout << "nr: " << vertex.vh_tg << std::endl;
    std::cout << "p: " << vertex.positive << ", is paired: " << vertex.isPaired << std::endl;
    std::cout << "eh_tg: " << vertex.eh_tg << ", eh_om: " << vertex.eh_om << std::endl;
    std::cout << "vh_om: " << vertex.vh_om << std::endl;
    std::cout << "---" << std::endl;
}

void topological_loops::cout_tg(tgE &edge) {

    std::set<int>::iterator set_it;
    std::cout << "eh_tg: " << edge.eh_tg << ", eh_om: " << edge.eh_om << std::endl;
    std::cout << "p: " << edge.positive << ", is paired: " << edge.isPaired << std::endl;
    std::cout << "fh_om: " << edge.fh_om << ", fh_tg: " << edge.fh_tg << std::endl;
    std::cout << "age: " << edge.age << std::endl;
    std::cout << "surface: " << edge.surfaceEdge << std::endl;
    std::cout << "killed vertices:";
    for (set_it = edge.killedVertices.begin(); set_it != edge.killedVertices.end(); ++set_it) {
        std::cout << " " << *set_it;
    }
    std::cout << std::endl;
    std::cout << "---" << std::endl;
}

void topological_loops::cout_tg(tgF &face) {

    int eh0 = face.eh0_tg;
    int eh1 = face.eh1_tg;
    int eh2 = face.eh2_tg;
    std::set<int>::iterator set_it;
    std::cout << "fh_tg: " << face.fh_tg << ", fh_om: " << face.fh_om << std::endl;
    std::cout << "p: " << face.positive << ", is paired: " << face.isPaired << std::endl;
    std::cout << "edges: " << eh0 << " " << eh1 << " " << eh2 << std::endl;
    std::cout << "surface: " << face.surfaceFace << std::endl;
    std::cout << "killed edges:";
    for (set_it = face.killedEdges.begin(); set_it != face.killedEdges.end(); ++set_it) {
        std::cout << " " << *set_it;
    }
    std::cout << std::endl;
    std::cout << "---" << std::endl;
}


/* Random code stuff

    int face0[] = {3,16,7};
    int face1[] = {4,16,7};
    int face2[] = {12,11,9};
    int face3[] = {12,16,4};
    int face4[] = {3,8,7};
    int face5[] = {8,13,7};
    int face6[] = {4,13,7};
    int face7[] = {13,2,8};
    int face8[] = {3,0,8};
    int face9[] = {19,16,12};
    int face10[] = {3,16,7};
    int face11[] = {4,16,7};
    int face12[] = {12,11,9};
    int face13[] = {12,16,4};
    int face14[] = {3,8,7};
    int face15[] = {8,13,7};
    int face16[] = {4,13,7};
    int face17[] = {13,2,8};

    tris.clear();
    std::set<int> t;
    t.insert(face0, face0+3);
    tris.push_back( t );
    t.clear();
    t.insert(face1, face1+3);
    tris.push_back( t );
    t.clear();
    t.insert(face2, face2+3);
    tris.push_back( t );
    t.clear();
    t.insert(face3, face3+3);
    tris.push_back( t );
    t.clear();
    t.insert(face4, face4+3);
    tris.push_back( t );
    t.clear();
    t.insert(face5, face5+3);
    tris.push_back( t );
    t.clear();
    t.insert(face6, face6+3);
    tris.push_back( t );
    t.clear();
    t.insert(face7, face7+3);
    tris.push_back( t );
    t.clear();
    t.insert(face8, face8+3);
    tris.push_back( t );
    t.clear();
    t.insert(face9, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face10, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face11, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face12, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face13, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face14, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face15, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face16, face9+3);
    tris.push_back( t );
    t.clear();
    t.insert(face17, face9+3);
    tris.push_back( t );

    std::cout << "-------------" << std::endl;

    std::vector<tgV> v;
    std::vector<tgE> e;
    std::vector<tgF> f;

    for (int n=0; n<verticesIn.size(); ++n) {
        tgV cv = verticesIn.at(n);
        if ( cv.surfaceVertex ) {
            cv.positive = true;
            cv.isPaired = false;
            v.push_back(cv);
        }
    }
    for (int n=0; n<verticesIn.size(); ++n) {
        tgV cv = verticesIn.at(n);
        if ( !cv.surfaceVertex ) {
            cv.positive = true;
            cv.isPaired = false;
            v.push_back(cv);
        }
    }
    std::cout << "Vdiff: " << v.size()-verticesIn.size() << std::endl;
    for (int n=0; n<edgesIn.size(); ++n) {
        tgE ce = edgesIn.at(n);
        if ( ce.surfaceEdge ) {
            ce.isPaired = false;
            ce.positive = false;
            e.push_back( ce );
        }
    }
    for (int n=0; n<edgesIn.size(); ++n) {
        tgE ce = edgesIn.at(n);
        if ( !ce.surfaceEdge ) {
            ce.isPaired = false;
            ce.positive = false;
            e.push_back( ce );
        }
    }
    for (int n=0; n<e.size(); ++n) {
        (e.at(n)).age = n;
    }
    std::cout << "Ediff: " << e.size()-edgesIn.size() << std::endl;
    for (int n=0; n<facesIn.size(); ++n) {
        tgF cf = facesIn.at(n);
        if ( cf.surfaceFace ) {
        cf.isPaired = false;
        cf.positive = false;
        f.push_back( cf );
        }
    }
    for (int n=0; n<facesIn.size(); ++n) {
        tgF cf = facesIn.at(n);
        if ( !cf.surfaceFace ) {
        cf.isPaired = false;
        cf.positive = false;
        f.push_back( cf );
        }
    }
    std::cout << "Fdiff: " << f.size()-facesIn.size() << std::endl;
*/
