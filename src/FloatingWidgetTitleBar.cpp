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

#include "DockManager.h"
#include "ElidingLabel.h"
#include "FloatingDockContainer.h"
#include "IconProvider.h"
#include "ads_globals.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QWindow>
#include <iostream>

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

	FloatingWidgetTitleBarPrivate(CFloatingWidgetTitleBar* _public)
		: _this(_public)
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
	CloseButton->setObjectName("floatingTitleCloseButton");
	CloseButton->setAutoRaise(true);

	MaximizeButton = new tMaximizeButton();
	MaximizeButton->setObjectName("floatingTitleMaximizeButton");
	MaximizeButton->setAutoRaise(true);

	QIcon CloseIcon = FloatingWidget->dockManager()->iconProvider().customIcon(ads::eIcon::DockAreaCloseIcon);
	internal::setButtonIcon(CloseButton, QStyle::SP_TitleBarCloseButton, ads::eIcon::DockAreaCloseIcon);
	CloseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CloseButton->setVisible(true);
	CloseButton->setFocusPolicy(Qt::NoFocus);
	_this->connect(CloseButton, SIGNAL(clicked()), SIGNAL(closeRequested()));

	_this->setMaximizedIcon(false);
	MaximizeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	MaximizeButton->setVisible(true);
	MaximizeButton->setFocusPolicy(Qt::NoFocus);
	if (ads::CDockManager::testConfigFlag(CDockManager::UseProxyStyle))
	{
		MaximizeIcon = FloatingWidget->dockManager()->iconProvider().customIcon(ads::eIcon::MaximizeIcon);
		NormalIcon = FloatingWidget->dockManager()->iconProvider().customIcon(ads::eIcon::NormalIcon);
	}
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

	// So icons may be provided from stylesheets using
	// QToolButton#floatingTitleXButton{icon: ...}
	CloseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CloseButton->setVisible(true);
	CloseButton->setFocusPolicy(Qt::NoFocus);
	if (ads::CDockManager::testConfigFlag(CDockManager::UseProxyStyle))
	{
		MaximizeIcon = FloatingWidget->dockManager()->iconProvider().customIcon(ads::eIcon::MaximizeIcon);
		NormalIcon = FloatingWidget->dockManager()->iconProvider().customIcon(ads::eIcon::NormalIcon);
		internal::setButtonIcon(CloseButton, QStyle::SP_TitleBarCloseButton, ads::eIcon::DockAreaCloseIcon);
	}
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

//============================================================================
CFloatingWidgetTitleBar::CFloatingWidgetTitleBar(CFloatingDockContainer* parent)
	: QFrame(parent)
	, d(new FloatingWidgetTitleBarPrivate(this))
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
		auto normalPixmap =
			this->style()->standardPixmap(QStyle::StandardPixmap::SP_TitleBarNormalButton, 0, d->MaximizeButton);
		d->NormalIcon.addPixmap(normalPixmap, QIcon::Normal);
		d->NormalIcon.addPixmap(internal::createTransparentPixmap(normalPixmap, 0.25), QIcon::Disabled);

		auto maxPixmap = this->style()->standardPixmap(QStyle::SP_TitleBarMaxButton, 0, d->MaximizeButton);
		d->MaximizeIcon.addPixmap(maxPixmap, QIcon::Normal);
		d->MaximizeIcon.addPixmap(internal::createTransparentPixmap(maxPixmap, 0.25), QIcon::Disabled);
		setMaximizedIcon(d->Maximized);
	}
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
		d->FloatingWidget->startDragging(ev->pos(), d->FloatingWidget->size(), this);
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
	if (!CDockManager::testConfigFlag(CDockManager::FloatingContainerForceQWidgetCustomStyledTitleBar))
	{
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
