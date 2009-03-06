#ifndef SHORTESTPATH_TREE_H_
#define SHORTESTPATH_TREE_H_

#include <map>
#include <string>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

template <class Heuristic> class QItem {
public:
  uint64_t node;
  double   dist;
  uint     hop;
  double   heuristic;
  double   potential;
  uint64_t parent;
  bool    is_error;
  QItem(){}
  QItem(uint64_t node, double dist, uint hop, double heuristic, uint64_t parent, double parent_potential) :
    node(node), dist(dist), hop(hop), heuristic(heuristic), parent(parent)
  {
    potential = dist + heuristic;
    is_error  = false;
    
    if(potential < parent_potential)
    {
      std::cout << "error: potential = " << potential << "  parent_potential = " << parent_potential << std::endl;
      is_error = true;
    }
  }
};

template <class Heuristic> class VItem {
public:
  uint64_t node;
  uint64_t parent;
  double   dist;
  uint     hop;
  VItem(){}
  VItem(uint64_t node, uint64_t parent, double dist, uint hop) :
    node(node), parent(parent), dist(dist), hop(hop) {}
};

template <class Heuristic> bool operator < (const QItem<Heuristic> &e, const QItem<Heuristic> &f) {
  return e.potential != f.potential ? e.potential > f.potential : e.node < f.node;
};

template <class Heuristic> class ShortestpathTree{
private:
  Btree<uint64_t, Edge>                 * _graph;
  std::priority_queue<QItem<Heuristic> >  _queue;
  std::map<uint64_t, VItem<Heuristic> >   _visited;
  std::map<uint64_t, double>              _labeled;
  uint64_t                                _source;
  uint64_t                                _target;
  Heuristic                             * _h;
  uint                                    _trycount;
public:
  ShortestpathTree(Btree<uint64_t, Edge> * graph, uint64_t source, uint64_t target, Heuristic * h)
  {
     _graph         = graph;
     _source        = source;
     _target        = target;
     _h             = h;
     _queue.push(QItem<Heuristic>(source, 0, 0, 0, 0, 0));
     _trycount      = 0;
  }

  bool next(VItem<Heuristic> &result, bool show_visit = false)
  {
     _trycount++;

     bool ret = false;
     while(! _queue.empty())
     {
        QItem<Heuristic> q = _queue.top();
        _queue.pop();

        typename std::map<uint64_t, VItem<Heuristic> >::iterator v = _visited.find(q.node);
        if(v != _visited.end()) continue;

        if(show_visit)
          std::cout << "visit\t" << q.node << std::endl;

        ret = true;
        result = VItem<Heuristic>(q.node, q.parent, q.dist, q.hop);
        _visited[q.node] = result;

        std::vector<Edge> result = _graph->find(q.node);
        for(std::vector<Edge>::iterator i = result.begin(); i < result.end(); i++)
        {
           //typename std::map<uint64_t, VItem<Heuristic> >::iterator to_v = _visited.find((*i).to);
           //if(to_v != _visited.end()) continue;
           double h = (* _h)(_source, _target, (*i).to);
           double potential = q.dist + (*i).length + h;
           typename std::map<uint64_t, double>::iterator labeled = _labeled.find((*i).to);
           if(labeled == _labeled.end() || labeled->second > potential){
             _labeled[(*i).to] = potential;
             QItem<Heuristic> new_item = QItem<Heuristic>( (*i).to, q.dist + (*i).length, q.hop + 1, h, q.node, q.potential);
             if(new_item.is_error)
             {
               //double p_h = (* _h)(_source, _target, (*i).key);
               //abort();
             }else{
               _queue.push(new_item);
             }
           }
        }
        if(ret) return true;
     }
     return false;
  }

  uint tryCount()
  {
    return _trycount;
  }

  double dist(uint64_t node)
  {
    typename std::map<uint64_t, VItem<Heuristic> >::iterator v = _visited.find(node);
    if(v != _visited.end())
    {
      return v->second.d;
    }else{
      return 0;
    }
  }

  uint64_t parent(uint64_t node)
  {
    typename std::map<uint64_t, VItem<Heuristic> >::iterator v = _visited.find(node);
    if(v != _visited.end())
    {
      return v->second.parent;
    }else{
      return 0;
    }
  }

  bool scaned(uint64_t node)
  {
    typename std::map<uint64_t, VItem<Heuristic> >::iterator v = _visited.find(node);
    return (v != _visited.end());
  }

};

# endif
