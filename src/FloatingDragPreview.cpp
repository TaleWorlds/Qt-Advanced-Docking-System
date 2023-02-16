//============================================================================
/// \file   FloatingDragPreview.cpp
/// \author Uwe Kindler
/// \date   26.11.2019
/// \brief  Implementation of CFloatingDragPreview
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "FloatingDragPreview.h"
#include <iostream>

#include <QEvent>
#include <QApplication>
#include <QPainter>
#include <QKeyEvent>

#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockContainerWidget.h"
#include "DockOverlay.h"

namespace ads
{

	/**
	 * Private data class (pimpl)
	 */
	struct FloatingDragPreviewPrivate
	{
		CFloatingDragPreview* _this;
		QWidget* Content;
		CDockAreaWidget* ContentSourceArea = nullptr;
		QPoint DragStartMousePosition;
		CDockManager* DockManager;
		CDockContainerWidget* DropContainer = nullptr;
		qreal WindowOpacity;
		bool Hidden = false;
		QPixmap ContentPreviewPixmap;
		bool Canceled = false;


		/**
		 * Private data constructor
		 */
		FloatingDragPreviewPrivate(CFloatingDragPreview* _public);
		void updateDropOverlays(const QPoint& GlobalPos);

		void setHidden(bool Value)
		{
			Hidden = Value;
			_this->update();
		}

		/**
		 * Cancel dragging and emit the draggingCanceled event
		 */
		void cancelDragging()
		{
			Canceled = true;
			Q_EMIT _this->draggingCanceled();
			if (ContentSourceArea && ContentSourceArea->dockContainer()->isFloating())
			{
				ContentSourceArea->dockContainer()->floatingWidget()->containerOverlay()->hideOverlay();
				ContentSourceArea->dockContainer()->floatingWidget()->dockAreaOverlay()->hideOverlay();
			}
			DockManager->containerOverlay()->hideOverlay();
			DockManager->dockAreaOverlay()->hideOverlay();
			_this->close();
		}

		/**
		 * Creates the real floating widget in case the mouse is released outside
		 * outside of any drop area
		 */
		void createFloatingWidget();
	};
	// struct LedArrayPanelPrivate


