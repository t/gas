#ifndef LANDMARK_DIST_H_
#define LANDMARK_DIST_H_

struct LandmarkDist{
  public:
  uint64_t key;
  uint64_t landmark;
  bool     forward;
  double   length;
};

#endif
