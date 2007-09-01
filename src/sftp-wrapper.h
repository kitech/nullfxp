/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
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
#ifndef SFTP_WRAPPER_H
#define SFTP_WRAPPER_H

#include <vector>
#include <map>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
    
    
#include "defines.h"
#include "includes.h"
#include "sftp-common.h"
#include "sftp-client.h"
#include "sftp-operation.h"

                 /* Separators for interactive commands */
#define WHITESPACE " \t\r\n"

                 /* ls flags */
#define LS_LONG_VIEW	0x01	/* Full view ala ls -l */
#define LS_SHORT_VIEW	0x02	/* Single row view ala ls -1 */
#define LS_NUMERIC_VIEW	0x04	/* Long view with numeric uid/gid */
#define LS_NAME_SORT	0x08	/* Sort by name (default) */
#define LS_TIME_SORT	0x10	/* Sort by mtime */
#define LS_SIZE_SORT	0x20	/* Sort by file size */
#define LS_REVERSE_SORT	0x40	/* Reverse sort order */
#define LS_SHOW_ALL	0x80	/* Don't skip filenames starting with '.' */

#define VIEW_FLAGS	(LS_LONG_VIEW|LS_SHORT_VIEW|LS_NUMERIC_VIEW)
#define SORT_FLAGS	(LS_NAME_SORT|LS_TIME_SORT|LS_SIZE_SORT)

                 /* Commands for interactive mode */
#define I_CHDIR		1
#define I_CHGRP		2
#define I_CHMOD		3
#define I_CHOWN		4
#define I_GET		5
#define I_HELP		6
#define I_LCHDIR	7
#define I_LLS		8
#define I_LMKDIR	9
#define I_LPWD		10
#define I_LS		11
#define I_LUMASK	12
#define I_MKDIR		13
#define I_PUT		14
#define I_PWD		15
#define I_QUIT		16
#define I_RENAME	17
#define I_RM		18
#define I_RMDIR		19
#define I_SHELL		20
#define I_SYMLINK	21
#define I_VERSION	22
#define I_PROGRESS	23
                 

         void
                 fxp_local_do_shell(const char *args);
         void
                 fxp_local_do_ls(const char *args,std::vector<std::map<char, std::string> > & fileinfos ) ;
         int     fxp_local_do_mkdir(const char * path );
         
         char *
                 fxp_path_strip(char *path, char *strip);
         char *
                 fxp_path_append(char *p1, char *p2);
         char *
                 fxp_make_absolute(char *p, char *pwd);
         int
                 fxp_infer_path(const char *p, char **ifp);
         int
                 fxp_parse_getput_flags(const char **cpp, int *pflag);
         int
                 fxp_parse_ls_flags(const char **cpp, int *lflag);
         int
                 fxp_get_pathname(const char **cpp, char **path);
         int
                 fxp_is_dir(char *path);
         int
                 fxp_is_reg(char *path);
         int
                 fxp_remote_is_dir(struct sftp_conn *conn, char *path);
         int
                 fxp_process_get(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag);
         int
                 fxp_process_put(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag);
         int
                 fxp_sdirent_comp(const void *aa, const void *bb);
         int
                 fxp_do_ls_dir(struct sftp_conn *conn, char *path, char *strip_path, int lflag , std::vector<std::map<char, std::string> > & fileinfos );
         int
                 fxp_do_globbed_ls(struct sftp_conn *conn, char *path, char *strip_path,
                               int lflag , std::vector<std::map<char, std::string> > & fileinfos );
         int
                 fxp_parse_args(const char **cpp, int *pflag, int *lflag, int *iflag,
                            unsigned long *n_arg, char **path1, char **path2);
         int
                 fxp_parse_dispatch_command(struct sftp_conn *conn, const char *cmd, char **pwd,
                                        int err_abort);
         void
                 fxp_connect_to_server(char *path, char **args, int *in, int *out);    

#ifdef __cplusplus
}
#endif


#endif



