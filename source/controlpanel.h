#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtGui>
#include <QLineEdit>
#include <QTest>
#include <QDateTime>

#include <QWidget>
#include "ui_controlpanel.h"

#include <QGLViewer/vec.h>
using qglviewer::Vec;


namespace Ui {
	class ControlPanel;
}

enum consoleMode {ALL, SAMPLING, INDEXING, DEBUG};

struct interactionVariables {
    interactionVariables() :
        mouseAction(0), povDecimation(false), povRatio(0.5f),
        keepSilhouette(false), silhouetteAngle(5),
        keepUVBoarders(false), cameraView(0.0f,0.0f,0.0f),
        intensity(0.0f) {}

    int mouseAction;
    bool povDecimation;
    float povRatio;
    bool keepSilhouette;
    int silhouetteAngle;
    bool keepUVBoarders;
    Vec cameraView;
    float intensity;
};


//unterschied zu... : public QWidget, private Ui::ControlPanel, wrecks the app - why?
class ControlPanel : public QWidget {
	Q_OBJECT

public:
	explicit ControlPanel(QWidget *parent = 0);
	~ControlPanel();


public slots:
	void writeToConsole(QString msg, int mode);
    void updateViewVec(Vec viewVec);

private slots:
	//--- filehandling
	void on_buttonLoad_clicked(); // Qt automatically connects ->
	void on_buttonSave_clicked();
    //--- sampling & remeshing
	void on_buttonCalculateWeights_clicked();
	void on_buttonRunSampling_clicked();	// <-
	void setSliderValue(int value);
	void on_buttonReIndex_clicked();
    //--- calculating and visualizing metrics
    void on_buttonHausdorff_clicked();
	//--- visualization
	void commitVisualOptions(int mode);
	void on_visualInvertNormals_stateChanged(int mode);
    void commitSliderValues();
    void on_buttonTurntable_clicked();
    //--- interaction
    void commitInteractionOptions();
    //--- topology
    void on_buttonFiltration_clicked();
    void on_buttonFindLoops_clicked();
    void on_buttonKillLoop_clicked();
    void on_buttonNextLoop_clicked();
    void on_buttonLastLoop_clicked();

    // testerei
	void on_buttonTest_clicked();

	void startConsole ();
	void startTabFunctions(int mode);
	void copyToClipboard();

	void debug ();

signals:
	void fileload (QString fileName);
	void filesave (QString fileName);

	void calculateweights (QString fileName);
	void runsampling (const float& adaptivity, const float& subsetTargetSize);

	void runremeshing (const QString& mode);

    void visualization(	int drawingMode, bool vertexWeights, bool sampledVertices, bool controlPoints, bool remeshedRegions, bool decimatedMesh, bool displayUpdate, int showBoundaries, bool showHausdroff);
    void interaction( interactionVariables options );
    void invertNormals();
    void hausdorff( double samplingDensityUser);
    void turntable();

    void filtration();
    void findLoops();
    void killLoop();

	void test();

private:
	Ui::ControlPanel *ui;

    float cutDecimal (float myNumber, int decimal);
	int consoleLineNumber;
    interactionVariables options;
    int showLoopNr;
    int loops;
};

#endif // CONTROLPANEL_H
