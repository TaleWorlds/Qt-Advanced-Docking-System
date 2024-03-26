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

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QWindow>

#include <AutoHideDockContainer.h>

#include <iostream>

#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockContainerWidget.h"
#include "DockManager.h"
#include "DockOverlay.h"
#include "DockWidget.h"
#include "ads_globals.h"
#ifdef Q_OS_WINDOWS
#    include "Windows.h"
#endif
namespace ads
{

/**
 * Private data class (pimpl)
 */
struct FloatingDragPreviewPrivate
{
    CFloatingDragPreview* _this;
    QWidget* Content;
    CDockWidget::DockWidgetFeatures ContentFeatures;
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
            ContentSourceArea->dockContainer()
                ->floatingWidget()
                ->containerOverlay()
                ->hideOverlay();
            ContentSourceArea->dockContainer()
                ->floatingWidget()
                ->dockAreaOverlay()
                ->hideOverlay();
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

    /**
     * Returns true, if the content is floatable
     */
    bool isContentFloatable() const
    {
        return this->ContentFeatures.testFlag(CDockWidget::DockWidgetFloatable);
    }

    /**
     * Returns true, if the content is pinnable
     */
    bool isContentPinnable() const
    {
        return this->ContentFeatures.testFlag(CDockWidget::DockWidgetPinnable);
    }

    /**
     * Returns the content features
     */
    CDockWidget::DockWidgetFeatures contentFeatures() const
    {
        CDockWidget* DockWidget = qobject_cast<CDockWidget*>(Content);
        if (DockWidget)
        {
            return DockWidget->features();
        }

        CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(Content);
        if (DockArea)
        {
            return DockArea->features();
        }

        return CDockWidget::DockWidgetFeatures();
    }
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
        ContainerOverlay = ContentSourceArea->dockContainer()
                               ->floatingWidget()
                               ->containerOverlay();
        DockAreaOverlay = ContentSourceArea->dockContainer()
                              ->floatingWidget()
                              ->dockAreaOverlay();
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
    if (DropContainer && DropContainer->window()->isMinimized())
    {
        ContainerOverlay->hideOverlay();
        DockAreaOverlay->hideOverlay();
        DockManager->dockAreaOverlay()->hideOverlay();
        DockManager->containerOverlay()->hideOverlay();
        return;
    }
    auto DockDropArea = DockAreaOverlay->dropAreaUnderCursor();
    auto ContainerDropArea = ContainerOverlay->dropAreaUnderCursor();
    int VisibleDockAreas = TopContainer->visibleDockAreaCount();
    // Include the overlay widget we're dragging as a visible widget
    auto dockAreaWidget = qobject_cast<CDockAreaWidget*>(Content);
    if (dockAreaWidget && dockAreaWidget->isAutoHide())
    {
        VisibleDockAreas++;
    }
    DockWidgetAreas AllowedContainerAreas =
        (VisibleDockAreas > 1) ? OuterDockAreas : CenterDockWidgetArea;
    auto DockArea = TopContainer->dockAreaAt(GlobalPos);
    // If the dock container contains only one single DockArea, then we need
    // to respect the allowed areas - only the center area is relevant here
    // because all other allowed areas are from the container
    if (VisibleDockAreas == 1 && DockArea)
    {
        AllowedContainerAreas.setFlag(CenterDockWidgetArea, false);
    }
    if (isContentPinnable())
    {
        AllowedContainerAreas |= AutoHideDockAreas;
    }
    ContainerOverlay->setAllowedAreas(AllowedContainerAreas);
    ContainerOverlay->enableDropPreview(ContainerDropArea
                                        != InvalidDockWidgetArea);
    DockWidgetArea DAWArea = InvalidDockWidgetArea;
    DockWidgetArea ContainerArea = InvalidDockWidgetArea;
    if (DockArea && DockArea->isVisible() && VisibleDockAreas >= 0
        && DockArea != ContentSourceArea)
    {
        DockAreaOverlay->enableDropPreview(true);
        DockAreaOverlay->setAllowedAreas(DockArea->allowedAreas());
        DockWidgetArea Area = DockAreaOverlay->showOverlay(DockArea);
        DAWArea = Area;
        _this->activateWindow();
        // A CenterDockWidgetArea for the dockAreaOverlay() indicates that
        // the mouse is in the title bar. If the ContainerArea is valid
        // then we ignore the dock area of the dockAreaOverlay() and disable
        // the drop preview
        if ((Area == CenterDockWidgetArea)
            && (ContainerDropArea != InvalidDockWidgetArea))
        {
            DockAreaOverlay->enableDropPreview(false);
            ContainerOverlay->enableDropPreview(true);
        }
        else
        {
            ContainerOverlay->enableDropPreview(InvalidDockWidgetArea == Area);
        }
        ContainerArea = ContainerOverlay->showOverlay(TopContainer);
        _this->activateWindow();
    }
    else
    {
        DockAreaOverlay->hideOverlay();
        // If there is only one single visible dock area in a container, then
        // it does not make sense to show a dock overlay because the dock area
        // would be removed and inserted at the same position. Only auto hide
        // area is allowed
        if (VisibleDockAreas == 1)
        {
            ContainerOverlay->setAllowedAreas(AutoHideDockAreas);
        }
        ContainerArea = ContainerOverlay->showOverlay(TopContainer);
        _this->activateWindow();
        if (DockArea == ContentSourceArea
            && InvalidDockWidgetArea == ContainerDropArea)
        {
            DropContainer = nullptr;
        }
    }

