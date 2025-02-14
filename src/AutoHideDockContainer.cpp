/*******************************************************************************
** Qt Advanced Docking System
** Copyright (C) 2017 Uwe Kindler
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

//============================================================================
/// \file   AutoHideDockContainer.cpp
/// \author Syarif Fakhri
/// \date   05.09.2022
/// \brief  Implementation of CAutoHideDockContainer class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "AutoHideDockContainer.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCursor>
#include <QPainter>
#include <QPointer>
#include <QSplitter>
#include <QXmlStreamWriter>

#include <iostream>

#include "AutoHideSideBar.h"
#include "AutoHideTab.h"
#include "DockAreaWidget.h"
#include "DockComponentsFactory.h"
#include "DockManager.h"
#include "DockSplitter.h"
#include "ResizeHandle.h"

namespace ads
{
static const int ResizeMargin = 30;

//============================================================================
bool static isHorizontalArea(SideBarLocation Area)
{
    switch (Area)
    {
    case SideBarLocation::SideBarTop:
    case SideBarLocation::SideBarBottom: return true;
    case SideBarLocation::SideBarLeft:
    case SideBarLocation::SideBarRight: return false;
    default: return true;
    }

    return true;
}

//============================================================================
Qt::Edge static edgeFromSideTabBarArea(SideBarLocation Area)
{
    switch (Area)
    {
    case SideBarLocation::SideBarTop: return Qt::BottomEdge;
    case SideBarLocation::SideBarBottom: return Qt::TopEdge;
    case SideBarLocation::SideBarLeft: return Qt::RightEdge;
    case SideBarLocation::SideBarRight: return Qt::LeftEdge;
    default: return Qt::LeftEdge;
    }

    return Qt::LeftEdge;
}

//============================================================================
int resizeHandleLayoutPosition(SideBarLocation Area)
{
    switch (Area)
    {
    case SideBarLocation::SideBarBottom:
    case SideBarLocation::SideBarRight: return 0;

    case SideBarLocation::SideBarTop:
    case SideBarLocation::SideBarLeft: return 1;

    default: return 0;
    }

    return 0;
}

/**
 * Private data of CAutoHideDockContainer - pimpl
 */
struct AutoHideDockContainerPrivate
{
    CAutoHideDockContainer* _this;
    CDockAreaWidget* DockArea{nullptr};
    CDockWidget* DockWidget{nullptr};
    SideBarLocation SideTabBarArea = SideBarNone;
    QBoxLayout* Layout = nullptr;
    CResizeHandle* ResizeHandle = nullptr;
    QSize Size;  // creates invalid size
    QPointer<CAutoHideTab> SideTab;
    QSize SizeCache;

    /**
     * Private data constructor
     */
    AutoHideDockContainerPrivate(CAutoHideDockContainer* _public);

    /**
     * Convenience function to get a dock widget area
     */
    DockWidgetArea getDockWidgetArea(SideBarLocation area)
    {
        switch (area)
        {
        case SideBarLocation::SideBarLeft: return LeftDockWidgetArea;
        case SideBarLocation::SideBarRight: return RightDockWidgetArea;
        case SideBarLocation::SideBarBottom: return BottomDockWidgetArea;
        case SideBarLocation::SideBarTop: return TopDockWidgetArea;
        default: return LeftDockWidgetArea;
        }

        return LeftDockWidgetArea;
    }

    /**
     * Update the resize limit of the resize handle
     */
    void updateResizeHandleSizeLimitMax()
    {
        auto Rect = _this->dockContainer()->contentRect();
        const auto maxResizeHandleSize =
            ResizeHandle->orientation() == Qt::Horizontal ? Rect.width() :
                                                            Rect.height();
        ResizeHandle->setMaxResizeSize(maxResizeHandleSize - ResizeMargin);
    }

    /**
     * Convenience function to check, if this is an horizontal area
     */
    bool isHorizontal() const { return isHorizontalArea(SideTabBarArea); }

    /**
     * Forward this event to the dock container
     */
    void forwardEventToDockContainer(QEvent* event)
    {
        auto DockContainer = _this->dockContainer();
        if (DockContainer)
        {
            DockContainer->handleAutoHideWidgetEvent(event, _this);
        }
    }

};  // struct AutoHideDockContainerPrivate

//============================================================================
AutoHideDockContainerPrivate::AutoHideDockContainerPrivate(
    CAutoHideDockContainer* _public)
    : _this(_public)
{}

