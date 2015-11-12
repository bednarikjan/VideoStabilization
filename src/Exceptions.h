// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#include <exception>
#include <string>

using namespace std;

struct Error : public std::exception
{
   string msg;
   Error(string _msg) : msg(_msg) {}
   ~Error() throw () {}
   const char* what() const throw() { return msg.c_str(); }
};

struct Usage : public std::exception
{
	Usage() {}
   ~Usage() throw () {}
   const char* what() const throw() { return "Usage: TODO"; }
};
