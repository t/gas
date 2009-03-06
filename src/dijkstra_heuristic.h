#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;

class DijkstraHeuristic
{
public:
  double operator()(uint source, uint target, uint current){
    return 0;
  }
};
