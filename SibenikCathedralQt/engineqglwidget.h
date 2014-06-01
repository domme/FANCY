#ifndef ENGINEQGLWIDGET_H
#define ENGINEQGLWIDGET_H

#include <Includes.h> //Must come before QGLWidget because of glew
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtOpenGL/QGLWidget>

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
	void processInputs();
	void update();

	virtual void initializeGL();
	virtual void resizeGL( int x, int y );
	virtual void paintGL();
	virtual void mouseMoveEvent( QMouseEvent* event );
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void keyPressEvent ( QKeyEvent* event );
	virtual void keyReleaseEvent ( QKeyEvent* event );
	void setupSibenikScene();
	void setupTestScene();
private:
	QTime m_clRenderTime;
	QTimer m_clFPStimer;
	QTableView* m_pTableFrameDurations;
	QStandardItemModel* m_pStatsItemModel;

	uint m_uCurrentDelay;
	int m_iLastMouseX;
	int m_iLastMouseY;

	bool m_bGLcontextReady;
	bool m_bMousePressed;

	
};

#endif // ENGINEQGLWIDGET_H
