//============================================================================
/// \file   ResizeHandle.cpp
/// \author Uwe Kindler
/// \date   24.10.2022
/// \brief  Implementation of CResizeHandle class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "ResizeHandle.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPointer>
#include <QRubberBand>
#include <QStyle>
#include <QStyleOption>

#include "AutoHideDockContainer.h"
#include "DockContainerWidget.h"
#include "ads_globals.h"

namespace ads
{
/**
 * Private data class of CResizeHandle class (pimpl)
 */
struct ResizeHandlePrivate
{
    CResizeHandle* _this;
    Qt::Edge HandlePosition = Qt::LeftEdge;
    QWidget* Target = nullptr;
    int MouseOffset = 0;
    bool Pressed = false;
    int MinSize = 0;
    int MaxSize = 1;
    QPointer<QRubberBand> RubberBand;
    bool OpaqueResize = false;
    int HandleWidth = 6;
    bool HasOverrideCursor = false;

    /**
     * Private data constructor
     */
    ResizeHandlePrivate(CResizeHandle* _public);

    /**
     * Pick position component from pos depending on orientation
     */
    int pick(const QPoint& pos) const
    {
        return _this->orientation() == Qt::Horizontal ? pos.x() : pos.y();
    }

    /**
     * Returns true, if orientation is horizontal
     */
    bool isHorizontal() const { return _this->orientation() == Qt::Horizontal; }

    /**
     * Set rubberband position
     */
    void setRubberBand(int Pos);

    /**
     * Calculates the resize position and geometry
     * returns the new geometry relative to the old geometry.
     */
    QRect doResizing(QMouseEvent* e, bool ForceResize = false);

	QRect newGeometry(QMouseEvent* e, int& outRubberBandPos);
};
// struct ResizeHandlePrivate

//============================================================================
ResizeHandlePrivate::ResizeHandlePrivate(CResizeHandle* _public) : _this(_public)
{}

//============================================================================
void ResizeHandlePrivate::setRubberBand(int Pos)
{
    if (!RubberBand)
    {
        RubberBand = new QRubberBand(QRubberBand::Line, Target->parentWidget());
    }

    auto Geometry = _this->geometry();
    auto TopLeft = Target->mapTo(Target->parentWidget(), Geometry.topLeft());
    switch (HandlePosition)
    {
    case Qt::LeftEdge:
    case Qt::RightEdge: TopLeft.rx() += Pos; break;
    case Qt::TopEdge:
    case Qt::BottomEdge: TopLeft.ry() += Pos; break;
    }

    Geometry.moveTopLeft(TopLeft);
    RubberBand->setGeometry(Geometry);
    RubberBand->show();
}

//============================================================================
QRect ResizeHandlePrivate::doResizing(QMouseEvent* e, bool ForceResize)
{
    auto OldGeometry = Target->geometry();
    int pos = 0;
    auto NewGeometry = newGeometry(e, pos);
    auto RetGeometry = NewGeometry;
    if (HandlePosition & Qt::TopEdge || HandlePosition & Qt::LeftEdge)
    {
        QSize DeltaSize = OldGeometry.size() - NewGeometry.size();
        NewGeometry.moveTo(OldGeometry.topLeft() + QPoint(DeltaSize.width(), DeltaSize.height()));
    }
    else
    {
        NewGeometry.moveTo(OldGeometry.topLeft());
    }
    if (_this->opaqueResize() || ForceResize)
    {
        Target->setGeometry(NewGeometry);
    }
    else
    {
        setRubberBand(pos);
    }
    return RetGeometry;
}

QRect ResizeHandlePrivate::newGeometry(QMouseEvent* e, int& outRubberBandPos)
{
    int pos = pick(e->pos()) - MouseOffset;
    auto OldGeometry = Target->geometry();
    auto NewGeometry = OldGeometry;
    switch (HandlePosition)
    {
    case Qt::LeftEdge:
    {
        NewGeometry.adjust(pos, 0, 0, 0);
        int Size = qBound(MinSize, NewGeometry.width(), MaxSize);
        pos += (NewGeometry.width() - Size);
        NewGeometry.setWidth(Size);
        NewGeometry.moveTopRight(OldGeometry.topRight());
    }
    break;

    case Qt::RightEdge:
    {
        NewGeometry.adjust(0, 0, pos, 0);
        int Size = qBound(MinSize, NewGeometry.width(), MaxSize);
        pos -= (NewGeometry.width() - Size);
        NewGeometry.setWidth(Size);
    }
    break;

    case Qt::TopEdge:
    {
        NewGeometry.adjust(0, pos, 0, 0);
        int Size = qBound(MinSize, NewGeometry.height(), MaxSize);
        pos += (NewGeometry.height() - Size);
        NewGeometry.setHeight(Size);
        NewGeometry.moveBottomLeft(OldGeometry.bottomLeft());
    }
    break;

    case Qt::BottomEdge:
    {
        NewGeometry.adjust(0, 0, 0, pos);
        int Size = qBound(MinSize, NewGeometry.height(), MaxSize);
        pos -= (NewGeometry.height() - Size);
        NewGeometry.setHeight(Size);
    }
    break;
    }
    if (HandlePosition & Qt::TopEdge || HandlePosition & Qt::LeftEdge)
    {
        QSize DeltaSize = OldGeometry.size() - NewGeometry.size();
        NewGeometry.moveTo(DeltaSize.width(), DeltaSize.height());
    }
    else
    {
        NewGeometry.moveTo(0, 0);
    }
    outRubberBandPos = pos;
    return NewGeometry;
}

//============================================================================
CResizeHandle::CResizeHandle(Qt::Edge HandlePosition, QWidget* parent)
    : Super(parent), d(new ResizeHandlePrivate(this))
{
    d->Target = parent;
    setMinResizeSize(48);
    setHandlePosition(HandlePosition);
    qApp->installEventFilter(this);
}

//============================================================================
CResizeHandle::~CResizeHandle()
{
    delete d;
}

//============================================================================
void CResizeHandle::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        d->doResizing(e);
    }
}

