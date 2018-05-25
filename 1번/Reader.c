#include "Header.h"

void sig_handler(int signo)
{
	if(signo == SIGUSR1)
	{
		puts("--------------------------------------------");
		puts("부모 프로세스가 공유메모리를 모두 채웠습니다");
	}
}

int main()
{
	void *shared_Mem = (void*)0;
	int shmid;
	int *shmaddr;
	pid_t my_pid, your_pid, fork_result;
	int i;
	FILE *fwp;
	
	// Shared Memory의 내용을 저장할 txt파일에 대한 포인터
	// 정상적으로 열리지 않았다면, 에러메시지 출력
	fwp = fopen("Backup.txt", "w+");
	if(fwp == NULL)
	{
		puts("File Open ERROR");
		exit(EXIT_FAILURE);
	}


	// 시그널 핸들러 등록
	signal(SIGUSR1, sig_handler);


	// 자신의 pid와 부모의 pid를 계산하고, 출력한다.
	my_pid = getpid();
	your_pid = getppid();
	puts("[자식 프로세스]");
	printf("My_pid=%d	Your_pid=%d\n",my_pid,your_pid);
	

	// 부모프로세스가
	// 자식 프로세스에서의 정상적인 pid 출력이 완료될 때 까지 대기중이므로
	// 부모프로세스에게 완료되었다는 signal을 보내준다.
	kill(your_pid, SIGUSR2);
	

	// 부모프로세스와 같은 Shared Memory를 get, 실패시 에러메시지 출력
	shmid = shmget((key_t)9134, sizeof(int) * SHMSIZE, 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}


	// Shared Memory attach, 실패한다면 에러메시지 출력
	shared_Mem = shmat(shmid, (void*)0, 0);
	if(shared_Mem == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}


	// shmaddr에 Shared Memory 시작주소값 저장,
	// shared_Mem이 void형이므로 int형 데이터의 주소값 계산을 위해
	// int형으로 강제 형변환
	shmaddr = (int *)shared_Mem;


	// 부모프로세스에서 Shared Memory에 Write마칠때까지 대기한다.
	pause();


	// Shared Memory의 내용을 Backup.txt파일에 저장한다.	
	for(i=1; i<=1024;i++)
	{
		fprintf(fwp,"%d\n", *(shmaddr + (i-1)));
	}


	// 부모프로세스에게 저장이 끝났음을 알린다.
	kill(your_pid,SIGUSR1);


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
