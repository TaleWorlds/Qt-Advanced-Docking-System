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
 /// \file   FloatingDockContainer.cpp
 /// \author Uwe Kindler
 /// \date   01.03.2017
 /// \brief  Implementation of CFloatingDockContainer class
 //============================================================================

 //============================================================================
 //                                   INCLUDES
 //============================================================================
#include "FloatingDockContainer.h"

#include <iostream>

#include <QBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QPointer>
#include <QAction>
#include <QDebug>
#include <QAbstractButton>
#include <QElapsedTimer>
#include <QTime>
#include <QStatusBar>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QWindow>

#include "DockContainerWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"
#include "DockOverlay.h"

#ifdef Q_OS_WIN
#include <windows.h>
#ifdef _MSC_VER
#pragma comment(lib, "User32.lib")
#endif
#endif
#include "FloatingWidgetTitleBar.h"
#ifdef Q_OS_LINUX
#include <xcb/xcb.h>
#endif

namespace ads
{
#ifdef Q_OS_WIN
#if 0 // set to 1 if you need this function for debugging
	/**
	 * Just for debuging to convert windows message identifiers to strings
	 */
	static const char* windowsMessageString(int MessageId)
	{
		switch (MessageId)
		{
		case 0: return "WM_NULL";
		case 1: return "WM_CREATE";
		case 2: return "WM_DESTROY";
		case 3: return "WM_MOVE";
		case 5: return "WM_SIZE";
		case 6: return "WM_ACTIVATE";
		case 7: return "WM_SETFOCUS";
		case 8: return "WM_KILLFOCUS";
		case 10: return "WM_ENABLE";
		case 11: return "WM_SETREDRAW";
		case 12: return "WM_SETTEXT";
		case 13: return "WM_GETTEXT";
		case 14: return "WM_GETTEXTLENGTH";
		case 15: return "WM_PAINT";
		case 16: return "WM_CLOSE";
		case 17: return "WM_QUERYENDSESSION";
		case 18: return "WM_QUIT";
		case 19: return "WM_QUERYOPEN";
		case 20: return "WM_ERASEBKGND";
		case 21: return "WM_SYSCOLORCHANGE";
		case 22: return "WM_ENDSESSION";
		case 24: return "WM_SHOWWINDOW";
		case 25: return "WM_CTLCOLOR";
		case 26: return "WM_WININICHANGE";
		case 27: return "WM_DEVMODECHANGE";
		case 28: return "WM_ACTIVATEAPP";
		case 29: return "WM_FONTCHANGE";
		case 30: return "WM_TIMECHANGE";
		case 31: return "WM_CANCELMODE";
		case 32: return "WM_SETCURSOR";
		case 33: return "WM_MOUSEACTIVATE";
		case 34: return "WM_CHILDACTIVATE";
		case 35: return "WM_QUEUESYNC";
		case 36: return "WM_GETMINMAXINFO";
		case 38: return "WM_PAINTICON";
		case 39: return "WM_ICONERASEBKGND";
		case 40: return "WM_NEXTDLGCTL";
		case 42: return "WM_SPOOLERSTATUS";
		case 43: return "WM_DRAWITEM";
		case 44: return "WM_MEASUREITEM";
		case 45: return "WM_DELETEITEM";
		case 46: return "WM_VKEYTOITEM";
		case 47: return "WM_CHARTOITEM";
		case 48: return "WM_SETFONT";
		case 49: return "WM_GETFONT";
		case 50: return "WM_SETHOTKEY";
		case 51: return "WM_GETHOTKEY";
		case 55: return "WM_QUERYDRAGICON";
		case 57: return "WM_COMPAREITEM";
		case 61: return "WM_GETOBJECT";
		case 65: return "WM_COMPACTING";
		case 68: return "WM_COMMNOTIFY";
		case 70: return "WM_WINDOWPOSCHANGING";
		case 71: return "WM_WINDOWPOSCHANGED";
		case 72: return "WM_POWER";
		case 73: return "WM_COPYGLOBALDATA";
		case 74: return "WM_COPYDATA";
		case 75: return "WM_CANCELJOURNAL";
		case 78: return "WM_NOTIFY";
		case 80: return "WM_INPUTLANGCHANGEREQUEST";
		case 81: return "WM_INPUTLANGCHANGE";
		case 82: return "WM_TCARD";
		case 83: return "WM_HELP";
		case 84: return "WM_USERCHANGED";
		case 85: return "WM_NOTIFYFORMAT";
		case 123: return "WM_CONTEXTMENU";
		case 124: return "WM_STYLECHANGING";
		case 125: return "WM_STYLECHANGED";
		case 126: return "WM_DISPLAYCHANGE";
		case 127: return "WM_GETICON";
		case 128: return "WM_SETICON";
		case 129: return "WM_NCCREATE";
		case 130: return "WM_NCDESTROY";
		case 131: return "WM_NCCALCSIZE";
		case 132: return "WM_NCHITTEST";
		case 133: return "WM_NCPAINT";
		case 134: return "WM_NCACTIVATE";
		case 135: return "WM_GETDLGCODE";
		case 136: return "WM_SYNCPAINT";
		case 160: return "WM_NCMOUSEMOVE";
		case 161: return "WM_NCLBUTTONDOWN";
		case 162: return "WM_NCLBUTTONUP";
		case 163: return "WM_NCLBUTTONDBLCLK";
		case 164: return "WM_NCRBUTTONDOWN";
		case 165: return "WM_NCRBUTTONUP";
		case 166: return "WM_NCRBUTTONDBLCLK";
		case 167: return "WM_NCMBUTTONDOWN";
		case 168: return "WM_NCMBUTTONUP";
		case 169: return "WM_NCMBUTTONDBLCLK";
		case 171: return "WM_NCXBUTTONDOWN";
		case 172: return "WM_NCXBUTTONUP";
		case 173: return "WM_NCXBUTTONDBLCLK";
		case 176: return "EM_GETSEL";
		case 177: return "EM_SETSEL";
		case 178: return "EM_GETRECT";
		case 179: return "EM_SETRECT";
		case 180: return "EM_SETRECTNP";
		case 181: return "EM_SCROLL";
		case 182: return "EM_LINESCROLL";
		case 183: return "EM_SCROLLCARET";
		case 185: return "EM_GETMODIFY";
		case 187: return "EM_SETMODIFY";
		case 188: return "EM_GETLINECOUNT";
		case 189: return "EM_LINEINDEX";
		case 190: return "EM_SETHANDLE";
		case 191: return "EM_GETHANDLE";
		case 192: return "EM_GETTHUMB";
		case 193: return "EM_LINELENGTH";
		case 194: return "EM_REPLACESEL";
		case 195: return "EM_SETFONT";
		case 196: return "EM_GETLINE";
		case 197: return "EM_LIMITTEXT / EM_SETLIMITTEXT";
		case 198: return "EM_CANUNDO";
		case 199: return "EM_UNDO";
		case 200: return "EM_FMTLINES";
		case 201: return "EM_LINEFROMCHAR";
		case 202: return "EM_SETWORDBREAK";
		case 203: return "EM_SETTABSTOPS";
		case 204: return "EM_SETPASSWORDCHAR";
		case 205: return "EM_EMPTYUNDOBUFFER";
		case 206: return "EM_GETFIRSTVISIBLELINE";
		case 207: return "EM_SETREADONLY";
		case 209: return "EM_SETWORDBREAKPROC / EM_GETWORDBREAKPROC";
		case 210: return "EM_GETPASSWORDCHAR";
		case 211: return "EM_SETMARGINS";
		case 212: return "EM_GETMARGINS";
		case 213: return "EM_GETLIMITTEXT";
		case 214: return "EM_POSFROMCHAR";
		case 215: return "EM_CHARFROMPOS";
		case 216: return "EM_SETIMESTATUS";
		case 217: return "EM_GETIMESTATUS";
		case 224: return "SBM_SETPOS";
		case 225: return "SBM_GETPOS";
		case 226: return "SBM_SETRANGE";
		case 227: return "SBM_GETRANGE";
		case 228: return "SBM_ENABLE_ARROWS";
		case 230: return "SBM_SETRANGEREDRAW";
		case 233: return "SBM_SETSCROLLINFO";
		case 234: return "SBM_GETSCROLLINFO";
		case 235: return "SBM_GETSCROLLBARINFO";
		case 240: return "BM_GETCHECK";
		case 241: return "BM_SETCHECK";
		case 242: return "BM_GETSTATE";
		case 243: return "BM_SETSTATE";
		case 244: return "BM_SETSTYLE";
		case 245: return "BM_CLICK";
		case 246: return "BM_GETIMAGE";
		case 247: return "BM_SETIMAGE";
		case 248: return "BM_SETDONTCLICK";
		case 255: return "WM_INPUT";
		case 256: return "WM_KEYDOWN";
		case 257: return "WM_KEYUP";
		case 258: return "WM_CHAR";
		case 259: return "WM_DEADCHAR";
		case 260: return "WM_SYSKEYDOWN";
		case 261: return "WM_SYSKEYUP";
		case 262: return "WM_SYSCHAR";
		case 263: return "WM_SYSDEADCHAR";
		case 265: return "WM_UNICHAR / WM_WNT_CONVERTREQUESTEX";
		case 266: return "WM_CONVERTREQUEST";
		case 267: return "WM_CONVERTRESULT";
		case 268: return "WM_INTERIM";
		case 269: return "WM_IME_STARTCOMPOSITION";
		case 270: return "WM_IME_ENDCOMPOSITION";
		case 272: return "WM_INITDIALOG";
		case 273: return "WM_COMMAND";
		case 274: return "WM_SYSCOMMAND";
		case 275: return "WM_TIMER";
		case 276: return "WM_HSCROLL";
		case 277: return "WM_VSCROLL";
		case 278: return "WM_INITMENU";
		case 279: return "WM_INITMENUPOPUP";
		case 280: return "WM_SYSTIMER";
		case 287: return "WM_MENUSELECT";
		case 288: return "WM_MENUCHAR";
		case 289: return "WM_ENTERIDLE";
		case 290: return "WM_MENURBUTTONUP";
		case 291: return "WM_MENUDRAG";
		case 292: return "WM_MENUGETOBJECT";
		case 293: return "WM_UNINITMENUPOPUP";
		case 294: return "WM_MENUCOMMAND";
		case 295: return "WM_CHANGEUISTATE";
		case 296: return "WM_UPDATEUISTATE";
		case 297: return "WM_QUERYUISTATE";
		case 306: return "WM_CTLCOLORMSGBOX";
		case 307: return "WM_CTLCOLOREDIT";
		case 308: return "WM_CTLCOLORLISTBOX";
		case 309: return "WM_CTLCOLORBTN";
		case 310: return "WM_CTLCOLORDLG";
		case 311: return "WM_CTLCOLORSCROLLBAR";
		case 312: return "WM_CTLCOLORSTATIC";
		case 512: return "WM_MOUSEMOVE";
		case 513: return "WM_LBUTTONDOWN";
		case 514: return "WM_LBUTTONUP";
		case 515: return "WM_LBUTTONDBLCLK";
		case 516: return "WM_RBUTTONDOWN";
		case 517: return "WM_RBUTTONUP";
		case 518: return "WM_RBUTTONDBLCLK";
		case 519: return "WM_MBUTTONDOWN";
		case 520: return "WM_MBUTTONUP";
		case 521: return "WM_MBUTTONDBLCLK";
		case 522: return "WM_MOUSEWHEEL";
		case 523: return "WM_XBUTTONDOWN";
		case 524: return "WM_XBUTTONUP";
		case 525: return "WM_XBUTTONDBLCLK";
		case 528: return "WM_PARENTNOTIFY";
		case 529: return "WM_ENTERMENULOOP";
		case 530: return "WM_EXITMENULOOP";
		case 531: return "WM_NEXTMENU";
		case 532: return "WM_SIZING";
		case 533: return "WM_CAPTURECHANGED";
		case 534: return "WM_MOVING";
		case 536: return "WM_POWERBROADCAST";
		case 537: return "WM_DEVICECHANGE";
		case 544: return "WM_MDICREATE";
		case 545: return "WM_MDIDESTROY";
		case 546: return "WM_MDIACTIVATE";
		case 547: return "WM_MDIRESTORE";
		case 548: return "WM_MDINEXT";
		case 549: return "WM_MDIMAXIMIZE";
		case 550: return "WM_MDITILE";
		case 551: return "WM_MDICASCADE";
		case 552: return "WM_MDIICONARRANGE";
		case 553: return "WM_MDIGETACTIVE";
		case 560: return "WM_MDISETMENU";
		case 561: return "WM_ENTERSIZEMOVE";
		case 562: return "WM_EXITSIZEMOVE";
		case 563: return "WM_DROPFILES";
		case 564: return "WM_MDIREFRESHMENU";
		case 640: return "WM_IME_REPORT";
		case 641: return "WM_IME_SETCONTEXT";
		case 642: return "WM_IME_NOTIFY";
		case 643: return "WM_IME_CONTROL";
		case 644: return "WM_IME_COMPOSITIONFULL";
		case 645: return "WM_IME_SELECT";
		case 646: return "WM_IME_CHAR";
		case 648: return "WM_IME_REQUEST";
		case 656: return "WM_IME_KEYDOWN";
		case 657: return "WM_IME_KEYUP";
		case 672: return "WM_NCMOUSEHOVER";
		case 673: return "WM_MOUSEHOVER";
		case 674: return "WM_NCMOUSELEAVE";
		case 675: return "WM_MOUSELEAVE";
		case 768: return "WM_CUT";
		case 769: return "WM_COPY";
		case 770: return "WM_PASTE";
		case 771: return "WM_CLEAR";
		case 772: return "WM_UNDO";
		case 773: return "WM_RENDERFORMAT";
		case 774: return "WM_RENDERALLFORMATS";
		case 775: return "WM_DESTROYCLIPBOARD";
		case 776: return "WM_DRAWCLIPBOARD";
		case 777: return "WM_PAINTCLIPBOARD";
		case 778: return "WM_VSCROLLCLIPBOARD";
		case 779: return "WM_SIZECLIPBOARD";
		case 780: return "WM_ASKCBFORMATNAME";
		case 781: return "WM_CHANGECBCHAIN";
		case 782: return "WM_HSCROLLCLIPBOARD";
		case 783: return "WM_QUERYNEWPALETTE";
		case 784: return "WM_PALETTEISCHANGING";
		case 785: return "WM_PALETTECHANGED";
		case 786: return "WM_HOTKEY";
		case 791: return "WM_PRINT";
		case 792: return "WM_PRINTCLIENT";
		case 793: return "WM_APPCOMMAND";
		case 856: return "WM_HANDHELDFIRST";
		case 863: return "WM_HANDHELDLAST";
		case 864: return "WM_AFXFIRST";
		case 895: return "WM_AFXLAST";
		case 896: return "WM_PENWINFIRST";
		case 897: return "WM_RCRESULT";
		case 898: return "WM_HOOKRCRESULT";
		case 899: return "WM_GLOBALRCCHANGE / WM_PENMISCINFO";
		case 900: return "WM_SKB";
		case 901: return "WM_HEDITCTL / WM_PENCTL";
		case 902: return "WM_PENMISC";
		case 903: return "WM_CTLINIT";
		case 904: return "WM_PENEVENT";
		case 911: return "WM_PENWINLAST";
		default:
			return "unknown WM_ message";
		}

		return "unknown WM_ message";
	}
#endif
#endif


