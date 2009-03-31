/*
 This code is based on the following Java Code by Ricardo Baeza-Yates, et al. 
 http://www.yr-bcn.es/webspam/code/ 
 The Original code is distributed under GPL. Below text is Original Code's licence.

 -----------------

 Copyright (C) 2006 - Ricardo Baeza-Yates, Luca Becchetti, Carlos Castillo
    Debora Donato, Stefano Leonardi :: http://www.dis.uniroma1.it/~ae/
 This work was partially supported by EU IST-FET project 001907 (DELIS).

 Portions derived from the WebGraph framework.
 Copyright (C) 2003, 2004, 2005 - Paolo Boldi, Massimo Santini
                                     and Sebastiano Vigna

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _ADAPTABLEBITPROPAGATION_HPP_
#define _ADAPTABLEBITPROPAGATION_HPP_

#include "adjlist.h"
#include "HomogeneousBitPropagation.hpp"

class AdaptableBitPropagation : public HomogeneousBitPropagation {
public:
    AdaptableBitPropagation(Adjlist& outlink, const short width, const uint64_t seed);
    virtual double estimateSupporters(double ones, double epsilon);
    virtual double estimateSupporters(int ones, int last_ones, double multiplier);
    virtual int estimateIfAbove(int minOnes, double multiplier);
    virtual int estimateIfBelow(int maxOnes, double multiplier);
    virtual int estimateAll(double multiplier);
};

#endif
