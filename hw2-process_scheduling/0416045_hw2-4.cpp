// OS hw2 process scheduling
// by 0416045 cysun

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <queue>

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
		if (i < 10 - 1)
			cout << "P[" << result[i].pid << std::left << setw(7) << "]" << result[i].waitingTime << "\t\t" << result[i].turnaroundTime << endl;
		else if (i < 100 - 1)
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
  char input_name[] = "Q4.txt";
	fstream in_p;
	in_p.open(input_name, ifstream::in);

  int p_num;
  in_p >> p_num;

  std::vector<Process> process, arrival, arrived, result, SJF;
  // std::vector<int> burstTime, arrivalTime, waitingTime, turnaroundTime;

  for (size_t i = 0; i < p_num; i++)
  {
      Process p;
      p.pid = i + 1;
      in_p >> p.arrivalTime;
      process.push_back(p);
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

  // time quantum for each queue
  int high_tq, medium_tq;
  in_p >> high_tq;
  in_p >> medium_tq;

  // sort
  sort(process.begin(), process.end(), cmp_arrival_time);

  // queue
  queue<Process> highPriority, mediumPriority;

  // scheduling
  int cur_time = 0;
  int finish = 0;
  int arr_pid = 0;

  while (result.size() < p_num)
  {
      // find the processes that new arrived
	  int id = (arr_pid == p_num) ? p_num - 1 : arr_pid;
	  while (highPriority.empty() && mediumPriority.empty() && SJF.empty() && process[id].arrivalTime > cur_time)
	  {
		  cur_time++;
	  }

      while (process[id].arrivalTime <= cur_time && arr_pid <= p_num - 1)
      {
		  // push the new arrived process into the queue
		  highPriority.push(process[arr_pid]);
		  if (arr_pid < p_num - 1)
		  {
			  arr_pid++;
			  id = (arr_pid == p_num) ? p_num - 1 : arr_pid;
		  }
		  else
		  {
			  arr_pid++;
			 // highPriority.push(process[arr_pid-1]);
		  }
      }

	  /* execute process */
	  // high priority queue
	  if(!highPriority.empty())
	  {
		  Process p = highPriority.front();

		  if (p.remainTime <= high_tq) // finish in one time quantum
		  {
			  cur_time += p.remainTime;
			  p.remainTime = 0;
			  p.waitingTime = cur_time - p.arrivalTime - p.burstTime;
			  p.turnaroundTime = cur_time - p.arrivalTime;
			  result.push_back(p);
		  }
		  else // can't finish in one time quantum, move to medium queue
		  {
			  cur_time += high_tq;
			  p.remainTime -= high_tq;
			  mediumPriority.push(p);
		  }

		  highPriority.pop();
	  }

	  // medium priority
	  else if (!mediumPriority.empty())
	  {
		  Process p = mediumPriority.front();

		  if (p.remainTime <= medium_tq) // finish in one time quantum
		  {
			  cur_time += p.remainTime;
			  p.remainTime = 0;
			  p.waitingTime = cur_time - p.arrivalTime - p.burstTime;
			  p.turnaroundTime = cur_time - p.arrivalTime;
			  result.push_back(p);
		  }
		  else // can't finish in one time quantum, move to SJF
		  {
			  cur_time += medium_tq;
			  p.remainTime -= medium_tq;

			  SJF.push_back(p);
		  }
		  mediumPriority.pop();
	  }

	  // third layer (SJF)
	  else if (!SJF.empty())
	  {
		  // sort by the remainTime time
		  sort(SJF.begin(), SJF.end(), cmp_remain_time);

		  // execute the process
		  Process p = SJF[0];

		  int start_time = cur_time;
		  cur_time += p.remainTime;
		  p.remainTime = 0;
		  p.turnaroundTime = cur_time - p.arrivalTime;
		  p.waitingTime = cur_time - p.arrivalTime - p.burstTime;
		  result.push_back(p);
		  swap(SJF[0], SJF[SJF.size() - 1]);
		  SJF.pop_back();
	  }
  }

  // print result
  print_result(result);

  return 0;
}
