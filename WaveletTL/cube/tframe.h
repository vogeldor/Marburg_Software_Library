// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of WaveletTL - the Wavelet Template Library      |
// |                                                                    |
// | Copyright (c) 2002-2009                                            |
// | Ulrich Friedrich, Alexander Sieber                                 |
// +--------------------------------------------------------------------+

#ifndef _WAVELETTL_TFRAME_H_
#define _WAVELETTL_TFRAME_H_
#define _TFRAME_DEBUGLEVEL_  1 // more output, more tests

/*
 * replace online computation of first_ and last_ wavelet indices by precomputation
 * This shifts work from the CPU to the Memory. 
 * Its faster in simple tests. Not sure about Programs that use a lot of memory.
 * 1 == precomputation is active
 */
//#define _PRECOMPUTE_FIRSTLAST_WAVELETS 1 

#include <list>

#include <algebra/infinite_vector.h>
#include <utils/fixed_array1d.h>
#include <utils/multiindex.h>
#include <cube/tframe_index.h>
#include <utils/function.h>
#include <geometry/point.h>
#include <utils/array1d.h>

#include <numerics/gauss_data.h>

// for convenience, include also some functionality
//#include <cube/tframe_support.h>

using std::list;
using MathTL::Point;
using MathTL::Function;
using MathTL::FixedArray1D;
using MathTL::MultiIndex;
using MathTL::InfiniteVector;
using MathTL::Array1D;

namespace WaveletTL
{
    /*
     * Template class for tensor product wavelet frames on the d-dimensional unit cube [0,1]^d (or mapped versions thereof) of the type
     * (V_0 \oplus W_0 \oplus W_1 \oplus W_2 \oplus ...)\times (V_0 \oplus W_0 \oplus W_1 \oplus W_2 \oplus ...)
     * For each of the 2*d facets, you can specify the orders s_i, sT_i of the Dirichlet boundary conditions of
     * the primal and dual frame in the normal direction (this is tailored for the DSFrame constructor...).
     */
    template <class IFRAME, unsigned int DIM = 2>
    class TensorFrame
    {
    public:
    	//! Interval frame
    	typedef IFRAME IntervalFrame;
        
        //! quarklet index class
        typedef TensorFrameIndex<IntervalFrame,DIM> Index;
        typedef typename Index::level_type level_type;
        typedef typename Index::polynomial_type polynomial_type;
        typedef typename Index::type_type type_type;
       
        
        //! Default constructor (no b.c.'s)
//        TensorFrame();
        
        //! copy constructor
//        TensorFrame(const TensorFrame<IFRAME, DIM>& other);

        /* Constructor with specified boundary condition orders
         * i-th direction at x=0 <-> index 2*i
         * i-th direction at x=1 <-> index 2*i+1
         */
        /* works only with DS Frame, not with PFrame
    	TensorFrame(const FixedArray1D<int,2*DIM>& s, const FixedArray1D<int,2*DIM>& sT);
         */

    	/*
    	 * Constructor with specified boundary condition orders
    	 * i-th direction at x=0 <-> index 2*i
    	 * i-th direction at x=1 <-> index 2*i+1
    	 */
//    	TensorFrame(const FixedArray1D<int,2*DIM>& s);

    	/*
    	 * Constructor with specified Dirichlet boundary conditions for
    	 * the primal functions, the dual functions will be constructed to
    	 * fulfill free b.c.'s
    	 */
//    	TensorFrame(const FixedArray1D<bool,2*DIM>& bc);

    	/*
    	 * Constructor with precomputed instances of the 1D frames;
    	 * in this case, the pointers are reused and
    	 * not deleted at destruction time.
    	 */
        TensorFrame(const FixedArray1D<IntervalFrame*,DIM> frames);

    	//! Destructor
    	~TensorFrame();

    	//! Coarsest possible level j0
    	inline const level_type& j0() const { return j0_; }
        
        /*
    	 * Geometric type of the support sets
         * (j,a,b) <-> 2^{-j_1}[a_1,b_1]x...x2^{-j_DIM}[a_n,b_DIM]
    	 */
    	typedef struct {
      		int j[DIM];
      		int a[DIM];
      		int b[DIM];
    	} Support;
        
        /*
         * For a given interval frame IFRAME, compute a cube
         * 2^{-j_}<a_,b_> = 2^{-j_1}[a_1,b_1]x...x2^{-j_n}[a_n,b_n]
         * which contains the support of a single primal cube generator
         * or wavelet psi_lambda.
         */
    	void support(const Index& lambda, Support& supp) const;
        
