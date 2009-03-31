#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <strstream>
#include <algorithm>

#include "modularityq.h"
#include "btree.h"
#include "edge.h"
#include "db.h"

using namespace std;

class Subtree{
public:
  uint64_t  node;
  uint64_t  idx;
  int       height;       // 0ならleaf
  double    a;
  double    q;
  Subtree * child_left;
  Subtree * child_right;
  bool      deleted;
};

struct qitem{
  double uq;
  Subtree * i;
  Subtree * j;
};

bool operator > (const qitem &a, const qitem &b) {
  return a.uq > b.uq ;
}

bool greater_qitem( const qitem &a, const qitem &b ){
	return a.uq > b.uq ;
}

struct ClusteringInfo{
  map<pair<uint64_t, uint64_t>, double> eij;
  vector<Subtree*>                      subtrees;    
  double                                total_length;
  uint64_t                              max_idx;
  vector<qitem>                         uqs;
  double                                q;
};

bool delete_qitem(const qitem &a)
{
  return (a.i->deleted || a.j->deleted);
}

bool delete_eij(const pair<pair<uint64_t, uint64_t>, double > &a)
{
  return false;
  //return (a.first.first->deleted || a.first.second->deleted);
}

double get_length(ClusteringInfo & ci, Subtree * i, Subtree * j)
{
  pair<uint64_t, uint64_t> key = make_pair(i->idx, j->idx);
  map<pair<uint64_t, uint64_t>, double >::const_iterator it =  ci.eij.find(key);
  if(it != ci.eij.end()){
    return (*it).second;
  }
  return 0;
}

double get_e(ClusteringInfo & ci, Subtree * i, Subtree * j)
{
  return get_length(ci, i, j) / ci.total_length;
}

void calc_a(ClusteringInfo & ci, Subtree * i)
{
  double total = 0;
  for(vector<Subtree *>::const_iterator j = ci.subtrees.begin(); j < ci.subtrees.end(); ++j){
    total += get_e(ci, i, ( * j));
  }
  i->a = total;
}

void join(ClusteringInfo & ci, Subtree * i, Subtree * j, int height, double q)
{
  Subtree * n = new Subtree();
  n->node        = 100;
  n->idx         = ci.max_idx;
  n->child_left  = i;
  n->child_right = j; 
  n->height      = height;
  n->deleted     = false;

  //leftとrightを削除をnを追加
  i->deleted = true;
  j->deleted = true;
  vector<Subtree *>::iterator r = remove(ci.subtrees.begin(), ci.subtrees.end(), i);
  ci.subtrees.erase( r, ci.subtrees.end() );
  r = remove(ci.subtrees.begin(), ci.subtrees.end(), j);
  ci.subtrees.erase( r, ci.subtrees.end() );

  ci.uqs.erase( std::remove_if( ci.uqs.begin(), ci.uqs.end(), delete_qitem ), ci.uqs.end());
  //_eij.erase( std::remove_if( _eij.begin(), _eij.end(), delete_eij ), _eij.end());

  // eijを再計算
  ci.eij[make_pair(n->idx,n->idx)] = get_length(ci, i,i) + get_length(ci, j,j) + get_length(ci, i,j) * 2;
  for(vector<Subtree *>::const_iterator k = ci.subtrees.begin(); k < ci.subtrees.end(); ++k){
    double edge_count = get_length(ci, i,(*k)) + get_length(ci, j,(*k));
    
    ci.eij[make_pair(n->idx,(*k)->idx)] = edge_count;
    ci.eij[make_pair((*k)->idx,n->idx)] = edge_count;	
  }

  //_ea.clear();
  ci.subtrees.push_back(n);
  ci.max_idx++;
  calc_a(ci, n);

  for(vector<Subtree *>::const_iterator k = ci.subtrees.begin(); k < ci.subtrees.end(); ++k){
    if( (* k) != n){
      double uq = 2 * ( get_e(ci, n, (* k)) - n->a * (*k)->a );
      if(uq >= 0){
        qitem item;
	item.i  = n;
	item.j  = (*k);
	item.uq = uq;

	//vector<qitem>::iterator idx = lower_bound(_uqs.begin(), _uqs.end(),item, greater_qitem);
	//_uqs.insert(idx, item);
	ci.uqs.push_back(item);
      }
    }
  }
}
	
void print_community(Subtree * n)
{
  //cout << n->idx << "(" << n->height << ")" << "," ;
  if(n->height == 0){
    cout << "      - " << n->node << endl;
  }else{
    print_community(n->child_left);
    print_community(n->child_right);
  }
}

void print_result(ClusteringInfo & ci)
{
  int num = 0;
  cout << "result:" << endl;
  cout << "  q: " << ci.q << endl;
  cout << "  clusters:" << endl;
  for(vector<Subtree *>::const_iterator i = ci.subtrees.begin() ; i < ci.subtrees.end() ; ++i){
    num++;
    cout << "    " << num << ":" << endl;
    print_community(( *i));
  }
}
	