//============================================================================
CDockContainerWidget* CAutoHideDockContainer::dockContainer() const
{
    return internal::findParent<CDockContainerWidget*>(this);
}

//============================================================================
CAutoHideDockContainer::CAutoHideDockContainer(CDockWidget* DockWidget,
                                               SideBarLocation area,
                                               CDockContainerWidget* parent)
    : Super(parent), d(new AutoHideDockContainerPrivate(this))
{
    // QT_ADS: temp fix, remove it on merge
	setWindowFlag(Qt::SubWindow, true);
    hide();  // auto hide dock container is initially always hidden
    d->SideTabBarArea = area;
    d->SideTab = componentsFactory()->createDockWidgetSideTab(nullptr);
    connect(d->SideTab, &CAutoHideTab::pressed, this,
            &CAutoHideDockContainer::toggleCollapseState);
    d->DockArea = new CDockAreaWidget(DockWidget->dockManager(), parent);
    d->DockArea->setObjectName("autoHideDockArea");
    d->DockArea->setAutoHideDockContainer(this);

    setObjectName("autoHideDockContainer");
    setAutoFillBackground(true);
    d->Layout = new QBoxLayout(isHorizontalArea(area) ? QBoxLayout::TopToBottom :
                                                        QBoxLayout::LeftToRight);
    d->Layout->setContentsMargins(0, 0, 0, 0);
    d->Layout->setSpacing(0);
    setLayout(d->Layout);
    d->ResizeHandle = new CResizeHandle(edgeFromSideTabBarArea(area), this);
    d->ResizeHandle->setMinResizeSize(64);
    bool OpaqueResize =
        CDockManager::testConfigFlag(CDockManager::OpaqueSplitterResize);
    d->ResizeHandle->setOpaqueResize(OpaqueResize);
    d->Size = d->DockArea->size();
    d->SizeCache = DockWidget->size();

    addDockWidget(DockWidget);
    parent->registerAutoHideWidget(this);
    // The dock area should not be added to the layout before it contains the
    // dock widget. If you add it to the layout before it contains the dock widget
    // then you will likely see this warning for OpenGL widgets or QAxWidgets:
    // setGeometry: Unable to set geometry XxY+Width+Height on
    // QWidgetWindow/'WidgetClassWindow
    d->Layout->addWidget(d->DockArea);
    d->Layout->insertWidget(resizeHandleLayoutPosition(area), d->ResizeHandle);
}

//============================================================================
void CAutoHideDockContainer::updateSize()
{
    auto dockContainerParent = dockContainer();
    if (!dockContainerParent)
    {
        return;
    }

    auto rect = dockContainerParent->contentRect();
    switch (sideBarLocation())
    {
    case SideBarLocation::SideBarTop:
        resize(rect.width(),
               qMin(rect.height() - ResizeMargin, d->Size.height()));
        move(rect.topLeft());
        break;

    case SideBarLocation::SideBarLeft:
        resize(qMin(d->Size.width(), rect.width() - ResizeMargin), rect.height());
        move(rect.topLeft());
        break;

    case SideBarLocation::SideBarRight:
    {
        resize(qMin(d->Size.width(), rect.width() - ResizeMargin), rect.height());
        QPoint p = rect.topRight();
        p.rx() -= (width() - 1);
        move(p);
    }
    break;

    case SideBarLocation::SideBarBottom:
    {
        resize(rect.width(),
               qMin(rect.height() - ResizeMargin, d->Size.height()));
        QPoint p = rect.bottomLeft();
        p.ry() -= (height() - 1);
        move(p);
    }
    break;

    default: break;
    }

    if (orientation() == Qt::Horizontal)
    {
        d->SizeCache.setHeight(this->height());
    }
    else
    {
        d->SizeCache.setWidth(this->width());
    }
}

//============================================================================
CAutoHideDockContainer::~CAutoHideDockContainer()
{
    ADS_PRINT("~CAutoHideDockContainer");

    // Remove event filter in case there are any queued messages
    qApp->removeEventFilter(this);
    if (dockContainer())
    {
        dockContainer()->removeAutoHideWidget(this);
    }

    if (d->SideTab)
    {
        delete d->SideTab;
    }

    delete d;
}

void CAutoHideDockContainer::setVisible(bool v)
{
	Super::setVisible(v);
}

