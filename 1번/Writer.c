#include "Header.h"

void sig_handler(int signo)
{
	if(signo == SIGUSR1)
	{
		puts("--------------------------------------------");
		puts("자식 프로세스가 공유메모리의 정보를 ");
		puts("            Backup.txt에 저장 완료하였습니다");
		puts("--------------------------------------------");
	}
	else if(signo == SIGUSR2)
	{
		puts("--------------------------------------------");
		puts("              PID 교환 완료");
	}
}
int main()
{
	void *shared_Mem = (void*)0;
	int shmid;
	int *shmaddr;
	pid_t my_pid, your_pid, fork_result;
	int i;
	int status;

	// 먼저 부모-자식 프로세스를 생성한다.
	fork_result = fork();
	

	// fork()가 정상적으로 이루어지지 않았을 경우
	if(fork_result == -1)
	{
		fprintf(stderr, "Fork Failure\n");
		exit(EXIT_FAILURE);
	}


	if(fork_result == 0) // 자식 프로세스
	{
		// execlp 함수를 사용하여 Reader 파일을 호출해준다.
		execlp("./Reader","Reader",(char *)0);	
	}
	
	else
	{
		//시그널 핸들러 등록
		signal(SIGUSR1, sig_handler);
		signal(SIGUSR2, sig_handler);		
		

		// fork()의 리턴값은 
		//   - 정상적으로 되지 않았을 때 : -1
		//   - 자식프로세스일 때 : 0
		//   - 부모프로세스일 때 : 자식프로세스의 pid
		my_pid = getpid();
		your_pid = fork_result;
		
		
		// 부모 프로세스에서 자신pid, 자식pid 출력
		puts("[부모 프로세스]");
		printf("My_pid=%d	Your_pid=%d\n",my_pid,your_pid);	

		
		// 자식 프로세스에서의 정상적인 pid출력이 완료될 때까지 대기
		pause();


		// Shared Memory 생성, 실패한다면 에러메시지 출력 
		shmid = shmget((key_t)9134, sizeof(int) * SHMSIZE, 0666|IPC_CREAT);
		if(shmid == -1)
		{
			fprintf(stderr, "shmget failed\n");
			exit(EXIT_FAILURE);
		}


		// Shared Memory attach, 실패한다면 에러메시지 출력
		shared_Mem = shmat(shmid, (void *)0, 0);
		if(shared_Mem == (void *)-1)
		{
			fprintf(stderr, "shmat failed\n");
			exit(EXIT_FAILURE);
		}


		// shmaddr에 Shared Memory 시작주소값 저장,
		// shared_Mem이 void형이므로 int형 데이터의 주소값 계산을 위해 
		// int형으로 강제 형변환
		shmaddr = (int *) shared_Mem;


		// Shared Memory에 데이터 입력
		for(i=1; i<=1024;i++)
		{
			*(shmaddr + (i-1)) = i;
		}	

		
		// Write가 완료되었음을 자식프로세스에게 알린다.
		kill(your_pid,SIGUSR1);


		// 자식이 공유메모리의 내용을 backup.txt에 저장할 때까지 대기한다.
		pause();
		
		
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

	}
	


	exit(EXIT_SUCCESS);
}
