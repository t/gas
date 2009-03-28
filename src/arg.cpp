#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <google/gflags.h>
#include <glog/logging.h>

using namespace std;

vector<string> g_argvs;

int init_arg(int argc, char *argv[])
{
  char *home(getenv("HOME"));
  if(home != NULL){
    string option_file = string(home) + "/.gas";
    FILE *fp = fopen(option_file.c_str(), "r");
    if(fp != NULL){
      fclose(fp);
      google::ReadFromFlagsFile(option_file, "gas", false);
    }
  }
  google::ParseCommandLineFlags(&argc, &argv, true);

  for(int i = 0; i < argc; i++){
    g_argvs.push_back( string(argv[i]) );
  }
}

vector<string>& get_argvs()
{
  return g_argvs;
}