//============================================================================
CAutoHideSideBar* CAutoHideDockContainer::autoHideSideBar() const
{
    if (d->SideTab)
    {
        return d->SideTab->sideBar();
    }
    else
    {
        auto DockContainer = dockContainer();
        return DockContainer ? DockContainer->autoHideSideBar(d->SideTabBarArea) :
                               nullptr;
    }
}

//============================================================================
CAutoHideTab* CAutoHideDockContainer::autoHideTab() const
{
    return d->SideTab;
}

//============================================================================
CDockWidget* CAutoHideDockContainer::dockWidget() const
{
    return d->DockWidget;
}

//============================================================================
void CAutoHideDockContainer::addDockWidget(CDockWidget* DockWidget)
{
    if (d->DockWidget)
    {
        // Remove the old dock widget at this area
        d->DockArea->removeDockWidget(d->DockWidget);
    }

    d->DockWidget = DockWidget;
    d->SideTab->setDockWidget(DockWidget);
    CDockAreaWidget* OldDockArea = DockWidget->dockAreaWidget();
    auto IsRestoringState = DockWidget->dockManager()->isRestoringState();
    if (OldDockArea && !IsRestoringState)
    {
        // The initial size should be a little bit bigger than the original dock
        // area size to prevent that the resize handle of this auto hid dock area
        // is near of the splitter of the old dock area.
        d->Size = OldDockArea->size() + QSize(16, 16);
        OldDockArea->removeDockWidget(DockWidget);
    }
    d->DockArea->addDockWidget(DockWidget);
    updateSize();
    // The dock area is not visible and will not update the size when updateSize()
    // is called for this auto hide container. Therefore we explicitly resize
    // it here. As soon as it will become visible, it will get the right size
    d->DockArea->resize(size());
}

//============================================================================
SideBarLocation CAutoHideDockContainer::sideBarLocation() const
{
    return d->SideTabBarArea;
}

//============================================================================
void CAutoHideDockContainer::setSideBarLocation(SideBarLocation SideBarLocation)
{
    if (d->SideTabBarArea == SideBarLocation)
    {
        return;
    }

    d->SideTabBarArea = SideBarLocation;
    d->Layout->removeWidget(d->ResizeHandle);
    d->Layout->setDirection(isHorizontalArea(SideBarLocation) ?
                                QBoxLayout::TopToBottom :
                                QBoxLayout::LeftToRight);
    d->Layout->insertWidget(resizeHandleLayoutPosition(SideBarLocation),
                            d->ResizeHandle);
    d->ResizeHandle->setHandlePosition(edgeFromSideTabBarArea(SideBarLocation));
    internal::repolishStyle(this, internal::RepolishDirectChildren);
}

//============================================================================
CDockAreaWidget* CAutoHideDockContainer::dockAreaWidget() const
{
    return d->DockArea;
}

//============================================================================
void CAutoHideDockContainer::moveContentsToParent()
{
    int s = this->getSize();
    auto orientation = this->orientation();
    auto DockContainer = dockContainer();
    auto sideBarLoc = sideBarLocation();
    auto totalSize = orientation == Qt::Orientation::Horizontal ?
                         DockContainer->size().height() :
                         DockContainer->size().width();
    cleanupAndDelete();
    // If we unpin the auto hide dock widget, then we insert it into the same
    // location like it had as a auto hide widget.  This brings the least surprise
    // to the user and he does not have to search where the widget was inserted.
    d->DockWidget->setDockArea(nullptr);
    auto newDockArea = DockContainer->addDockWidget(
        d->getDockWidgetArea(d->SideTabBarArea), d->DockWidget);
    auto newDockAreaParentSplitter = newDockArea->parentSplitter();
    if (newDockAreaParentSplitter)
    {
        auto newDAsizes = newDockAreaParentSplitter->sizes();
        switch (sideBarLoc)
        {
        case ads::SideBarTop:
        case ads::SideBarLeft:
        {
            if (newDAsizes.size() > 2)
            {
                *newDAsizes.begin() = s;
                for (int i = 1; i < newDAsizes.size(); i++)
                {
                    float ratio = float(newDAsizes[i]) / float(totalSize);
                    int newSize = qRound((totalSize - s) * ratio);
                    newDAsizes[i] = newSize;
                }
                newDockAreaParentSplitter->setSizes(newDAsizes);
			}
            else if (newDAsizes.size() == 2)
            {
                newDockArea->parentSplitter()->setSizes({s, totalSize - s});
            }

        }
        break;
        case ads::SideBarRight:
        case ads::SideBarBottom:
        {
            if (newDAsizes.size() > 2)
            {
                *(newDAsizes.end() - 1) = s;
                for (int i = 0; i < newDAsizes.size() - 1; i++)
                {
                    float ratio = float(newDAsizes[i]) / float(totalSize);
                    int newSize = qRound((totalSize - s) * ratio);
                    newDAsizes[i] = newSize;
                }
                newDockAreaParentSplitter->setSizes(newDAsizes);
            }
            else if (newDAsizes.size() == 2)
            {
                newDockArea->parentSplitter()->setSizes({totalSize - s, s});
            }
        }
        break;
        default: break;
        }
    }
}

