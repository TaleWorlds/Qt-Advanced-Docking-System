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
/// \file   MergedDockWidget.h
/// \author TaleWorlds
/// \date   14.05.2025
/// \brief  Declaration of CMergedDockWidget class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "MergedDockWidget.h"

#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidgetTab.h"
#include "DockSplitter.h"

#include <QSplitter>
#include <QXmlStreamWriter>

namespace ads
{
/**
 * Private data class of CMergedDockWidget class (pimpl)
 */
struct MergedDockWidgetPrivate
{
	QWidget* Widget1 = nullptr;
	QWidget* Widget2 = nullptr;
	CDockWidget* TempDockWidget1 = nullptr;
	CDockWidget* TempDockWidget2 = nullptr;
	CDockWidget::eInsertMode Widget1InsertMode;
	CDockWidget::eInsertMode Widget2InsertMode;
	CDockWidget::DockWidgetFeatures Widget1Features;
	CDockWidget::DockWidgetFeatures Widget2Features;
	QMap<CDockWidget::CustomButtonData*, CDockWidget::CustomButtonData*> CustomButtonMap;
	QSplitter* WidgetMain = nullptr;
	CDockWidget* OldDockWidget1 = nullptr;
	CDockWidget* OldDockWidget2 = nullptr;
	Qt::Orientation Orient = Qt::Vertical;
	bool FirstShow = true;
	CDockManager* DockManager = nullptr;
	CMergedDockWidget* q;