        //! compute the support of psi_lambda, using the internal cache; only works for precomputed supports
        void support(const int& lambda_num, Support& supp) const;
        
        /*
    	 * Critical Sobolev regularity for the primal generators/wavelets.
    	 * We assume the same regularity in each dimension.
    	 */
    	static double primal_regularity() { return IFRAME::primal_regularity(); }

    	/*
    	 * Degree of polynomial reproduction for the primal generators/wavelets
    	 * We assume the same polynomial degree in each dimension.
    	 * */
    	static unsigned int primal_polynomial_degree() { return IFRAME::primal_polynomial_degree(); }

    	/*
    	 * Number of vanishing moments for the primal wavelets.
    	 * We assume the same number of vanishing moments in each dimension.
    	 */
    	static unsigned int primal_vanishing_moments() { return IFRAME::primal_vanishing_moments(); }

        //! Read access to the frames
    	const FixedArray1D<IFRAME*,DIM>& frames() const { return frames_; }

        //! size of Delta_j
        const int Deltasize(const int j) const;
        
    	//! Index of first generator 
//    	Index first_generator(const typename Index::polynomial_type p = typename Index::polynomial_type()) const;

        /*! 
         * For compatibility reasons, returns the same (!) as first_generator()
         * parameter j is ignored !
         */
//        Index first_generator(const unsigned int j, const typename Index::polynomial_type p) const;
        Index first_generator(const level_type& j, const polynomial_type& p = polynomial_type()) const;

        //! Index of last generator on level j0
    	Index last_generator(const level_type& j, const polynomial_type& p = polynomial_type()) const;

    	/*! 
         * Index of first quarklet on level j >= j0.
         * Method does not check, whether j is valid, i.e., if j-j0 is nonnegative
         */
    	Index first_quarklet(const level_type& j, const polynomial_type& p=polynomial_type(), const int& number=-1) const;
        
        //! Index of first quarklet on level j >= ||j0||_1
//        Index first_quarklet(const int levelsum, const typename Index::polynomial_type p = typename Index::polynomial_type()) const;

    	/*! 
         * Index of last quarklet on sublevel j >= j0.
         * Method does not check, whether j is valid, i.e., if j-j0 is nonnegative
         */
    	Index last_quarklet(const level_type& j, const polynomial_type& p = polynomial_type(), const int& number=-1) const;

        //! Index of last quarklet on level j >= ||j0||_1
    	Index last_quarklet(const int& levelsum, const polynomial_type& p = polynomial_type(), const int& number=-1) const;
        
        /*!
      Evaluate a single primal generator or quarklet \psi_\lambda
      on a dyadic subgrid of the L-shaped domain
        */
        SampledMapping<DIM> 
        evaluate
        (const typename TensorFrame<IntervalFrame,DIM>::Index& lambda,
            const int resolution) const;
    
        /*!
       Evaluate an arbitrary linear combination of primal/dual quarklets
          on a dyadic subgrid of the L-shaped domain
        */
        SampledMapping<DIM> 
        evaluate
        (const InfiniteVector<double, typename TensorFrame<IntervalFrame,DIM>::Index>& coeffs,
         const int resolution) const;

        
//    	void set_jpmax(const MultiIndex<int,DIM> jmax, const MultiIndex<int,DIM> pmax) {
//      		jmax_ = multi_degree(jmax);
//                pmax_ =multi_degree(pmax);
//      		setup_full_collection();
//#if _PRECOMPUTE_FIRSTLAST_WAVELETS
//                precompute_firstlast_wavelets();
//#endif
//    	}
        
        void set_jpmax(const int jmax, const int pmax=0) {
      		jmax_ = jmax;
                pmax_ = pmax;
//#if _PRECOMPUTE_FIRSTLAST_WAVELETS
//                precompute_firstlast_wavelets();
//#endif
                setup_full_collection();
    	}

        inline const unsigned int get_jmax() const {
            return jmax_;
        }
        
        inline const unsigned int get_pmax() const {
            return pmax_;
        }
        
        inline const int get_Nablasize() const {
            return Nablasize_;
        }
    
        inline const Array1D<int> get_first_wavelet_numbers() const {
            return first_wavelet_numbers;
        }
    
        inline const Array1D<int> get_last_wavelet_numbers() const {
            return last_wavelet_numbers;
        }

