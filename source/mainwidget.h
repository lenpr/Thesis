#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

#include "viewer.h"
#include "controlpanel.h"

class MainWidget : public QWidget {
    Q_OBJECT

public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
		Viewer *viewer;
		ControlPanel *controlpanel;
};

#endif // MAINWIDGET_H