	MergedDockWidgetPrivate(CMergedDockWidget* this_);
};// struct MergedDockWidgetPrivate

//============================================================================
MergedDockWidgetPrivate::MergedDockWidgetPrivate(CMergedDockWidget* this_)
	: q(this_)
{
}

//============================================================================
CMergedDockWidget::CMergedDockWidget(CDockWidget* widget1, CDockWidget* widget2, Qt::Orientation orient, QWidget* parent)
	: CDockWidget(widget1->windowTitle() + " | " + widget2->windowTitle(), parent)
	, d(new MergedDockWidgetPrivate(this))
{
	d->Widget1Features = widget1->features();
	d->Widget2Features = widget2->features();

	auto customButtonsFirst = widget1->customButtons();
	auto customButtonsSecond = widget2->customButtons();
	QSet<QString> secondButtonObjectNames, firstButtonObjectNames;
	for (auto customButtonOne : customButtonsFirst)
	{
		firstButtonObjectNames.insert(customButtonOne->ObjectName);
	}
	for (auto customButtonTwo : customButtonsSecond)
	{
		auto tooltip = customButtonTwo->Tooltip;
		if (!customButtonTwo->IgnoreDuplicates && firstButtonObjectNames.constFind(customButtonTwo->ObjectName) != firstButtonObjectNames.constEnd())
		{
			tooltip += " (2)";
		}
		if (customButtonTwo->InitialState == (Qt::CheckState)-1)
		{
			addCustomButton(customButtonTwo->Icon, tooltip,
				customButtonTwo->ObjectName, customButtonTwo->IgnoreDuplicates, customButtonTwo->Alignment, customButtonTwo->OnClicked);
		}
		else
		{
			addCustomButton(customButtonTwo->Icon, customButtonTwo->InitialState == Qt::Checked,
				tooltip, customButtonTwo->ObjectName, customButtonTwo->IgnoreDuplicates, customButtonTwo->Alignment, customButtonTwo->OnClicked);
		}
		auto newCustomButton = customButtons().last();
		newCustomButton->CurrentState = customButtonTwo->CurrentState;
		secondButtonObjectNames.insert(customButtonTwo->ObjectName);
		d->CustomButtonMap.insert(newCustomButton, customButtonTwo);
	}
	for (auto customButtonOne : customButtonsFirst)
	{
		if (customButtonOne->IgnoreDuplicates && 
			secondButtonObjectNames.constFind(customButtonOne->ObjectName) == secondButtonObjectNames.constEnd() 
			|| !customButtonOne->IgnoreDuplicates)
		{
			auto tooltip = customButtonOne->Tooltip;
			if (!customButtonOne->IgnoreDuplicates && secondButtonObjectNames.constFind(customButtonOne->ObjectName) != secondButtonObjectNames.constEnd())
			{
				tooltip += " (1)";
			}
			if (customButtonOne->InitialState == (Qt::CheckState)-1)
			{
				addCustomButton(customButtonOne->Icon, tooltip,
					customButtonOne->ObjectName, customButtonOne->IgnoreDuplicates, customButtonOne->Alignment, customButtonOne->OnClicked);
			}
			else
			{
				addCustomButton(customButtonOne->Icon, customButtonOne->InitialState == Qt::Checked,
					tooltip, customButtonOne->ObjectName, customButtonOne->IgnoreDuplicates, customButtonOne->Alignment, customButtonOne->OnClicked);
			}
			auto newCustomButton = customButtons().last();
			newCustomButton->CurrentState = customButtonOne->CurrentState;
			d->CustomButtonMap.insert(newCustomButton, customButtonOne);
		}
	}

	d->Widget1InsertMode = widget1->widgetInsertMode();
	d->Widget2InsertMode = widget2->widgetInsertMode();
	d->OldDockWidget1 = widget1;
	d->OldDockWidget2 = widget2;
	d->DockManager = d->OldDockWidget1->dockManager();
	setObjectName(widget1->objectName() + "_" + widget2->objectName());
	d->Widget1 = widget1->takeWidget();
	d->Widget2 = widget2->takeWidget();
	d->Orient = orient;
	d->WidgetMain = new CDockSplitter(d->Orient, this);
	d->WidgetMain->setContentsMargins(0, 0, 0, 0);
	d->WidgetMain->setOpaqueResize(
		CDockManager::testConfigFlag(CDockManager::OpaqueSplitterResize));
	d->WidgetMain->setChildrenCollapsible(false);
	d->WidgetMain->setProperty("ads-splitter", QVariant(true));
	d->WidgetMain->addWidget(d->TempDockWidget1 = new CDockWidget("", d->WidgetMain));
	d->WidgetMain->addWidget(d->TempDockWidget2 = new CDockWidget("", d->WidgetMain));
	d->TempDockWidget1->setWidget(d->Widget1, d->Widget1InsertMode); 
	d->TempDockWidget2->setWidget(d->Widget2, d->Widget2InsertMode);

	QObject::connect(widget1, &QWidget::windowTitleChanged, this, &CMergedDockWidget::OnDockWidgetTitleChanged);
	QObject::connect(widget2, &QWidget::windowTitleChanged, this, &CMergedDockWidget::OnDockWidgetTitleChanged);

	DockWidgetFeatures mergedFeatures = CDockWidget::DefaultDockWidgetFeatures;

	// Disable default features if both of them don't have it
	if (!d->Widget1Features.testFlag(CDockWidget::DockWidgetClosable) && !d->Widget2Features.testFlag(CDockWidget::DockWidgetClosable))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetClosable, false);
	}
	if (!d->Widget1Features.testFlag(CDockWidget::DockWidgetMovable) && !d->Widget2Features.testFlag(CDockWidget::DockWidgetMovable))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetMovable, false);
	}
	if (!d->Widget1Features.testFlag(CDockWidget::DockWidgetFloatable) && !d->Widget2Features.testFlag(CDockWidget::DockWidgetFloatable))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetFloatable, false);
	}
	if (!d->Widget1Features.testFlag(CDockWidget::DockWidgetFocusable) && !d->Widget2Features.testFlag(CDockWidget::DockWidgetFocusable))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetFocusable, false);
	}
	if (!d->Widget1Features.testFlag(CDockWidget::DockWidgetPinnable) && !d->Widget2Features.testFlag(CDockWidget::DockWidgetPinnable))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetPinnable, false);
	}

	// if one of the widgets have them, do it
	if (d->Widget1Features.testFlag(CDockWidget::DockWidgetIndependent) || d->Widget2Features.testFlag(CDockWidget::DockWidgetIndependent))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetIndependent);
	}

	// this feature can't be merged into one:
	mergedFeatures.setFlag(CDockWidget::CustomCloseHandling, false);

	// these features need to be in both widgets to be enabled
	if (d->Widget1Features.testFlag(CDockWidget::DockWidgetDeleteOnClose) && d->Widget2Features.testFlag(CDockWidget::DockWidgetDeleteOnClose))
	{
		mergedFeatures.setFlag(CDockWidget::DockWidgetDeleteOnClose);
	}
	if (d->Widget1Features.testFlag(CDockWidget::NoTab) && d->Widget2Features.testFlag(CDockWidget::NoTab))
	{
		mergedFeatures.setFlag(CDockWidget::NoTab);
	}
	if (d->Widget1Features.testFlag(CDockWidget::DeleteContentOnClose) && d->Widget2Features.testFlag(CDockWidget::DeleteContentOnClose))
	{
		mergedFeatures.setFlag(CDockWidget::DeleteContentOnClose);
	}
	setFeatures(mergedFeatures);
	setWidget(d->WidgetMain, ForceNoScrollArea);
	setContentsMargins(0, 0, 0, 0);
}

//============================================================================
CMergedDockWidget::~CMergedDockWidget()
{
	delete d;
}