    	//! Get the wavelet index corresponding to a specified number
    	const inline Index* get_quarklet (const int number) const {
      		return &full_collection[number];
    	}
        
        inline const bool get_setup_full_collection() const {
            return setup_full_collection_;
        }
        
        const inline Support& get_support (const int number) const {
            return all_supports_[number];
        }
    	
        //! Number of wavelets between coarsest and finest level
    	const int degrees_of_freedom() const { return full_collection.size(); };
        
        //alternative for SupportCache
        Array1D<Support> all_supports_;

    	
        
    	
//#if _PRECOMPUTE_FIRSTLAST_WAVELETS
//        /*
//         * Compute the indices of the first and last wavelet for all wavelet levels up to
//         * \|\lambda\| == jmax. Indices are stored. 
//         * first/last_wavelet routines do not compute indices any more
//         */
//        void precompute_firstlast_wavelets();
//#endif
        
    	/*!
    	 * For a given function, compute all integrals w.r.t. the primal
    	 * or dual generators/wavelets \psi_\lambda with |\lambda|\le jmax.
    	 * - When integrating against the primal functions, the integrand has to be smooth
    	 *   to be accurately reproduced by the dual frame.
    	 * - When integration against dual functions is specified,
    	 *   we integrate against the primal ones instead and multiply the resulting
    	 *   coefficients with the inverse of the primal gramian.
    	 *
    	 * Maybe a thresholding of the returned coefficients is helpful (e.g. for
    	 * expansions of spline functions).
         * 
         * For the special case that f is a tensor a faster routine can be developed.
    	 */
//    	void expand(const Function<DIM>* f,
//                    const bool primal,
//                    const MultiIndex<int,DIM> jmax,
//                    InfiniteVector<double,Index>& coeffs) const;

        /*!
         * As above, but with integer max level
         */

//        void expand(const Function<DIM>* f,
//                    const bool primal,
//                    const unsigned int jmax,
//                    InfiniteVector<double,Index>& coeffs) const;
    	/*
    	 * Helper function, integrate a smooth function f against a
    	 * primal generator or wavelet
    	 */
//    	double integrate(const Function<DIM>* f,
//                         const Index& lambda) const;

    	//! Point evaluation of (derivatives of) primal generators or wavelets \psi_\lambda
//    	double evaluate(const unsigned int derivative,
//                        const Index& lambda,
//                        const Point<DIM> x) const;

    	
        
        


    	
        

    protected:
        /*
    *  Collection of first and last wavelet numbers on all levels up to jmax
    *  Precomputed for speedup
    */
        Array1D<int> first_wavelet_numbers, last_wavelet_numbers;
    
        //! Coarsest possible level j0
    	level_type j0_;
    	//int j0_[DIM];
        
        //! Finest possible level jmax
    	//MultiIndex<int,DIM> jmax_;
        // quarklet indices with \|level\|\leq jmax_ are stored in full_collection
    	int jmax_;
        
        // quarklet indices with \|polynomial\|\leq pmax_ are stored in full_collection
    	int pmax_;
        
        /*!
         * Compute all wavelet indices between beginning at j0_ and the last_wavelet 
         * with \|level\|\leq jmax_, i.e.,
         * degrees_of_freedom = last_wavelet_num<IFRAME,DIM,TensorFrame<IFRAME,DIM> >(this, jmax_) +1
         * many wavelet indices are computed. They are stored in full_collection
         * This codes the mapping \N -> wavelet_indices
         */
    	void setup_full_collection();
    
        //! setup_full_collection_ is set to 1 after initialising full collection
        bool setup_full_collection_;
    
    	//! Collection of all wavelets between coarsest and finest level
    	Array1D<Index> full_collection;
        
        /*
    	 * The instances of the 1D frames
    	 */
    	list<IFRAME*> frames_infact;

    	//! For faster access, all relevant pointers to the 1D frames
    	FixedArray1D<IFRAME*,DIM> frames_;
        
        //! Degrees of freedom on level p=(0,0) 
        int Nablasize_;
        
        
//#if _PRECOMPUTE_FIRSTLAST_WAVELETS
//        /*
//         *  Collection of first and last wavelet indices on all levels up to jmax
//         *  Precomputed for speedup
//         */
//        Array1D<Index> first_wavelets, last_wavelets;
//#endif
        
    	//! Flag for deletion of the pointers
    	bool delete_pointers;
        
        
        bool precomputed_supports_;
  	};
}

#include <cube/tframe.cpp>

#endif /*_WAVELETTL_TFRAME_H_*/
