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

#ifndef SFTP_OPERATION_H
#define SFTP_OPERATION_H



#ifdef __cplusplus
extern "C"
{
#endif

    //int     remote_glob(struct sftp_conn *, const char *, int,
    //                int (*)(const char *, int), glob_t *); /* proto for sftp-glob.c */
    
    void
            local_do_shell(const char *args);
    void
            local_do_ls(const char *args) ;
    char *
            path_strip(char *path, char *strip);
    char *
            path_append(char *p1, char *p2);
    char *
            make_absolute(char *p, char *pwd);
    int
            infer_path(const char *p, char **ifp);
    int
            parse_getput_flags(const char **cpp, int *pflag);
    int
            parse_ls_flags(const char **cpp, int *lflag);
    int
            get_pathname(const char **cpp, char **path);
    int
            is_dir(char *path);
    int
            is_reg(char *path);
    int
            remote_is_dir(struct sftp_conn *conn, char *path);
    int
            remote_is_reg(struct sftp_conn *conn, char *path);
    int
            process_get(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag);
    int
            process_put(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag);
    int
            sdirent_comp(const void *aa, const void *bb);
    int
            do_ls_dir(struct sftp_conn *conn, char *path, char *strip_path, int lflag);
    int
            do_globbed_ls(struct sftp_conn *conn, char *path, char *strip_path,
                          int lflag);
    int
            parse_args(const char **cpp, int *pflag, int *lflag, int *iflag,
                       unsigned long *n_arg, char **path1, char **path2);
    int
            parse_dispatch_command(struct sftp_conn *conn, const char *cmd, char **pwd,
                                   int err_abort);
    void
            connect_to_server(char *path, char **args, int *in, int *out,int * child_pid );    
    

#ifdef __cplusplus
}
#endif

#endif


