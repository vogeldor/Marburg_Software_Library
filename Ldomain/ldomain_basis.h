// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of WaveletTL - the Wavelet Template Library      |
// |                                                                    |
// | Copyright (c) 2002-2006                                            |
// | Thorsten Raasch, Manuel Werner                                     |
// +--------------------------------------------------------------------+

#ifndef _WAVELETTL_LDOMAIN_BASIS_H
#define _WAVELETTL_LDOMAIN_BASIS_H

#include <list>

#include <algebra/vector.h>
#include <algebra/infinite_vector.h>
#include <utils/fixed_array1d.h>
#include <utils/multiindex.h>

#include <Ldomain/ldomain_index.h>

// for convenience, include also some functionality
// #include <Ldomain/ldomain_support.h>
#include <Ldomain/ldomain_evaluate.h>

using std::list;
using MathTL::FixedArray1D;
using MathTL::InfiniteVector;

namespace WaveletTL
{
  /*!
    Template class for composite wavelet bases over the L-shaped domain
      (-1,1)^2 \ (0,1)^2
    such that the primal and the dual basis fulfill homogeneous Dirichlet b.c.'s
    on the outer domain boundary.

    References:
    [DS] Dahmen, Schneider:
         Composite Wavelet Bases for Operator Equations
	 Math. Comput. 68 (1999), 1533-1567
  */
  template <class IBASIS>
  class LDomainBasis
  {
  public:
    //! default constructor
    LDomainBasis();

    //! interval basis
    typedef IBASIS IntervalBasis;

    //! coarsest possible level j0
    inline const int j0() const { return basis1d_.j0(); }
    
    //! wavelet index class
    typedef LDomainIndex<IBASIS> Index;

    //! critical Sobolev regularity for the primal generators/wavelets
    static double primal_regularity() { return IBASIS::primal_regularity(); }
    
    //! number of vanishing moments for the primal wavelets
    static unsigned int primal_vanishing_moments() { return IBASIS::primal_vanishing_moments(); }

    //! number of vanishing moments for the dual wavelets
    static unsigned int dual_vanishing_moments() { return IBASIS::dual_vanishing_moments(); }

    //! read access to the underlying 1D basis
    const IntervalBasis& basis1d() const { return basis1d_; }

    //! RECONSTRUCT routine, simple version
    /*!
      Constructs for a given single wavelet index lambda a coefficient set c,
      such that
      \psi_lambda = \sum_{\lambda'}c_{\lambda'}\psi_{\lambda'}
      where always |\lambda'|>=j
    */
    void reconstruct_1(const Index& lambda, const int j,
		       InfiniteVector<double, Index>& c) const;

    //! RECONSTRUCT routine, full version
    /*!
      Constructs for a given coefficient set c another one v,
      such that
      \sum_{\lambda}c_\lambda\psi_lambda = \sum_{\lambda'}v_{\lambda'}\psi_{\lambda'}
      where always |\lambda'|>=j
    */
    void reconstruct(const InfiniteVector<double, Index>& c, const int j,
		     InfiniteVector<double, Index>& v) const;

    //! dual RECONSTRUCT routine, simple version
    /*!
      Constructs for a given single wavelet index lambda a coefficient set c,
      such that
      \tilde\psi_lambda = \sum_{\lambda'}c_{\lambda'}\tilde\psi_{\lambda'}
      where always |\lambda'|>=j
    */
    void reconstruct_t_1(const Index& lambda, const int j,
			 InfiniteVector<double, Index>& c) const;

    //! dual RECONSTRUCT routine, full version
    /*!
      Constructs for a given coefficient set c another one v,
      such that
      \sum_{\lambda}c_\lambda\tilde\psi_\lambda = \sum_{\lambda'}v_{\lambda'}\tilde\psi_{\lambda'}
      where always |\lambda'|>=j
    */
    void reconstruct_t(const InfiniteVector<double, Index>& c, const int j,
		       InfiniteVector<double, Index>& v) const;

  protected:
    //! the interval wavelet basis
    IntervalBasis basis1d_;
  };
}

#include <Ldomain/ldomain_basis.cpp>

#endif
