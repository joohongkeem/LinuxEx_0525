#include "Header.h"

void sig_handler(int signo)
{
	if(signo == SIGUSR1)
	{
		printf("SIG>>child\n");
	}
}

int main(int argc, char *argv[])
{
	void *shared_Mem = (void*)0;
	int shmid;
	char *shmaddr;
	pid_t my_pid, your_pid, fork_result;
	int i=0,j;
	FILE *fwp;
	char buffer[BUFFSIZE];
	int tmp;
	int count;

	fwp = fopen(argv[1], "wb+");

	if(fwp == NULL)
	{
		puts("File Open ERROR");
		exit(EXIT_FAILURE);
	}


	signal(SIGUSR1, sig_handler);


	my_pid = getpid();
	your_pid = getppid();
	puts("[자식 프로세스]");
	printf("My_pid=%d	Your_pid=%d\n",my_pid,your_pid);
	

	kill(your_pid, SIGUSR2);
	

	shmid = shmget((key_t)9134, BUFFSIZE+2, 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	
	shared_Mem = shmat(shmid, (void*)0, 0);
	if(shared_Mem == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	shmaddr = (char *)shared_Mem;
	
	pause();
	kill(your_pid,SIGUSR1);



	while(*shmaddr == 0);	// 부모프로세스가 먼저 실행되도록 하기 위한 작업.
	while(1)
	{
		if(*shmaddr == 1)
		{
			strcpy(buffer, shmaddr+1);
			count = fwrite(buffer,1,BUFFSIZE,fwp);			//라즈베리는 반환값있으면
										//앞에count꼭써줘야하는듯
			*shmaddr = 0;
		}
		else if(*shmaddr == 2)
		{	
			strncpy(buffer, shmaddr+1, *(shmaddr+BUFFSIZE+1));
			count = fwrite(buffer,1,*(shmaddr+BUFFSIZE+1),fwp);
						break;
		}
		while(*shmaddr ==0);
	}
	/*
	i=0;
	//for(i=0; i<*(shmaddr+SHMSIZE); i++)
	for(i=0;i<=200;i++)
	{
		pause();

	
		buffer = *(shmaddr);
		printf("%c",buffer);
		tmp=fwrite(&buffer,1,1,fwp);
		if(feof(fwp)!=0) printf("똥똥");
		//if(feof(fwp)==0) printf("%d또잉\n",i);
		//else printf("ㅋㅋㅋㅋ%dㅋㅋㅋㅋ",feof(fwp));
		
		kill(your_pid,SIGUSR1);
	
	}	
*/
	// Shared Memory detach, 실패시 에러메시지 출력
	if(shmdt(shared_Mem)==-1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}


	// 파일 포인터 삭제.
	fclose(fwp);

	exit(EXIT_SUCCESS);
}
