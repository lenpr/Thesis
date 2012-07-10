
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
    ui->tabWidgetFunctions->setEnabled(false);
    ui->visualVertexWeights->setEnabled(false);
    ui->visualSampledVertices->setEnabled(false);
    ui->visualControlPoints->setEnabled(false);
    ui->visualRemeshedRegions->setEnabled(true);
    ui->visualDecimatedMesh->setEnabled(false);
    ui->visualShowBoundaries->setEnabled(true);
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
    showLoopNr = 1;
    loops = 0;

    // signals/slots connect
    connect( ui->consoleSwitch,	SIGNAL(clicked()),
             this, SLOT(startConsole()));
    connect( ui->consoleClear, SIGNAL(clicked()),
             ui->ConsoleOutput,	SLOT(clear()));
    connect( ui->consoleCopy, SIGNAL(clicked()),
             this, SLOT(copyToClipboard()));
    connect( ui->sliderSubset,	SIGNAL(valueChanged(int)),
             this, SLOT(setSliderValue(int)));
    // sliders
    connect( ui->sliderVecX, SIGNAL(valueChanged(int)),
             this, SLOT(commitSliderValues()) );
    connect( ui->sliderVecY, SIGNAL(valueChanged(int)),
             this, SLOT(commitSliderValues()) );
    connect( ui->sliderVecZ, SIGNAL(valueChanged(int)),
             this, SLOT(commitSliderValues()) );
    connect( ui->sliderIntensity, SIGNAL(valueChanged(int)),
             this, SLOT(commitSliderValues()));
    // visualization
    connect( ui->visualDrawingMethod, SIGNAL(currentIndexChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualVertexWeights, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualSampledVertices, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualControlPoints, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualRemeshedRegions, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualDecimatedMesh, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualDisplayUpdate, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->visualShowBoundaries, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)));
    connect( ui->checkBoxVisualizeHausdorff, SIGNAL(stateChanged(int)),
             this, SLOT(commitVisualOptions(int)) );
    // interaction
    connect( ui->comboBoxMouseAction, SIGNAL(currentIndexChanged(int)),
             this, SLOT(commitInteractionOptions()));
    connect( ui->checkBoxPOV, SIGNAL(stateChanged(int)),
             this, SLOT(commitInteractionOptions()));
    connect( ui->spinBoxPOV, SIGNAL(valueChanged(double)),
             this, SLOT(commitInteractionOptions()));
    connect( ui->checkBoxSilhouette, SIGNAL(stateChanged(int)),
             this, SLOT(commitInteractionOptions()));
    connect( ui->spinBoxSilhouette, SIGNAL(valueChanged(int)),
             this, SLOT(commitInteractionOptions()));
    connect( ui->checkBoxUVBoarders, SIGNAL(stateChanged(int)),
             this, SLOT(commitInteractionOptions()));
    // debug
    ui->buttonTest->setVisible (true);
    connect( ui->buttonTest, SIGNAL(clicked()),
             this, SLOT(debug()));
}

ControlPanel::~ControlPanel() {
	delete ui;
}

void ControlPanel::on_buttonLoad_clicked () {
	QString filePath;

	int mode = ui->visualDrawingMethod->currentIndex();

    emit visualization ( mode, false, false, false, false, false, true, 1, false );
	ui->visualVertexWeights->setChecked (false);
	ui->visualSampledVertices->setChecked (false);
	ui->visualControlPoints->setChecked (false);
    ui->visualRemeshedRegions->setChecked (true);
	ui->visualDecimatedMesh->setChecked (false);
    ui->visualDisplayUpdate->setChecked (true);
    ui->visualShowBoundaries->setChecked (true);

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

    showLoopNr = 0;
}

