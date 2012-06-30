#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include "ui_controlpanel.h"

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


private slots:
	//--- filehandling
	void on_buttonLoad_clicked(); // Qt automatically connects ->
	void on_buttonSave_clicked();
	//--- sampling
	void on_buttonCalculateWeights_clicked();
	void on_buttonRunSampling_clicked();	// <-
	void setSliderValue(int value);
	//--- remeshing
	void on_buttonReIndex_clicked();
	//--- visualization
	void commitVisualOptions(int mode);
	void on_visualInvertNormals_stateChanged(int mode);
	// auto connect
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
	void test();

private:
	Ui::ControlPanel *ui;

	int consoleLineNumber;
};

#endif // CONTROLPANEL_H
