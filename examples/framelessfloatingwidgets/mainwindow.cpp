#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QWidgetAction>
#include <QLabel>
#include <QCalendarWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSaveFile>
#include <QToolBar>
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
#include "DockAreaTabBar.h"
#include "DockComponentsFactory.h"

using namespace ads;

// Created a simple example for frameless titled dock widgets

CMainWindow::CMainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::CMainWindow)
{
	ui->setupUi(this);
	CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
	CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
	CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
	CDockManager::setConfigFlag(CDockManager::FloatingShadowEnabled, false);
	// only need this for removing native title bar - this demo also tests styling from stylesheet for the title bar
	CDockManager::setConfigFlag(CDockManager::FloatingContainerForceQWidgetCustomStyledTitleBar, true);
    CDockManager::setConfigFlag(
        CDockManager::FloatingContainerForceNativeTitleBar, false);
    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);
	DockManager = new CDockManager(this);
	// Set central widget
	QPlainTextEdit* w = new QPlainTextEdit();
	w->setPlaceholderText("This is the central editor. Enter your text here.");
	CDockWidget* CentralDockWidget = new CDockWidget("CentralWidget");
	CentralDockWidget->setWidget(w);
	auto* CentralDockArea = /*DockManager->setCentralWidget(CentralDockWidget)*/DockManager->addDockWidget(ads::CenterDockWidgetArea, CentralDockWidget);
	CentralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

	// create other dock widgets
	QTableWidget* table = new QTableWidget();
	table->setColumnCount(3);
	table->setRowCount(10);
	CDockWidget* TableDockWidget = new CDockWidget("Table 1");
	TableDockWidget->setWidget(table);
	TableDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
	TableDockWidget->resize(250, 150);
	TableDockWidget->setMinimumSize(200, 150);
	auto TableArea = DockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, TableDockWidget);
	ui->menuView->addAction(TableDockWidget->toggleViewAction());

	table = new QTableWidget();
	table->setColumnCount(5);
	table->setRowCount(1020);
	TableDockWidget = new CDockWidget("Table 2");
	TableDockWidget->setWidget(table);
	TableDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
	TableDockWidget->resize(250, 150);
	TableDockWidget->setMinimumSize(200, 150);
	DockManager->addDockWidget(DockWidgetArea::BottomDockWidgetArea, TableDockWidget, TableArea);
	ui->menuView->addAction(TableDockWidget->toggleViewAction());

	QTableWidget* propertiesTable = new QTableWidget();
	propertiesTable->setColumnCount(3);
	propertiesTable->setRowCount(10);
	CDockWidget* PropertiesDockWidget = new CDockWidget("Properties");
	PropertiesDockWidget->setFeature(CDockWidget::DockWidgetIndependent, true);
	PropertiesDockWidget->setWidget(propertiesTable);
	PropertiesDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
	PropertiesDockWidget->resize(250, 150);
	PropertiesDockWidget->setMinimumSize(200, 150);
	DockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea, PropertiesDockWidget, CentralDockArea);
	ui->menuView->addAction(PropertiesDockWidget->toggleViewAction());

	QByteArray example_state;
	if (QFile::exists("example.xml"))
	{
		QFile state_file("example.xml");
		state_file.open(QIODevice::ReadOnly);
		example_state = state_file.readAll();
		DockManager->restoreState(example_state);
		state_file.close();
	}
	createPerspectiveUi();
}

CMainWindow::~CMainWindow()
{
	delete ui;
}


void CMainWindow::createPerspectiveUi()
{
	SavePerspectiveAction = new QAction("Create Perspective", this);
	connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(savePerspective()));
	PerspectiveListAction = new QWidgetAction(this);
	PerspectiveComboBox = new QComboBox(this);
	PerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	PerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	connect(PerspectiveComboBox, SIGNAL(activated(const QString&)),
		DockManager, SLOT(openPerspective(const QString&)));
	PerspectiveListAction->setDefaultWidget(PerspectiveComboBox);
	ui->toolBar->addSeparator();
	ui->toolBar->addAction(PerspectiveListAction);
	ui->toolBar->addAction(SavePerspectiveAction);
}


void CMainWindow::savePerspective()
{
	QString PerspectiveName = QInputDialog::getText(this, "Save Perspective", "Enter unique name:");
	if (PerspectiveName.isEmpty())
	{
		return;
	}

	DockManager->addPerspective(PerspectiveName);
	QSignalBlocker Blocker(PerspectiveComboBox);
	PerspectiveComboBox->clear();
	PerspectiveComboBox->addItems(DockManager->perspectiveNames());
	PerspectiveComboBox->setCurrentText(PerspectiveName);
}


//============================================================================
void CMainWindow::closeEvent(QCloseEvent* event)
{
	// Delete dock manager here to delete all floating widgets. This ensures
	// that all top level windows of the dock manager are properly closed
	QByteArray example_state;
	example_state = DockManager->saveState();
	QSaveFile file("example.xml");
	file.open(QIODevice::WriteOnly);
	file.write(example_state);
	file.commit();
	DockManager->deleteLater();
	QMainWindow::closeEvent(event);
}


