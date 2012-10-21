/********************************************************************************
** Form generated from reading UI file 'deferredrendererqt.ui'
**
** Created: Sat 20. Oct 23:27:50 2012
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
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
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
    QTextEdit *durationsTextEdit;
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
        DeferredRendererQtClass->resize(947, 599);
        centralWidget = new QWidget(DeferredRendererQtClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        engineQGLwidget = new EngineQGLwidget(centralWidget);
        engineQGLwidget->setObjectName(QString::fromUtf8("engineQGLwidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(engineQGLwidget->sizePolicy().hasHeightForWidth());
        engineQGLwidget->setSizePolicy(sizePolicy);
        engineQGLwidget->setMinimumSize(QSize(320, 320));
        engineQGLwidget->setBaseSize(QSize(320, 320));
        engineQGLwidget->setAutoFillBackground(false);

        horizontalLayout_3->addWidget(engineQGLwidget);

        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
        line->setSizePolicy(sizePolicy1);
        line->setFocusPolicy(Qt::StrongFocus);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_3->addWidget(line);

        SideControlsWidget = new QWidget(centralWidget);
        SideControlsWidget->setObjectName(QString::fromUtf8("SideControlsWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(SideControlsWidget->sizePolicy().hasHeightForWidth());
        SideControlsWidget->setSizePolicy(sizePolicy2);
        SideControlsWidget->setBaseSize(QSize(100, 100));
        verticalLayout_3 = new QVBoxLayout(SideControlsWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        RenderDurationLabel = new QLabel(SideControlsWidget);
        RenderDurationLabel->setObjectName(QString::fromUtf8("RenderDurationLabel"));

        verticalLayout_3->addWidget(RenderDurationLabel);

        durationsTextEdit = new QTextEdit(SideControlsWidget);
        durationsTextEdit->setObjectName(QString::fromUtf8("durationsTextEdit"));
        durationsTextEdit->setMaximumSize(QSize(16777215, 16777215));
        durationsTextEdit->setSizeIncrement(QSize(1, 1));
        durationsTextEdit->setUndoRedoEnabled(false);
        durationsTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        durationsTextEdit->setReadOnly(true);

        verticalLayout_3->addWidget(durationsTextEdit);

        leftHorLine = new QFrame(SideControlsWidget);
        leftHorLine->setObjectName(QString::fromUtf8("leftHorLine"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(leftHorLine->sizePolicy().hasHeightForWidth());
        leftHorLine->setSizePolicy(sizePolicy3);
        leftHorLine->setFrameShape(QFrame::HLine);
        leftHorLine->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(leftHorLine);

        hdrCB = new QCheckBox(SideControlsWidget);
        hdrCB->setObjectName(QString::fromUtf8("hdrCB"));

        verticalLayout_3->addWidget(hdrCB);

        fxaaCB = new QCheckBox(SideControlsWidget);
        fxaaCB->setObjectName(QString::fromUtf8("fxaaCB"));

        verticalLayout_3->addWidget(fxaaCB);

        debugDisplayCB = new QCheckBox(SideControlsWidget);
        debugDisplayCB->setObjectName(QString::fromUtf8("debugDisplayCB"));

        verticalLayout_3->addWidget(debugDisplayCB);

        bloomCB = new QCheckBox(SideControlsWidget);
        bloomCB->setObjectName(QString::fromUtf8("bloomCB"));

        verticalLayout_3->addWidget(bloomCB);

        ExitButton = new QPushButton(SideControlsWidget);
        ExitButton->setObjectName(QString::fromUtf8("ExitButton"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(ExitButton->sizePolicy().hasHeightForWidth());
        ExitButton->setSizePolicy(sizePolicy4);

        verticalLayout_3->addWidget(ExitButton);


        horizontalLayout_3->addWidget(SideControlsWidget);

        DeferredRendererQtClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(DeferredRendererQtClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 947, 20));
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
