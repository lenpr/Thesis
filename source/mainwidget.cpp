#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent), viewer(0), controlpanel(0) {

	controlpanel = new ControlPanel();
	viewer = new Viewer();

	QHBoxLayout* mainlayout = new QHBoxLayout;
	mainlayout->addWidget (viewer);
	mainlayout->addWidget (controlpanel);

	setLayout(mainlayout);

	QDesktopWidget *desktop = QApplication::desktop();
	int screenWidth, width;
	int screenHeight, height;
	int x, y;
	QSize windowSize;

    // place widget in the middle of the screen
	screenWidth = desktop->width(); // get width of screen
	screenHeight = desktop->height(); // get height of screen
	windowSize = size(); // size of our application window
	width = windowSize.width();
	height = windowSize.height();
	x = (screenWidth - width)/2;
	y = (screenHeight - height)/2;
    move (x, y); // move window to desired coordinates

    setWindowTitle(tr("Fast User-Guided Mesh Simplification with Topology Control"));

	// Connects
	//--- cosole to viewer
	connect (controlpanel, SIGNAL(fileload(QString)),
					 viewer, SLOT(loadModel(QString)));
	connect (controlpanel, SIGNAL(filesave(QString)),
					 viewer, SLOT(saveModel(QString)));
	connect (controlpanel, SIGNAL(calculateweights(QString)),
					 viewer, SLOT(stocWeights(QString)));
	connect (controlpanel, SIGNAL(runsampling(float, float)),
					 viewer, SLOT(stocSampling(float, float)));
	connect (controlpanel, SIGNAL(runremeshing(QString)),
					 viewer, SLOT(topReMeshing(QString)));
	connect (controlpanel, SIGNAL(visualization(int,bool,bool,bool,bool,bool)),
					 viewer, SLOT(visualization(int,bool,bool,bool,bool,bool)));
	connect (controlpanel, SIGNAL(invertNormals()),
					 viewer,	 SLOT(invertNormals()));
	connect (controlpanel, SIGNAL(test()),
					 viewer, SLOT(test()));

	//--- viewer to console
	connect (viewer, SIGNAL(writeToConsole(QString,int)),
					 controlpanel, SLOT(writeToConsole(QString,int)));
	connect (viewer, SIGNAL(meshstatus(int)),
					 controlpanel, SLOT(startTabFunctions(int)));
}

MainWidget::~MainWidget() {
	delete controlpanel;
	delete viewer;
}
