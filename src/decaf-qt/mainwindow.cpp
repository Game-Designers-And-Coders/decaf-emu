#include "mainwindow.h"

#include "inputsettings.h"
#include "vulkanwindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>

#include <libdecaf/decaf.h>

MainWindow::MainWindow(QVulkanInstance *vulkanInstance, DecafInterface *decafInterface, InputDriver *inputDriver, QWidget* parent) :
   QMainWindow(parent),
   mDecafInterface(decafInterface),
   mInputDriver(inputDriver)
{
   mVulkanWindow = new VulkanWindow(vulkanInstance, decafInterface);

   QWidget* wrapper = QWidget::createWindowContainer((QWindow*)mVulkanWindow);
   wrapper->setFocusPolicy(Qt::StrongFocus);
   wrapper->setFocus();
   setCentralWidget(QWidget::createWindowContainer(mVulkanWindow));

   mUi.setupUi(this);

   QObject::connect(decafInterface, &DecafInterface::titleLoaded,
                    this, &MainWindow::titleLoaded);
}

void
MainWindow::menuOpenInputSettings()
{
   InputSettings dialog(mInputDriver, this);
   dialog.exec();
}

void
MainWindow::titleLoaded(quint64 id, const QString &name)
{
   setWindowTitle(QString("decaf-qt - %1").arg(name));
}

void
MainWindow::menuOpenFile()
{
   // You only get one chance to run a game out here buddy.
   mUi.actionOpen->setDisabled(true);

   auto filename = QFileDialog::getOpenFileName(this, tr("Open Application"), "", tr("RPX Files (*.rpx);;cos.xml (cos.xml);;"));
   mDecafInterface->startGame(filename);
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
#if 0
   QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Are you sure?",
                                                              tr("A game is running, are you sure you want to exit?\n"),
                                                              QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                              QMessageBox::Yes);
   if (resBtn != QMessageBox::Yes) {
      event->ignore();
      return;
   }
#endif
   mDecafInterface->shutdown();
   event->accept();
}
