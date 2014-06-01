#ifndef DEFERREDRENDERERQAPP_H
#define DEFERREDRENDERERQAPP_H

#include <QtWidgets/QApplication>

class DeferredRendererQApp : public QApplication
{
	Q_OBJECT

public slots:
	void onGLinit();
	void onGLresize( int x, int y );
	void onGLpaint();
	

public:
	DeferredRendererQApp( int& argc, char** argv );
	~DeferredRendererQApp();

private:
	
};

#endif // DEFERREDRENDERERQAPP_H
