#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "db.h"
#include "edge.h"

#include "../lib/gflags/src/gflags/gflags.h"

DEFINE_string(hoge2, "", "default language");
DEFINE_string(moge2, "japanese", "default language");

using namespace std;
using namespace boost;
using namespace boost::filesystem;

int db_init(const std::string& db_dir)
{
  cout << "moge2 = " << FLAGS_moge2 << endl;

  path d(db_dir);
  create_directory(d);
  
  edge_init(db_dir);
}

int db_tmp_init(const std::string& db_dir)
{
  

}

