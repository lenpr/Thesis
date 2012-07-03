#include "viewer.h"
#include <QDebug>
#include <QMouseEvent>

using namespace qglviewer;

Viewer::Viewer() :
		topstoc(),
		drawingMode(0), vertexWeights(false), sampledVertices(false),
		controlPoints(false), remeshedRegions(false), decimatedMesh(false) {

	connect (&topstoc, SIGNAL(writeToConsole(QString, int)),
					 this,	SLOT(passToConsole(QString, int)));
}

void Viewer::init() {

    this->setMouseTracking(true);
}

void Viewer::draw() {

	switch (drawingMode) {

		// draw point cloud
	case 0:        
		glDisable(GL_LIGHTING);
        glPointSize(2.5f);
        glBegin (GL_POINTS);
		if (decimatedMesh)
			topstoc.drawDecimatedMesh(vertexWeights);
		else
            topstoc.drawMesh(vertexWeights, remeshedRegions);
        glEnd();
		glEnable (GL_LIGHTING);

		if (sampledVertices || controlPoints)
			topstoc.drawSamplAndControlPoints (sampledVertices, controlPoints);

        this->sendCameraPosition();
		break;

		// draw wireframe
	case 1:
		// turn on wireframe mode
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		glBegin(GL_TRIANGLES);
		if (decimatedMesh)
			topstoc.drawDecimatedMesh(vertexWeights);
		else
            topstoc.drawMesh(vertexWeights, remeshedRegions);
		glEnd();
		// turn off wireframe mode
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);

		if (sampledVertices || controlPoints)
			topstoc.drawSamplAndControlPoints (sampledVertices, controlPoints);

        this->sendCameraPosition();
		glEnable (GL_LIGHTING);
		break;

        // draw flat shading
	case 2:
		glEnable (GL_LIGHT0);
        glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		if (decimatedMesh)
			topstoc.drawDecimatedMesh(vertexWeights);
		else
            //topstoc.drawTriangles();
            topstoc.drawMesh(vertexWeights, remeshedRegions);
		glEnd();
		if (sampledVertices || controlPoints)
			topstoc.drawSamplAndControlPoints (sampledVertices, controlPoints);

        this->sendCameraPosition();
        glDisable(GL_LIGHT0);
        break;

        // draw smooth shading
    case 3:
        glEnable(GL_LIGHT0);
        glShadeModel(GL_SMOOTH);
        glBegin(GL_TRIANGLES);
        if (decimatedMesh)
            topstoc.drawDecimatedMesh(vertexWeights);
        else
            //topstoc.drawTriangles();
            topstoc.drawMesh(vertexWeights, remeshedRegions);
        glEnd();
        if (sampledVertices || controlPoints)
            topstoc.drawSamplAndControlPoints (sampledVertices, controlPoints);

        this->sendCameraPosition();
        glDisable(GL_LIGHT0);
        break;

        // draw textures
	case 4:
		break;
	}
}

QSize Viewer::minimumSizeHint () const {
	return QSize(450,600);
}

QSize Viewer::sizeHint () const {
	return QSize(600,600);
}



void Viewer::mouseReleaseEvent(QMouseEvent* e) {

    qDebug() << "x " << e->x() << "y " << e->y();

    //topstoc.gl_select(e->x(), e->y());
    this->selectVertex(e->x(), e->y());

    QGLViewer::mouseReleaseEvent(e);
}


void Viewer::keyPressEvent(QKeyEvent *k) {

    int keyboardcharIdx = k->key();
    QString keyboardchar = k->text();

    if (keyboardchar == "j")
        qDebug() << "DOWN";
    if (keyboardchar == "k")
        qDebug() << "UP";
    qDebug() << "Keyboard: " << keyboardchar << " - " << keyboardcharIdx;

    QGLViewer::keyPressEvent(k);
}


