#include "viewer.h"
#include <QDebug>
#include <QMouseEvent>

using namespace qglviewer;

Viewer::Viewer() :
    topstoc(), drawingMode(0), drawList(0), vertexWeights(false), sampledVertices(false),
    controlPoints(false), remeshedRegions(false), decimatedMesh(false), displayUpdate(true),
    pickingEvent(false)
{
    connect(&topstoc, SIGNAL(writeToConsole(QString, int)),
            this, SLOT(passToConsole(QString, int)));
}

void Viewer::init() {
    drawList = glGenLists(1);
    this->setMouseTracking(false);
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

void Viewer::mouseMoveEvent(QMouseEvent *e) {

    if (e->buttons() & Qt::RightButton){
        QGLViewer::mouseMoveEvent(e);
    } else {

        switch (topstoc.options.mouseAction) {
        case 1:
            this->selectVertex(e->x(), e->y(), 1); break;
        case 2:
            this->selectVertex(e->x(), e->y(), 2); break;
        default: QGLViewer::mouseMoveEvent(e);
        }
    }
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
//    QString keyboardchar = k->text();
//    qDebug() << "Keyboard: " << keyboardchar << " - " << keyboardcharIdx;
    /* Numbers
        Reserved by libqglviewer: a, g, f, ?, s, h, esc, space, ent, c, arrows
        j = 74    + = 43    < = 60    ↑ = 16777235    del = 16777219
        k = 75    * = 42    > = 62    ←	= 16777234    ent = 16777220
        x = 88    - = 45              ↓ = 16777237
        u = 85    _ = 95              → = 16777236
    */
    float currentValue;

    switch (keyboardcharIdx) {
        // update display
        case 85: this->updateDisplay(); break;
        // selceted vertices are fix
        case 42: topstoc.setUserWeights(+1.0f); break;
        // selceted vertices increase weight
        case 43: topstoc.setUserWeights(+0.05f); return;
        // selceted vertices decrease weight
        case 45: topstoc.setUserWeights(-0.05f); return;
        // selceted vertices are to dismiss
        case 95: topstoc.setUserWeights(-1.0f); break;
        // set to slider value
        case 35:
            topstoc.setUserWeights(topstoc.options.intensity);
            break;
        // delete seletion
        case 16777219:
            topstoc.clearSelection();
            this->updateDisplay(); return;
        // change mouse action
        case 74:
            if (topstoc.options.mouseAction > 0) {
                --topstoc.options.mouseAction;
                QString msg = "Mouse Action: " + QString::number(topstoc.options.mouseAction);
                writeToConsole(msg, 10);
            }
            return;
        case 75:
            if (topstoc.options.mouseAction < 2) {
                ++topstoc.options.mouseAction;
                QString msg = "Mouse Action: " + QString::number(topstoc.options.mouseAction);
                writeToConsole(msg, 11);
            }
            return;
        // increase change slider position +0,01
        case 80:
            currentValue = topstoc.options.intensity;
            currentValue += 0.01f;
            if (currentValue > 1.0f)
                currentValue = 1.0f;
            topstoc.options.intensity = currentValue;
            writeToConsole( QString::number(currentValue), 12);
            return;
        // increase change slider position +0,10
        case 220:
            currentValue = topstoc.options.intensity;
            currentValue += 0.10f;
            if (currentValue > 1.0f)
                currentValue = 1.0f;
            topstoc.options.intensity = currentValue;
            writeToConsole( QString::number(currentValue), 12);
            return;
        // decrease change slider position -0,01
        case 214:
            currentValue = topstoc.options.intensity;
            currentValue -= 0.01f;
            if (currentValue < -1.0f)
                currentValue = -1.0f;
            topstoc.options.intensity = currentValue;
            writeToConsole( QString::number(currentValue), 12);
            return;
        // decrease change slider position -0,10
        case 196:
            currentValue = topstoc.options.intensity;
            currentValue -= 0.10f;
            if (currentValue < -1.0f)
                currentValue = -1.0f;
            topstoc.options.intensity = currentValue;
            writeToConsole( QString::number(currentValue), 12);
            return;
        // ---
        default : break;
    }

    QGLViewer::keyPressEvent(k);
    if (displayUpdate)
        this->updateDisplay();
}


void Viewer::selectVertex(int x, int y, int mode) {

    MyMesh::FaceHandle intersectionFace;
    intersectionFace = topstoc.rayIntersectsTriangle(x, y);
//    std::cout << "Mode: " << mode << std::endl;
//    std::cout << "Tri: " << intersectionFace << " - " << noHit << std::endl;
    if (intersectionFace.is_valid()) {

        switch (mode) {
        case 1:
            topstoc.setUserWeights(intersectionFace, 0.0f);
            break;
        case 2:
            topstoc.setUserWeights(intersectionFace, topstoc.options.intensity);
            break;
        default: break;
        }
    } else {
        passToConsole("no triangle hit", 3);
    }
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