    if (CDockManager::testConfigFlag(CDockManager::DragPreviewIsDynamic))
    {
        setHidden(DAWArea != InvalidDockWidgetArea
                  || ContainerArea != InvalidDockWidgetArea);
    }
}

//============================================================================
FloatingDragPreviewPrivate::FloatingDragPreviewPrivate(
    CFloatingDragPreview* _public)
    : _this(_public)
{}

//============================================================================
void FloatingDragPreviewPrivate::createFloatingWidget()
{
    CDockWidget* DockWidget = qobject_cast<CDockWidget*>(Content);
    CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(Content);

    CFloatingDockContainer* FloatingWidget = nullptr;

    if (DockWidget
        && DockWidget->features().testFlag(CDockWidget::DockWidgetFloatable))
    {
        FloatingWidget = new CFloatingDockContainer(DockWidget);
    }
    else if (DockArea
             && DockArea->features().testFlag(CDockWidget::DockWidgetFloatable))
    {
        FloatingWidget = new CFloatingDockContainer(DockArea);
    }

    if (FloatingWidget)
    {
        QRect geo;
        FloatingWidget->setGeometry(_this->geometry());
        geo = FloatingWidget->geometry();
        FloatingWidget->show();
        if (!CDockManager::testConfigFlag(
                CDockManager::DragPreviewHasWindowFrame))
        {
            // QApplication::processEvents();
            int FrameHeight = FloatingWidget->frameGeometry().height()
                              - FloatingWidget->geometry().height();
            QRect FixedGeometry = geo;
            FixedGeometry.adjust(0, FrameHeight, 0, 0);
            FloatingWidget->setGeometry(FixedGeometry);
        }
        QTimer::singleShot(100, FloatingWidget, [FloatingWidget]() {
            FloatingWidget->activateWindow();
        });
    }
}

//============================================================================
CFloatingDragPreview::CFloatingDragPreview(QWidget* Content, QWidget* parent)
    : QWidget(parent), d(new FloatingDragPreviewPrivate(this))
{
    d->Content = Content;
    d->ContentFeatures = d->contentFeatures();
    setAttribute(Qt::WA_DeleteOnClose);
    if (CDockManager::testConfigFlag(CDockManager::DragPreviewHasWindowFrame))
    {
        setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint
                       | Qt::WindowCloseButtonHint);
    }
    else
    {
        setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
    }

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    auto Flags = windowFlags();
    Flags |= Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint;
    setWindowFlags(Flags);
#endif