	//============================================================================
	void FloatingDragPreviewPrivate::updateDropOverlays(const QPoint& GlobalPos)
	{
		if (!_this->isVisible() || !DockManager)
		{
			return;
		}

		// Prevent display of drop overlays and docking as long as a modal dialog
		// is active
		if (qApp->activeModalWidget())
		{
			return;
		}

		auto Containers = DockManager->dockContainers();
		CDockContainerWidget* TopContainer = nullptr;
		for (auto ContainerWidget : Containers)
		{
			if (!ContainerWidget->isVisible())
			{
				continue;
			}

			QPoint MappedPos = ContainerWidget->mapFromGlobal(GlobalPos);
			if (ContainerWidget->rect().contains(MappedPos))
			{
				if (!TopContainer || ContainerWidget->isInFrontOf(TopContainer))
				{
					TopContainer = ContainerWidget;
				}
			}
		}

		DropContainer = TopContainer;
		CDockOverlay* ContainerOverlay = 0;
		CDockOverlay* DockAreaOverlay = 0;
		if (ContentSourceArea->dockContainer()->isFloating())
		{
			ContainerOverlay = ContentSourceArea->dockContainer()->floatingWidget()->containerOverlay();
			DockAreaOverlay = ContentSourceArea->dockContainer()->floatingWidget()->dockAreaOverlay();
		}
		else
		{
			ContainerOverlay = DockManager->containerOverlay();
			DockAreaOverlay = DockManager->dockAreaOverlay();
		}

		if (!TopContainer)
		{
			ContainerOverlay->hideOverlay();
			DockAreaOverlay->hideOverlay();
			DockManager->containerOverlay()->hideOverlay();
			DockManager->dockAreaOverlay()->hideOverlay();
			if (CDockManager::testConfigFlag(CDockManager::DragPreviewIsDynamic))
			{
				setHidden(false);
			}
			return;
		}

		// Do not show drop overlay if the dropped widget's window is minimized
		if (DropContainer &&
			DropContainer->window()->isMinimized())
		{
			ContainerOverlay->hideOverlay();
			DockAreaOverlay->hideOverlay();
			DockManager->dockAreaOverlay()->hideOverlay();
			DockManager->containerOverlay()->hideOverlay();
			return;
		}

		int VisibleDockAreas = TopContainer->visibleDockAreaCount();
		ContainerOverlay->setAllowedAreas(
			VisibleDockAreas > 1 ? OuterDockAreas : AllDockAreas);
		DockWidgetArea ContainerArea = ContainerOverlay->showOverlay(TopContainer);
		_this->activateWindow();
		ContainerOverlay->enableDropPreview(ContainerArea != InvalidDockWidgetArea);
		auto DockAreaWidget = TopContainer->dockAreaAt(GlobalPos);
		DockWidgetArea DAWArea = InvalidDockWidgetArea;
		if (DockAreaWidget && DockAreaWidget->isVisible() && VisibleDockAreas > 0)
		{
			DockAreaOverlay->enableDropPreview(true);
			DockAreaOverlay->setAllowedAreas(
				(VisibleDockAreas == 1) ? NoDockWidgetArea : DockAreaWidget->allowedAreas());
			DAWArea = DockAreaOverlay->showOverlay(DockAreaWidget);
			_this->activateWindow();
			// A CenterDockWidgetArea for the dockAreaOverlay() indicates that
			// the mouse is in the title bar. If the ContainerArea is valid
			// then we ignore the dock area of the dockAreaOverlay() and disable
			// the drop preview
			if ((DAWArea == CenterDockWidgetArea)
				&& (ContainerArea != InvalidDockWidgetArea))
			{
				DockAreaOverlay->enableDropPreview(false);
				ContainerOverlay->enableDropPreview(true);
			}
			else
			{
				ContainerOverlay->enableDropPreview(InvalidDockWidgetArea == DAWArea);
			}
		}
		else
		{
			DockAreaOverlay->hideOverlay();
		}

		if (CDockManager::testConfigFlag(CDockManager::DragPreviewIsDynamic))
		{
			setHidden(DAWArea != InvalidDockWidgetArea || ContainerArea != InvalidDockWidgetArea);
		}
	}

	//============================================================================
	FloatingDragPreviewPrivate::FloatingDragPreviewPrivate(CFloatingDragPreview* _public) :
		_this(_public)
	{

	}

	//============================================================================
	void FloatingDragPreviewPrivate::createFloatingWidget()
	{
		CDockWidget* DockWidget = qobject_cast<CDockWidget*>(Content);
		CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(Content);

		CFloatingDockContainer* FloatingWidget = nullptr;

		if (DockWidget && DockWidget->features().testFlag(CDockWidget::DockWidgetFloatable))
		{
			FloatingWidget = new CFloatingDockContainer(DockWidget);
		}
		else if (DockArea && DockArea->features().testFlag(CDockWidget::DockWidgetFloatable))
		{
			FloatingWidget = new CFloatingDockContainer(DockArea);
		}

		if (FloatingWidget)
		{
			QRect geo;
			FloatingWidget->setGeometry(_this->geometry());
			geo = FloatingWidget->geometry();
			FloatingWidget->show();
			if (!CDockManager::testConfigFlag(CDockManager::DragPreviewHasWindowFrame))
			{
				QApplication::processEvents();
				int FrameHeight = FloatingWidget->frameGeometry().height() - FloatingWidget->geometry().height();
				QRect FixedGeometry = geo;
				FixedGeometry.adjust(0, FrameHeight, 0, 0);
				FloatingWidget->setGeometry(FixedGeometry);
			}
		}
	}

