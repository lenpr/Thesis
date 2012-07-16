#include <QtGui/QApplication>
#include "mainwidget.h"

#include <QSplashScreen>

int main(int argc, char *argv[]) {

	QApplication a(argc, argv);
	MainWidget w;
    w.resize(1100,750);

//    system ("pwd");

    QPixmap pixmap("splash_screen.jpg");
    pixmap.scaled(500,425);
    QSplashScreen splash(pixmap);
    splash.show();
    QString msg = "Loading...";
    splash.showMessage(msg, Qt::AlignLeft, Qt::white);
//    QTest::qWait (4000);
    a.processEvents();
    splash.finish(&w);

	w.show();
	return a.exec();
}