    // setWindowOpacity(0.6);
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
                                         const QSize& Size, eDragState DragState,
                                         QWidget* MouseEventHandler)
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
        containerOverlay = d->ContentSourceArea->dockContainer()
                               ->floatingWidget()
                               ->containerOverlay();
        dockAreaOverlay = d->ContentSourceArea->dockContainer()
                              ->floatingWidget()
                              ->dockAreaOverlay();
    }
    else
    {
        containerOverlay = d->DockManager->containerOverlay();
        dockAreaOverlay = d->DockManager->dockAreaOverlay();
    }
    auto DockDropArea = dockAreaOverlay->visibleDropAreaUnderCursor();
    auto ContainerDropArea = containerOverlay->visibleDropAreaUnderCursor();
    bool ValidDropArea = (DockDropArea != InvalidDockWidgetArea)
                         || (ContainerDropArea != InvalidDockWidgetArea);

    // Non floatable auto hide widgets should stay in its current auto hide
    // state if they are dragged into a floating window
    if (ValidDropArea || d->isContentFloatable())
    {
        cleanupAutoHideContainerWidget(ContainerDropArea);
    }
    auto SourceContainer =
        d->ContentSourceArea ? d->ContentSourceArea->dockContainer() : nullptr;
    bool target_had_independent =
        d->DropContainer ? d->DropContainer->hasIndependentWidget() : false;
    bool source_had_independent =
        SourceContainer ? SourceContainer->hasIndependentWidget() : false;
    if (!d->DropContainer)
    {
        d->createFloatingWidget();
    }
    else if (DockDropArea != InvalidDockWidgetArea)
    {
        d->DropContainer->dropWidget(
            d->Content, DockDropArea,
            d->DropContainer->dockAreaAt(QCursor::pos()));
    }
    else if (ContainerDropArea != InvalidDockWidgetArea)
    {
        // If there is only one single dock area, and we drop into the center
        // then we tabify the dropped widget into the only visible dock area
        if (d->DropContainer->visibleDockAreaCount() <= 1
            && CenterDockWidgetArea == ContainerDropArea)
        {
            d->DropContainer->dropWidget(
                d->Content, ContainerDropArea,
                d->DropContainer->dockAreaAt(QCursor::pos()));
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
    if (SourceContainer)
        SourceContainer->fetchIndependentCount();
    if (d->DropContainer)
        d->DropContainer->fetchIndependentCount();
    bool aboutToDeleteOriginal = false;
    auto dockManager_ = d->DockManager;
    // Case - Existing Floating Drop Container is just changed
    auto SourceFloatingContainer =
        SourceContainer ? SourceContainer->floatingWidget() : nullptr;
    auto DropFloatingContainer =
        d->DropContainer ? d->DropContainer->floatingWidget() : nullptr;
    if (d->DropContainer && d->DropContainer->isFloating()  // Target is FDC
        && (ContainerDropArea != InvalidDockWidgetArea
            || DockDropArea != InvalidDockWidgetArea)          // Target exists
        && (SourceFloatingContainer != DropFloatingContainer)  // No update if the
                                                               // widget is
                                                               // dropped into
                                                               // itself
    )
    {
        // Check if the independence of this floating container changed by this
        // drop
        bool targetHasIndependent =
            d->DropContainer ? d->DropContainer->hasIndependentWidget() : false;
        if (targetHasIndependent != target_had_independent)
        {
            CFloatingDockContainer* RestoredFloatingWidget =
                DropFloatingContainer->moveContainerAndDelete();
            RestoredFloatingWidget->setUpdatesEnabled(true);
            QTimer::singleShot(100, RestoredFloatingWidget,
                               [RestoredFloatingWidget]() {
                                   RestoredFloatingWidget->activateWindow();
                               });
            aboutToDeleteOriginal = true;
        }
    }
    bool deleted = false;
    // Case - Source Container was Also Changed
    if (SourceContainer && SourceContainer->isFloating()
        && (SourceFloatingContainer != DropFloatingContainer)  // No update if the
                                                               // widget is
                                                               // dropped into
                                                               // itself
    )
    {
        bool sourceHasIndependent =
            SourceContainer ? SourceContainer->hasIndependentWidget() : false;
        if (sourceHasIndependent != source_had_independent)
        {
            if (SourceFloatingContainer->dockContainer() == SourceContainer)
            {
                deleted = true;
            }
            CFloatingDockContainer* RestoredFloatingWidget =
                SourceFloatingContainer->moveContainerAndDelete();
            RestoredFloatingWidget->setUpdatesEnabled(true);
            containerOverlay = nullptr;
            dockAreaOverlay = nullptr;
            if (!deleted)
            {
                d->ContentSourceArea = nullptr;
            }
        }
    }
    if (!aboutToDeleteOriginal)
    {
        close();
    }
    if (containerOverlay)
        containerOverlay->hideOverlay();
    if (dockAreaOverlay)
        dockAreaOverlay->hideOverlay();
    if (dockManager_)
        dockManager_->containerOverlay()->hideOverlay();
    if (dockManager_)
        dockManager_->dockAreaOverlay()->hideOverlay();
}

//============================================================================
void CFloatingDragPreview::cleanupAutoHideContainerWidget(
    DockWidgetArea ContainerDropArea)
{
    auto DroppedDockWidget = qobject_cast<CDockWidget*>(d->Content);
    auto DroppedArea = qobject_cast<CDockAreaWidget*>(d->Content);
    auto AutoHideContainer = DroppedDockWidget ?
                                 DroppedDockWidget->autoHideDockContainer() :
                                 DroppedArea->autoHideDockContainer();

    if (!AutoHideContainer)
    {
        return;
    }

    // If the dropped widget is already an auto hide widget and if it is moved
    // to a new side bar location in the same container, then we do not need
    // to cleanup
    if (ads::internal::isSideBarArea(ContainerDropArea)
        && (d->DropContainer == AutoHideContainer->dockContainer()))
    {
        return;
    }

    AutoHideContainer->cleanupAndDelete();
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
    painter.setOpacity(0.6);
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
    bool isAppActive = false;
#ifdef Q_OS_WINDOWS
    for (auto topLevel : QApplication::topLevelWindows())
    {
        bool isWindowActive = (HWND)topLevel->winId() == ::GetForegroundWindow();
        isAppActive |= isWindowActive;
    }
#endif
    if (state != Qt::ApplicationActive && !isAppActive)
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

}  // namespace ads

//---------------------------------------------------------------------------
// EOF FloatingDragPreview.cpp
