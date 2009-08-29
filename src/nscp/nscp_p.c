// nscp_p.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-19 21:07:41 +0800
// Version: $Id$
// 

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "nscp_p.h"

static char kbd_password[60];
static void kbd_callback(const char *name, int name_len, 
             const char *instruction, int instruction_len, int num_prompts,
             const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
             LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
             void **abstract)
{
    (void)name;
    (void)name_len;
    (void)instruction;
    (void)instruction_len;
    if (num_prompts == 1) {
        // responses[0].text = strdup(password);
        responses[0].text = strdup(kbd_password);
        responses[0].length = strlen(kbd_password);
    }
    (void)prompts;
    (void)abstract;
} /* kbd_callback */

ssh_conn_t *connect_to_host(char *host, short port)
{
    ssh_conn_t *conn;
    struct sockaddr_in sin;
    unsigned long hostaddr;
    int rc;

    fprintf(stderr, "Connecting: %s ...\n", host);
    conn = (ssh_conn_t*)calloc(1, sizeof(ssh_conn_t));
#ifdef WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif

    hostaddr = inet_addr(host);
    conn->sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(conn->sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "connect faild: %s!\n", strerror(errno));
        free(conn);
        return NULL;
    }
    
    conn->sess = libssh2_session_init();
    if (!conn->sess) {
        free(conn);
        return NULL;
    }
    
    rc = libssh2_session_startup(conn->sess, conn->sock);
    if (rc) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        free(conn);
        return NULL;
    }

    return conn;
}

int  disconnect_from_host(ssh_conn_t *conn)
{
    assert(conn != NULL);
    if (conn->sftp) {
        libssh2_sftp_shutdown(conn->sftp);
    }
    if (conn->sess) {
        libssh2_session_disconnect(conn->sess, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(conn->sess);
    }
#ifdef WIN32
    ::Sleep(1000);
    ::closesocket(conn->sock);
#else
    sleep(1);
    close(conn->sock);
#endif

    free(conn);
    return 0;
}

int connect_auth_password(ssh_conn_t *conn, char *username, char *password)
{
    const char *fingerprint;
    
    fingerprint = libssh2_hostkey_hash(conn->sess, LIBSSH2_HOSTKEY_HASH_MD5);
    fprintf(stderr, "Fingerprint: ");
    for (int i = 0; i < 16; i++) {
        fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(stderr, "\n");

    /* We could authenticate via password */
    if (libssh2_userauth_password(conn->sess, username, password)) {
        fprintf(stderr, "Authentication by password failed. trying keyboard_interactive ...\n");
        // kbd_password = strdup(password);
        strcpy(kbd_password, password);
        if (libssh2_userauth_keyboard_interactive(conn->sess, username, kbd_callback)) {
            fprintf(stderr, "Authentication by keyboard interactive failed.\n");
            return -1;
        }
    }
    
    return 0;
}

int scp_file_to_server_on_sftp(ssh_conn_t *conn, char *local_file, char *remote_file)
{
    FILE *fp = NULL;
    LIBSSH2_SFTP_HANDLE *hsftp = NULL;
    int rlen;
    int rc;

    fp = fopen(local_file, "r");
    assert(fp != NULL);

    conn->sftp = libssh2_sftp_init(conn->sess);
    assert(conn->sftp != NULL);
    rc = libssh2_sftp_shutdown(conn->sftp);
    fprintf(stderr, "SFTP shutdown: %d\n", rc);
    assert(rc == 0);
    conn->sftp = libssh2_sftp_init(conn->sess);
    assert(conn->sftp != NULL);

    hsftp = libssh2_sftp_open(conn->sftp, remote_file, 
                              LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC , 0755);
    assert(hsftp != NULL);

    do {
        char mem[1000];
        rlen = fread(mem, 1, sizeof(mem), fp);
        if (rlen > 0) {
            rc = libssh2_sftp_write(hsftp, mem, rlen);
            fprintf(stderr, "Write SFTP: %d/%d\n", rc, rlen);
            assert(rc == rlen);
        } else {
            break;
        }        
    } while (1);
    
    libssh2_sftp_close(hsftp);

    fclose(fp);
    
    return 0;
}

int scp_file_to_server_on_scp(ssh_conn_t *conn, char *local_file, char *remote_file)
{
    LIBSSH2_CHANNEL *channel = NULL;
    struct stat fileinfo;
    FILE *fp = NULL;
    int rlen;
    int rc;

    
    stat(local_file, &fileinfo);
    channel = libssh2_scp_send(conn->sess, remote_file, 0755, (unsigned long)fileinfo.st_size);
    assert(channel != NULL);

    fp = fopen(local_file, "r");
    assert(fp != NULL);
    
    do {
        char mem[1000];
        rlen = fread(mem, 1, sizeof(mem), fp);
        if (rlen > 0) {
            rc = libssh2_channel_write(channel, mem, rlen);
            // fprintf(stderr, "Write SCP: %d/%d\n", rc, rlen);
            assert(rc == rlen);
        } else {
            break;
        }        
    } while (1);
    libssh2_channel_send_eof(channel);
    libssh2_channel_close(channel);

    fclose(fp);

    return 0;
}

int do_scp(ssh_conn_t *conn, nscp_option_t *nopt)
{
    assert(conn != NULL);
    assert(nopt != NULL);

    nscp_path_t *np;
    char *s_path;
    char *d_path;
    int rc;
    LIBSSH2_SFTP_ATTRIBUTES attr;
    LIBSSH2_SFTP *sftp;
    struct stat file_info;

    sftp = libssh2_sftp_init(conn->sess);
    assert(sftp != NULL);
    d_path = nopt->d_path->path;
    if (nopt->direct == DIRECT_UPLOAD) {
        // dest path must be a directory and must exists
        rc = libssh2_sftp_stat(sftp, d_path, &attr);
        if (rc != 0) {
            rc = -1;
            return -1; // here return is bad
        }
        if (!S_ISDIR(attr.permissions)) {
            fprintf(stderr, "The destination is not a directory.\n");
            rc = -1;
            return -1; // here return is bad
        }
        for (int i = 0; i < nopt->s_path_count; i++) {
            np = nopt->s_paths[i];
            s_path = np->path;
            assert(np->path_type == PATH_FILE);
            rc = stat(s_path, &file_info);
            if (rc != 0) {
                fprintf(stderr, "get source file error: %s.\n", strerror(errno));
                return -1;
            }
            if (S_ISDIR(file_info.st_mode)) {
                fprintf(stderr, "Warning: not implmentioned.\n");
            } else {
                char buf[1000] = {0};
                strcpy(buf, d_path);
                strcat(buf, "/");
                strcat(buf, basename(s_path));


                fprintf(stderr, "Info: %s to %s\n", s_path, buf);
                rc = scp_file_to_server_on_scp(conn, s_path, buf);
            }
        }
    }
    libssh2_sftp_shutdown(sftp);
    
    return rc;
}