//============================================================================
void CAutoHideDockContainer::cleanupAndDelete()
{
    const auto dockWidget = d->DockWidget;
    if (dockWidget)
    {
        auto SideTab = d->SideTab;
        SideTab->removeFromSideBar();
        SideTab->setParent(nullptr);
        SideTab->hide();
    }

    hide();
    deleteLater();
}

//============================================================================
void CAutoHideDockContainer::saveState(QXmlStreamWriter& s)
{
    s.writeStartElement("Widget");
    s.writeAttribute("Name", d->DockWidget->objectName());
    s.writeAttribute("Closed",
                     QString::number(d->DockWidget->isClosed() ? 1 : 0));
    s.writeAttribute(
        "Size",
        QString::number(d->isHorizontal() ? d->Size.height() : d->Size.width()));
    s.writeEndElement();
}

//============================================================================
void CAutoHideDockContainer::toggleView(bool Enable)
{
    if (Enable)
    {
        if (d->SideTab)
        {
            d->SideTab->show();
        }
    }
    else
    {
        if (d->SideTab)
        {
            d->SideTab->hide();
        }
        hide();
        qApp->removeEventFilter(this);
    }
}

//============================================================================
void CAutoHideDockContainer::collapseView(bool Enable)
{
    if (Enable)
    {
        hide();
        qApp->removeEventFilter(this);
    }
    else
    {
        updateSize();
        d->updateResizeHandleSizeLimitMax();
        raise();
        show();
        d->DockWidget->dockManager()->setDockWidgetFocused(d->DockWidget);
        qApp->installEventFilter(this);
    }

    ADS_PRINT("CAutoHideDockContainer::collapseView " << Enable);
    d->SideTab->updateStyle();
}

//============================================================================
void CAutoHideDockContainer::toggleCollapseState()
{
    collapseView(isVisible());
}

//============================================================================
void CAutoHideDockContainer::setSize(int Size)
{
    if (d->isHorizontal())
    {
        d->Size.setHeight(Size);
    }
    else
    {
        d->Size.setWidth(Size);
    }

    updateSize();
}

//============================================================================
int CAutoHideDockContainer::getSize() const
{
    if (d->isHorizontal())
    {
        return d->Size.height();
    }
    else
    {
        return d->Size.width();
    }
}

//============================================================================
/**
 * Returns true if the object given in ancestor is an ancestor of the object
 * given in descendant
 */
static bool objectIsAncestorOf(const QObject* descendant, const QObject* ancestor)
{
    if (!ancestor)
    {
        return false;
    }
    while (descendant)
    {
        if (descendant == ancestor)
        {
            return true;
        }
        descendant = descendant->parent();
    }
    return false;
}

//============================================================================
/**
 * Returns true if the object given in ancestor is the object given in descendant
 * or if it is an ancestor of the object given in descendant
 */
static bool isObjectOrAncestor(const QObject* descendant, const QObject* ancestor)
{
    if (ancestor && (descendant == ancestor))
    {
        return true;
    }
    else
    {
        return objectIsAncestorOf(descendant, ancestor);
    }
}

