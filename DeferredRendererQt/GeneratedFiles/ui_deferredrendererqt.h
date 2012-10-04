/********************************************************************************
** Form generated from reading UI file 'deferredrendererqt.ui'
**
** Created: Thu 4. Oct 22:06:29 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEFERREDRENDERERQT_H
#define UI_DEFERREDRENDERERQT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <engineqglwidget.h>

QT_BEGIN_NAMESPACE

class Ui_DeferredRendererQtClass
{
public:
    QWidget *centralWidget;
    EngineQGLwidget *engineQGLwidget;
    QFrame *line;
    QPushButton *pushButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DeferredRendererQtClass)
    {
        if (DeferredRendererQtClass->objectName().isEmpty())
            DeferredRendererQtClass->setObjectName(QString::fromUtf8("DeferredRendererQtClass"));
        DeferredRendererQtClass->resize(670, 399);
        centralWidget = new QWidget(DeferredRendererQtClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        engineQGLwidget = new EngineQGLwidget(centralWidget);
        engineQGLwidget->setObjectName(QString::fromUtf8("engineQGLwidget"));
        engineQGLwidget->setGeometry(QRect(0, 0, 501, 341));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(engineQGLwidget->sizePolicy().hasHeightForWidth());
        engineQGLwidget->setSizePolicy(sizePolicy);
        engineQGLwidget->setAutoFillBackground(false);
        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(500, 0, 20, 341));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(520, 320, 141, 23));
        DeferredRendererQtClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(DeferredRendererQtClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 670, 21));
        DeferredRendererQtClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DeferredRendererQtClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DeferredRendererQtClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(DeferredRendererQtClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DeferredRendererQtClass->setStatusBar(statusBar);

        retranslateUi(DeferredRendererQtClass);
        QObject::connect(pushButton, SIGNAL(clicked()), DeferredRendererQtClass, SLOT(close()));

        QMetaObject::connectSlotsByName(DeferredRendererQtClass);
    } // setupUi

    void retranslateUi(QMainWindow *DeferredRendererQtClass)
    {
        DeferredRendererQtClass->setWindowTitle(QApplication::translate("DeferredRendererQtClass", "DeferredRendererQt", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("DeferredRendererQtClass", "Exit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DeferredRendererQtClass: public Ui_DeferredRendererQtClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEFERREDRENDERERQT_H
