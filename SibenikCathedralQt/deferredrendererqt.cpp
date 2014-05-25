#include "deferredrendererqt.h"

DeferredRendererQt::DeferredRendererQt(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
}

DeferredRendererQt::~DeferredRendererQt()
{

}