//============================================================================
bool CAutoHideDockContainer::eventFilter(QObject* watched, QEvent* event)
{
    // A switch case statement would be nicer here, but we cannot use
    // internal::FloatingWidgetDragStartEvent in a switch case
    if (event->type() == QEvent::Resize)
    {
        if (!d->ResizeHandle->isResizing())
        {
            updateSize();
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        auto widget = qobject_cast<QWidget*>(watched);
        // Ignore non widget events
        if (!widget)
        {
            return Super::eventFilter(watched, event);
        }

        // Now check, if the user clicked into the side tab and ignore this event,
        // because the side tab click handler will call collapseView(). If we
        // do not ignore this here, then we will collapse the container and the
        // side tab click handler will uncollapse it
        if (widget == d->SideTab.data())
        {
            return Super::eventFilter(watched, event);
        }

        // Now we check, if the user clicked inside of this auto hide container.
        // If the click is inside of this auto hide container, then we can
        // ignore the event, because the auto hide overlay should not get
        // collapsed if user works in it
        if (isObjectOrAncestor(widget, this))
        {
            return Super::eventFilter(watched, event);
        }

        // Ignore the mouse click if it is not inside of this container
        if (!isObjectOrAncestor(widget, dockContainer()))
        {
            return Super::eventFilter(watched, event);
        }

        // clicking into a sidebar should not close the auto hide widget
        if (widget == d->SideTab->sideBar() || qobject_cast<CAutoHideSideBar*>(widget))
		{
			return Super::eventFilter(watched, event);
        }
        
        // clicking into a different sidebar tab will also close this container
        if (qobject_cast<CAutoHideTab*>(widget))
        {
			collapseView(true);
			return Super::eventFilter(watched, event);
        }

        // otherwise, only clicking into a dock area should close it
        for (auto dockAreaIn : dockContainer()->openedDockAreas())
        {
            if (isObjectOrAncestor(widget, dockAreaIn))
            {
        collapseView(true);
                return Super::eventFilter(watched, event);
    }
        }
    }
    else if (event->type() == internal::FloatingWidgetDragStartEvent)
    {
        // If we are dragging our own floating widget, the we do not need to
        // collapse the view
        auto FloatingWidget = dockContainer()->floatingWidget();
        if (FloatingWidget != watched)
        {
            collapseView(true);
        }
    }
    else if (event->type() == internal::DockedWidgetDragStartEvent)
    {
		internal::CFloatingWidgetDragStartEvent* cEvent = (internal::CFloatingWidgetDragStartEvent*)event;
		// close the auto hide containers that are not this
		// we cant close the current auto hide container because if we do
		// then the title bar cant process mouse move events correctly
		if (!objectIsAncestorOf(cEvent->content(), this))
		{
        collapseView(true);
    }
    }

    return Super::eventFilter(watched, event);
}

//============================================================================
void CAutoHideDockContainer::resizeEvent(QResizeEvent* event)
{
    Super::resizeEvent(event);
    if (d->ResizeHandle->isResizing())
    {
        d->Size = this->size();
        d->updateResizeHandleSizeLimitMax();
    }
}

//============================================================================
void CAutoHideDockContainer::leaveEvent(QEvent* event)
{
    // Resizing of the dock container via the resize handle in non opaque mode
    // mays cause a leave event that is not really a leave event. Therefore
    // we check here, if we are really outside of our rect.
    auto pos = mapFromGlobal(QCursor::pos());
    if (!rect().contains(pos))
    {
        d->forwardEventToDockContainer(event);
    }
    Super::leaveEvent(event);
}

//============================================================================
bool CAutoHideDockContainer::event(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::Enter:
    case QEvent::Hide: d->forwardEventToDockContainer(event); break;

    case QEvent::MouseButtonPress: return true; break;

    default: break;
    }

    return Super::event(event);
}

//============================================================================
Qt::Orientation CAutoHideDockContainer::orientation() const
{
    return ads::internal::isHorizontalSideBarLocation(d->SideTabBarArea) ?
               Qt::Horizontal :
               Qt::Vertical;
}

//============================================================================
void CAutoHideDockContainer::resetToInitialDockWidgetSize()
{
    if (orientation() == Qt::Horizontal)
    {
        setSize(d->SizeCache.height());
    }
    else
    {
        setSize(d->SizeCache.width());
    }
}

//============================================================================
void CAutoHideDockContainer::moveToNewSideBarLocation(
    SideBarLocation NewSideBarLocation, int TabIndex)
{
    if (NewSideBarLocation == sideBarLocation() && TabIndex == this->tabIndex())
    {
        return;
    }

    auto OldOrientation = orientation();
    auto SideBar = dockContainer()->autoHideSideBar(NewSideBarLocation);
    SideBar->addAutoHideWidget(this, TabIndex);
    // If we move a horizontal auto hide container to a vertical position
    // then we resize it to the original dock widget size, to avoid
    // an extremely stretched dock widget after insertion
    if (SideBar->orientation() != OldOrientation)
    {
        resetToInitialDockWidgetSize();
    }
}

//============================================================================
int CAutoHideDockContainer::tabIndex() const
{
    return d->SideTab->tabIndex();
}

}  // namespace ads
