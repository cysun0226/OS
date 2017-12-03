#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <algorithm>
#include <cstring>

using namespace std;

#define SIZE 800
#define MSG 0

struct timeval start, end;

void show_matrix(int* shmaddr, int d)
{
	for (int r = 0; r < d; r++)
	{
		for (int c = 0; c < d; c++)
		{
			cout << shmaddr[r*d+c] << ' ';
		}
		cout << endl;
	}
}

// unsigned int mul_matrix(int* )

unsigned int checksum(int* shmaddr, int d) 
{
    unsigned int sum = 0;
    
    for (int i = 0; i < d*d; i++)
	{
		sum += *(shmaddr+i);
	}
    
    return sum;
}

int main()
{
	int di = 0; //dimemsion
	int A_shmid, B_shmid, C_shmid, C2_shmid;
    int *A_shmaddr, *B_shmaddr, *C_shmaddr, *C2_shmaddr;
    clock_t start, finish;

    
    /* shmget for three matrix */
    // A
    A_shmid = shmget(IPC_PRIVATE, sizeof(int)*SIZE*SIZE, IPC_CREAT|0600 );
    if( A_shmid < 0)
    {
    	cout << "failed to get shm." << endl;
    	return -1;
    }
    
    A_shmaddr = (int *)shmat(A_shmid, NULL, 0);
    
    //B
    B_shmid = shmget(IPC_PRIVATE, sizeof(int)*SIZE*SIZE, IPC_CREAT|0600 );
    if( B_shmid < 0)
    {
    	cout << "failed to get shm." << endl;
    	return -1;
    }

    B_shmaddr = (int *)shmat(B_shmid, NULL, 0);
    
    // C
    C_shmid = shmget(IPC_PRIVATE, sizeof(int)*SIZE*SIZE, IPC_CREAT|0666 );
    if( C_shmid < 0)
    {
    	cout << "failed to get shm." << endl;
    	return -1;
    }

    C_shmaddr = (int *)shmat(C_shmid, NULL, 0);
    
    // C2
    C2_shmid = shmget(IPC_PRIVATE, sizeof(int)*SIZE*SIZE, IPC_CREAT|0666 );
    if( C2_shmid < 0)
    {
    	cout << "failed to get shm." << endl;
    	return -1;
    }

    C2_shmaddr = (int *)shmat(C2_shmid, NULL, 0);

	cout <<  "Dimemsion: ";
	cin >> di;

	// initialize A & B
	int i = 0;
	for (int r = 0; r < di; r++)
    {
		for (int c = 0; c < di; c++)
        { 
            *(A_shmaddr+i) = i;
            *(B_shmaddr+i) = i;
            i++;
        }
	}
    
    
    /* 1-process */ 
    // gettimeofday(&start, 0);
    start = clock();

	// multiply A and B
    unsigned int sum = 0;
    
    for (int i=0; i<di; i++) 
    {
        for (int j=0; j<di; j++) 
        {
            C_shmaddr[i*di + j] = 0;
            
            for(int k = 0; k < di; k++) 
            {
                C_shmaddr[i*di + j] = C_shmaddr[i*di+j] +A_shmaddr[i*di+k] * B_shmaddr[k*di+j];
            }
        }
    }
    
	// unsigned int sum = 0;
	sum = checksum(C_shmaddr, di);
    
    // finish time
    finish = clock();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;

	cout << "1-process, checksum = " << sum << endl;
	cout << "elapsed " << duration << " s" << endl;
    
    /* 4-process */
    int d0, d1, d2, d3;
    pid_t pid;
    pid = getpid();
    sum = 0;
    
    
    // start
    start = clock();
    
    d0 = di/4;
    d1 = d0*2;
    d2 = d0*3;
    d3 = di;
    
    pid = fork();
    
    // fork 1
    if( pid == 0 )
    {
        pid = getpid();
        if(MSG) cout << "pid " << pid << " mul 0~d0 " << endl;
        for (int i=0; i<d0; i++) 
        {
            for (int j=0; j<di; j++) 
            {
                C2_shmaddr[i*di + j] = 0;
                
                for(int k = 0; k < di; k++) 
                {
                    C2_shmaddr[i*di + j] = C2_shmaddr[i*di+j] +A_shmaddr[i*di+k] * B_shmaddr[k*di+j];
                    // S_shmaddr[0] += C_shmaddr[i*di + j];
                }
            }
        }
        if(MSG) cout << "mul 0~d0 finish" << endl;
        exit(0);
    }
    
    // fork 2
    pid = fork();
    if(pid == 0)
    {
        pid = getpid();
        if(MSG) cout << "pid " << pid << " mul d0~d1" << endl;
        for (int i=d0; i<d1; i++) 
        {
            for (int j=0; j<di; j++) 
            {
                C2_shmaddr[i*di + j] = 0;
                
                for(int k = 0; k < di; k++) 
                {
                    C2_shmaddr[i*di + j] = C2_shmaddr[i*di+j] +A_shmaddr[i*di+k] * B_shmaddr[k*di+j];
                    // S_shmaddr[1] += C_shmaddr[i*di + j];
                }
            }
        }
        if(MSG) cout << "mul d0~d1 finish" << endl;
        exit(0);
    }

    // fork 3
    pid = fork();
    if(pid == 0)
    {
        pid = getpid();
        if(MSG) cout << "pid " << pid << " mul d1~d2" << endl;
        for (int i=d1; i<d2; i++) 
        {
            for (int j=0; j<di; j++) 
            {
                C2_shmaddr[i*di + j] = 0;
                
                for(int k = 0; k < di; k++) 
                {
                    C2_shmaddr[i*di + j] = C2_shmaddr[i*di+j] +A_shmaddr[i*di+k] * B_shmaddr[k*di+j];
                    // S_shmaddr[2] += C_shmaddr[i*di + j];
                }
            }
        }
        if(MSG) cout << "mul d1~d2 finish" << endl;
        exit(0);
    }
    
    if(MSG) cout << "main process, d2~d3" << endl;
    for (int i=d2; i<d3; i++) 
    {
        for (int j=0; j<di; j++) 
        {
            C2_shmaddr[i*di + j] = 0;
            
            for(int k = 0; k < di; k++) 
            {
                C2_shmaddr[i*di + j] = C2_shmaddr[i*di+j] +A_shmaddr[i*di+k] * B_shmaddr[k*di+j];
                // S_shmaddr[3] += C_shmaddr[i*di + j];
            }
        }
    }
    if(MSG) cout << "d2~d3 finish" << endl;
    
    /* wait for child process*/
    int status;
    pid = wait(&status);
    if(MSG) cout << pid << " is finish" << endl;
    pid = wait(&status);
    if(MSG) cout << pid << " is finish" << endl;
    pid = wait(&status);
    if(MSG) cout << pid << " is finish" << endl;
    
    sum = 0;
    sum = checksum(C2_shmaddr, di);
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    
    // unsigned int Msum;
    // Msum = S_shmaddr[0]+S_shmaddr[1]+S_shmaddr[2]+S_shmaddr[3];
    
    cout << "4-process, checksum = " << sum << endl;
    // cout << "4-process, checksum(4sum) = " << Msum << endl;
	cout << "elapsed " << duration << " s" << endl;
    
    shmctl(A_shmid, IPC_RMID, NULL);
    shmctl(B_shmid, IPC_RMID, NULL);
    shmctl(C_shmid, IPC_RMID, NULL);
    shmctl(C2_shmid, IPC_RMID, NULL);
	
	return 0;
}
