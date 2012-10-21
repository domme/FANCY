#ifndef ENGINEQGLWIDGET_H
#define ENGINEQGLWIDGET_H

#include <Includes.h> //Must come before QGLWidget because of glew
#include <QGLWidget>
#include <QTime>
#include <QTimer>
#include <QTextEdit>

#include <QKeyEvent>
#include <QMouseEvent>

class EngineQGLwidget : public QGLWidget
{
	Q_OBJECT

public:
	EngineQGLwidget(QWidget *parent);
	virtual ~EngineQGLwidget();

private slots:
	void onPaintGL();

public slots:
	void onToggleDebugDisplay( int iCheckState );
	void onToggleFXAA( int iCheckState );
	void onToggleBloom( int iCheckState );
	void onToggleHDR( int iCheckState );


protected:
	void setupEngineScene();
	void drawStats();

	virtual void initializeGL();
	virtual void resizeGL( int x, int y );
	virtual void paintGL();
	virtual void mouseMoveEvent( QMouseEvent* event );
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void keyPressEvent ( QKeyEvent* event );
	virtual void keyReleaseEvent ( QKeyEvent* event );

private:
	QTime m_clRenderTime;
	QTimer m_clFPStimer;
	QTextEdit* m_pTextEditFrameDurations;

	uint m_uCurrentDelay;
	int m_iLastMouseX;
	int m_iLastMouseY;

	bool m_bGLcontextReady;
	bool m_bMousePressed;

	
};

#endif // ENGINEQGLWIDGET_H