//============================================================================
void CResizeHandle::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        d->MouseOffset = d->pick(e->pos());
        d->Pressed = true;
        update();
    }
}

//============================================================================
void CResizeHandle::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        int unused;
        QRect NewGeo = d->newGeometry(e, unused);
		if (orientation() == Qt::Horizontal)
        {
            NewGeo.setWidth(d->HandleWidth);
		}
		else
        {
            NewGeo.setHeight(d->HandleWidth);
		}
        QPoint p_ = e->globalPosition().toPoint();
        CAutoHideDockContainer* ahdc =
            qobject_cast<CAutoHideDockContainer*>(d->Target);
        if (d->HasOverrideCursor && !NewGeo.contains(ahdc->mapFromGlobal(p_)))
        {
            QApplication::restoreOverrideCursor();
            d->HasOverrideCursor = false;
        }
    }
    if (!opaqueResize() && e->button() == Qt::LeftButton)
    {
        if (d->RubberBand)
        {
            d->RubberBand->deleteLater();
        }
        d->doResizing(e, true);
    }
    if (e->button() == Qt::LeftButton)
    {
        d->Pressed = false;
        update();
    }
}

//============================================================================
bool CResizeHandle::eventFilter(QObject* receiver, QEvent* event)
{
    if (d->Target->isVisible())
    {
        if (event->type() == QEvent::Type::MouseMove)
        {
            CAutoHideDockContainer* ahdc =
                qobject_cast<CAutoHideDockContainer*>(d->Target);
            QMouseEvent* me = (QMouseEvent*)event;
            if (!d->HasOverrideCursor && (me->buttons() == Qt::NoButton)
                && rect().contains(mapFromGlobal(me->globalPosition().toPoint())))
            {
                Qt::CursorShape s = Qt::CursorShape::ArrowCursor;
                switch (d->HandlePosition)
                {
                case Qt::LeftEdge:  // fall through
                case Qt::RightEdge: s = Qt::SizeHorCursor; break;

                case Qt::TopEdge:  // fall through
                case Qt::BottomEdge: s = Qt::SizeVerCursor; break;
                }
                QApplication::setOverrideCursor(s);
                d->HasOverrideCursor = true;
            }
            else if (d->HasOverrideCursor && !(me->buttons() & Qt::LeftButton)
                     && !rect().contains(
                         mapFromGlobal(me->globalPosition().toPoint())))
            {
                QApplication::restoreOverrideCursor();
                d->HasOverrideCursor = false;
            }
        }
    }
    return false;
}

//============================================================================
void CResizeHandle::setHandlePosition(Qt::Edge HandlePosition)
{
    d->HandlePosition = HandlePosition;

    setMaxResizeSize(d->isHorizontal() ? parentWidget()->height() :
                                         parentWidget()->width());
    if (!d->isHorizontal())
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    else
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
}

//============================================================================
Qt::Edge CResizeHandle::handlePostion() const
{
    return d->HandlePosition;
}

//============================================================================
Qt::Orientation CResizeHandle::orientation() const
{
    switch (d->HandlePosition)
    {
    case Qt::LeftEdge:  // fall through
    case Qt::RightEdge: return Qt::Horizontal;

    case Qt::TopEdge:  // fall through
    case Qt::BottomEdge: return Qt::Vertical;
    }

    return Qt::Horizontal;
}

//============================================================================
QSize CResizeHandle::sizeHint() const
{
    QSize Result;
    switch (d->HandlePosition)
    {
    case Qt::LeftEdge:  // fall through
    case Qt::RightEdge:
        Result = QSize(d->HandleWidth, d->Target->height());
        break;

    case Qt::TopEdge:  // fall through
    case Qt::BottomEdge:
        Result = QSize(d->Target->width(), d->HandleWidth);
        break;
    }

    return Result;
}

//============================================================================
bool CResizeHandle::isResizing() const
{
    return d->Pressed;
}

//============================================================================
void CResizeHandle::setMinResizeSize(int MinSize)
{
    d->MinSize = MinSize;
}

//============================================================================
void CResizeHandle::setMaxResizeSize(int MaxSize)
{
    d->MaxSize = MaxSize >= d->MinSize ? MaxSize : d->MinSize;
    Q_ASSERT(d->MaxSize > 0 && d->MaxSize >= d->MinSize);
}

//============================================================================
void CResizeHandle::setOpaqueResize(bool opaque)
{
    d->OpaqueResize = opaque;
}

//============================================================================
bool CResizeHandle::opaqueResize() const
{
    return d->OpaqueResize;
}
}  // namespace ads

//---------------------------------------------------------------------------
// EOF ResizeHandle.cpp