	static unsigned int zOrderCounter = 0;
	/**
	 * Private data class of CFloatingDockContainer class (pimpl)
	 */
	struct FloatingDockContainerPrivate
	{
		/// <summary>
		/// Cursor direction relative to the floating window
		/// </summary>
		enum class eDirection {
			UP = 0,
			DOWN = 1,
			LEFT,
			RIGHT,
			LEFTTOP,
			LEFTBOTTOM,
			RIGHTBOTTOM,
			RIGHTTOP,
			NONE
		};
		bool LeftMBPressed;
		eDirection CursorDirection;
		int ResizeRegionPadding;
		QPoint DragStartPosition;
		QMargins TransparentMargins;

		CFloatingDockContainer* _this;
		CDockContainerWidget* DockContainer;
		unsigned int zOrderIndex = ++zOrderCounter;
		QPointer<CDockManager> DockManager;
		eDragState DraggingState = DraggingInactive;
		QPoint DragStartMousePosition;
		CDockContainerWidget* DropContainer = nullptr;
		CDockAreaWidget* SingleDockArea = nullptr;
		QPoint DragStartPos;
		bool Hiding = false;
		bool AutoHideChildren = true;
		QWidget* MouseEventHandler = nullptr;
		CFloatingWidgetTitleBar* TitleBar = nullptr;
		bool IsResizing = false;
		QStatusBar* StatusBar = nullptr;

