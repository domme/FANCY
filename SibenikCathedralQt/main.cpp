#include "deferredrendererqt.h"
#include <QtWidgets/QApplication.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  DeferredRendererQt w;
  //w.show();
  return a.exec();
}
