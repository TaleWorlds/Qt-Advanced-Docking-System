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
 /// \file   FloatingWidgetTitleBar.cpp
 /// \author Uwe Kindler
 /// \date   13.05.2019
 /// \brief  Implementation of CFloatingWidgetTitleBar class
 //============================================================================

 //============================================================================
 //                                   INCLUDES
 //============================================================================
#include "FloatingWidgetTitleBar.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QPixmap>
#include <QStyle>
#include <QMouseEvent>
#include <QWindow>

#include "ads_globals.h"
#include "ElidingLabel.h"
#include "DockManager.h"
#include "FloatingDockContainer.h"

namespace ads
{

	using tTabLabel = CElidingLabel;
	using tCloseButton = QToolButton;
	using tMaximizeButton = QToolButton;

	/**
	 * @brief Private data class of public interface CFloatingWidgetTitleBar
	 */
	struct FloatingWidgetTitleBarPrivate
	{
		CFloatingWidgetTitleBar* _this; ///< public interface class
		QLabel* IconLabel = nullptr;
		tTabLabel* TitleLabel;
		tCloseButton* CloseButton = nullptr;
		tMaximizeButton* MaximizeButton = nullptr;
		CFloatingDockContainer* FloatingWidget = nullptr;
		QBoxLayout* Layout = nullptr;
		eDragState DragState = DraggingInactive;
		QIcon MaximizeIcon;
		QIcon NormalIcon;
		bool Maximized = false;

		FloatingWidgetTitleBarPrivate(CFloatingWidgetTitleBar* _public) :
			_this(_public)
		{
		}

		/**
		 * Creates the complete layout including all controls
		 */
		void createLayout();

		/**
		 * Creates an incomplete layout including all controls, but allows the user to
		 * specify icons from stylesheet
		 */
		void createCustomLayout();
	};