		/**
		 * Private data constructor
		 */
		FloatingDockContainerPrivate(CFloatingDockContainer* _public);

		void titleMouseReleaseEvent();
		void updateDropOverlays(const QPoint& GlobalPos);

		/**
		 * Returns true if the given config flag is set
		 */
		static bool testConfigFlag(CDockManager::eConfigFlag Flag)
		{
			return CDockManager::testConfigFlag(Flag);
		}

		/**
		 * Tests is a certain state is active
		 */
		bool isState(eDragState StateId) const
		{
			return StateId == DraggingState;
		}

		void setState(eDragState StateId)
		{
			DraggingState = StateId;
		}

		void setWindowTitle(const QString& Text)
		{
			if (TitleBar)
			{
				TitleBar->setTitle(Text);
			}
			_this->setWindowTitle(Text);
		}

		/**
		 * Reflect the current dock widget title in the floating widget windowTitle()
		 * depending on the CDockManager::FloatingContainerHasWidgetTitle flag
		 */
		void reflectCurrentWidget(CDockWidget* CurrentWidget)
		{
			// reflect CurrentWidget's title if configured to do so, otherwise display application name as window title
			if (testConfigFlag(CDockManager::FloatingContainerHasWidgetTitle))
			{
				setWindowTitle(CurrentWidget->windowTitle());
			}
			else
			{
				setWindowTitle(qApp->applicationDisplayName());
			}

			// reflect CurrentWidget's icon if configured to do so, otherwise display application icon as window icon
			QIcon CurrentWidgetIcon = CurrentWidget->icon();
			if (testConfigFlag(CDockManager::FloatingContainerHasWidgetIcon)
				&& !CurrentWidgetIcon.isNull())
			{
				_this->setWindowIcon(CurrentWidget->icon());
			}
			else
			{
				_this->setWindowIcon(QApplication::windowIcon());
			}
		}

		/**
		 * Handles escape key press when dragging around the floating widget
		 */
		void handleEscapeKey();
	};
	// struct FloatingDockContainerPrivate

	//============================================================================
	FloatingDockContainerPrivate::FloatingDockContainerPrivate(
		CFloatingDockContainer* _public) :
		_this(_public)
	{

	}

	//============================================================================
	void FloatingDockContainerPrivate::titleMouseReleaseEvent()
	{
		setState(DraggingInactive);
		if (DropContainer && (DockManager->dockAreaOverlay()->dropAreaUnderCursor()
			!= InvalidDockWidgetArea
			|| DockManager->containerOverlay()->dropAreaUnderCursor()
			!= InvalidDockWidgetArea))
		{
			CDockOverlay* Overlay = DockManager->containerOverlay();
			if (!Overlay->dropOverlayRect().isValid())
			{
				Overlay = DockManager->dockAreaOverlay();
			}

			// Resize the floating widget to the size of the highlighted drop area
			// rectangle
			QRect Rect = Overlay->dropOverlayRect();
			int FrameWidth = (_this->frameSize().width() - _this->rect().width())
				/ 2;
			int TitleBarHeight = _this->frameSize().height()
				- _this->rect().height() - FrameWidth;
			if (Rect.isValid())
			{
				QPoint TopLeft = Overlay->mapToGlobal(Rect.topLeft());
				TopLeft.ry() += TitleBarHeight;
				_this->setGeometry(
					QRect(TopLeft,
						QSize(Rect.width(), Rect.height() - TitleBarHeight)));
				QApplication::processEvents();
			}
			DropContainer->dropFloatingWidget(_this, QCursor::pos());
		}

		// Non-native tiling
		//else if (QCursor::pos().y() <= 0 
		//	&& QCursor::pos().x() < _this->screen()->availableGeometry().width() - 1
		//	&& QCursor::pos().x() > 0)
		//{
		//	// maximize window instead
		//	_this->showMaximized();
		//}
		//else if (QCursor::pos().y() > 0 
		//	&& QCursor::pos().y() < _this->screen()->availableGeometry().height() - 1
		//	&& QCursor::pos().x() <= 0)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height());
		//	_this->move(0, 0);
		//	_this->resize(s);
		//}
		//else if (QCursor::pos().y() > 0
		//	&& QCursor::pos().y() < _this->screen()->availableGeometry().height() - 1
		//	&& QCursor::pos().x() >= _this->screen()->availableGeometry().width()-1)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height());
		//	_this->move(_this->screen()->availableGeometry().width() / 2, 0);
		//	_this->resize(s);
		//}
		//else if (QCursor::pos().y() == 0 && QCursor::pos().x() == 0)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height() / 2);
		//	_this->move(0, 0);
		//	_this->resize(s);
		//}
		//else if (QCursor::pos().y() == 0 && QCursor::pos().x() >= _this->screen()->availableGeometry().width() - 1)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height() / 2);
		//	_this->move(_this->screen()->availableGeometry().width() / 2, 0);
		//	_this->resize(s); 
		//}
		//else if (QCursor::pos().x() == 0 && QCursor::pos().y() >= _this->screen()->availableGeometry().height() - 1)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height() / 2);
		//	_this->move(0, _this->screen()->availableGeometry().height() / 2);
		//	_this->resize(s);
		//}
		//else if (QCursor::pos().x() >= _this->screen()->availableGeometry().width() - 1 && QCursor::pos().y() >= _this->screen()->availableGeometry().height() - 1)
		//{
		//	QSize s(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height() / 2);
		//	_this->move(_this->screen()->availableGeometry().width() / 2, _this->screen()->availableGeometry().height() / 2);
		//	_this->resize(s);
		//}
		DockManager->containerOverlay()->hideOverlay();
		DockManager->dockAreaOverlay()->hideOverlay();
	}