void ControlPanel::on_buttonSave_clicked () {

	QString filePath = QFileDialog::getSaveFileName(
			this,
			"Save File",
			QString::null,
            "Meshes (*.off *.obj *.stla *.stl *.stlb *.stl);;Text Files (*.txt)",0,0);

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
    int cIdx;
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
    case 9:
        ui->labelElementNr2->setText (msg);
        return;
    case 10:
        cIdx = ui->comboBoxMouseAction->currentIndex();
        --cIdx;
        ui->comboBoxMouseAction->setCurrentIndex(cIdx);
        writeToConsole(msg, 3);
        return;
    case 11:
        cIdx = ui->comboBoxMouseAction->currentIndex();
        ++cIdx;
        ui->comboBoxMouseAction->setCurrentIndex(cIdx);
        writeToConsole(msg, 3);
        return;
    case 12: { // explicit because defining a new variable
        int intensityValue = msg.toFloat()*100;
        ui->sliderIntensity->setValue(intensityValue);
        return;}
    case 13: {
        QStringList list = msg.split(",");
        ui->topInfoBetti0->setText(list.at(0));
        ui->topInfoBetti1->setText(list.at(1));
        ui->topInfoBetti2->setText(list.at(2));
        if (list.at(0) != "-"){
            int chi = (list.at(0)).toInt() - (list.at(1)).toInt() + (list.at(2)).toInt();
            ui->topInfoEuler->setText( QString::number(chi) );
        } else {
            ui->topInfoEuler->setText( "-" );
        }
        return; }
    case 14: {
        QStringList list = msg.split(",");
        ui->topInfoV->setText(list.at(0));
        ui->topInfoE->setText(list.at(1));
        ui->topInfoF->setText(list.at(2));
        return; }
    case 15:
        if ( msg == "-" )
            ui->topInfoGenus->setText(msg);
        else if ( msg == "-1" )
            ui->topInfoGenus->setText("Bd.");
        else
            ui->topInfoGenus->setText(msg);
        return;
    case 16:
        if (msg == "-") {
            loops = 0;
        } else {
            loops = msg.toInt();
        }
        ui->topInfoLoops->setText(msg);
        commitVisualOptions(1);
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
// dumm gemacht, hätte ein struct machen sollen jetzt zu spät zum Umbauen
void ControlPanel::startTabFunctions (int mode) {
	switch (mode) {
    // mesh loaded
	case 1:
		ui->tabWidgetFunctions->setEnabled (true);
        ui->TabTopology->setEnabled(true);
        ui->TabDecimation->setEnabled(true);
        ui->TabMetrics->setEnabled(false);
        ui->visualVertexWeights->setEnabled(false);
        ui->visualSampledVertices->setEnabled(false);
        ui->visualControlPoints->setEnabled(true);
        ui->visualRemeshedRegions->setEnabled(true);
        ui->visualDecimatedMesh->setEnabled(false);
        writeToConsole ("mesh can be sampled and triangulated", 1);
		break;
    // weights calculated
	case 2:
        ui->tabWidgetFunctions->setEnabled (true);
        ui->TabTopology->setEnabled(true);
        ui->TabDecimation->setEnabled(true);
        ui->TabMetrics->setEnabled(false);
        ui->visualVertexWeights->setEnabled(true);
        ui->visualSampledVertices->setEnabled(false);
        ui->visualControlPoints->setEnabled(true);
        ui->visualRemeshedRegions->setEnabled(true);
        ui->visualDecimatedMesh->setEnabled(false);
		writeToConsole ("mesh can be re-indexed", 2);
		break;
    // vertices sampled for decimated mesh
	case 3:
        ui->tabWidgetFunctions->setEnabled (true);
        ui->TabTopology->setEnabled(true);
        ui->TabDecimation->setEnabled(true);
        ui->TabMetrics->setEnabled(false);
        ui->visualVertexWeights->setEnabled(true);
        ui->visualSampledVertices->setEnabled(true);
        ui->visualControlPoints->setEnabled(true);
        ui->visualRemeshedRegions->setEnabled(true);
        ui->visualDecimatedMesh->setEnabled(false);
        writeToConsole ("custom re-indexing", 2);
		break;
    // decimated mesh built
    case 4:
        ui->tabWidgetFunctions->setEnabled (true);
        ui->TabDecimation->setEnabled(true);
        ui->TabTopology->setEnabled(true);
        ui->TabMetrics->setEnabled(true);
        ui->visualVertexWeights->setEnabled(true);
        ui->visualSampledVertices->setEnabled(true);
        ui->visualControlPoints->setEnabled(true);
        ui->visualRemeshedRegions->setEnabled(true);
        ui->visualDecimatedMesh->setEnabled(true);
        writeToConsole ("custom re-indexing", 2);
        break;
	default:
        ui->tabWidgetFunctions->setEnabled(false);
        ui->visualVertexWeights->setEnabled(false);
        ui->visualSampledVertices->setEnabled(false);
        ui->visualControlPoints->setEnabled(false);
        ui->visualRemeshedRegions->setEnabled(false);
        ui->visualDecimatedMesh->setEnabled(false);
		writeToConsole ("sampling/re-indexing disabled", 0);
		break;
	}

}

void ControlPanel::setSliderValue (int value) {

	ui->labelSliderValue->setText ( QString::number(value) );
}


void ControlPanel::on_buttonReIndex_clicked () {
	emit runremeshing ("mesh it baby");
}


void ControlPanel::commitVisualOptions (int mode) {

	// this line is just to make the compiler happy
	if (mode != ui->visualDrawingMethod->currentIndex())
		mode = ui->visualDrawingMethod->currentIndex();

    int value = ui->visualShowBoundaries->isChecked();
    if (showLoopNr > 1)
        value = showLoopNr;

	emit visualization ( mode,
				ui->visualVertexWeights->isChecked(),
				ui->visualSampledVertices->isChecked(),
				ui->visualControlPoints->isChecked(),
				ui->visualRemeshedRegions->isChecked(),
                ui->visualDecimatedMesh->isChecked(),
                ui->visualDisplayUpdate->isChecked(),
                value,
                ui->checkBoxVisualizeHausdorff->isChecked() );
}

void ControlPanel::commitInteractionOptions() {

    options.mouseAction = ui->comboBoxMouseAction->currentIndex();
    options.povDecimation = ui->checkBoxPOV->isChecked();
    options.povRatio = ui->spinBoxPOV->value();
    options.keepSilhouette = ui->checkBoxSilhouette->isChecked();
    options.silhouetteAngle = ui->spinBoxSilhouette->value();
    options.keepUVBoarders = ui->checkBoxUVBoarders->isChecked();
    options.cameraView[0] = (float)(ui->sliderVecX->value())/100;
    options.cameraView[1] = (float)(ui->sliderVecY->value())/100;
    options.cameraView[2] = (float)(ui->sliderVecZ->value())/100;
    options.intensity = (ui->sliderIntensity->value()/100.0f);

    if ( options.cameraView[0] != 0.0f ||
         options.cameraView[1] != 0.0f ||
         options.cameraView[2] != 0.0f ) {
        options.cameraView.normalize();
    }

    if (options.povDecimation || options.keepSilhouette) {
        if ( options.cameraView[0] == 0.0f &&
             options.cameraView[1] == 0.0f &&
             options.cameraView[2] == 0.0f ) {
            ui->checkBoxPOV->setChecked(false);
            ui->checkBoxSilhouette->setChecked(false);
            options.povDecimation = false;
            options.keepSilhouette = false;
            writeToConsole ("No camera vector defined", 3);
        }
    }
    emit interaction( options );
}

void ControlPanel::commitSliderValues() {

    Vec cameraVec;

    cameraVec[0] = ui->sliderVecX->value();
    cameraVec[1] = ui->sliderVecY->value();
    cameraVec[2] = ui->sliderVecZ->value();

    ui->labelVecX->setText( QString::number(cameraVec[0]/100) );
    ui->labelVecY->setText( QString::number(cameraVec[1]/100) );
    ui->labelVecZ->setText( QString::number(cameraVec[2]/100) );

    QString value = QString::number(ui->sliderIntensity->value()/100.0f);
    ui->labelSliderIntensity->setText(value);

    commitInteractionOptions();
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

void ControlPanel::on_buttonFiltration_clicked () {
    emit filtration();
}

void ControlPanel::on_buttonFindLoops_clicked () {
    emit findLoops();
}

void ControlPanel::on_buttonKillLoop_clicked () {
    emit killLoop();
}

void ControlPanel::on_buttonLastLoop_clicked() {
    --showLoopNr;
    if (showLoopNr < 1)
        showLoopNr = 1+loops;
    emit commitVisualOptions(2);
}

void ControlPanel::on_buttonNextLoop_clicked() {
    ++showLoopNr;
    if ( showLoopNr > (1+loops) )
        showLoopNr = 1;
    emit commitVisualOptions(2);
}

void ControlPanel::on_buttonTurntable_clicked() {
    turntable();
}
