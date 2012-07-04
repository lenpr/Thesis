#include "viewer.h"
#include <QDebug>
#include <QMouseEvent>

using namespace qglviewer;

Viewer::Viewer() :
		topstoc(),
		drawingMode(0), vertexWeights(false), sampledVertices(false),
    controlPoints(false), remeshedRegions(false), decimatedMesh(false), displayUpdate(true) {

    connect(&topstoc, SIGNAL(writeToConsole(QString, int)),
            this, SLOT(passToConsole(QString, int)));
}

void Viewer::init() {
    drawList = glGenLists(1);
    this->setMouseTracking(true);
}

void Viewer::draw() {
    glCallList(drawList);
    this->sendCameraPosition();
}

QSize Viewer::minimumSizeHint () const {
	return QSize(450,600);
}

QSize Viewer::sizeHint () const {
	return QSize(600,600);
}


void Viewer::mouseReleaseEvent(QMouseEvent* e) {

    //qDebug() << "x " << e->x() << "y " << e->y();
    //topstoc.gl_select(e->x(), e->y());

    /*
      Modes:
      0 - no mouse action
      1 - select faces
      2 - fix face
      3 - inc face weight
      4 - dec face weight
      5 - dismiss face
    */
    switch (topstoc.options.mouseAction) {
    case 0: break;
    case 1:
        this->selectVertex(e->x(), e->y(), 1); break;
    case 2:
        this->selectVertex(e->x(), e->y(), 2); break;
    case 3:
        this->selectVertex(e->x(), e->y(), 3); break;
    case 4:
        this->selectVertex(e->x(), e->y(), 4); break;
    case 5:
        this->selectVertex(e->x(), e->y(), 5); break;
    default: break;
    }

    if (topstoc.options.mouseAction != 0) {
        if (displayUpdate)
            this->updateDisplay();
    }
    QGLViewer::mouseReleaseEvent(e);
}

void Viewer::keyPressEvent(QKeyEvent *k) {

    int keyboardcharIdx = k->key();
    QString keyboardchar = k->text();

    qDebug() << "Keyboard: " << keyboardchar << " - " << keyboardcharIdx;
    /* Numbers
        j = 74
        k = 75
        x = 88
        u = 85
    */
    switch (keyboardcharIdx) {
        case 85: this->updateDisplay(); break;
        case 74: qDebug() << "UP"; break;
        case 75: qDebug() << "DOWN"; break;
        default : break;
    }

    QGLViewer::keyPressEvent(k);
    if (displayUpdate)
        this->updateDisplay();
}


void Viewer::selectVertex(int x, int y, int mode) {

    mode = 0;
    MyMesh::FaceHandle intersectionFace;
    std::vector<MyMesh::FaceHandle> selectedTriangles;

    intersectionFace = topstoc.rayIntersectsTriangle(x, y);

    if (intersectionFace.is_valid())
        selectedTriangles.push_back(intersectionFace);

    topstoc.setUserWeights(selectedTriangles, 1.0f);
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

    this->updateDisplay();
}

void Viewer::saveModel (const QString &fileName) {
	if (!topstoc.saveMeshToFile (fileName.toStdString ())) {
		emit writeToConsole ("could not write", 0);
	}
}

void Viewer::stocWeights (const QString &mode) {
	emit writeToConsole ("calculating vertex weights", 0);
	if (!topstoc.calculateWeights (mode)) {
        emit writeToConsole ("couldn't calculate weights", 0);
	} else {
		emit writeToConsole ("weights calculated", 0);
		emit meshstatus (2);
	}
    this->updateDisplay();
}

void Viewer::stocSampling (const float& adaptivity, const float& subsetTargetSize) {
	emit writeToConsole ("try sampling mesh", 0);
	if ( !topstoc.runStocSampling(adaptivity, subsetTargetSize) ) {
		emit writeToConsole ("could not sample mesh", 0);
	} else {
		emit writeToConsole ("mesh sampled", 0);
        emit meshstatus (3);
	}
    this->updateDisplay();
}

void Viewer::topReMeshing (const QString &mode) {
	emit writeToConsole ("try remeshing", 0);
	if (!topstoc.runTopReMeshing (mode)) {
		emit writeToConsole ("could not remesh", 0);
        emit meshstatus (0);
	} else {
		emit writeToConsole ("mesheshing done", 0);
        emit meshstatus (4);
	}
    this->updateDisplay();
}

void Viewer::updateDisplay() {

    glNewList(drawList, GL_COMPILE);

    if (drawingMode != DRAW_MODE_POINT && drawingMode != DRAW_MODE_WIREFRAME
        && drawingMode != DRAW_MODE_FLAT && drawingMode != DRAW_MODE_SMOOTH) {
        glEndList();
        updateGL ();
        return;
    }

    switch (drawingMode) {

    case DRAW_MODE_POINT:
        glDisable(GL_LIGHTING);
        glPointSize(2.5f);
        glBegin (GL_POINTS);
        break;

    case DRAW_MODE_WIREFRAME:
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glBegin(GL_TRIANGLES);
        break;

    case DRAW_MODE_FLAT:
        glEnable (GL_LIGHT0);
        glShadeModel(GL_FLAT);
        glBegin(GL_TRIANGLES);
        break;

    case DRAW_MODE_SMOOTH:
        glEnable(GL_LIGHT0);
        glShadeModel(GL_SMOOTH);
        glBegin(GL_TRIANGLES);
        break;
    }

    if (decimatedMesh)
        topstoc.drawDecimatedMesh(vertexWeights);
    else
        topstoc.drawMesh(vertexWeights, remeshedRegions);
    glEnd();

    switch (drawingMode) {

    case DRAW_MODE_POINT:
        glEnable (GL_LIGHTING);
        break;

    case DRAW_MODE_WIREFRAME:
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_FILL);
        break;
    }

    if (sampledVertices || controlPoints) {
        glDisable(GL_LIGHTING);
        glPointSize(3.0f);
        glBegin(GL_POINTS);

        topstoc.drawSamplAndControlPoints (sampledVertices, controlPoints);

        glEnd();
        glEnable (GL_LIGHTING);
    }

    switch (drawingMode) {

    case DRAW_MODE_WIREFRAME:
        glEnable(GL_LIGHTING);
        break;
    case DRAW_MODE_FLAT: case DRAW_MODE_SMOOTH:
        glDisable(GL_LIGHT0);
        break;
    }

    glEndList();
    updateGL ();
}

void Viewer::visualization (	int drawingMode, bool vertexWeights, bool sampledVertices,
                                bool controlPoints, bool remeshedRegions, bool decimatedMesh,
                                bool displayUpdate) {

	this->drawingMode = drawingMode;
	this->vertexWeights = vertexWeights;
    this->sampledVertices = sampledVertices;
	this->controlPoints = controlPoints;
	this->remeshedRegions = remeshedRegions;
	this->decimatedMesh = decimatedMesh;
    this->displayUpdate = displayUpdate;

	emit writeToConsole ("new visualization options set", 3);

    this->updateDisplay();
}

void Viewer::interaction(interactionVariables currentOptions) {

    topstoc.options = currentOptions;
}

void Viewer::invertNormals () {
	topstoc.invertNormals ();
    this->updateDisplay();
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
