#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int M;
int N;
unsigned char* prefix;
unsigned char* Goal;
unsigned char* outfile;
int preLen;
pthread_t tidp[10];
unsigned char result[10][5][256];
int threadNo;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int DFS_find(unsigned char *cur, int layer, int curLayer, int len, int rank, int ID){
    
    if (curLayer == layer){
	unsigned char crypted[MD5_DIGEST_LENGTH];
	MD5(cur, len+layer, crypted);
	char cry[MD5_DIGEST_LENGTH*2+1];
	int i = 0;
	int loop = 0;
	while (loop < MD5_DIGEST_LENGTH){
	    sprintf(cry+i, "%02x", crypted[loop]);
	    loop++;
	    i += 2;
	}
	cry[MD5_DIGEST_LENGTH*2] = '\0';
	//printf("%s\n", cry);
	if (strncmp(cry, Goal, rank+1) == 0){
	    strcpy(result[ID][rank], cur);
#ifdef DEBUG
	    printf("%s\n", crypted);
	    printf("%s\n", cry);
#endif
	    return 1;
	}
	else{
	    return 0;
	}
    }

    for (int i = 33; i <= 126; i++){
	cur[len+curLayer] = (unsigned char)i;
	if (DFS_find(cur, layer, curLayer+1, len, rank, ID) == 1){
	    return 1;
	}
    }
    return 0;
}

void* thread_find(void* data){
    int curID;
    pthread_mutex_lock(&mutex1);
    curID = threadNo;
    threadNo++;
    pthread_mutex_unlock(&mutex1);

    unsigned char first = *((unsigned char*)data);
    int len = preLen;
    unsigned char curStr[256];
    strcpy(curStr, prefix);
    curStr[len++] = first;
    for (int rank = 0; rank < N; rank++){
	int layer = 1;
	while(1){
	    if(DFS_find(curStr, layer, 0, len, rank, curID) == 1){
		len += layer;
		break;
	    }
	    else{
		layer++;
	    }
	}
    }
}

int main(int argc, char *argv[]){
    prefix = argv[1];
    Goal = argv[2];
    N = atoi(argv[3]);
    M = atoi(argv[4]);
    outfile = argv[5];
    //printf("%s\n%s\n%s\n", prefix, Goal, outfile);
    threadNo = 0;
    
    preLen = strlen(prefix);

    unsigned char firstChr[10];
    for (int i = 0; i < M; i++){
	firstChr[i] = (unsigned char)68+i;
	pthread_create(&tidp[i], NULL, thread_find, &firstChr[i]);
    }
    for (int i = 0; i < M; i++){
	pthread_join(tidp[i], NULL);
    }
#ifdef DEBUG
    for (int i = 0; i < M; i++){
	for (int j = 0; j < N; j++){
	    printf("%s\n", result[i][j]);
	}
	printf("===\n");
    }
#endif
#ifndef DEBUG
    int fd = open(outfile, O_WRONLY | O_CREAT);
    for (int i = 0; i < M; i++){
	for (int j = 0; j < N; j++){
	    write(fd, result[i][j], strlen(result[i][j]));
	    write(fd, "\n", 1);
	}
	write(fd, "===\n", 4);
    }
    close(fd);
#endif

    return 0;
}