	//============================================================================
	void FloatingDockContainerPrivate::updateDropOverlays(const QPoint& GlobalPos)
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

			if (DockContainer == ContainerWidget)
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
		auto ContainerOverlay = DockManager->containerOverlay();
		auto DockAreaOverlay = DockManager->dockAreaOverlay();

		if (!TopContainer)
		{
			ContainerOverlay->hideOverlay();
			DockAreaOverlay->hideOverlay();
			return;
		}

		// Do not show drop overlay if the independent window is being dragged and dropped window is minimized
		if (DropContainer &&
			(DropContainer->window()->isMinimized()
				|| (DockManager->window()->isMinimized()
					&& !DropContainer->topLevelDockWidget()->features().testFlag(CDockWidget::DockWidgetIndependent)
					))
			)
		{
			ContainerOverlay->hideOverlay();
			DockAreaOverlay->hideOverlay();
			return;
		}


		int VisibleDockAreas = TopContainer->visibleDockAreaCount();
		ContainerOverlay->setAllowedAreas(
			VisibleDockAreas > 1 ? OuterDockAreas : AllDockAreas);
		DockWidgetArea ContainerArea = ContainerOverlay->showOverlay(TopContainer);
		_this->activateWindow();
		ContainerOverlay->enableDropPreview(ContainerArea != InvalidDockWidgetArea);
		auto DockArea = TopContainer->dockAreaAt(GlobalPos);
		if (DockArea && DockArea->isVisible() && VisibleDockAreas > 0)
		{
			DockAreaOverlay->enableDropPreview(true);
			DockAreaOverlay->setAllowedAreas(
				(VisibleDockAreas == 1) ? NoDockWidgetArea : DockArea->allowedAreas());
			DockWidgetArea Area = DockAreaOverlay->showOverlay(DockArea);
			_this->activateWindow();
			// A CenterDockWidgetArea for the dockAreaOverlay() indicates that
			// the mouse is in the title bar. If the ContainerArea is valid
			// then we ignore the dock area of the dockAreaOverlay() and disable
			// the drop preview
			if ((Area == CenterDockWidgetArea)
				&& (ContainerArea != InvalidDockWidgetArea))
			{
				DockAreaOverlay->enableDropPreview(false);
				ContainerOverlay->enableDropPreview(true);
			}
			else
			{
				ContainerOverlay->enableDropPreview(InvalidDockWidgetArea == Area);
			}
		}
		else
		{
			DockAreaOverlay->hideOverlay();
		}
	}


	//============================================================================
	void FloatingDockContainerPrivate::handleEscapeKey()
	{
		ADS_PRINT("FloatingDockContainerPrivate::handleEscapeKey()");
		setState(DraggingInactive);
		DockManager->containerOverlay()->hideOverlay();
		DockManager->dockAreaOverlay()->hideOverlay();
	}

	//============================================================================
	CFloatingDockContainer::CFloatingDockContainer(CDockManager* DockManager, bool inheritance_flag) :
		tFloatingWidgetBase(DockManager),
		d(new FloatingDockContainerPrivate(this))
	{
		setMouseTracking(true);
		this->installEventFilter(this);
		d->LeftMBPressed = false;
		d->CursorDirection = FloatingDockContainerPrivate::eDirection::NONE;
		if (CDockManager::testConfigFlag(CDockManager::eConfigFlag::FloatingShadowEnabled))
		{
			d->ResizeRegionPadding = 10;
		}
		else
		{
			d->ResizeRegionPadding = 0;
		}
		hide();
		d->DockManager = DockManager;
		d->DockContainer = new CDockContainerWidget(DockManager, this);
		bool result = connect(d->DockContainer, SIGNAL(dockAreasAdded()), this,
			SLOT(onDockAreasAddedOrRemoved()));
		assert(result);
		result = connect(d->DockContainer, SIGNAL(dockAreasRemoved()), this,
			SLOT(onDockAreasAddedOrRemoved()));
		assert(result);
		if (!inheritance_flag)
		{
			decideNative();
		}
	}

	//============================================================================
	CFloatingDockContainer::CFloatingDockContainer(CDockAreaWidget* DockArea) :
		CFloatingDockContainer(DockArea->dockManager(), true)
	{
		decideNative(DockArea);
		d->DockContainer->addDockArea(DockArea);
		auto TopLevelDockWidget = topLevelDockWidget();
		if (TopLevelDockWidget)
		{
			TopLevelDockWidget->emitTopLevelChanged(true);
		}

		d->DockManager->notifyWidgetOrAreaRelocation(DockArea);
	}

	//============================================================================
	CFloatingDockContainer::CFloatingDockContainer(CDockWidget* DockWidget) :
		CFloatingDockContainer(DockWidget->dockManager(), true)
	{
		decideNative(DockWidget);
		d->DockContainer->addDockWidget(CenterDockWidgetArea, DockWidget);
		auto TopLevelDockWidget = topLevelDockWidget();
		if (TopLevelDockWidget)
		{
			TopLevelDockWidget->emitTopLevelChanged(true);
		}
		d->DockManager->notifyWidgetOrAreaRelocation(DockWidget);
	}

	CFloatingDockContainer::CFloatingDockContainer(CDockManager* DockManager, CDockContainerWidget* DockContainer) :
		tFloatingWidgetBase(DockManager),
		d(new FloatingDockContainerPrivate(this))
	{
		setMouseTracking(true);
		this->installEventFilter(this);
		d->LeftMBPressed = false;
		d->CursorDirection = FloatingDockContainerPrivate::eDirection::NONE;
		if (CDockManager::testConfigFlag(CDockManager::eConfigFlag::FloatingShadowEnabled))
		{
			d->ResizeRegionPadding = 10;
		}
		else
		{
			d->ResizeRegionPadding = 0;
		}
		hide();
		d->DockManager = DockManager;
		d->DockContainer = DockContainer;
		DockContainer->setParent(this);

		d->StatusBar = dynamic_cast<QStatusBar*>(DockContainer->findChild<QWidget*>("floatingWidgetStatusBar", Qt::FindChildrenRecursively));

		bool result = connect(d->DockContainer, SIGNAL(dockAreasAdded()), this,
			SLOT(onDockAreasAddedOrRemoved()));
		assert(result);
		result = connect(d->DockContainer, SIGNAL(dockAreasRemoved()), this,
			SLOT(onDockAreasAddedOrRemoved()));
		assert(result);
		decideNative();
		auto TopLevelDockWidget = topLevelDockWidget();
		if (TopLevelDockWidget)
		{
			TopLevelDockWidget->emitTopLevelChanged(true);
		}
		for (int i = 0; i < DockContainer->dockAreaCount(); i++)
		{
			auto DA = DockContainer->dockArea(i);
			if (DA)
			{
				d->DockManager->notifyWidgetOrAreaRelocation(DA);
			}
		}
	}

	//============================================================================
	CFloatingDockContainer::~CFloatingDockContainer()
	{
		ADS_PRINT("~CFloatingDockContainer");
		if (d->DockManager)
		{
			d->DockManager->removeFloatingWidget(this);
		}
		delete d;
	}

	//============================================================================
	CDockContainerWidget* CFloatingDockContainer::dockContainer() const
	{
		return d->DockContainer;
	}

	//============================================================================
	void CFloatingDockContainer::changeEvent(QEvent* event)
	{
		Super::changeEvent(event);
		if ((event->type() == QEvent::ActivationChange) && isActiveWindow())
		{
			ADS_PRINT("FloatingWidget::changeEvent QEvent::ActivationChange ");
			d->zOrderIndex = ++zOrderCounter;

			if (d->DraggingState == DraggingFloatingWidget)
			{
				d->titleMouseReleaseEvent();
				d->DraggingState = DraggingInactive;
			}
		}
	}