//============================================================================
void CMergedDockWidget::splitWidgets(CDockWidget*& outDockWidget1, CDockWidget*& outDockWidget2)
{
	auto splittersizes = d->WidgetMain->sizes();
	takeWidget();
	d->OldDockWidget1->setParent(nullptr);
	d->OldDockWidget2->setParent(nullptr);
	d->WidgetMain->hide();
	d->WidgetMain->deleteLater();
	d->OldDockWidget1->setWidget(d->Widget1, d->Widget1InsertMode);
	d->OldDockWidget2->setWidget(d->Widget2, d->Widget2InsertMode);
	auto mergedCustomButtons = customButtons();
	for (auto mergedCustomButton : mergedCustomButtons)
	{
		if (mergedCustomButton->InitialState != (Qt::CheckState)-1 && d->CustomButtonMap.contains(mergedCustomButton))
		{
			 d->CustomButtonMap[mergedCustomButton]->CurrentState = mergedCustomButton->CurrentState;
		}
	}
	d->CustomButtonMap.clear();
	d->OldDockWidget1->setClosedState(false);
	d->OldDockWidget2->setClosedState(false);
	if (!isAutoHide())
	{
		auto daw = dockAreaWidget();
		auto parentSplitter = daw->parentSplitter();
		QList<int> parentSizes;
		QList<int> newParentSplitSizes;
		int parentSplitIdx = -1;
		if (parentSplitter)
		{
			parentSplitIdx = parentSplitter->indexOf(daw);
			parentSizes = parentSplitter->sizes();
			newParentSplitSizes = parentSizes;
			newParentSplitSizes.erase(newParentSplitSizes.begin() + parentSplitIdx);
			newParentSplitSizes.insert(newParentSplitSizes.begin() + parentSplitIdx, splittersizes[1]);
			newParentSplitSizes.insert(newParentSplitSizes.begin() + parentSplitIdx, splittersizes[0]);
		}
		auto daw1 = dockManager()->addDockWidget(CenterDockWidgetArea, d->OldDockWidget1, daw);
		CDockAreaWidget* daw2;
		if (d->Orient == Qt::Vertical)
		{
			daw2 = dockManager()->addDockWidget(BottomDockWidgetArea, d->OldDockWidget2, daw);
		}
		else
		{
			daw2 = dockManager()->addDockWidget(RightDockWidgetArea, d->OldDockWidget2, daw);
		}
		auto parentSplit = daw2->parentSplitter();
		if (parentSplit->count() == 2)
		{
			parentSplit->setSizes(splittersizes);
		}
		else
		{
			parentSplit->setSizes(newParentSplitSizes);
		}
	}
	else
	{
		int autoHideOldSize = this->autoHideDockContainer()->getSize();
		auto ahw1 = dockManager()->addAutoHideDockWidget(this->autoHideLocation(), d->OldDockWidget1);
		auto ahw2 = dockManager()->addAutoHideDockWidget(this->autoHideLocation(), d->OldDockWidget2);
		ahw1->setSize(autoHideOldSize);
		ahw2->setSize(autoHideOldSize);
		ahw1->collapseView(false);
	}
	outDockWidget1 = d->OldDockWidget1;
	outDockWidget2 = d->OldDockWidget2;
	auto dockMgr = dockManager();
	dockMgr->removeDockWidget(this);
	hide();
	deleteLater();
	dockMgr->restartViewMenu();
}

//============================================================================
void CMergedDockWidget::takeWidgets(QWidget*& outWidget1, QWidget*& outWidget2)
{
	takeWidget();
	d->WidgetMain->hide();
	d->WidgetMain->deleteLater();
}

//============================================================================
void CMergedDockWidget::saveState(QXmlStreamWriter& Stream) const
{
	Stream.writeStartElement("MergedWidget");
	Stream.writeAttribute("Name", objectName());
	Stream.writeAttribute("Closed", QString::number(isClosed() ? 1 : 0));
	if (isAutoHide())
	{
		Stream.writeAttribute("Size", QString::number(autoHideDockContainer()->getSize()));
	}
	{
		Stream.writeStartElement("Splitter");
		Stream.writeAttribute("Orientation", (d->WidgetMain->orientation() == Qt::Horizontal) ? "|" : "-");
		Stream.writeAttribute("Count", "2");
		{
			Stream.writeStartElement("Widget");
			Stream.writeAttribute("Name", d->OldDockWidget1->objectName());
			Stream.writeEndElement();
		}
		{
			Stream.writeStartElement("Widget");
			Stream.writeAttribute("Name", d->OldDockWidget2->objectName());
			Stream.writeEndElement();
		}
		{
			Stream.writeStartElement("Sizes");
			auto sizesSplit = d->WidgetMain->sizes();
			Stream.writeCharacters(QString::number(sizesSplit[0]) + " " + QString::number(sizesSplit[1]) + "");
			Stream.writeEndElement();
		}
		Stream.writeEndElement();
	}
	Stream.writeEndElement();
}

//============================================================================
void CMergedDockWidget::showEvent(QShowEvent* event)
{
	if (d->FirstShow)
	{
		auto Splitter = d->WidgetMain;
		int AreaSize = (d->WidgetMain->orientation() == Qt::Horizontal) ?
			Splitter->width() :
			Splitter->height();
		auto SplitterSizes = Splitter->sizes();
		qreal TotRatio = SplitterSizes.size();
		for (int i = 0; i < SplitterSizes.size(); i++)
		{
			SplitterSizes[i] = AreaSize / TotRatio;
		}
		Splitter->setSizes(SplitterSizes);
		d->FirstShow = false;
	}
	CDockWidget::showEvent(event);
}

void CMergedDockWidget::OnDockWidgetTitleChanged(const QString&)
{
	setWindowTitle(d->OldDockWidget1->windowTitle() + " | " + d->OldDockWidget2->windowTitle());
}

}// namespace ads

//---------------------------------------------------------------------------
// EOF MergedDockWidget.cpp
