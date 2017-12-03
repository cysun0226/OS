/* OS hw1 */
// by 0416045

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <sys/wait.h>

using namespace std;

int main()
{
	pid_t pid, p_pid;
	int status;
	pid = getpid();
	cout << "Main Process ID : " <<  pid << endl << endl;

	// fork 1
	int fork_num = 1;
	p_pid = pid;
	pid = fork();

	if (pid == 0)
	{
		pid_t c_pid = 0;
		pid = getpid();
		cout << "Fork " << fork_num << ". I'm the child " << pid << ", my parent is " << p_pid << '.' << endl;

		
		// fork 2-1
		fork_num++;
		p_pid = pid;
		pid = fork();
		
		if( pid == 0 )
		{
			pid = getpid();
			cout << "Fork " << fork_num << ". I'm the child " << pid << ", my parent is " << p_pid << '.' << endl;

			p_pid = pid;
			c_pid = 1;
			// usleep(10);
		}

		// fork 2-2
		if( c_pid == 0)
		{
			pid = fork();

			if(pid == 0)
			{
				pid = getpid();
				cout << "Fork " << fork_num << ". I'm the child " << pid << ", my parent is " << p_pid  << '.' << endl;	
				p_pid = pid;
				c_pid = pid;
				// usleep(1);
			}
		}

		// fork 3
		pid = getpid();
		p_pid = pid;

		if(c_pid != 0)
			pid = fork();

		if(pid == 0)
		{
			fork_num++;
			c_pid = getpid();
			cout << "Fork " << fork_num << ". I'm the child " << c_pid << ", my parent is " << p_pid << '.' << endl;
            
			exit(0);
		}
        
        if(c_pid != 0)
        {
            pid = getpid();
            c_pid = wait(&status);
            exit(0);
        }
            

        pid = getpid();
	   c_pid = wait(&status);
        
        c_pid = wait(&status);
        
	   exit(0);
	}

    pid = wait(&status);

	return 0;
}