	//============================================================================
	CFloatingDragPreview::CFloatingDragPreview(QWidget* Content, QWidget* parent) :
		QWidget(parent),
		d(new FloatingDragPreviewPrivate(this))
	{
		d->Content = Content;
		setAttribute(Qt::WA_DeleteOnClose);
		if (CDockManager::testConfigFlag(CDockManager::DragPreviewHasWindowFrame))
		{
			setWindowFlags(
				Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
		}
		else
		{
			setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
			setAttribute(Qt::WA_NoSystemBackground);
			setAttribute(Qt::WA_TranslucentBackground);
		}

#ifdef Q_OS_LINUX
		auto Flags = windowFlags();
		Flags |= Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint;
		setWindowFlags(Flags);
#endif

		setWindowOpacity(0.6);

		// Create a static image of the widget that should get undocked
		// This is like some kind preview image like it is uses in drag and drop
		// operations
		if (CDockManager::testConfigFlag(CDockManager::DragPreviewShowsContentPixmap))
		{
			d->ContentPreviewPixmap = QPixmap(Content->size());
			Content->render(&d->ContentPreviewPixmap);
		}

		connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
			SLOT(onApplicationStateChanged(Qt::ApplicationState)));

		// The only safe way to receive escape key presses is to install an event
		// filter for the application object
		qApp->installEventFilter(this);
	}


	//============================================================================
	CFloatingDragPreview::CFloatingDragPreview(CDockWidget* Content)
		: CFloatingDragPreview((QWidget*)Content, Content)
	{
		d->DockManager = Content->dockManager();
		d->ContentSourceArea = Content->dockAreaWidget();
		setWindowTitle(Content->windowTitle());
	}

	//============================================================================
	CFloatingDragPreview::CFloatingDragPreview(CDockAreaWidget* Content)
		: CFloatingDragPreview((QWidget*)Content, Content)
	{
		d->DockManager = Content->dockManager();
		d->ContentSourceArea = Content;
		setWindowTitle(Content->currentDockWidget()->windowTitle());
	}


	//============================================================================
	CFloatingDragPreview::~CFloatingDragPreview()
	{
		delete d;
	}


	//============================================================================
	void CFloatingDragPreview::moveFloating()
	{
		int BorderSize = (frameSize().width() - size().width()) / 2;
		const QPoint moveToPos = QCursor::pos() - d->DragStartMousePosition
			- QPoint(BorderSize, 0);
		move(moveToPos);
		d->updateDropOverlays(QCursor::pos());
	}


	//============================================================================
	void CFloatingDragPreview::startFloating(const QPoint& DragStartMousePos,
		const QSize& Size, eDragState DragState, QWidget* MouseEventHandler)
	{
		Q_UNUSED(MouseEventHandler)
			Q_UNUSED(DragState)
			resize(Size);
		d->DragStartMousePosition = DragStartMousePos;
		moveFloating();
		show();
	}


