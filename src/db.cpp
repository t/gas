#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "db.h"
#include "edge.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

int db_init(const std::string& db_dir)
{
  path d(db_dir);
  create_directory(d);
  
  edge_init(db_dir);
}

int db_tmp_init(const std::string& db_dir)
{
  

}

