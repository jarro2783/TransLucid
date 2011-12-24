#include <iostream>
#include <memory>
#include <sstream>
#include <map>
#include <fstream>

#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

void htmlHead()
{
  std::cout << "Content-type: text/html; charset=UTF-8\r\n\r\n";
  std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  std::cout << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
    " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" 
    << std::endl;
  std::cout 
    << "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>" << std::endl 
    << "<title>TLWeb Matrix multiply example</title></head><body>" 
    << std::endl;
}

std::string
htmlescape(const std::string& in)
{
  std::string out;

  for (char c : in)
  {
    if (c == '=')
    {
      out += "%3D";
    }
    else if (c == ' ')
    {
      out += "%20";
    }
    else if (c == '\n')
    {
      out += "%0A";
    }
    else if (c == '%')
    {
      out += "%25";
    }
    else if (c == '*')
    {
      out += "%2A";
    }
    else if (c == '+')
    {
      out += "%2B";
    }
    else
    {
      out += c;
    }
  }

  return out;
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

std::string
matrix_prog()
{
  return R"**(
var multiply = \d_r -> \d_c -> \k -> \\X -> \\Y -> W where
  dim d <- 0;;
  var Xr = rotate.d_c.d X;;
  var Yr = rotate.d_r.d Y;;
  var Z = Xr * Yr;;
  var W = sum.d.k Z;;

  var rotate = \d1 -> \d2 -> \\X -> X @ [d1 <- #!d2];;
  var sum = \dx -> \n -> \\X -> Y @ [dx <- n - 1] where
    var Y = fby.dx X (Y + next.dx X);;
  end;;
end;;
)**";
}

void
printForms()
{
  std::cout 
    << R"(<form action="matrixweb" method="post">)" << std::endl
    << "What are the dimensions of your output variable?<br>"
    << R"(Cols: <input type="text" name="outrows" size="4" value="3">)"
    << R"(Rows: <input type="text" name="outcols" size="4", value="3">)"
    << "<h2>A</h2>"
    << R"(<textarea rows="15" cols="30" name="Ain">
indexedby = {0,1};;
entries = {
{1, 2, 3},
{4, 5, 6},
{7, 8, 9}
;;
</textarea>)"
    << "<h2>B</h2>"
    << R"(<textarea rows="15" cols="30" name="Bin">
indexedby = {0,1};;
entries = {
{2, 0, 0},
0, 2, 0},
0, 0, 2}
;;
</textarea>)"
    << "<h2>Matrix Program</h2>"
    << R"(<textarea rows="15" cols="80" readonly="readonly" name="prog">)"
    << matrix_prog()
    << "</textarea>"
    << "<h2>Declarations</h2>"
    << R"(<textarea rows="15" cols="80" name="decls">
assign C [arg0 : 0..2, arg1 : 0..2, time : 0] := 
  multiply.arg0.arg1.3 A B;;
</textarea>)"
    << "<h2>Expressions</h2>"
    << R"(<textarea rows="15" cols="80" name="exprs">)"
    << "</textarea>"
    << R"(<br><input type="submit" value="Run">)"
    << "</form>" << std::endl;
}

//the io decls and the "your output is at ..." string
std::pair<std::string, std::string>
generate_io
(
  const std::string& Ain,
  const std::string& Bin,
  const std::string& numcols, 
  const std::string& numrows
)
{
  timeval tv;
  gettimeofday(&tv, nullptr);

  srandom(tv.tv_sec * 1000000 + tv.tv_usec);

  int aprefix = random();
  int bprefix = random();
  int outprefix = random();

  std::ostringstream aname;
  std::ostringstream bname;
  std::ostringstream outname;

  std::string dir = "../matrixhds/";

  aname << dir << aprefix << "_ainput";
  bname << dir << bprefix << "_binput";
  outname << dir << outprefix << "_output.txt";

  std::ostringstream output;

  output 
    << "in A = file_array_in_hd!\"" << aname.str() << "\";;\n"
    << "in B = file_array_in_hd!\"" << bname.str() << "\";;\n\n"
    << "out C [0 : 0.." << numcols << ", 1 : 0.." << numrows << 
       "] = file_array_out_hd!\"" << outname.str() << ";;\n\n";

  std::ostringstream outlink;

  outlink 
    << R"(`your output is at <a href=")" 
    << outname.str()
    << R"(">)"
    << outname.str()
    << "</a>`;;";

  //actually write A and B
  std::string anamestr = aname.str();
  std::string bnamestr = bname.str();

  std::ofstream Astream{anamestr.c_str()};
  std::ofstream Bstream{bnamestr.c_str()};

  Astream << Ain;
  Bstream << Bin;

  return std::make_pair(output.str(), outlink.str());
}

int main(int argc, char* argv[])
{
  umask(0002);

  //get the input length
  char* lengthstr = getenv("CONTENT_LENGTH");
  if (lengthstr == nullptr)
  {
    htmlHead();
    std::cout << "<h1>TLWeb Matrix Multiplication</h1>" << std::endl;
    std::cout 
      << "<p>Type your matrices into the forms, they will be "
         "accessible from the variables A and B in your program."
         " Integers should be separated by spaces, each line is a row of the"
         " matrix."
         "</p>"
      << std::endl;
    printForms();
    std::cout << "</body></html>" << std::endl;
  }
  else
  {
    //otherwise we're running tlweb and using its output
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

    auto decls = invars.find("decls");
    auto exprs = invars.find("exprs");
    auto Ain = invars.find("Ain");
    auto Bin = invars.find("Bin");
    auto prog = invars.find("prog");
    auto outcols = invars.find("outcols");
    auto outrows = invars.find("outrows");

    auto iodecls = 
      generate_io
      (
        Ain->second,
        Bin->second,
        outcols->second, 
        outrows->second
      );

    std::ostringstream programin;
    std::ostringstream tlwebin;
    
    programin << prog->second << '\n' << decls->second << '\n' 
      << iodecls.first
      << "%%\n" << exprs->second << iodecls.second;

    tlwebin << "program=" << htmlescape(programin.str());

    pid_t pid = fork();

    if (pid == 0)
    {
      //in the child

      //get the length of the input
      std::string input = tlwebin.str();
      std::ostringstream length;
      length << input.size();

      //redirect the input
      int fd[2];
      pipe(fd);

      dup2(fd[0], 0);

      std::string webin = tlwebin.str();
      write(fd[1], webin.c_str(), webin.size());

      close(fd[1]);

      //set the env args
      std::string lengthstr = length.str();
      setenv("CONTENT_LENGTH", lengthstr.c_str(), 1);

      setenv("SCRIPT_NAME", "tlweb", 1);

      //run tlweb
      execl("tlweb", "tlweb", nullptr);
    }
    else
    {
      waitpid(pid, nullptr, 0);
    }
  }

  return 0;
}