double get_q(ClusteringInfo & ci)
{
  double q = 0;
  for(vector<Subtree *>::const_iterator i = ci.subtrees.begin(); i < ci.subtrees.end(); ++i){
    double a = (*i)->a; 
    q += get_e(ci, (* i), (* i)) - a * a;
  }
  return q;
}

int init( ClusteringInfo & ci )
{
  LOG(INFO) << "newman clustering is starting";

  Btree<uint64_t, Edge> * btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  false);

  LOG(INFO) << "newman clustering is initializing";

  ci.total_length = 0;
  ci.max_idx      = 0;

  set<uint64_t> added;
  Btree<uint64_t, Edge>::iterator end = btree->end(); 
  for(Btree<uint64_t, Edge>::iterator i = btree->begin(); i != end; i++) {
    LOG(INFO) << "key = " << i->key << " to = " << i->to;

    Subtree * n_a;
    Subtree * n_b;

    vector<uint64_t> new_subtrees;
    new_subtrees.push_back(i->key);
    new_subtrees.push_back(i->to);
    for(vector<uint64_t>::iterator j = new_subtrees.begin(); j < new_subtrees.end(); j++){
      set<uint64_t >::iterator f = added.find((*j));
      if(f == added.end()){
        Subtree * n = new Subtree();
        n->node      = (*j);
        n->idx       = ci.max_idx;
        n->height    = 0;
        n->q         = 0;
        n->deleted   = false;
        ci.subtrees.push_back(n);
        ci.max_idx++;
        added.insert((*j));
        LOG(INFO) << "add: " << (*j); 
      }
    }

    // eijを登録する
    ci.eij[make_pair(i->key,i->to)] += i->length;
    //ci.eij[make_pair(i->to,i->key)] += i->length;

    ci.total_length += i->length;
    //ci.total_length += i->length * 2;
  }

  for(vector<Subtree*>::iterator i = ci.subtrees.begin(); i < ci.subtrees.end(); i++){
    calc_a(ci, (*i));
  }

  LOG(INFO) << "c: node size is "    << ci.subtrees.size();
  LOG(INFO) << "c: total length is " << ci.total_length; 

  LOG(INFO) << "newman clustering finished.";
}

int clustering( ClusteringInfo & ci)
{
  ci.q = get_q(ci);

  for(vector<Subtree *>::const_iterator i = ci.subtrees.begin() ; i < ci.subtrees.end() - 1; ++i){
    for(vector<Subtree *>::const_iterator j = i + 1; j < ci.subtrees.end(); ++j){
      double uq = 2 * ( get_e(ci, (*i), (*j)) - (*i)->a * (*j)->a );
      if(uq >= 0){
        qitem item;
        item.i  = (*i);
        item.j  = (*j);
        item.uq = uq;
        ci.uqs.push_back(item);
      }
    }
  }

  int height = 0;
  while(ci.subtrees.size() > 1){
    height++;

    if(height % 20 == 19){
      //cout << "clear eij" << endl;
      map<pair<uint64_t, uint64_t>, double>::iterator itr = ci.eij.begin();
      while (itr != ci.eij.end()) {
        if (delete_eij( (*itr) ) ) {
          ci.eij.erase(itr++);
        }else{
          ++itr;
        }
      }
    }

    // 一番q値が大きくなる組み合わせを探す
    double  max_uq = -1;
    Subtree *  max_i;
    Subtree *  max_j;
    bool    max_uq_not_set = true;
			
    // sort版
			
    sort( ci.uqs.begin(), ci.uqs.end(), greater_qitem );
    for(vector<qitem>::const_iterator i = ci.uqs.begin(); i < ci.uqs.end(); ++i){
      if((*i).i->deleted == false && (*i).j->deleted == false){
        max_uq = (*i).uq;
        max_i  = (*i).i;
        max_j  = (*i).j;
        max_uq_not_set = false;
        //cout << "(x) max_uq: " << max_uq << " i: " << max_i << " j: " << max_j << endl;
        break;
      }
    }
			
    //cout << "(x) max_uq: " << max_uq << " i: " << max_i << " j: " << max_j << endl;
			
    if(max_uq < 0){
      LOG(INFO) << "finish.";
      LOG(INFO) << "node size is " << ci.subtrees.size();
  
      print_result(ci);
      return 1;
    }

    // JOIN!
    ci.q += max_uq;
    join(ci, max_i, max_j, height, ci.q);

    LOG(INFO) << "q is " << ci.q;
    //cout << "recalc( " << get_q() << " )" << endl;
  }
 
  print_result(ci);

  LOG(INFO) << "fin" << endl;
}

int modularityq_clustering()
{
  ClusteringInfo ci;

  init(ci);
  clustering(ci);
}

int modularityq()
{
  modularityq_clustering();
}

