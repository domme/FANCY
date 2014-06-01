#ifndef DEFERREDRENDERERQT_H
#define DEFERREDRENDERERQT_H

#include <QtWidgets/qmainwindow.h>
#include "ui_deferredrendererqt.h"

class DeferredRendererQt : public QMainWindow
{
	Q_OBJECT

public:
	DeferredRendererQt(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~DeferredRendererQt();

private:
	Ui::DeferredRendererQtClass ui;
};

#endif // DEFERREDRENDERERQT_H