#ifdef Q_OS_WIN
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	bool CFloatingDockContainer::nativeEvent(const QByteArray& eventType, void* message, long* result)
#else
	bool CFloatingDockContainer::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#endif
	{
		QWidget::nativeEvent(eventType, message, result);
		MSG* msg = static_cast<MSG*>(message);
		switch (msg->message)
		{
		case WM_MOVING:
		{
			if (d->isState(DraggingFloatingWidget))
			{
				d->updateDropOverlays(QCursor::pos());
				QApplication::setActiveWindow(this);
				activateWindow();
			}
		}
		break;

		case WM_NCLBUTTONDOWN:
			if (msg->wParam == HTCAPTION && d->isState(DraggingInactive))
			{
				ADS_PRINT("CFloatingDockContainer::nativeEvent WM_NCLBUTTONDOWN");
				d->DragStartPos = pos();
				d->setState(DraggingMousePressed);
			}
			break;

		case WM_NCLBUTTONDBLCLK:
			d->setState(DraggingInactive);
			break;

		case WM_ENTERSIZEMOVE:
			if (d->isState(DraggingMousePressed))
			{
				ADS_PRINT("CFloatingDockContainer::nativeEvent WM_ENTERSIZEMOVE");
				d->setState(DraggingFloatingWidget);
				d->updateDropOverlays(QCursor::pos());
				QApplication::setActiveWindow(this);
				activateWindow();
			}
			break;

		case WM_EXITSIZEMOVE:
			if (d->isState(DraggingFloatingWidget))
			{
				ADS_PRINT("CFloatingDockContainer::nativeEvent WM_EXITSIZEMOVE");
				if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				{
					d->handleEscapeKey();
				}
				else
				{
					d->titleMouseReleaseEvent();
				}
			}
			break;
		}
		return false;
	}
