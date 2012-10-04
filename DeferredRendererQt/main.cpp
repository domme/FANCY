#include "deferredrendererqt.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DeferredRendererQt w;
	w.show();
	return a.exec();
}
