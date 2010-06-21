#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include "filter.h"

#define MF_CONF_MAX		1024
#define DWORD_LEN_MAX	1024

struct kmfServer {
	int port;
	char *dwfile;
	char *pidfile;
	int daemon;
};

static struct kmfServer server;

char *kmfstrdup(char *p){
	int len = strlen(p);
	char *s = (char *)malloc(len+1);
	strcpy(s,p);
	return s;
}

static void initConfig(char * filename){
	const char *split = " ";
	FILE *fp;
	char buf[MF_CONF_MAX+1];
	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "Config File Open Failed!\n");
		exit(1);
	}
	
	char *key;
	server.port = 1234;
	server.daemon = 0;
	while(fgets(buf, MF_CONF_MAX+1, fp) != NULL){
		if(buf[0] == '#')continue;

		buf[strlen(buf)-1] = '\0';
		key = strtok(buf, split);

		if (!strcasecmp(key, "port")) {
			server.port = atoi(strtok(NULL, split));
		} else if (!strcasecmp(key, "dwfile")) {
			server.dwfile = kmfstrdup(strtok(NULL, split));
		} else if (!strcasecmp(key, "pidfile")) {
			server.pidfile = kmfstrdup(strtok(NULL, split));
		} else if (!strcasecmp(key, "daemon")) {
			server.daemon = atoi(strtok(NULL, split));
		} else{
			fprintf(stderr, "Unkonw Config %s\n", key);
			exit(1);
		}
	}
	fclose(fp);
}

bool createFTree(){
	FILE *fp;
	char buf[DWORD_LEN_MAX+1];
	if((fp = fopen(server.dwfile, "r")) == NULL){
		fprintf(stderr, "DWord File Open Failed!\n");
		exit(1);
	}
	tree_init();
	while(fgets(buf, DWORD_LEN_MAX+1, fp)){
		buf[strlen(buf)-1] = '\0';
		tree_add(buf);
	}
	return true;

}
void sock_read(int fd, short event, void *arg){
	char buf[1024];
	char bufret[1024];
	int len;
	struct event *ev = arg;
	if((len = recv(fd, buf, sizeof(buf)-1,0)) == -1){
		fprintf(stderr, "Sock Recv Failed!\n");
		exit(0);
	}
	strtok(buf,"\r");
	fprintf(stdout, "buf:%sxxx", buf);
	if(!strcasecmp(buf,"quit")){
		close(fd);
		free(ev);
		return;
	}
	char *ret = tree_filter(buf);
	sprintf(bufret, "result:%s\n", ret);
	send(fd, bufret, strlen(bufret)+1, 0);
	event_add(ev, NULL);
}
static void sock_accept(int fd, short event, void *arg){
	struct event *ev = arg;
	struct sockaddr addr;
	int s;
	socklen_t len = sizeof(addr);
	struct event *rev = (struct event *)malloc(sizeof(*rev));

	if((s = accept(fd, &addr, &len)) == -1){
		fprintf(stderr, "Sock Accept Failed!\n");
		exit(0);
	}
    fprintf(stdout, "accept socketzz: %d\n", s);
	event_set(rev, s, EV_READ, sock_read, rev);
	event_add(rev, NULL);
	event_add(ev, NULL);
}

static void initServer(){
    struct event *ev;
    int fd;
    struct sockaddr_in addr;    
	
	ev = (struct event *)malloc(sizeof(*ev)); 

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Sock Create Failed!\n");
        exit(1);
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server.port);
    addr.sin_addr.s_addr = 0;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        fprintf(stderr, "Sock Bind Failed!\n");
        exit(1);
    }

    if (listen(fd, 5) == -1)
    {
        fprintf(stderr,"Sock Listen Failed!\n");    
        exit(1);
    }

    event_init();
    event_set(ev, fd, EV_READ, sock_accept, ev);
    event_add(ev, NULL);
    event_dispatch();
}

static void daemonize(){
	int fd;
	if(fork() != 0)exit(0);
	setsid();
	if((fd = open("/dev/null", O_RDWR, 0)) != -1){
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if(fd > STDERR_FILENO)close(fd);
	}
}

int main(int argc, char **argv){
	/* Load Config From kmfilter.conf */
	if( argc == 2){
		initConfig(argv[1]);
	}else{
		fprintf(stderr, "Usage: kmfilterpath/kmfilter configpath/kmfilter.conf\n");
		exit(1);
	}
	/* create filter tree according to the dword.txt. or other file you named in config file */
	createFTree();
	/* daemon start */
	if(server.daemon)daemonize();
	/* create the levent to accept requests */
	initServer();
	return 0;
}
