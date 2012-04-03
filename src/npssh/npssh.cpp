#include <stdio.h>
#include <stdlib.h>

#include "libssh/libssh.h"

int main(int argc, char **argv)
{
    int verbose = SSH_LOG_PROTOCOL;
    int port = 22;
    int rc = 0;
    char *username = "gzleo";
    char *passwd = "2113";
    ssh_session my_ssh_session = ssh_new();

    if (my_ssh_session == NULL)
        exit(-1);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbose);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
  
    rc = ssh_connect(my_ssh_session);
    printf("conn: ok=?%d, %d, %s\n", rc==SSH_OK, rc, ssh_get_error(my_ssh_session));

    rc = ssh_userauth_password(my_ssh_session, username, passwd);
    printf("conn: ok=?%d, %d, %s\n", rc==SSH_OK, rc, ssh_get_error(my_ssh_session));

    ssh_channel channel;

    channel = ssh_channel_new(my_ssh_session);
    if (channel == NULL) {

    }

    rc = ssh_channel_open_session(channel);
    printf("conn: ok=?%d, %d, %s\n", rc==SSH_OK, rc, ssh_get_error(my_ssh_session));
    
    rc = ssh_channel_request_exec(channel, "ls -lh");
    printf("conn: ok=?%d, %d, %s\n", rc==SSH_OK, rc, ssh_get_error(my_ssh_session));

    char buffer[100];
    unsigned int nbytes;


    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer)-1, 0);
    buffer[nbytes] = '\0';
    printf("ssh out byte: %d, %s\n", nbytes, buffer);

    while (nbytes > 0) {
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer)-1, 0);
        buffer[nbytes] = '\0';
        printf("ssh out byte: %d, %s\n", nbytes, buffer);
    }

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer)-1, 1);
    buffer[nbytes] = '\0';
    printf("ssh err byte: %d, %s\n", nbytes, buffer);

    while (nbytes > 0) {
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer)-1, 1);
        buffer[nbytes] = '\0';
        printf("ssh err byte: %d, %s\n", nbytes, buffer);
    } 

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    ssh_disconnect(my_ssh_session);

    ssh_free(my_ssh_session);

    return 0;
}
