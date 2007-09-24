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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <QCoreApplication>
#include "nullfxp.h"

// #include "libssh2.h"
// 
// char *username=(char *)"username";
// char *password=(char *)"password";
// 
// 
// static void kbd_callback(const char *name, int name_len, 
//                          const char *instruction, int instruction_len, int num_prompts,
//                          const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
//                          LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
//                          void **abstract)
// {
//     (void)name;
//     (void)name_len;
//     (void)instruction;
//     (void)instruction_len;
//     if (num_prompts == 1) {
//         responses[0].text = strdup(password);
//         responses[0].length = strlen(password);
//     }
//     (void)prompts;
//     (void)abstract;
// } /* kbd_callback */

int main(int argc, char *argv[])
{
//     int ret = 0 ;
//     int sockfd ;
//     char buff ;
//     struct sockaddr_in serv_addr ;
//     
//     LIBSSH2_SESSION * ssh2_sess = 0 ;
//     LIBSSH2_CHANNEL *ssh2_channel;
//     
//     ssh2_sess = libssh2_session_init();
//     //ssh2_sess = libssh2_session_init();
//     
//     sockfd = socket(AF_INET,SOCK_STREAM,0);
//     printf("socket : %d \n" , sockfd );
//     
//     bzero(&serv_addr,sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(22);
//     ret = inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr.s_addr);
//     printf("inet_pton: %d \n" , ret );
//     
//     ret = connect(sockfd , (struct sockaddr *)&serv_addr,sizeof(serv_addr));
//     printf("connect: %d \n" , ret );
//     
//     ret = libssh2_session_startup(ssh2_sess,sockfd);
//     printf("sess startup: %d \n" , ret );
//     
//     //char * auth_list = libssh2_userauth_list(ssh2_sess,"gzl",3);
//     //printf("auth list: %s \n" , auth_list );
//     
//     password = "2113";
//     ret = libssh2_userauth_keyboard_interactive(ssh2_sess,"gzl",&kbd_callback);
//     printf(" user auth:  %d \n" , ret );
//     printf("LIBSSH2_ERROR_ALLOC = %d \n"
//             "LIBSSH2_ERROR_SOCKET_SEND=%d\n"
//             "LIBSSH2_ERROR_PASSWORD_EXPIRED=%d\n",
//             LIBSSH2_ERROR_ALLOC,LIBSSH2_ERROR_SOCKET_SEND,LIBSSH2_ERROR_PASSWORD_EXPIRED);
//     ret = libssh2_userauth_authenticated(ssh2_sess) ;
//     printf(" authed : %d \n" , ret );
//     
//     char *errmsg = 0;
//     int  errmsg_len = 0 ;
//     ret = libssh2_session_last_errno(ssh2_sess);
//     printf( "last errno: %d \n" , ret );
//     ret = libssh2_session_last_error(ssh2_sess,&errmsg,&errmsg_len,1000);
//     printf("sess last error: %d %p %d \n", ret , errmsg,errmsg_len);
//     printf("auth error: %s \n" , errmsg );
//     
//     ssh2_channel = libssh2_channel_open_session(ssh2_sess);
//     printf(" ssh2 channel: %p \n" , ssh2_channel );
//     
//     libssh2_channel_setenv(ssh2_channel, (char *)"FOO", (char *)"bar");
//     
//     ret = libssh2_channel_request_pty(ssh2_channel ,"liuguangzhao's ssh");
//     printf("request pty : %d \n" , ret );
//     
//     ret = libssh2_channel_shell(ssh2_channel);
//     printf(" open shell on channel: %d \n" , ret );
//     
//     pause();
//     
//     ret = libssh2_session_disconnect(ssh2_sess,"hehehe");
//     printf("sess disconnect: %d \n" , ret );
//     close(sockfd);    
//     ret = libssh2_session_free(ssh2_sess);
//     printf("sess free: %d \n", ret );
//     
// 
//     
//     return 0 ;
      QApplication app(argc, argv);
      qDebug("Hello from Qt 4!");
      NullFXP nfxp ;
      nfxp.showNormal ();
      
      return app.exec();
}

