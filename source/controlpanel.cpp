
#include "controlpanel.h"


ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
		ui(new Ui::ControlPanel) {
	ui->setupUi(this);

	// init of ControlPanel
	//---
	ui->ProgressBar->setVisible (false);
	//ui->boxMeshInfo->setVisible (false);
	ui->ConsoleOutput->setReadOnly (true);
	//---
	ui->tabWidgetFunctions->setDisabled (true);
	//--- console default is deactivated
	//QFont font; font.setBold(true);
	//ui->consoleSwitch->setFont (font);
	//ui->consoleSwitch->setText ("Activate");
	//ui->consoleClear->setDisabled (true);
	//ui->consoleCopy->setDisabled (true);
	//ui->consolOutputSelect->setDisabled (true);
	//ui->LabelConsoleOutput->setVisible (false);
	//ui->ConsoleOutput->setVisible (false);
	//---
	consoleLineNumber = 0;

	// signals/slots connect
	connect(		ui->consoleSwitch,	SIGNAL(clicked()),
							this,							SLOT(startConsole()));
	connect(		ui->consoleClear, SIGNAL(clicked()),
							ui->ConsoleOutput,	SLOT(clear()));
	connect(		ui->consoleCopy,		SIGNAL(clicked()),
							this,	SLOT(copyToClipboard()));
	connect(		ui->sliderSubset,	SIGNAL(valueChanged(int)),
							this, SLOT(setSliderValue(int)));
    //
    connect(    ui->sliderVecX, SIGNAL(valueChanged(int)),
                this, SLOT(commitSliderValues()) );
    connect(    ui->sliderVecY, SIGNAL(valueChanged(int)),
                this, SLOT(commitSliderValues()) );
    connect(    ui->sliderVecZ, SIGNAL(valueChanged(int)),
                this, SLOT(commitSliderValues()) );
	// visualization
	connect(		ui->visualDrawingMethod, SIGNAL(currentIndexChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	connect(		ui->visualVertexWeights, SIGNAL(stateChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	connect(		ui->visualSampledVertices, SIGNAL(stateChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	connect(		ui->visualControlPoints, SIGNAL(stateChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	connect(		ui->visualRemeshedRegions, SIGNAL(stateChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	connect(		ui->visualDecimatedMesh, SIGNAL(stateChanged(int)),
							this,	SLOT(commitVisualOptions(int)));
	//connect(		ui->visualInvertNormals, SIGNAL(stateChanged(int)),
	//						this,	SLOT(invertNormals()));
	// debug
	ui->buttonTest->setVisible (true);
	connect(		ui->buttonTest, SIGNAL(clicked()),
							this,							SLOT(debug()));
}

ControlPanel::~ControlPanel() {
	delete ui;
}

void ControlPanel::on_buttonLoad_clicked () {
	QString filePath;

	// QUICK FIX ACHTUNG ###
	int mode = ui->visualDrawingMethod->currentIndex();

	emit visualization ( mode, false, false, false, false, false );
	ui->visualVertexWeights->setChecked (false);
	ui->visualSampledVertices->setChecked (false);
	ui->visualControlPoints->setChecked (false);
	ui->visualRemeshedRegions->setChecked (false);
	ui->visualDecimatedMesh->setChecked (false);

	// ###

	// Bedingungen sollten nicht hart gecoded werden
	// OpenMesh::IO::_IOManager_::qt_read_filters ();
	filePath = QFileDialog::getOpenFileName(
			this,
			"Choose a file to open:",
			QString::null,
			"Meshes (*.off *.obj *.stla *om .stl	*.stlb *.stl);;Text Files (*.txt)", 0,
			QFileDialog::ReadOnly);

	if (filePath.isEmpty ()) {
		writeToConsole ("loading canceled by user", 0);
		return;
	}
	else
		writeToConsole ("try opening file: " + filePath, 0);

	ui->ProgressBar->setVisible (true);
	ui->ProgressBar->setValue (10);
	QTest::qWait (1);

	emit fileload (filePath);

	for (int i=11; i<101; ++i) {
		ui->ProgressBar->setValue (i);
		QTest::qWait (1);
	}
	ui->ProgressBar->setVisible (false);
}

void ControlPanel::on_buttonSave_clicked () {

	QString filePath = QFileDialog::getSaveFileName(
			this,
			"Save File",
			QString::null,
			"Meshes (*.off *.obj *.stla *om .stl	*.stlb *.stl);;Text Files (*.txt)",0,0);

	if (filePath.isEmpty ()) {
		writeToConsole ("saving canceled by user", 0);
		return;
	}
	else {
		ui->ProgressBar->setVisible (true);
		ui->ProgressBar->setValue (10);
		QTest::qWait (1);

		emit filesave (filePath);

		for (int i=11; i<101; ++i) {
			ui->ProgressBar->setValue (i);
			QTest::qWait (5);
		}
		writeToConsole ("saving file: "+filePath, 0);
		ui->ProgressBar->setVisible (false);
	}
}

void ControlPanel::on_buttonCalculateWeights_clicked () {
	emit calculateweights ("bla2");
}

void ControlPanel::on_buttonRunSampling_clicked () {

	float subsetTargetSize = ui->sliderSubset->value ();
	float adaptivity = 0.65;
	if ( ui->checkBoxAdaptivity->isChecked() )
		adaptivity = ui->spinBoxAdaptivity->value();
	emit runsampling (adaptivity, subsetTargetSize);
}

void ControlPanel::startConsole () {
	bool state = ui->LabelConsoleOutput->isVisible ();

	ui->consoleClear->setDisabled (state);
	ui->consoleCopy->setDisabled (state);
	ui->consolOutputSelect->setDisabled (state);

	ui->LabelConsoleOutput->setVisible (!state);
	ui->ConsoleOutput->setVisible (!state);

	if (!state) {
		QFont buttonStyle;
		buttonStyle.setBold (state);
		ui->consoleSwitch->setFont (buttonStyle);
		ui->consoleSwitch->setText ("Deactivate");
	} else {
		QFont buttonStyle;
		buttonStyle.setBold (state);
		ui->consoleSwitch->setFont (buttonStyle);
		ui->consoleSwitch->setText ("Activate");
	}
}

void ControlPanel::copyToClipboard () {
	QClipboard *cb = QApplication::clipboard();
	QString str = ui->ConsoleOutput->toPlainText ();
	// Copy text into the clipboard
	cb->setText (str);
	ui->ConsoleOutput->append ("# copied to clipboard #");
}

void ControlPanel::writeToConsole (QString msg, int mod) {
	if (!ui->LabelConsoleOutput->isVisible ())
		return;

	QString output;
	if (ui->consoleLineNumbers->isChecked ())
		output.append ("["+ QString::number(consoleLineNumber) + "] ");
	if (ui->consoleTimeStamp->isChecked ()) {
		QTime time = QTime::currentTime();
		QString timeString = time.toString();
		output.append (timeString + "\n - ");
	}

	// for the information panel, I don't like the code
    switch (mod) {
    case 4:
        ui->labelNumberVerticesValue->setText (msg);
        return;
    case 5:
        ui->labelSampledVerticesValue->setText (msg);
        return;
    case 6:
        ui->labelSampledVerticesRatio->setText (msg);
        return;
    case 7:
        ui->labelHausdorffMeanDistance->setText (msg);
        return;
    case 8:
        ui->labelHausdorffMaxDistance->setText (msg);
        return;
    }
	// ---

	switch (ui->consolOutputSelect->currentIndex()) {
	case 1:
		if (mod == 1) {
			output.append (msg);
			++consoleLineNumber;
			ui->ConsoleOutput->append(output);
		}
		break;
	case 2:
		if (mod == 2) {
			output.append (msg);
			++consoleLineNumber;
			ui->ConsoleOutput->append(output);
		}
		break;
	case 3:
		if (mod == 3) {
			output.append (msg);
			++consoleLineNumber;
			ui->ConsoleOutput->append(output);
		}
		break;
	default:
		output.append (msg);
		++consoleLineNumber;
		ui->ConsoleOutput->append(output);
		break;
	}
}
// ToDo dieses Funktionalität wurde halb zerstört
void ControlPanel::startTabFunctions (int mode) {
	switch (mode) {
	case 1:
		ui->tabWidgetFunctions->setEnabled (true);
        ui->TabDecimation->setEnabled(true);
        ui->TabTopology->setEnabled(true);
        ui->TabMetrics->setEnabled(false);
        writeToConsole ("mesh can be sampled and triangulated", 1);
		break;
	case 2:
        ui->tabWidgetFunctions->setEnabled (true);
        ui->TabDecimation->setEnabled(true);
        ui->TabTopology->setEnabled(true);
        ui->TabMetrics->setEnabled(true);
		writeToConsole ("mesh can be re-indexed", 2);
		break;
	case 3:
        ui->tabWidgetFunctions->setEnabled (true);
        ui->TabDecimation->setEnabled(true);
        ui->TabTopology->setEnabled(true);
        ui->TabMetrics->setEnabled(true);
        writeToConsole ("custom re-indexing", 2);
		break;
	default:
		ui->tabWidgetFunctions->setDisabled (true);
		writeToConsole ("sampling/re-indexing disabled", 0);
		break;
	}

}

void ControlPanel::setSliderValue (int value) {
	if (value > 80) {
		ui->sliderSubset->setValue(80);
		value = 80;
	}
	if (value < 1) {
		ui->sliderSubset->setValue (1);
		value = 1;
	}
	ui->labelSliderValue->setText ( QString::number(value) );
}


void ControlPanel::on_buttonReIndex_clicked () {
	emit runremeshing ("mesh it baby");
}


void ControlPanel::commitVisualOptions (int mode) {

	// this line is just to make the compiler happy
	if (mode != ui->visualDrawingMethod->currentIndex())
		mode = ui->visualDrawingMethod->currentIndex();

	emit visualization ( mode,
				ui->visualVertexWeights->isChecked(),
				ui->visualSampledVertices->isChecked(),
				ui->visualControlPoints->isChecked(),
				ui->visualRemeshedRegions->isChecked(),
				ui->visualDecimatedMesh->isChecked() );
}


void ControlPanel::on_visualInvertNormals_stateChanged (int mode) {
	emit invertNormals();
	writeToConsole ("inverting normals #" + QString::number(mode), 3);
}

// debug
void ControlPanel::debug () {
	/*
		MyMesh mesh = makeMesh ();
		for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) {

			float x = mesh.point(v_it)[0];
			float y = mesh.point(v_it)[1];
			float z = mesh.point(v_it)[2];

			std::cout <<x<<","<<y<<","<<z<< std::endl;
		}
	*/
}

void ControlPanel::on_buttonHausdorff_clicked() {
    double samplingDensityUser = 0.0l;

    ui->ProgressBar->setVisible (true);
    ui->ProgressBar->setValue (20);
    QTest::qWait (1);

    emit hausdorff(samplingDensityUser);

    for (int i=21; i<101; ++i) {
        ui->ProgressBar->setValue (i);
    }
    QTest::qWait (1);
    ui->ProgressBar->setVisible (false);
}

void ControlPanel::commitSliderValues() {

    Vec cameraVec;

    cameraVec[0] = ui->sliderVecX->value();
    cameraVec[1] = ui->sliderVecY->value();
    cameraVec[2] = ui->sliderVecZ->value();

    ui->labelVecX->setText( QString::number(cameraVec[0]/10) );
    ui->labelVecY->setText( QString::number(cameraVec[1]/10) );
    ui->labelVecZ->setText( QString::number(cameraVec[2]/10) );

    emit cameraPosition( cameraVec );
}

void ControlPanel::updateViewVec(Vec viewVec) {

    ui->labelVecWorldX->setText( QString::number(cutDecimal(viewVec[0],1)) );
    ui->labelVecWorldY->setText( QString::number(cutDecimal(viewVec[1],1)) );
    ui->labelVecWorldZ->setText( QString::number(cutDecimal(viewVec[2],1)) );
}

float ControlPanel::cutDecimal(float myNumber, int decimal) {

    myNumber = myNumber*( pow(10,decimal) );
    myNumber = (int)myNumber;
    myNumber = myNumber/( pow(10,decimal) );

    return myNumber;
}

void ControlPanel::on_buttonTest_clicked () {

	emit test();
}