#endif


	//============================================================================
	void CFloatingDockContainer::closeEvent(QCloseEvent* event)
	{
		ADS_PRINT("CFloatingDockContainer closeEvent");
		d->setState(DraggingInactive);
		event->ignore();
		if (!isClosable())
		{
			return;
		}

		bool HasOpenDockWidgets = false;
		for (auto DockWidget : d->DockContainer->openedDockWidgets())
		{
			if (DockWidget->features().testFlag(CDockWidget::DockWidgetDeleteOnClose) || DockWidget->features().testFlag(CDockWidget::CustomCloseHandling))
			{
				bool Closed = DockWidget->closeDockWidgetInternal();
				if (!Closed)
				{
					HasOpenDockWidgets = true;
				}
			}
			else
			{
				DockWidget->toggleView(false);
			}
		}

		if (HasOpenDockWidgets)
		{
			return;
		}

		// In Qt version after 5.9.2 there seems to be a bug that causes the
		// QWidget::event() function to not receive any NonClientArea mouse
		// events anymore after a close/show cycle. The bug is reported here:
		// https://bugreports.qt.io/browse/QTBUG-73295
		// The following code is a workaround for Qt versions > 5.9.2 that seems
		// to work
		// Starting from Qt version 5.12.2 this seems to work again. But
		// now the QEvent::NonClientAreaMouseButtonPress function returns always
		// Qt::RightButton even if the left button was pressed
		this->hide();
	}

	//============================================================================
	void CFloatingDockContainer::hideEvent(QHideEvent* event)
	{
		Super::hideEvent(event);
		if (event->spontaneous())
		{
			return;
		}

		// Prevent toogleView() events during restore state
		if (d->DockManager->isRestoringState())
		{
			return;
		}

		if (d->AutoHideChildren)
		{
			d->Hiding = true;
			for (auto DockArea : d->DockContainer->openedDockAreas())
			{
				for (auto DockWidget : DockArea->openedDockWidgets())
				{
					DockWidget->toggleView(false);
				}
			}
			d->Hiding = false;
		}
	}


	//============================================================================
	void CFloatingDockContainer::showEvent(QShowEvent* event)
	{
		Super::showEvent(event);
		if (CDockManager::testConfigFlag(CDockManager::FocusHighlighting))
		{
			this->window()->activateWindow();
		}
	}


	//============================================================================
	void CFloatingDockContainer::startFloating(const QPoint& DragStartMousePos,
		const QSize& Size, eDragState DragState, QWidget* MouseEventHandler)
	{
		if (!isMaximized())
		{
			resize(Size);
			d->DragStartMousePosition = DragStartMousePos;
		}
		d->setState(DragState);
		if (DraggingFloatingWidget == DragState)
		{
			d->MouseEventHandler = MouseEventHandler;
			if (d->MouseEventHandler)
			{
				d->MouseEventHandler->grabMouse();
			}
		}

		if (!isMaximized())
		{
			moveFloating();
		}
		show();
	}

	//============================================================================
	void CFloatingDockContainer::moveFloating()
	{
		int BorderSize = (frameSize().width() - size().width()) / 2;
		const QPoint moveToPos = QCursor::pos() - d->DragStartMousePosition
			- QPoint(BorderSize, 0);

		move(moveToPos);
		switch (d->DraggingState)
		{
		case DraggingMousePressed:
			d->setState(DraggingFloatingWidget);
			d->updateDropOverlays(QCursor::pos());
			break;

		case DraggingFloatingWidget:
			d->updateDropOverlays(QCursor::pos());
			break;
		default:
			break;
		}
		bool p = this != QApplication::activeWindow();
		QApplication::setActiveWindow(this);
		activateWindow();
		bool p_ = this != QApplication::activeWindow();
		printf("");
	}

	//============================================================================
	bool CFloatingDockContainer::isClosable() const
	{
		return d->DockContainer->features().testFlag(
			CDockWidget::DockWidgetClosable);
	}

	//============================================================================
	void CFloatingDockContainer::onDockAreasAddedOrRemoved()
	{
		ADS_PRINT("CFloatingDockContainer::onDockAreasAddedOrRemoved()");
		auto TopLevelDockArea = d->DockContainer->topLevelDockArea();
		auto TopLevelDockWidget = d->DockContainer->topLevelDockWidget();

		if (TopLevelDockArea || d->DockContainer->dockAreaCount() == 1)
		{
			d->SingleDockArea = TopLevelDockArea;
			if (d->SingleDockArea == nullptr && d->DockContainer->dockAreaCount() == 1)
			{
				d->SingleDockArea = d->DockContainer->dockArea(0);
			}
			CDockWidget* CurrentWidget = d->SingleDockArea->currentDockWidget();
			d->reflectCurrentWidget(CurrentWidget);
			connect(d->SingleDockArea, SIGNAL(currentChanged(int)), this,
				SLOT(onDockAreaCurrentChanged(int)));
		}
		else
		{
			if (d->SingleDockArea)
			{
				disconnect(d->SingleDockArea, SIGNAL(currentChanged(int)), this,
					SLOT(onDockAreaCurrentChanged(int)));
				d->SingleDockArea = nullptr;
			}
			if (d->DockContainer->dockAreaCount() > 0)
			{
				d->setWindowTitle(qApp->applicationDisplayName());
				setWindowIcon(QApplication::windowIcon());
			}
		}
		if (!d->DockManager->isRestoringState())
		{
			bool to_nonnative = !d->DockContainer->hasIndependentWidget() && hasNativeTitleBar();
			bool to_native = d->DockContainer->hasIndependentWidget() && !hasNativeTitleBar();
			// If there are no independent widgets & window is native, copy everything into an updated FDC
			if (to_native != to_nonnative && to_nonnative)
			{
				QByteArray b = saveGeometry();
				std::unordered_map<CDockWidget*, bool> open_map;
				for (int j = 0; d->DockContainer && j < d->DockContainer->dockAreaCount(); j++)
				{
					auto DA = d->DockContainer->dockArea(j);
					for (int i = 0; DA && i < DA->dockWidgetsCount(); i++)
					{
						auto DW = DA->dockWidget(i);
						if (DW)
						{
							open_map[DW] = DW->toggleViewAction()->isChecked();
						}
					}
				}
				QWidget* w = new QWidget(this);
				setWidget(w);
				auto Container = dockContainer();
				Container->setParent(nullptr);
				CFloatingDockContainer* RestoredFloatingWidget = new CFloatingDockContainer(d->DockManager, Container);
				RestoredFloatingWidget->restoreGeometry(b);
				for (int j = 0; j < Container->dockAreaCount(); j++)
				{
					auto DA = Container->dockArea(j);
					for (int i = 0; i < DA->dockWidgetsCount(); i++)
					{
						DA->updateTitleBarVisibility();
						auto DW = DA->dockWidget(i);
						if (DW && open_map[DW])
						{
							DW->setVisible(true);
							DW->toggleView(true);
						}
					}
				}
				RestoredFloatingWidget->restoreGeometry(b);
				RestoredFloatingWidget->show();
				hide();
				delete this;
			}
		}
	}

	//============================================================================
	void CFloatingDockContainer::updateWindowTitle()
	{
		// If this floating container will be hidden, then updating the window
		// tile is not required anymore
		if (d->Hiding)
		{
			return;
		}


		auto TopLevelDockArea = d->DockContainer->topLevelDockArea();
		if (TopLevelDockArea)
		{
			CDockWidget* CurrentWidget = TopLevelDockArea->currentDockWidget();
			if (CurrentWidget)
			{
				d->reflectCurrentWidget(CurrentWidget);
			}
		}
		else
		{
			d->setWindowTitle(qApp->applicationDisplayName());
			setWindowIcon(QApplication::windowIcon());
		}
	}

	//============================================================================
	void CFloatingDockContainer::decideNative(QWidget* w /*= nullptr*/)
	{
		bool native_window = true;

		// FloatingContainerForce*TitleBar is overwritten by the "ADS_UseNativeTitle" environment variable if set.
		auto env = qgetenv("ADS_UseNativeTitle").toUpper();
		if (env == "1")
		{
			native_window = true;
		}
		else if (env == "0")
		{
			native_window = false;
		}
		else if (d->DockManager->testConfigFlag(CDockManager::FloatingContainerForceNativeTitleBar))
		{
			native_window = true;
		}
		else if (d->DockManager->testConfigFlag(CDockManager::FloatingContainerForceQWidgetTitleBar)
			|| d->DockManager->testConfigFlag(CDockManager::FloatingContainerForceQWidgetCustomStyledTitleBar))
		{
			native_window = false;
		}
		else
		{
#ifdef Q_OS_LINUX
			// KDE doesn't seem to fire MoveEvents while moving windows, so for now no native title bar for everything using KWin.
			QString window_manager = internal::windowManager().toUpper().split(" ")[0];
			native_window = window_manager != "KWIN";
#endif
		}

		auto DA = dynamic_cast<CDockAreaWidget*>(w);
		auto DW = dynamic_cast<CDockWidget*>(w);
		bool has_independent_widget = false;
		if (DA)
		{
			for (int i = 0; DA && i < DA->dockWidgetsCount(); i++)
			{
				auto DW_ = DA->dockWidget(i);
				if (DW_ && DW_->features().testFlag(CDockWidget::DockWidgetIndependent))
				{
					has_independent_widget = true;
					break;
				}
			}
		}
		else if (DW)
		{
			has_independent_widget = (DW && DW->features().testFlag(CDockWidget::DockWidgetIndependent));
		}
		else if (!w)
		{
			has_independent_widget = d->DockContainer->hasIndependentWidget();
		}
		else
		{
			Q_ASSERT(false);
		}

		// Independent widget to be put on native window
		if (has_independent_widget || native_window)
		{
			setWindowFlags(Qt::Window);
			tFloatingWidgetBase::setWidget(d->DockContainer);
			tFloatingWidgetBase::setFeatures(tFloatingWidgetBase::DockWidgetClosable);
			if (graphicsEffect())
			{
				graphicsEffect()->deleteLater();
				setGraphicsEffect(nullptr);
			}
			if (d->TitleBar)
			{
				setTitleBarWidget(new QWidget());
				d->TitleBar->deleteLater();
				d->TitleBar = nullptr;
				QObject::disconnect(d->TitleBar, SIGNAL(closeRequested()), this, SLOT(close()));
				QObject::disconnect(d->TitleBar, &CFloatingWidgetTitleBar::maximizeRequested,
					this, &CFloatingDockContainer::onMaximizeRequest);
			}
			if (d->StatusBar)
			{
				d->DockContainer->layout()->removeWidget(d->StatusBar);
				d->StatusBar->deleteLater();
				d->StatusBar = nullptr;
			}
		}
		// No independent widgets, put it on non-native window
		else if (!has_independent_widget && !native_window)
		{
			tFloatingWidgetBase::setWidget(d->DockContainer);
			tFloatingWidgetBase::setFloating(true);
			tFloatingWidgetBase::setFeatures(tFloatingWidgetBase::DockWidgetClosable
				| tFloatingWidgetBase::DockWidgetMovable | tFloatingWidgetBase::DockWidgetFloatable);
			setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint);
			if (CDockManager::testConfigFlag(CDockManager::eConfigFlag::FloatingShadowEnabled))
			{
				if (d->StatusBar)
				{
					d->DockContainer->layout()->removeWidget(d->StatusBar);
					d->StatusBar->deleteLater();
					d->StatusBar = nullptr;
				}
				d->TransparentMargins = QMargins(5, 5, 5, 5);
				auto shadow_ = new QGraphicsDropShadowEffect(this);
				setAttribute(Qt::WA_TranslucentBackground);
				shadow_->setObjectName("floatingDockContainerShadow");
				shadow_->setBlurRadius(d->ResizeRegionPadding);
				shadow_->setOffset(0);
				shadow_->setColor({ 0, 0, 0, 255 }); // black shadow
				shadow_->setEnabled(true);
				setGraphicsEffect(shadow_);
				setAutoFillBackground(true);
			}
			else
			{
				if (graphicsEffect())
				{
					graphicsEffect()->deleteLater();
					setGraphicsEffect(nullptr);
				}
				if (!d->StatusBar)
				{
					d->StatusBar = new QStatusBar(this);
					d->StatusBar->setObjectName("floatingWidgetStatusBar");
					d->StatusBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
					d->StatusBar->setContentsMargins(0, 0, 0, 0);
					d->StatusBar->setFixedHeight(10);
					d->DockContainer->layout()->addWidget(d->StatusBar);
				}
			}
			if (!d->TitleBar)
			{
				d->TitleBar = new CFloatingWidgetTitleBar(this);
				setTitleBarWidget(d->TitleBar);

				d->TitleBar->enableCloseButton(isClosable());
				connect(d->TitleBar, SIGNAL(closeRequested()), SLOT(close()));
				connect(d->TitleBar, &CFloatingWidgetTitleBar::maximizeRequested,
					this, &CFloatingDockContainer::onMaximizeRequest);
			}
		}
		d->DockManager->registerFloatingWidget(this);
		updateWindowTitle();
	}

	//============================================================================
	void CFloatingDockContainer::onDockAreaCurrentChanged(int Index)
	{
		Q_UNUSED(Index);
		CDockWidget* CurrentWidget = d->SingleDockArea->currentDockWidget();
		d->reflectCurrentWidget(CurrentWidget);
	}

	//============================================================================
	bool CFloatingDockContainer::restoreState(CDockingStateReader& Stream,
		bool Testing)
	{
		if (!d->DockContainer->restoreState(Stream, Testing))
		{
			return false;
		}
		onDockAreasAddedOrRemoved();
		if (d->TitleBar)
		{
			d->TitleBar->setMaximizedIcon(windowState() == Qt::WindowMaximized);
		}
		return true;
	}


	//============================================================================
	bool CFloatingDockContainer::hasTopLevelDockWidget() const
	{
		return d->DockContainer->hasTopLevelDockWidget();
	}

	//============================================================================
	CDockWidget* CFloatingDockContainer::topLevelDockWidget() const
	{
		return d->DockContainer->topLevelDockWidget();
	}

	//============================================================================
	QList<CDockWidget*> CFloatingDockContainer::dockWidgets() const
	{
		return d->DockContainer->dockWidgets();
	}

	//============================================================================
	void CFloatingDockContainer::hideAndDeleteLater()
	{
		// Widget has been redocked, so it must be hidden right way (see 
		// https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/351)
		// but AutoHideChildren must be set to false because "this" still contains
		// dock widgets that shall not be toggled hidden.
		d->AutoHideChildren = false;
		hide();
		deleteLater();
	}

	//============================================================================
	void CFloatingDockContainer::finishDragging()
	{
		ADS_PRINT("CFloatingDockContainer::finishDragging");
		setWindowOpacity(1);
		activateWindow();

		if (d->MouseEventHandler)
		{
			d->MouseEventHandler->releaseMouse();
			d->MouseEventHandler = nullptr;
		}
		d->titleMouseReleaseEvent();
	}