void Viewer::selectVertex(int x, int y) {

    // This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
       double mvmatrix[16];
       double projmatrix[16];
       int viewport[4];
       double dX1, dY1, dZ1, dX2, dY2, dZ2, dClickY; // glUnProject uses doubles, but I'm using floats for these 3D vectors

       glGetIntegerv(GL_VIEWPORT, viewport);
       glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
       glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
       dClickY = double (viewport[3] - y); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

       gluUnProject ((double) x, dClickY, 0.0, mvmatrix, projmatrix, viewport, &dX1, &dY1, &dZ1);
       // ClickRayP1 = Vector3 ( (float) dX, (float) dY, (float) dZ );
       gluUnProject ((double) x, dClickY, 1.0, mvmatrix, projmatrix, viewport, &dX2, &dY2, &dZ2);
       // ClickRayP2 = Vector3 ( (float) dX, (float) dY, (float) dZ );

       //qDebug() << "x " << dX1 << " - y " << dY1 << " - z " << dZ1 << endl;

       OpenMesh::Vec3f rayP1(dX1, dY1, dZ1);
       OpenMesh::Vec3f rayP2(dX2, dY2, dZ2);

       topstoc.rayIntersectsTriangle(rayP1, rayP2);
}



/* slots */

void Viewer::loadModel (const QString& fileName) {

	if (!topstoc.loadMeshFromFile (fileName.toStdString ())) {
		emit writeToConsole("could not load", 0);
	} else {
		emit writeToConsole("loaded", 0);
		emit meshstatus(1);
	}

	// calculate bounds of model and set scene accordingly
	topstoc.setModelBounds();
    Vec sceneMin = topstoc.bbox.getMinPoint();
    Vec sceneMax = topstoc.bbox.getMaxPoint();
    setSceneBoundingBox(sceneMin, sceneMax);
	camera()->showEntireScene();

	// generat vertex/face normals if needed
	topstoc.initMesh();

	updateGL ();
}

void Viewer::saveModel (const QString &fileName) {
	if (!topstoc.saveMeshToFile (fileName.toStdString ())) {
		emit writeToConsole ("could not write", 0);
	}
}

void Viewer::stocWeights (const QString &mode) {
	emit writeToConsole ("calculating vertex weights", 0);
	if (!topstoc.calculateWeights (mode)) {
		emit writeToConsole ("could calculate weights", 0);
	} else {
		emit writeToConsole ("weights calculated", 0);
		emit meshstatus (2);
	}
	updateGL ();
}

void Viewer::stocSampling (const float& adaptivity, const float& subsetTargetSize) {
	emit writeToConsole ("try sampling mesh", 0);
	if ( !topstoc.runStocSampling(adaptivity, subsetTargetSize) ) {
		emit writeToConsole ("could not sample mesh", 0);
	} else {
		emit writeToConsole ("mesh sampled", 0);
		emit meshstatus (2);
	}
	updateGL ();
}

void Viewer::topReMeshing (const QString &mode) {
	emit writeToConsole ("try remeshing", 0);
	if (!topstoc.runTopReMeshing (mode)) {
		emit writeToConsole ("could not remesh", 0);
	} else {
		emit writeToConsole ("mesheshing done", 0);
		emit meshstatus (2);
	}
	updateGL ();
}

void Viewer::visualization (	int drawingMode, bool vertexWeights, bool sampledVertices,
															bool controlPoints, bool remeshedRegions, bool decimatedMesh) {

	this->drawingMode = drawingMode;
	this->vertexWeights = vertexWeights;
	this->sampledVertices = sampledVertices;
	this->controlPoints = controlPoints;
	this->remeshedRegions = remeshedRegions;
	this->decimatedMesh = decimatedMesh;

	emit writeToConsole ("new visualization options set", 3);

	updateGL ();
}

void Viewer::invertNormals () {
	topstoc.invertNormals ();
	updateGL ();
}

void Viewer::passToConsole (const QString& msg, int mode) {
	emit writeToConsole (msg, mode);
}

void Viewer::hausdorff (double sampling_density_user) {
    topstoc.calculateHausdorff (sampling_density_user);
}

void Viewer::sendCameraPosition() {

    GLfloat m[16];
    glGetFloatv (GL_MODELVIEW_MATRIX, m);
    Vec viewVec(m[8], m[9], m[10]);
    //qDebug() << "x " << m[8] << " - y " << m[9] << " - z " << m[10] << endl;
    emit updateViewVec(viewVec);
}

void Viewer::test () {
	topstoc.test ();
}
