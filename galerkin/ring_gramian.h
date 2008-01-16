// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of WaveletTL - the Wavelet Template Library      |
// |                                                                    |
// | Copyright (c) 2002-2008                                            |
// | Thorsten Raasch, Manuel Werner                                     |
// +--------------------------------------------------------------------+

#ifndef _WAVELETTL_RING_GRAMIAN_H
#define _WAVELETTL_RING_GRAMIAN_H

#include <set>
#include <utils/fixed_array1d.h>
#include <utils/array1d.h>
#include <algebra/infinite_vector.h>
#include <algebra/vector.h>
#include <adaptive/compression.h>
#include <ring/ring_basis.h>

using MathTL::InfiniteVector;

namespace WaveletTL
{
  /*!
    This class models the "polar coordinate" Gramian matrix
    over a ring-shaped domain, to be used in adaptive algorithms.
  */
  template <int d, int dt, int s0, int s1>
  class RingGramian
  {
  public:
    //! wavelet basis class
    typedef typename WaveletTL::RingBasis<d,dt,s0,s1> WaveletBasis;

    //! wavelet index class
    typedef typename WaveletBasis::Index Index;
    
    //! index type of vectors and matrices
    typedef typename Vector<double>::size_type size_type;
    
    //! constructor from a given wavelet basis and a given right-hand side y
    RingGramian(const WaveletBasis& basis,
		const InfiniteVector<double,Index>& y);

    //! read access to the basis
    const WaveletBasis& basis() const { return basis_; }
    

    //! space dimension of the problem
    static const int space_dimension = 2;

    //! identity operator is local
    static bool local_operator() { return true; }

    //! (half) order t of the operator
    static double operator_order() { return 0; }

    //! evaluate the diagonal preconditioner D (essentially, we don't need any)
    double D(const Index& lambda) const { return 1; }

    //! evaluate the (unpreconditioned) bilinear form a
    double a(const Index& lambda,
 	     const Index& nu) const;

    //! estimate the spectral norm ||A||
    double norm_A() const;

    //! estimate the spectral norm ||A^{-1}||
    double norm_Ainv() const;

    //! estimate compressibility exponent s^*
    double s_star() const {
      return WaveletBasis::primal_vanishing_moments();
    }
    
    /*!
      estimate the compression constants alpha_k in
      ||A-A_k|| <= alpha_k * 2^{-s*k}
    */
    double alphak(const unsigned int k) const {
      return 2*norm_A(); // suboptimal
    }

    //! evaluate the (unpreconditioned) right-hand side f
    double f(const Index& lambda) const {
      return y_.get_coefficient(lambda);
    }

    /*!
      approximate the wavelet coefficient set of the preconditioned right-hand side F
      within a prescribed \ell_2 error tolerance
    */
    void RHS(const double eta,
  	     InfiniteVector<double,Index>& coeffs) const {
      coeffs = y_; // dirty
    }

    //! compute (or estimate) ||F||_2
    double F_norm() const { return l2_norm(y_); }

    //! set right-hand side y
    void set_rhs(const InfiniteVector<double,Index>& y) const {
      y_ = y;
    }

  protected:
    const WaveletBasis& basis_;
    
    // rhs, mutable to have 'const' method
    mutable InfiniteVector<double,Index> y_;
    
    // estimates for ||A|| and ||A^{-1}||
    mutable double normA, normAinv;
  };
  
}

#include <galerkin/ring_gramian.cpp>

#endif
