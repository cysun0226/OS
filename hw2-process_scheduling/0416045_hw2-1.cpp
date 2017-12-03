// OS hw2 process scheduling
// by 0416045 cysun

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>


using namespace std;

void print_result(int p_num, vector<int> waitingTime,vector<int> turnaroundTime)
{
	std::cout << std::left << setw(10) << "Process" << "Waiting Time\t" << "Turnaround Time" << endl;

  for (size_t i = 1; i <= p_num; i++)
  {
	  if (i < 10)
			cout << "P[" << i << std::left << setw(7) << "]" << waitingTime[i] << "\t\t" << turnaroundTime[i] <<endl;
		else if ( i < 100)
			cout << "P[" << i << std::left << setw(6) << "]" << waitingTime[i] << "\t\t" << turnaroundTime[i] <<endl;
		else
			cout << "P[" << i << std::left << setw(5) << "]" << waitingTime[i] << "\t\t" << turnaroundTime[i] <<endl;
  }


  // avg time
  int total_wait = 0, total_turnaround = 0;
  float avg_waiting, avg_turnaround;
  for (size_t i = 0; i <= p_num ; i++)
  {
    total_wait += waitingTime[i];
    total_turnaround += turnaroundTime[i];
  }
  avg_waiting = (float)total_wait / p_num;
  avg_turnaround = (float)total_turnaround / p_num;

  cout << endl;
  std::cout << "Average waiting time : " << avg_waiting << endl;
  std::cout << "Average turnaround time : " << avg_turnaround << endl;
}

int main()
{
  char input_name[] = "Q1.txt";
	fstream in_p;
	in_p.open(input_name, ifstream::in);

  int p_num;
  in_p >> p_num;

  std::vector<int> burstTime, waitingTime, turnaroundTime;
  burstTime.push_back(0);
  waitingTime.push_back(0);
  turnaroundTime.push_back(0);

  for (size_t i = 1; i <= p_num; i++)
  {
    int tmp;
    in_p >> tmp;
    burstTime.push_back(tmp);
  }

  // scheduling
  int cur_time = 0;
  for (size_t i = 1; i <= p_num; i++)
  {
    // process now be served
    waitingTime.push_back(cur_time);

    // add process's burst time to cur_time
    cur_time += burstTime[i];

    // process finish
    turnaroundTime.push_back(cur_time);
  }

  // print result
  print_result(p_num, waitingTime, turnaroundTime);

  return 0;
}
