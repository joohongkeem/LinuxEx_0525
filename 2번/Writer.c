#include "Header.h"

void sig_handler(int signo)
{
	if(signo == SIGUSR1)
	{
		printf(" ");
	}
	if(signo == SIGUSR2)
	{
		puts("--------------------------------------------");
		puts("              PID 교환 완료");
		puts("--------------------------------------------");
	}
}
int main()
{
	void *shared_Mem = (void*)0;
	int shmid;
	char *shmaddr;
	pid_t my_pid, your_pid, fork_result;
	int j,i=0;
	int status;
	FILE *frp;
	int count;
	char buffer[BUFFSIZE];

	frp = fopen("128byte.txt","rb");

	fork_result = fork();

	if(fork_result == -1)
	{
		fprintf(stderr, "Fork Failure\n");
		exit(EXIT_FAILURE);
	}

	if(fork_result == 0) // 자식 프로세스
	{
		execlp("./Reader","Reader",(char *)0);	
	}
	
	else
	{
		signal(SIGUSR1, sig_handler);
		signal(SIGUSR2, sig_handler);		

		my_pid = getpid();
		your_pid = fork_result;
		
		puts("[부모 프로세스]");
		printf("My_pid=%d	Your_pid=%d\n",my_pid,your_pid);	
		
		pause();


		shmid = shmget((key_t)9134, BUFFSIZE+2, 0666|IPC_CREAT);
		if(shmid == -1)
		{
			fprintf(stderr, "shmget failed\n");
			exit(EXIT_FAILURE);
		}


		shared_Mem = shmat(shmid, (void *)0, 0);
		if(shared_Mem == (void *)-1)
		{
			fprintf(stderr, "shmat failed\n");
			exit(EXIT_FAILURE);
		}
		shmaddr = (char *) shared_Mem;
		
		*shmaddr = 0;

		kill(your_pid,SIGUSR1);
		pause();

		while(1)
		{
			memset(buffer,'\0',BUFFSIZE);
			if(*shmaddr==0)
			{
				count = fread(buffer, 1, BUFFSIZE, frp);
				if(feof(frp)==0)
				{
					strcpy(shmaddr+1, buffer);
					*shmaddr = 1;
				}	
				else
				{
					strcpy(shmaddr+1, buffer);
					*(shmaddr+BUFFSIZE+1) = count;
					*shmaddr = 2;
					break;
				}
			}	
			while(*shmaddr == 1);
		}
		
		
		
		
		
		// Shared Memory detach, 실패한다면 에러메시지 출력
		if(shmdt(shared_Mem) == -1)
		{
			fprintf(stderr, "shmdt failed\n");
			exit(EXIT_FAILURE);
		}


		// 자식프로세스가 Shared Memory를 detach하고 종료할때 까지 대기
		wait(&status);


		// 모두 detach되었으므로 Shared Memory 삭제
		if(shmctl(shmid, IPC_RMID,0) == -1)
		{
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			exit(EXIT_FAILURE);
		}


		// 파일 포인터 삭제
		fclose(frp);
	}
	


	exit(EXIT_SUCCESS);
}
