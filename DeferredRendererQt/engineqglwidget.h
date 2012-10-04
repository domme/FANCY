#ifndef ENGINEQGLWIDGET_H
#define ENGINEQGLWIDGET_H

#include <Includes.h> //Must come before QGLWidget because of glew
#include <QGLWidget>

class EngineQGLwidget : public QGLWidget
{
	Q_OBJECT

public:
	EngineQGLwidget(QWidget *parent);
	~EngineQGLwidget();

protected:
	void setupEngineScene();

	void initializeGL();
	void resizeGL( int x, int y );
	void paintGL();

private:
	
};

#endif // ENGINEQGLWIDGET_H