	//============================================================================
	void FloatingWidgetTitleBarPrivate::createLayout()
	{
		TitleLabel = new tTabLabel();
		TitleLabel->setElideMode(Qt::ElideRight);
		TitleLabel->setText("DockWidget->windowTitle()");
		TitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		CloseButton = new tCloseButton();
		CloseButton->setAutoRaise(true);

		MaximizeButton = new tMaximizeButton();
		MaximizeButton->setAutoRaise(true);

		// The standard icons do does not look good on high DPI screens
		QIcon CloseIcon;
		QPixmap normalPixmap = _this->style()->standardPixmap(
			QStyle::SP_TitleBarCloseButton, 0, CloseButton);
		CloseIcon.addPixmap(normalPixmap, QIcon::Normal);
		CloseIcon.addPixmap(internal::createTransparentPixmap(normalPixmap, 0.25),
			QIcon::Disabled);
		CloseButton->setIcon(
			_this->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
		CloseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		CloseButton->setVisible(true);
		CloseButton->setFocusPolicy(Qt::NoFocus);
		_this->connect(CloseButton, SIGNAL(clicked()), SIGNAL(closeRequested()));

		_this->setMaximizedIcon(false);
		MaximizeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		MaximizeButton->setVisible(true);
		MaximizeButton->setFocusPolicy(Qt::NoFocus);
		_this->connect(MaximizeButton, &QPushButton::clicked, _this, &CFloatingWidgetTitleBar::maximizeRequested);

		QFontMetrics fm(TitleLabel->font());
		int Spacing = qRound(fm.height() / 4.0);

		// Fill the layout
		Layout = new QBoxLayout(QBoxLayout::LeftToRight);
		Layout->setContentsMargins(6, 0, 0, 0);
		Layout->setSpacing(0);
		_this->setLayout(Layout);
		Layout->addWidget(TitleLabel, 1);
		Layout->addSpacing(Spacing);
		Layout->addWidget(MaximizeButton);
		Layout->addWidget(CloseButton);
		Layout->setAlignment(Qt::AlignCenter);

		TitleLabel->setVisible(true);
	}

	void FloatingWidgetTitleBarPrivate::createCustomLayout()
	{
		TitleLabel = new tTabLabel();
		TitleLabel->setElideMode(Qt::ElideRight);
		TitleLabel->setText("DockWidget->windowTitle()");
		TitleLabel->setObjectName("floatingTitleLabel");
		TitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		CloseButton = new tCloseButton();
		CloseButton->setObjectName("floatingTitleCloseButton");
		CloseButton->setAutoRaise(true);

		MaximizeButton = new tMaximizeButton();
		MaximizeButton->setObjectName("floatingTitleMaximizeButton");
		MaximizeButton->setAutoRaise(true);

		// So icons may be provided from stylesheets using QToolButton#floatingTitleXButton{icon: ...}
		CloseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		CloseButton->setVisible(true);
		CloseButton->setFocusPolicy(Qt::NoFocus);
		_this->connect(CloseButton, SIGNAL(clicked()), SIGNAL(closeRequested()));

		MaximizeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		MaximizeButton->setVisible(true);
		MaximizeButton->setFocusPolicy(Qt::NoFocus);
		_this->connect(MaximizeButton, &QPushButton::clicked, _this, &CFloatingWidgetTitleBar::maximizeRequested);

		QFontMetrics fm(TitleLabel->font());
		int Spacing = qRound(fm.height() / 4.0);

		// Fill the layout
		Layout = new QBoxLayout(QBoxLayout::LeftToRight);
		Layout->setContentsMargins(6, 0, 0, 0);
		Layout->setSpacing(0);
		_this->setLayout(Layout);
		Layout->addWidget(TitleLabel, 1);
		Layout->addSpacing(Spacing);
		Layout->addWidget(MaximizeButton);
		Layout->addWidget(CloseButton);
		Layout->setAlignment(Qt::AlignCenter);

		TitleLabel->setVisible(true);
	}

	//============================================================================
	CFloatingWidgetTitleBar::CFloatingWidgetTitleBar(CFloatingDockContainer* parent) :
		QFrame(parent),
		d(new FloatingWidgetTitleBarPrivate(this))
	{
		setObjectName("floatingTitleBar");
		d->FloatingWidget = parent;

		if (CDockManager::testConfigFlag(CDockManager::FloatingContainerForceQWidgetTitleBar))
		{
			d->createLayout();
		}
		else if (CDockManager::testConfigFlag(CDockManager::FloatingContainerForceQWidgetCustomStyledTitleBar))
		{
			d->createCustomLayout();
		}
		d->Layout->setParent(this);
		d->MaximizeButton->setParent(this);
		d->CloseButton->setParent(this);
		d->TitleLabel->setParent(this);
		setParent(parent);
		if (CDockManager::testConfigFlag(CDockManager::FloatingContainerForceQWidgetTitleBar))
		{
			auto normalPixmap = this->style()->standardPixmap(QStyle::StandardPixmap::SP_TitleBarNormalButton, 0, d->MaximizeButton);
			d->NormalIcon.addPixmap(normalPixmap, QIcon::Normal);
			d->NormalIcon.addPixmap(internal::createTransparentPixmap(normalPixmap, 0.25), QIcon::Disabled);

			auto maxPixmap = this->style()->standardPixmap(QStyle::SP_TitleBarMaxButton, 0, d->MaximizeButton);
			d->MaximizeIcon.addPixmap(maxPixmap, QIcon::Normal);
			d->MaximizeIcon.addPixmap(internal::createTransparentPixmap(maxPixmap, 0.25), QIcon::Disabled);
			setMaximizedIcon(d->Maximized);
		}
	}


	CFloatingWidgetTitleBar::CFloatingWidgetTitleBar(int i)
	{
		printf("%d", i);
	}

	//============================================================================
	CFloatingWidgetTitleBar::~CFloatingWidgetTitleBar()
	{
		delete d;
		d = nullptr;
	}

	//============================================================================
	void CFloatingWidgetTitleBar::setFloatingWidget(CFloatingDockContainer* parent)
	{
		if (!d->FloatingWidget)
		{
			d->FloatingWidget = parent;
		}
	}

	//============================================================================
	void CFloatingWidgetTitleBar::mousePressEvent(QMouseEvent* ev)
	{
		if (ev->button() == Qt::LeftButton)
		{
			d->DragState = DraggingFloatingWidget;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
			bool result = windowHandle()->startSystemMove();
			Q_ASSERT_X(result, "mousePressEvent()",
				"this->windowHandle()->startSystemMove() failed");
#endif
			d->FloatingWidget->startDragging(ev->pos(), d->FloatingWidget->size(),
				this);
			return;
		}
		Super::mousePressEvent(ev);
	}


	//============================================================================
	void CFloatingWidgetTitleBar::mouseReleaseEvent(QMouseEvent* ev)
	{
		d->DragState = DraggingInactive;
		if (d->FloatingWidget)
		{
			d->FloatingWidget->finishDragging();
		}
		Super::mouseReleaseEvent(ev);
	}


	//============================================================================
	void CFloatingWidgetTitleBar::mouseMoveEvent(QMouseEvent* ev)
	{
		if (!(ev->buttons() & Qt::LeftButton) || DraggingInactive == d->DragState)
		{
			d->DragState = DraggingInactive;
			Super::mouseMoveEvent(ev);
			return;
		}

		// Do not move the floating container outside the available screen geometry
		if ((ev->buttons() & Qt::LeftButton) && !d->_this->screen()->availableGeometry().contains(internal::globalPositionOf(ev)))
		{
			QPoint new_cursor_pos(internal::globalPositionOf(ev));
			if (internal::globalPositionOf(ev).x() < d->_this->screen()->availableGeometry().left())
			{
				new_cursor_pos.setX(d->_this->screen()->availableGeometry().left());
			}
			if (internal::globalPositionOf(ev).x() > d->_this->screen()->availableGeometry().right())
			{
				new_cursor_pos.setX(d->_this->screen()->availableGeometry().right());
			}
			if (internal::globalPositionOf(ev).y() < d->_this->screen()->availableGeometry().top())
			{
				new_cursor_pos.setY(d->_this->screen()->availableGeometry().top());
			}
			if (internal::globalPositionOf(ev).y() > d->_this->screen()->availableGeometry().bottom())
			{
				new_cursor_pos.setY(d->_this->screen()->availableGeometry().bottom());
			}
			QCursor::setPos(new_cursor_pos);
			return;
		}

		// move floating window
		if (DraggingFloatingWidget == d->DragState)
		{
			if (d->FloatingWidget->isMaximized())
			{
				d->FloatingWidget->showNormal(true);
			}
			d->FloatingWidget->moveFloating();
			Super::mouseMoveEvent(ev);
			return;
		}
		Super::mouseMoveEvent(ev);
	}


	//============================================================================
	void CFloatingWidgetTitleBar::enableCloseButton(bool Enable)
	{
		d->CloseButton->setEnabled(Enable);
	}


	//============================================================================
	void CFloatingWidgetTitleBar::setTitle(const QString& Text)
	{
		d->TitleLabel->setText(Text);
	}

	//============================================================================
	void CFloatingWidgetTitleBar::updateStyle()
	{
		internal::repolishStyle(this, internal::RepolishDirectChildren);
	}


	//============================================================================
	void CFloatingWidgetTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
	{
		if (event->buttons() & Qt::LeftButton)
		{
			emit maximizeRequested();
			event->accept();
		}
		else
		{
			QWidget::mouseDoubleClickEvent(event);
		}
	}


	//============================================================================
	void CFloatingWidgetTitleBar::setMaximizedIcon(bool maximized)
	{
		d->Maximized = maximized;
		if (!CDockManager::testConfigFlag(CDockManager::FloatingContainerForceQWidgetCustomStyledTitleBar)) {
			if (maximized)
			{
				d->MaximizeButton->setIcon(d->NormalIcon);
			}
			else
			{
				d->MaximizeButton->setIcon(d->MaximizeIcon);
			}
		}
	}


	//============================================================================
	void CFloatingWidgetTitleBar::setMaximizeIcon(const QIcon& Icon)
	{
		d->MaximizeIcon = Icon;
		if (d->Maximized)
		{
			setMaximizedIcon(d->Maximized);
		}
	}


	//============================================================================
	void CFloatingWidgetTitleBar::setNormalIcon(const QIcon& Icon)
	{
		d->NormalIcon = Icon;
		if (!d->Maximized)
		{
			setMaximizedIcon(d->Maximized);
		}
	}


	//============================================================================
	QIcon CFloatingWidgetTitleBar::maximizeIcon() const
	{
		return d->MaximizeIcon;
	}


	bool CFloatingWidgetTitleBar::maximized() const
	{
		return d->Maximized;
	}

	//============================================================================
	QIcon CFloatingWidgetTitleBar::normalIcon() const
	{
		return d->NormalIcon;
	}


} // namespace ads
