/* A simple client program for server.c

   To compile: gcc client.c -o client

   To run: start the server, then the client */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <regex.h>
typedef struct node node_t;
typedef struct
{
   char protocol[10];
   char hostname[1000];
   int  port;
   char path[1000];
   char absoluteurl[1000];
} pause_url;


struct node {
	pause_url *data;
	node_t *next;
    int visited;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;


void deletevisiteddata(char *buffer,int end);


list_t *make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(list));

	list->head = list->foot = NULL;
	return list;
}

int
is_empty_list(list_t *list) {

	return list->head==NULL;
}

void
free_list(list_t *list) {
	node_t *curr, *prev;
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(list);
}
int judgetruncatedpages(char* bufferget){
    regex_t regforcolen;
    regex_t regforheaderlen;
    const char * patternforconlen="<";
    int cflags=REG_EXTENDED;
    const char * patternforheaderlen="Content-Length:\\s*[0-9]{1,7}";
    regcomp(&regforcolen,patternforconlen,cflags);
    regcomp(&regforheaderlen,patternforheaderlen,cflags);
    regmatch_t pm[5];
    char headerconlen[10];
    int j=0;
    int i;
    int actuallen=0;
    if(regexec(&regforheaderlen,bufferget,2,pm,REG_NOTBOL)==0){
            for(i=pm[0].rm_so;i<pm[0].rm_eo;i++){
                if(bufferget[i]>='0' && bufferget[i]<='9'){
                    headerconlen[j]=bufferget[i];
                    j++;
                }
            }
        }
    regexec(&regforcolen,bufferget,2,pm,REG_NOTBOL);
    actuallen=(strlen(bufferget)-pm[0].rm_so);
    regfree(&regforcolen);
    regfree(&regforheaderlen);
    return actuallen==atoi(headerconlen);
}

int judgemimetype(char* bufferget){
    regex_t regformime;
    const char * patternformime="text/html";
    int cflags=REG_EXTENDED;
    regmatch_t pm[5];
    regcomp(&regformime,patternformime,cflags);
    if(regexec(&regformime,bufferget,2,pm,REG_NOTBOL)==0){
        regfree(&regformime);
        return 1;
    }
    regfree(&regformime);
    return 0;
}
pause_url *getpauseurl(char *url,node_t *curnode){
    //printf("%s\n",url);
    if(strstr(url,"..")!=NULL){
        return NULL;
    }
    if(strstr(url,"./")!=NULL){
        return NULL;
    }
    if(strstr(url,"?")!=NULL){
        return NULL;
    }
    if(strstr(url,"#")!=NULL){
        return NULL;
    }
    pause_url *pausedurl=(pause_url *)malloc(sizeof(pause_url));
    memset(pausedurl->hostname,0,1000);
    memset(pausedurl->path,0,1000);
    memset(pausedurl->absoluteurl,0,1000);
    int cflags=REG_EXTENDED;
    regmatch_t pm[5];
    regex_t reg1;
    regex_t reg2;
    regex_t reg3;
    int hostend=0;
    const char * pattern1="((h|H)(R|r)(e|E)(f|F)\\s*=\\s*\")?(\")?(http:)?";
    const char * pattern2="//";
    const char * pattern3="/[a-zA-Z0-9_-]+\.html/*";
    regcomp(&reg1,pattern1,cflags);
    regcomp(&reg2,pattern2,cflags);
    regcomp(&reg3,pattern3,cflags);
    if(regexec(&reg1,url,2,pm,REG_NOTBOL)==0){
            hostend=pm[0].rm_eo;
            regfree(&reg1);
    }
    int i=0;
    int j=0;
    if(regexec(&reg2,url,2,pm,REG_NOTBOL)==0){
        if(pm[0].rm_so==hostend){ 
            for(i=pm[0].rm_so+2;i<strlen(url);i++){
                if(url[i]=='/'){
                    pausedurl->hostname[j]='\0';
                    break;
                }
                else{
                    pausedurl->hostname[j]=url[i];
                    j++;
                }
            }
        }
        regfree(&reg2);
    }
    if(j==0){
        i=hostend;
        strcat(pausedurl->hostname,curnode->data->hostname);
        if(url[i]!='/'){
            if(regexec(&reg3,curnode->data->path,2,pm,REG_NOTBOL)==0){
                strncat(pausedurl->path,curnode->data->path,pm[0].rm_so+1);
        }
        else{
            strcat(pausedurl->path,curnode->data->path);
            strcat(pausedurl->path,"/");
        }
        }
    }
    j=strlen(pausedurl->path);
    for(i;i<strlen(url);i++){
        if(url[i]=='\"'){
            pausedurl->path[j]='\0';
            break;
        }
        else{
            pausedurl->path[j]=url[i];
            j++;
        }
    }
    strcat(pausedurl->absoluteurl,pausedurl->hostname);
    strcat(pausedurl->absoluteurl,pausedurl->path);
    return pausedurl;
}
int checkrepetition(list_t *list,char *absoluteurl){
    if(is_empty_list(list)==1){
        return 0;
    }
    int len;
    len=strlen(absoluteurl);
    node_t *cur;
    cur=list->head;
    char *otherexpress=(char *)malloc(sizeof(char)*(len + 5));
    if(absoluteurl[len-1]=='/'){
        memcpy(otherexpress,absoluteurl,len-1);
        otherexpress[len]='\0';
    }
    else{
        memcpy(otherexpress,absoluteurl,len);
        strcat(otherexpress,"/");
        strcat(otherexpress,"\0");

    }
    while(cur!=NULL){
        if(strcmp(cur->data->absoluteurl,absoluteurl)==0||strcmp(cur->data->absoluteurl,otherexpress)==0){
            return 1;
        }
        cur=cur->next;
    }
    return 0;

}
void
*insert_at_foot(list_t *list,char *url,node_t *curnode) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	new->data=getpauseurl(url,curnode);
    if(new->data==NULL){
        free(new);
        return;
    }
    if(checkrepetition(list,new->data->absoluteurl)==1){
        free(new);
        return;
    }
	new->visited=0;
	new->next = NULL;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}
}

