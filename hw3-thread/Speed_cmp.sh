#!/bin/bash
echo ""
read -p "Input a number of times to run: " T
printf "\nNon-thread run time:"

### Time Memo ###
#(s) second: date +%s
#(ms)millisecond: date +%s%3N
#(μs)microsecond: date +%s%6N
#(ns)nanosecond: date +%s%N

### Speed.sh

### example
g++ -std=c++11 -pthread $1 -o naive.out
start=`date "+%s%6N"`
./naive.out
printf "\n   Finished once."
for i in $(seq 2 ${T})
do
	./naive.out
done
end=`date "+%s%6N"`
t1=$((($end-$start)/$T))
printf "\n   Avg time: %d μs\n" ${t1}

### thread
printf "\nThread run time:"
g++ -std=c++11 -pthread $2 -o a.out
start=`date "+%s%6N"`
./a.out
printf "\n   Finished once."
for i in $(seq 2 ${T})
do
	./a.out
done
end=`date "+%s%6N"`
t2=$((($end-$start)/$T))
printf "\n   Avg time: %d μs\n" ${t2}

printf "\n----------\n"
./cal_speed.out ${t1} ${t2}

echo ""

exit
