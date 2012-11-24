/********************************************************************************
** Form generated from reading UI file 'deferredrendererqt.ui'
**
** Created: Sat 24. Nov 23:03:49 2012
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEFERREDRENDERERQT_H
#define UI_DEFERREDRENDERERQT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTableView>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <engineqglwidget.h>

QT_BEGIN_NAMESPACE

class Ui_DeferredRendererQtClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_3;
    EngineQGLwidget *engineQGLwidget;
    QFrame *line;
    QWidget *SideControlsWidget;
    QVBoxLayout *verticalLayout_3;
    QLabel *RenderDurationLabel;
    QTableView *statsTableView;
    QFrame *leftHorLine;
    QCheckBox *hdrCB;
    QCheckBox *fxaaCB;
    QCheckBox *debugDisplayCB;
    QCheckBox *bloomCB;
    QPushButton *ExitButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DeferredRendererQtClass)
    {
        if (DeferredRendererQtClass->objectName().isEmpty())
            DeferredRendererQtClass->setObjectName(QString::fromUtf8("DeferredRendererQtClass"));
        DeferredRendererQtClass->resize(1280, 720);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DeferredRendererQtClass->sizePolicy().hasHeightForWidth());
        DeferredRendererQtClass->setSizePolicy(sizePolicy);
        DeferredRendererQtClass->setBaseSize(QSize(0, 0));
        DeferredRendererQtClass->setDockOptions(QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks);
        centralWidget = new QWidget(DeferredRendererQtClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        engineQGLwidget = new EngineQGLwidget(centralWidget);
        engineQGLwidget->setObjectName(QString::fromUtf8("engineQGLwidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(engineQGLwidget->sizePolicy().hasHeightForWidth());
        engineQGLwidget->setSizePolicy(sizePolicy1);
        engineQGLwidget->setMinimumSize(QSize(320, 320));
        engineQGLwidget->setBaseSize(QSize(320, 320));
        engineQGLwidget->setAutoFillBackground(false);

        horizontalLayout_3->addWidget(engineQGLwidget);

        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
        line->setSizePolicy(sizePolicy2);
        line->setFocusPolicy(Qt::StrongFocus);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_3->addWidget(line);

        SideControlsWidget = new QWidget(centralWidget);
        SideControlsWidget->setObjectName(QString::fromUtf8("SideControlsWidget"));
        sizePolicy2.setHeightForWidth(SideControlsWidget->sizePolicy().hasHeightForWidth());
        SideControlsWidget->setSizePolicy(sizePolicy2);
        SideControlsWidget->setBaseSize(QSize(100, 100));
        verticalLayout_3 = new QVBoxLayout(SideControlsWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        RenderDurationLabel = new QLabel(SideControlsWidget);
        RenderDurationLabel->setObjectName(QString::fromUtf8("RenderDurationLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(RenderDurationLabel->sizePolicy().hasHeightForWidth());
        RenderDurationLabel->setSizePolicy(sizePolicy3);

        verticalLayout_3->addWidget(RenderDurationLabel);

        statsTableView = new QTableView(SideControlsWidget);
        statsTableView->setObjectName(QString::fromUtf8("statsTableView"));
        statsTableView->setSortingEnabled(true);

        verticalLayout_3->addWidget(statsTableView);

        leftHorLine = new QFrame(SideControlsWidget);
        leftHorLine->setObjectName(QString::fromUtf8("leftHorLine"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(leftHorLine->sizePolicy().hasHeightForWidth());
        leftHorLine->setSizePolicy(sizePolicy4);
        leftHorLine->setFrameShape(QFrame::HLine);
        leftHorLine->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(leftHorLine);

        hdrCB = new QCheckBox(SideControlsWidget);
        hdrCB->setObjectName(QString::fromUtf8("hdrCB"));
        hdrCB->setChecked(true);

        verticalLayout_3->addWidget(hdrCB);

        fxaaCB = new QCheckBox(SideControlsWidget);
        fxaaCB->setObjectName(QString::fromUtf8("fxaaCB"));
        fxaaCB->setChecked(true);

        verticalLayout_3->addWidget(fxaaCB);

        debugDisplayCB = new QCheckBox(SideControlsWidget);
        debugDisplayCB->setObjectName(QString::fromUtf8("debugDisplayCB"));

        verticalLayout_3->addWidget(debugDisplayCB);

        bloomCB = new QCheckBox(SideControlsWidget);
        bloomCB->setObjectName(QString::fromUtf8("bloomCB"));
        bloomCB->setChecked(true);

        verticalLayout_3->addWidget(bloomCB);

        ExitButton = new QPushButton(SideControlsWidget);
        ExitButton->setObjectName(QString::fromUtf8("ExitButton"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(ExitButton->sizePolicy().hasHeightForWidth());
        ExitButton->setSizePolicy(sizePolicy5);

        verticalLayout_3->addWidget(ExitButton);


        horizontalLayout_3->addWidget(SideControlsWidget);

        DeferredRendererQtClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(DeferredRendererQtClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 20));
        DeferredRendererQtClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DeferredRendererQtClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DeferredRendererQtClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(DeferredRendererQtClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DeferredRendererQtClass->setStatusBar(statusBar);

        retranslateUi(DeferredRendererQtClass);
        QObject::connect(ExitButton, SIGNAL(clicked()), DeferredRendererQtClass, SLOT(close()));
        QObject::connect(debugDisplayCB, SIGNAL(stateChanged(int)), engineQGLwidget, SLOT(onToggleDebugDisplay(int)));
        QObject::connect(bloomCB, SIGNAL(stateChanged(int)), engineQGLwidget, SLOT(onToggleBloom(int)));
        QObject::connect(hdrCB, SIGNAL(stateChanged(int)), engineQGLwidget, SLOT(onToggleHDR(int)));
        QObject::connect(fxaaCB, SIGNAL(stateChanged(int)), engineQGLwidget, SLOT(onToggleFXAA(int)));

        QMetaObject::connectSlotsByName(DeferredRendererQtClass);
    } // setupUi

    void retranslateUi(QMainWindow *DeferredRendererQtClass)
    {
        DeferredRendererQtClass->setWindowTitle(QApplication::translate("DeferredRendererQtClass", "DeferredRendererQt", 0, QApplication::UnicodeUTF8));
        RenderDurationLabel->setText(QApplication::translate("DeferredRendererQtClass", "Frame-Durations:", 0, QApplication::UnicodeUTF8));
        hdrCB->setText(QApplication::translate("DeferredRendererQtClass", "Use HDR-Rendering", 0, QApplication::UnicodeUTF8));
        fxaaCB->setText(QApplication::translate("DeferredRendererQtClass", "Use FXAA", 0, QApplication::UnicodeUTF8));
        debugDisplayCB->setText(QApplication::translate("DeferredRendererQtClass", "Debug Display", 0, QApplication::UnicodeUTF8));
        bloomCB->setText(QApplication::translate("DeferredRendererQtClass", "Use Bloom", 0, QApplication::UnicodeUTF8));
        ExitButton->setText(QApplication::translate("DeferredRendererQtClass", "Exit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DeferredRendererQtClass: public Ui_DeferredRendererQtClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEFERREDRENDERERQT_H
