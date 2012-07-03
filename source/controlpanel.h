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

//unterschied zu... : public QWidget, private Ui::ControlPanel, wrecks the app
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
    //--- interaction
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

	void visualization(	int drawingMode, bool vertexWeights,		bool sampledVertices,
											bool controlPoints, bool remeshedRegions,	bool decimatedMesh);
	void invertNormals();
    void hausdorff( double samplingDensityUser);
    void cameraPosition ( Vec cameraVec );
	void test();

private:
	Ui::ControlPanel *ui;

    float cutDecimal (float myNumber, int decimal);
	int consoleLineNumber;
};

#endif // CONTROLPANEL_H
