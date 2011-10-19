/* tltext web app main file.
   Copyright (C) 2011 Jarryd Beck

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#include "tltext.hpp"

#include <iostream>
#include <sstream>

#include <signal.h>
#include <sys/time.h>

//This runs the main web app interface to tltext.
//We are using the POST method for html.

//we'll set up tltext to read and write to and from
//i/ostringstream
//then output appropriately

//the timeout handler function
void
timeoutHandler(int sig)
{
  std::cout << "CPU time limit exceeded" << std::endl;
  exit(1);
}

// Parse character as hex digit.
// Returns -1 on error.
int hex2dec(char hex) {
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    if (hex >= 'A' && hex <= 'F')
        return 10 + hex - 'A';
    if (hex >= 'a' && hex <= 'f')
        return 10 + hex - 'a';
    return -1;
}

// Return a URI-unescaped version of s.
// Unparseable %xx sequences are dropped.
std::string unescape(const std::string& s) 
{
  std::string result;
  result.reserve(s.length());
  for 
  (
    std::string::const_iterator it = s.begin();
    it != s.end();
    it++
  ) 
  {
    if (*it == '%') 
    {
      if (s.end() - it >= 2) 
      {
        int a = hex2dec(*++it);
        int b = hex2dec(*++it);
        if (a >= 0 && b >= 0)
        {
          result += (char)(a * 16 + b);
        }
      }
    } 
    else if (*it == '+') 
    {
        result += ' ';
    } 
    else
    {
      result += *it;
    }
  }
  return result;
}

void htmlHead()
{
  std::cout << "Content-type: text/html; charset=UTF-8\r\n\r\n";
  std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  std::cout << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
    " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" 
    << std::endl;
  std::cout 
    << "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>" << std::endl 
    << "<title>TLWeb</title></head><body>" << std::endl;
}

//converts newlines to <br /> and outputs the rest as is
void htmlOut(const std::string& s)
{
  for (char c : s)
  {
    if (c == '\n')
    {
      std::cout << "<br />";
    }
    else
    {
      std::cout << c;
    }
  }
}

std::string
defaultProgram()
{
  std::string prog =
    "var f = 42;;\n"
    "%%\n"
    "f;;\n"
  ;

  return prog;
}

void
printForm(const std::string& program)
{
  std::cout <<
    "<h1>Input program</h1>"
    "<form action=\"tlweb\" method=\"post\">\n"
    "<p>\n"
    "<textarea cols=\"80\" rows=\"30\" name=\"program\">\n"
    << program <<
    "</textarea>\n"
    "<br />\n"
    "<input type=\"submit\" />\n"
    "</p>\n"
    "</form>";
}

int main(int argc, char* argv[])
{
  htmlHead();

  //get the input length
  char* lengthstr = getenv("CONTENT_LENGTH");
  if (lengthstr == nullptr)
  {
    std::cout << "<h1>Run TransLucid on the Web</h1>" << std::endl;
    printForm(defaultProgram());
  }
  else
  {

    int length = atoi(lengthstr) + 1;

    std::unique_ptr<char[]> inbuf(new char[length]);
    std::cin.get(inbuf.get(), length);

    std::istringstream input(inbuf.get());

    std::map<std::string, std::string> invars;

    while (!input.eof())
    {
      std::string var;
      std::string data;
      std::getline(input, var, '=');
      std::getline(input, data, '&');

      invars.insert(std::make_pair(var, unescape(data)));
    }

    auto prog = invars.find("program");
    if (prog == invars.end())
    {
      std::cout << "No program specified" << std::endl;
      exit(1);
    }

    std::cout << "<h1>Output</h1>" << std::endl;

    std::ostringstream os;

    TransLucid::TLText::TLText tl("TLWeb...");

    std::istringstream progstream(prog->second);
    tl.set_input(&progstream);
    tl.set_output(&os);
    tl.set_error(&os);

    //kill the program if it runs too long
    signal(SIGVTALRM, timeoutHandler);
    itimerval timer{{0, 0}, {5, 0}};
    setitimer(ITIMER_VIRTUAL, &timer, 0);

    tl.add_header(WEBPREFIX "/share/tl/tltext/header.tl");

    tl.run();

    std::cout << "<p>";
    htmlOut(os.str());
    std::cout << "</p>";

    printForm(prog->second);
  }

  std::cout << "</body></html>" << std::endl;

  return 0;
}