#ifdef Q_OS_MACOS
	//============================================================================
	bool CFloatingDockContainer::event(QEvent* e)
	{
		switch (d->DraggingState)
		{
		case DraggingInactive:
		{
			// Normally we would check here, if the left mouse button is pressed.
			// But from QT version 5.12.2 on the mouse events from
			// QEvent::NonClientAreaMouseButtonPress return the wrong mouse button
			// The event always returns Qt::RightButton even if the left button
			// is clicked.
			// It is really great to work around the whole NonClientMouseArea
			// bugs
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 2))
			if (e->type() == QEvent::NonClientAreaMouseButtonPress /*&& QGuiApplication::mouseButtons().testFlag(Qt::LeftButton)*/)
#else
			if (e->type() == QEvent::NonClientAreaMouseButtonPress && QGuiApplication::mouseButtons().testFlag(Qt::LeftButton))
#endif
			{
				ADS_PRINT("FloatingWidget::event Event::NonClientAreaMouseButtonPress" << e->type());
				d->DragStartPos = pos();
				d->setState(DraggingMousePressed);
			}
		}
		break;

		case DraggingMousePressed:
			switch (e->type())
			{
			case QEvent::NonClientAreaMouseButtonDblClick:
				ADS_PRINT("FloatingWidget::event QEvent::NonClientAreaMouseButtonDblClick");
				d->setState(DraggingInactive);
				break;

			case QEvent::Resize:
				// If the first event after the mouse press is a resize event, then
				// the user resizes the window instead of dragging it around.
				// But there is one exception. If the window is maximized,
				// then dragging the window via title bar will cause the widget to
				// leave the maximized state. This in turn will trigger a resize event.
				// To know, if the resize event was triggered by user via moving a
				// corner of the window frame or if it was caused by a windows state
				// change, we check, if we are not in maximized state.
				if (!isMaximized())
				{
					d->setState(DraggingInactive);
				}
				break;

			default:
				break;
			}
			break;

		case DraggingFloatingWidget:
			if (e->type() == QEvent::NonClientAreaMouseButtonRelease)
			{
				ADS_PRINT("FloatingWidget::event QEvent::NonClientAreaMouseButtonRelease");
				d->titleMouseReleaseEvent();
			}
			break;

		default:
			break;
		}

#if (ADS_DEBUG_LEVEL > 0)
		qDebug() << QTime::currentTime() << "CFloatingDockContainer::event " << e->type();
#endif
		return QWidget::event(e);
	}


	//============================================================================
	void CFloatingDockContainer::moveEvent(QMoveEvent* event)
	{
		QWidget::moveEvent(event);
		switch (d->DraggingState)
		{
		case DraggingMousePressed:
			d->setState(DraggingFloatingWidget);
			d->updateDropOverlays(QCursor::pos());
			break;

		case DraggingFloatingWidget:
			d->updateDropOverlays(QCursor::pos());
			break;
		default:
			break;
		}
		QApplication::setActiveWindow(this);
		activateWindow();

	}
