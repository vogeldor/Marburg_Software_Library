// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of WaveletTL - the Wavelet Template Library      |
// |                                                                    |
// | Copyright (c) 2002-2005                                            |
// | Thorsten Raasch                                                    |
// +--------------------------------------------------------------------+

#ifndef _WAVELETTL_GALERKIN_UTILS_H
#define _WAVELETTL_GALERKIN_UTILS_H

#include <set>

#include <algebra/sparse_matrix.h>
#include <algebra/vector.h>

using MathTL::SparseMatrix;
using MathTL::Vector;

namespace WaveletTL
{
  /*!
    Setup the (sparse, preconditioned) stiffness matrix for a given problem and a given active
    index set Lambda.
  */
  template <class PROBLEM>
  void setup_stiffness_matrix(const PROBLEM& P,
			      const std::set<typename PROBLEM::WaveletBasis::Index>& Lambda,
			      SparseMatrix<double>& A_Lambda); 
  
  /*!
    Setup the (preconditioned) right-hand side for a given problem and a given active
    index set Lambda.
  */
  template <class PROBLEM>
  void setup_righthand_side(const PROBLEM& P,
			    const std::set<typename PROBLEM::WaveletBasis::Index>& Lambda,
			    Vector<double>& F_Lambda); 
}

#include <galerkin/galerkin_utils.cpp>

#endif