	//============================================================================
	void CFloatingDragPreview::finishDragging()
	{
		ADS_PRINT("CFloatingDragPreview::finishDragging");
		CDockOverlay* containerOverlay = 0;
		CDockOverlay* dockAreaOverlay = 0;
		if (d->ContentSourceArea
			&& d->ContentSourceArea->dockContainer()->isFloating())
		{
			containerOverlay = d->ContentSourceArea->dockContainer()->floatingWidget()->containerOverlay();
			dockAreaOverlay = d->ContentSourceArea->dockContainer()->floatingWidget()->dockAreaOverlay();
		}
		else
		{
			containerOverlay = d->DockManager->containerOverlay();
			dockAreaOverlay = d->DockManager->dockAreaOverlay();
		}
		auto DockDropArea = dockAreaOverlay->visibleDropAreaUnderCursor();
		auto ContainerDropArea = containerOverlay->visibleDropAreaUnderCursor();
		auto SourceContainer = d->ContentSourceArea ? d->ContentSourceArea->dockContainer() : nullptr;
		bool target_had_independent = d->DropContainer ? d->DropContainer->hasIndependentWidget() : false;
		bool source_had_independent = SourceContainer ? SourceContainer->hasIndependentWidget() : false;
		if (!d->DropContainer)
		{
			d->createFloatingWidget();
		}
		else if (DockDropArea != InvalidDockWidgetArea)
		{
			d->DropContainer->dropWidget(d->Content, DockDropArea, d->DropContainer->dockAreaAt(QCursor::pos()));
		}
		else if (ContainerDropArea != InvalidDockWidgetArea)
		{
			// If there is only one single dock area, and we drop into the center
			// then we tabify the dropped widget into the only visible dock area
			if (d->DropContainer->visibleDockAreaCount() <= 1 && CenterDockWidgetArea == ContainerDropArea)
			{
				d->DropContainer->dropWidget(d->Content, ContainerDropArea, d->DropContainer->dockAreaAt(QCursor::pos()));
			}
			else
			{
				d->DropContainer->dropWidget(d->Content, ContainerDropArea, nullptr);
			}
		}
		else
		{
			d->createFloatingWidget();
		}
		if (SourceContainer) SourceContainer->fetchIndependentCount();
		if (d->DropContainer) d->DropContainer->fetchIndependentCount();
		// Case - Existing Floating Drop Container is just changed
		auto SourceFloatingContainer = SourceContainer ? SourceContainer->floatingWidget() : nullptr;
		auto DropFloatingContainer = d->DropContainer ? d->DropContainer->floatingWidget() : nullptr;
		if (d->DropContainer
			&& d->DropContainer->isFloating()					// Target is FDC
			&& (ContainerDropArea != InvalidDockWidgetArea
				|| DockDropArea != InvalidDockWidgetArea)		// Target exists
			&& (SourceFloatingContainer != DropFloatingContainer) // No update if the widget is dropped into itself
			)
		{
			// Check if the independence of this floating container changed by this drop
			bool target_has_independent = d->DropContainer ? d->DropContainer->hasIndependentWidget() : false;;
			if (target_has_independent != target_had_independent)
			{
				CFloatingDockContainer* RestoredFloatingWidget = DropFloatingContainer->moveContainerAndDelete();
			}
		}
		// Case - Source Container was Also Changed
		if (SourceContainer
			&& SourceContainer->isFloating()
			&& (SourceFloatingContainer != DropFloatingContainer) // No update if the widget is dropped into itself
			)
		{
			bool source_has_independent = SourceContainer ? SourceContainer->hasIndependentWidget() : false;
			if (source_has_independent != source_had_independent)
			{
				CFloatingDockContainer* RestoredFloatingWidget = SourceFloatingContainer->moveContainerAndDelete();
				containerOverlay = nullptr;
				dockAreaOverlay = nullptr;
				d->ContentSourceArea = nullptr;
			}
		}
		this->close();
		if (containerOverlay) containerOverlay->hideOverlay();
		if (dockAreaOverlay) dockAreaOverlay->hideOverlay();
		d->DockManager->containerOverlay()->hideOverlay();
		d->DockManager->dockAreaOverlay()->hideOverlay();
	}


	//============================================================================
	void CFloatingDragPreview::paintEvent(QPaintEvent* event)
	{
		Q_UNUSED(event);
		if (d->Hidden)
		{
			return;
		}

		QPainter painter(this);
		if (CDockManager::testConfigFlag(CDockManager::DragPreviewShowsContentPixmap))
		{
			painter.drawPixmap(QPoint(0, 0), d->ContentPreviewPixmap);
		}

		// If we do not have a window frame then we paint a QRubberBand like
		// frameless window
		if (!CDockManager::testConfigFlag(CDockManager::DragPreviewHasWindowFrame))
		{
			QColor Color = palette().color(QPalette::Active, QPalette::Window);
			QPen Pen = painter.pen();
			Pen.setColor(Color.darker(120));
			Pen.setStyle(Qt::SolidLine);
			Pen.setWidth(1);
			Pen.setCosmetic(true);
			painter.setPen(Pen);
			Color = Color.lighter(130);
			Color.setAlpha(64);
			painter.setBrush(Color);
			painter.drawRect(rect().adjusted(0, 0, -1, -1));
		}
	}

	//============================================================================
	void CFloatingDragPreview::onApplicationStateChanged(Qt::ApplicationState state)
	{
		if (state != Qt::ApplicationActive)
		{
			disconnect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
				this, SLOT(onApplicationStateChanged(Qt::ApplicationState)));
			d->cancelDragging();
		}
	}


	//============================================================================
	bool CFloatingDragPreview::eventFilter(QObject* watched, QEvent* event)
	{
		Q_UNUSED(watched);
		if (!d->Canceled && event->type() == QEvent::KeyPress)
		{
			QKeyEvent* e = static_cast<QKeyEvent*>(event);
			if (e->key() == Qt::Key_Escape)
			{
				watched->removeEventFilter(this);
				d->cancelDragging();
			}
		}

		return false;
	}



} // namespace ads

//---------------------------------------------------------------------------
// EOF FloatingDragPreview.cpp