#endif


	//============================================================================
	void CFloatingDockContainer::onMaximizeRequest()
	{
		if (windowState() == Qt::WindowMaximized)
		{
			showNormal();
		}
		else
		{
			showMaximized();
		}
	}


	//============================================================================
	void CFloatingDockContainer::showNormal(bool fixGeometry)
	{
		if (windowState() == Qt::WindowMaximized)
		{
			QRect oldNormal = normalGeometry();
			Super::showNormal();
			if (fixGeometry)
			{
				setGeometry(oldNormal);
			}
		}
		if (d->TitleBar)
		{
			d->TitleBar->setMaximizedIcon(false);
		}
	}


	//============================================================================
	void CFloatingDockContainer::showMaximized()
	{
		Super::showMaximized();
		if (d->TitleBar)
		{
			d->TitleBar->setMaximizedIcon(true);
		}
	}


	//============================================================================
	bool CFloatingDockContainer::isMaximized() const
	{
		return windowState() == Qt::WindowMaximized;
	}


	//============================================================================
	void CFloatingDockContainer::show()
	{
		// Prevent this window from showing in the taskbar and pager (alt+tab)
#ifdef Q_OS_LINUX
		internal::xcb_add_prop(true, winId(), "_NET_WM_STATE", "_NET_WM_STATE_SKIP_TASKBAR");
		internal::xcb_add_prop(true, winId(), "_NET_WM_STATE", "_NET_WM_STATE_SKIP_PAGER");
#endif
		Super::show();
	}


	//============================================================================
	void CFloatingDockContainer::resizeEvent(QResizeEvent* event)
	{
		d->IsResizing = true;
		Super::resizeEvent(event);
	}


	static bool s_mousePressed = false;
	//============================================================================
	void CFloatingDockContainer::moveEvent(QMoveEvent* event)
	{
		Super::moveEvent(event);
		if (!d->IsResizing && event->spontaneous() && s_mousePressed)
		{
			d->DraggingState = DraggingFloatingWidget;
			d->updateDropOverlays(QCursor::pos());
			QApplication::setActiveWindow(this);
			activateWindow();
		}
		d->IsResizing = false;
	}

	//============================================================================
	bool CFloatingDockContainer::event(QEvent* e)
	{
		bool result = Super::event(e);
		switch (e->type())
		{
		case QEvent::WindowActivate:
			s_mousePressed = false;
			break;
		case QEvent::WindowDeactivate:
			s_mousePressed = true;
			break;
		default:
			break;
		}
		return result;
	}

	//============================================================================
	void CFloatingDockContainer::region(const QPoint& cursorGlobalPoint)
	{
		QRect rect = this->contentsRect();
		rect.setLeft(rect.left() + d->TransparentMargins.left());
		rect.setTop(rect.top() + d->TransparentMargins.top());
		rect.setRight(rect.right() - d->TransparentMargins.right());
		rect.setBottom(rect.bottom() - d->TransparentMargins.bottom());

		QPoint tl = mapToGlobal(rect.topLeft());
		QPoint rb = mapToGlobal(rect.bottomRight());
		int x = cursorGlobalPoint.x();
		int y = cursorGlobalPoint.y();

		if (tl.x() - d->ResizeRegionPadding >= x && tl.x() <= x &&
			tl.y() - d->ResizeRegionPadding >= y && tl.y() <= y) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::LEFTTOP;
			this->setCursor(QCursor(Qt::SizeFDiagCursor));
		}
		else if (x >= rb.x() + d->ResizeRegionPadding && x <= rb.x() &&
			y >= rb.y() + d->ResizeRegionPadding && y <= rb.y()) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::RIGHTBOTTOM;
			this->setCursor(QCursor(Qt::SizeFDiagCursor));
		}
		else if (x <= tl.x() - d->ResizeRegionPadding && x >= tl.x() &&
			y >= rb.y() + d->ResizeRegionPadding && y <= rb.y()) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::LEFTBOTTOM;
			this->setCursor(QCursor(Qt::SizeBDiagCursor));
		}
		else if (x <= rb.x() && x >= rb.x() + d->ResizeRegionPadding && y >= tl.y() &&
			y <= tl.y() - d->ResizeRegionPadding) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::RIGHTTOP;
			this->setCursor(QCursor(Qt::SizeBDiagCursor));
		}
		else if (x <= tl.x() - d->ResizeRegionPadding && x >= tl.x()) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::LEFT;
			this->setCursor(QCursor(Qt::SizeHorCursor));
		}
		else if (x <= rb.x() && x >= rb.x() + d->ResizeRegionPadding) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::RIGHT;
			this->setCursor(QCursor(Qt::SizeHorCursor));
		}
		else if (y >= tl.y() && y <= tl.y() - d->ResizeRegionPadding) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::UP;
			this->setCursor(QCursor(Qt::SizeVerCursor));
		}
		else if (y <= rb.y() && y >= rb.y() + d->ResizeRegionPadding) {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::DOWN;
			this->setCursor(QCursor(Qt::SizeVerCursor));
		}
		else {
			d->CursorDirection = FloatingDockContainerPrivate::eDirection::NONE;
			this->setCursor(QCursor(Qt::ArrowCursor));
		}
	}

	//============================================================================
	void CFloatingDockContainer::mousePressEvent(QMouseEvent* event)
	{
		if (d->DockManager->testConfigFlag(CDockManager::FloatingShadowEnabled))
		{
			if (event->button() == Qt::LeftButton) {
				if (d->CursorDirection != FloatingDockContainerPrivate::eDirection::NONE) {
					d->LeftMBPressed = true;
					//this->grabMouse();
				}
				else {
					QWidget* action = QApplication::widgetAt(internal::globalPositionOf(event));
					if (action) {
						bool inTitlebar = false;
						if (action == d->TitleBar) {
							d->LeftMBPressed = true;
							d->DragStartPosition = mapToParent(mapFromGlobal(internal::globalPositionOf(event))) - frameGeometry().topLeft()
								- QPoint(d->ResizeRegionPadding, d->ResizeRegionPadding);
						}
					}
				}
			}
		}
		return Super::mousePressEvent(event);
	}

	//============================================================================
	void CFloatingDockContainer::mouseReleaseEvent(QMouseEvent* event)
	{
		if (d->DockManager->testConfigFlag(CDockManager::FloatingShadowEnabled))
		{
			d->LeftMBPressed = false;
			if (d->CursorDirection != FloatingDockContainerPrivate::eDirection::NONE)
			{
				d->CursorDirection = FloatingDockContainerPrivate::eDirection::NONE;
				//this->releaseMouse();
				this->setCursor(QCursor(Qt::ArrowCursor));
			}
		}
		return Super::mouseReleaseEvent(event);
	}

	//============================================================================
	void CFloatingDockContainer::mouseMoveEvent(QMouseEvent* event)
	{
		if (d->DockManager->testConfigFlag(CDockManager::FloatingShadowEnabled))
		{
			QPoint globalPoint = internal::globalPositionOf(event);
			if (d->LeftMBPressed) {
				bool bIgnore = true;
				QList<QScreen*> screens = QGuiApplication::screens();
				for (int i = 0; i < screens.size(); i++) {
					QScreen* pScreen = screens[i];
					QRect geometryRect = pScreen->availableGeometry();
					if (geometryRect.contains(globalPoint)) {
						bIgnore = false;
						break;
					}
				}

				if (bIgnore) {
					event->ignore();
					return;
				}

				if (d->CursorDirection != FloatingDockContainerPrivate::eDirection::NONE) {
					QRect rect = this->rect();
					QPoint tl = mapToGlobal(rect.topLeft());
					QPoint rb = mapToGlobal(rect.bottomRight());

					QRect rMove(tl, rb);

					switch (d->CursorDirection) {
					case FloatingDockContainerPrivate::eDirection::LEFT:
						if (rb.x() - globalPoint.x() <= this->minimumWidth())
							rMove.setX(tl.x());
						else
							rMove.setX(globalPoint.x());
						break;
					case FloatingDockContainerPrivate::eDirection::RIGHT:
						rMove.setWidth(globalPoint.x() - tl.x());
						break;
					case FloatingDockContainerPrivate::eDirection::UP:
						if (rb.y() - globalPoint.y() <= this->minimumHeight())
							rMove.setY(tl.y());
						else
							rMove.setY(globalPoint.y());
						break;
					case FloatingDockContainerPrivate::eDirection::DOWN:
						rMove.setHeight(globalPoint.y() - tl.y());
						break;
					case FloatingDockContainerPrivate::eDirection::LEFTTOP:
						if (rb.x() - globalPoint.x() <= this->minimumWidth())
							rMove.setX(tl.x());
						else
							rMove.setX(globalPoint.x());
						if (rb.y() - globalPoint.y() <= this->minimumHeight())
							rMove.setY(tl.y());
						else
							rMove.setY(globalPoint.y());
						break;
					case FloatingDockContainerPrivate::eDirection::RIGHTTOP:
						rMove.setWidth(globalPoint.x() - tl.x());
						rMove.setY(globalPoint.y());
						break;
					case FloatingDockContainerPrivate::eDirection::LEFTBOTTOM:
						rMove.setX(globalPoint.x());
						rMove.setHeight(globalPoint.y() - tl.y());
						break;
					case FloatingDockContainerPrivate::eDirection::RIGHTBOTTOM:
						rMove.setWidth(globalPoint.x() - tl.x());
						rMove.setHeight(globalPoint.y() - tl.y());
						break;
					default:
						break;
					}
					rMove.moveTopLeft(rMove.topLeft() + QPoint(d->ResizeRegionPadding, d->ResizeRegionPadding));
					this->setGeometry(rMove);
				}
				else {
					this->move(mapToParent(mapFromGlobal(internal::globalPositionOf(event))) - d->DragStartPosition);
					event->accept();
				}
			}
			else {
				region(globalPoint);
			}
		}
		Super::mouseMoveEvent(event);
	}

	//============================================================================
	bool CFloatingDockContainer::hasNativeTitleBar()
	{
		return d->TitleBar == nullptr;
	}

} // namespace ads

//---------------------------------------------------------------------------
// EOF FloatingDockContainer.cpp
