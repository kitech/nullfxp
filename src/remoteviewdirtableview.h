/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef REMOTEVIEWDIRTABLEVIEW_H
#define REMOTEVIEWDIRTABLEVIEW_H

#include <QTableView>

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteViewDirTableView : public QTableView
{
		Q_OBJECT
	public:

		RemoteViewDirTableView ( QWidget* parent );

		~RemoteViewDirTableView();
    signals:
        void drag_ready( );
        
	protected:
		virtual void dragEnterEvent ( QDragEnterEvent * event ) ;
		virtual void dragLeaveEvent ( QDragLeaveEvent * event ) ;
		virtual void dragMoveEvent ( QDragMoveEvent * event );
		virtual void dropEvent ( QDropEvent * event ) ;
		virtual void startDrag ( Qt::DropActions supportedActions );
		virtual void mouseMoveEvent ( QMouseEvent * event );
		virtual void mousePressEvent ( QMouseEvent * event );
    private:
        QPoint dragStartPosition;
        
	public :
		int test_use_qt_designer_prompt ;

};

#endif
