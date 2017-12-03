#include <iostream>
#include <sstream>
using namespace std;


int main(int argc, char const *argv[])
{
  istringstream ss1(argv[1]);
  float n1;
  ss1 >> n1;

  istringstream ss2(argv[2]);
  float n2;
  ss2 >> n2;

  cout << "Speed up: " << n1 / n2 << endl;
  return 0;
}
