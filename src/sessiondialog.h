/* sessiondialog.h --- 
 * 
 * Filename: sessiondialog.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Created: 六  4月  5 18:11:12 2008 (CST)
 * Version: 
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.

 */

/* Code: */





#include <QtCore>
#include <QtGui>

#include "basestorage.h"
#include "ui_hostlistdialog.h"

class SessionDialog: public QDialog
{
  Q_OBJECT

public:
    SessionDialog(QWidget * parent = 0);
    ~SessionDialog();

public slots:
    void slot_conntect_selected_host();
    void slot_conntect_selected_host(const QModelIndex & index);
    void slot_edit_selected_host();
    void slot_remove_selected_host();

    QMap<QString,QString>  get_host_map();

 signals:
    void connect_remote_host_requested(QMap<QString,QString> host);

private slots:
    bool loadHost();
    void slot_ctx_menu_requested(const QPoint & pos);
private:
    Ui::HostListDialog sess_dlg;
    BaseStorage * storage;
    QStringListModel * host_list_model;
    QMenu * host_list_ctx_menu;
    QAction * action_connect;
    QAction * action_edit;
    QAction * action_remove;
    void * info_dlg;
};


/* sessiondialog.h ends here */
