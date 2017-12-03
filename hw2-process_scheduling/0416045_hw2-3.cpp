// OS hw2 process scheduling
// by 0416045 cysun

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <stack>

struct Process
{
	int pid;
	int arrivalTime;
	int burstTime;
	int remainTime;
	int waitingTime;
	int turnaroundTime;
};

using namespace std;

void swap(Process &a, Process &b)
{
    Process tmp;
    tmp = a;
    a = b;
    b = tmp;
}

int cmp_arrival_time(const Process &a, const Process &b)
{
    return a.arrivalTime < b.arrivalTime;
}

int cmp_pid(const Process &a, const Process &b)
{
    return a.pid < b.pid;
}

int cmp_burst_time(const Process &a, const Process &b)
{
    if (a.burstTime != b.burstTime)
        return a.burstTime < b.burstTime;
	else
        return a.pid < b.pid;
}

int cmp_remain_time(const Process &a, const Process &b)
{
    if (a.remainTime != b.remainTime)
        return a.remainTime < b.remainTime;
	else
        return a.pid < b.pid;
}

void print_result(vector<Process> result)
{
    sort(result.begin(), result.end(), cmp_pid);
    std::cout << std::left << setw(10) << "Process" << "Waiting Time\t" << "Turnaround Time" << endl;

	for (size_t i = 0; i < result.size(); i++)
	{
		if (i < 10-1)
			cout << "P[" << result[i].pid << std::left << setw(7) << "]" << result[i].waitingTime << "\t\t" << result[i].turnaroundTime << endl;
		else if ( i < 100-1)
			cout << "P[" << result[i].pid << std::left << setw(6) << "]" << result[i].waitingTime << "\t\t" << result[i].turnaroundTime << endl;
		else
			cout << "P[" << result[i].pid << std::left << setw(5) << "]" << result[i].waitingTime << "\t\t" << result[i].turnaroundTime << endl;
	}

  // avg time
  int total_wait = 0, total_turnaround = 0;
  float avg_waiting, avg_turnaround;
  for (size_t i = 0; i < result.size() ; i++)
  {
    total_wait += result[i].waitingTime;
    total_turnaround += result[i].turnaroundTime;
  }
  avg_waiting = (float)total_wait / result.size();
  avg_turnaround = (float)total_turnaround / result.size();

  cout << endl;
  std::cout << "Average waiting time : " << avg_waiting << endl;
  std::cout << "Average turnaround time : " << avg_turnaround << endl;
}

int main()
{
  char input_name[] = "Q3.txt";
	fstream in_p;
	in_p.open(input_name, ifstream::in);

  int p_num;
  in_p >> p_num;

  std::vector<Process> process, arrival, arrived, result;
  // std::vector<int> burstTime, arrivalTime, waitingTime, turnaroundTime;

  for (size_t i = 0; i < p_num; i++)
  {
      Process p;
      p.pid = i + 1;
      in_p >> p.arrivalTime;
      process.push_back(p);
      arrival.push_back(p);
  }

  // calculate total burst time
  int total_time = 0;

  for (size_t i = 0; i < p_num; i++)
  {
      in_p >> process[i].burstTime;
	  process[i].remainTime = process[i].burstTime;
	  process[i].waitingTime = -1;
      total_time += process[i].burstTime;
  }

  // sort
  sort(process.begin(), process.end(), cmp_arrival_time);

  // scheduling
  int cur_time = 0;
  int finish = 0;
  int arr_pid = 0;
  int last_arr;

  while (finish < p_num)
  {
      // find the processes that has arrived
	  last_arr = arrived.size();
	  int id = (arr_pid == p_num) ? p_num - 1 : arr_pid;
	  while (arrived.empty() && process[id].arrivalTime > cur_time)
	  {
		  cur_time++;
	  }

      while (process[id].arrivalTime <= cur_time && arr_pid <= p_num - 1)
      {
          arrived.push_back(process[arr_pid]);
		  if (arr_pid < p_num - 1)
		  {
			  arr_pid++;
			  id = (arr_pid == p_num) ? p_num - 1 : arr_pid;
		  }
		  else
		  {
			  arr_pid++;
			  break;
		  }

      }

      // sort by the remainTime time
	  sort(arrived.begin(), arrived.end(), cmp_remain_time);

      // execute the process
      int start_time = cur_time;
      cur_time ++;
	  arrived[0].remainTime--;

	  // if done
	  if (arrived[0].remainTime == 0)
	  {
		  arrived[0].turnaroundTime = cur_time - arrived[0].arrivalTime;
		  arrived[0].waitingTime = cur_time - arrived[0].arrivalTime - arrived[0].burstTime;
          finish++;

		  // remove the process from arrived and push into result
	      result.push_back(arrived[0]);
	      swap(arrived[0], arrived[arrived.size()-1]);
	      arrived.pop_back();
	  }
  }

  // print result
  print_result(result);

  return 0;
}