node_t *get_unvisited_url(list_t *list_url){
    if(is_empty_list(list_url)==1){
        return NULL;
    }
    node_t *cur;
    cur=list_url->head;
    while(cur!=NULL){
        if(cur->visited==0){
            return cur;
        }
        cur=cur->next;
    }
    return NULL;
}
void deletevisiteddata(char *buffer,int end){
    int i;
    for(i=0;i<=end;i++){
        buffer[i]='1';
    }
}


void test1(list_t *list_url){
    printf("here are unvisited list:\n");
    if(is_empty_list(list_url)==1){
        return NULL;
    }
    node_t *cur;
    cur=list_url->head;
    while(cur!=NULL){
        printf("%d  ,%s\n",cur->visited,cur->data->absoluteurl);
        cur=cur->next;
    }
    return NULL;
}


int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent * server;
    struct hostent *hptr;
    list_t *list_url;
    list_url=make_empty_list();
    insert_at_foot(list_url,argv[1],NULL);
    int cflags=REG_EXTENDED;
    regmatch_t pm[5];
    regex_t reg;
    regex_t regforstatus;
   
    int len;
    const char * pattern="(h|H)(R|r)(e|E)(f|F)\\s*=\\s*\"(http:)?/*([0-9a-zA-Z_-]+\.?@?:?/*)+\"";
    const char * patternforstatus="[0-9]{3}";
    regcomp(&reg,pattern,cflags);
    regcomp(&regforstatus,patternforstatus,cflags);
    char buffersend[100000];
    char bufferget[100000];
    node_t *curnode;
    int i=0;
    char statuscode[5]; 
    while((curnode=get_unvisited_url(list_url))!=NULL){
        test1(list_url);
        if(strstr(curnode->data->absoluteurl,"test-cases/basic/fully-connected-basic/bear/lobster.html")!=NULL){
        printf("try to run this website%s\n",curnode->data->absoluteurl);
    }
        //printf("%s\n",curnode->data->absoluteurl);
        bzero(buffersend, 100000);
        hptr=gethostbyname(curnode->data->hostname);
        if(hptr==NULL){
            perror("ERROR GET HOST Fail");
        }
        sockfd= socket(AF_INET, SOCK_STREAM, 0);
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char*)hptr->h_addr,(char *) &serv_addr.sin_addr.s_addr, hptr->h_length);
        serv_addr.sin_port = htons(80);
        /* Create TCP socket -- active open
    * Preliminary steps: Setup: creation of active open socket
    */
        if (sockfd < 0)
        {
            perror("ERROR opening socket");
            exit(0);
        }
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("ERROR connecting");
            exit(0);
        }    
        else{
            strcat(buffersend,"GET http://");
            strcat(buffersend,curnode->data->absoluteurl);
            strcat(buffersend," HTTP/1.1\r\n");
            strcat(buffersend, "Host:");
            strcat(buffersend, curnode->data->hostname);
            strcat(buffersend, "\r\n");
            strcat(buffersend, "User-Agent: zhichengh\r\n");
            //strcat(buffersend, "Connection: Keep-alive\r\n");
            //strcat(buffersend, "Content-Type: text/html\r\n");
            strcat(buffersend, "\r\n");
            printf("%s\n",buffersend);
        }
        /* Do processing
        */
        int n;
        n = write(sockfd, buffersend, strlen(buffersend));
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(0);
        }
        bzero(bufferget, 100000);
        n = read(sockfd, bufferget, 100000);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(0);
        }
        regexec(&regforstatus,bufferget,2,pm,REG_NOTBOL);
        statuscode[0]='\0';
        statuscode[3]='\0';
        statuscode[1]='\0';
        statuscode[2]='\0';
        memcpy(statuscode,bufferget+pm[0].rm_so,3);
        printf("%s\n",bufferget);
        if(strcmp(statuscode,"200")!=0){
            if(strcmp(statuscode,"404")==0||strcmp(statuscode,"410")==0||strcmp(statuscode,"414")==0){}
            else if(strcmp(statuscode,"301")==0){
            //printf("%s\n",buffersend);
            //printf("%s\n",bufferget);
        }
         else if(strcmp(statuscode,"401")==0){
            //printf("%s\n",buffersend);
            //printf("%s\n",bufferget);
        }
            else if(strcmp(statuscode,"503")==0||strcmp(statuscode,"504")==0){
            for(i=0;i<5;i++){
                connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                sockfd= socket(AF_INET, SOCK_STREAM, 0);
                bzero((char *)&serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                bcopy((char*)hptr->h_addr,(char *) &serv_addr.sin_addr.s_addr, hptr->h_length);
                serv_addr.sin_port = htons(80);
                n = write(sockfd, buffersend, strlen(buffersend));
                printf(buffersend);
                 if (n < 0)
                {
                perror("ERROR writing to socket");
                exit(0);
                }
                bzero(bufferget, 100000);
                n = read(sockfd, bufferget, 100000);
                //printf("%s\n",bufferget);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(0);
                }
                    regexec(&regforstatus,bufferget,2,pm,REG_NOTBOL);
                     statuscode[0]='\0';
                     statuscode[3]='\0';
                     statuscode[1]='\0';
                     statuscode[2]='\0';
                    memcpy(statuscode,bufferget+pm[0].rm_so,3);
                    if(strcmp(statuscode,"200")){
                        break;
                    }
                }
        }
        }

        

        else{
                if(judgetruncatedpages(bufferget)==1 &&judgemimetype(bufferget)==1){
                //printf("%s\n",bufferget);
                while(regexec (&reg,bufferget,2,pm,REG_NOTBOL)==0){
                len = pm[0].rm_eo - pm[0].rm_so;
                char *value = (char *)malloc(sizeof(char)*(len + 1));
                memset(value,0,len + 1);
                memcpy(value,bufferget+pm[0].rm_so,len);
                insert_at_foot(list_url,value,curnode);
                deletevisiteddata(bufferget,pm[0].rm_eo);
                }
            }
        }

        curnode->visited=1;
    }
    return 0;
}

