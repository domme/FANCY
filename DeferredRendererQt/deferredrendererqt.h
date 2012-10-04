#ifndef DEFERREDRENDERERQT_H
#define DEFERREDRENDERERQT_H

#include <QtGui/QMainWindow>
#include "ui_deferredrendererqt.h"

class DeferredRendererQt : public QMainWindow
{
	Q_OBJECT

public:
	DeferredRendererQt(QWidget *parent = 0, Qt::WFlags flags = 0);
	~DeferredRendererQt();

private:
	Ui::DeferredRendererQtClass ui;
};

#endif // DEFERREDRENDERERQT_H
