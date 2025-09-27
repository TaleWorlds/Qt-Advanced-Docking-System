#pragma once

#ifndef MergedDockWidgetH
#define MergedDockWidgetH
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
#include "DockWidget.h"

namespace ads
{
struct MergedDockWidgetPrivate;

/**
 * Merged dock widget that contains two dock widgets 
 * stacked on top of each other. Can be either vertical or horizontal
 */
class ADS_EXPORT CMergedDockWidget : public CDockWidget
{
	Q_OBJECT
private:
	MergedDockWidgetPrivate* d;  ///< private data (pimpl)
	friend struct MergedDockWidgetPrivate;
public:
	CMergedDockWidget(CDockWidget* widget1, CDockWidget* widget2, Qt::Orientation orient, QWidget* parent);
	~CMergedDockWidget();

	void splitWidgets(CDockWidget*& outDockWidget1, CDockWidget*& outDockWidget2);
	void takeWidgets(QWidget*& outWidget1, QWidget*& outWidget2);
	void saveState(QXmlStreamWriter& Stream) const override;
protected:
	void showEvent(QShowEvent* event) override;
protected Q_SLOTS:
	void OnDockWidgetTitleChanged(const QString&);
};  // class CMergedDockWidget
}  // namespace ads


//-----------------------------------------------------------------------------
#endif  // MergedDockWidgetH