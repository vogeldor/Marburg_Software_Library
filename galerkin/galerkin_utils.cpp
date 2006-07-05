// implementation for galerkin_utils.h

namespace WaveletTL
{
  template <class PROBLEM>
  void setup_stiffness_matrix(const PROBLEM& P,
			      const std::set<typename PROBLEM::Index>& Lambda,
			      SparseMatrix<double>& A_Lambda)
  {
    A_Lambda.resize(Lambda.size(), Lambda.size());
    
    typedef typename SparseMatrix<double>::size_type size_type;

    size_type row = 0;
    typedef typename PROBLEM::Index Index;
    for (typename std::set<Index>::const_iterator it1(Lambda.begin()), itend(Lambda.end());
	 it1 != itend; ++it1, ++row)
      {
	const double d1 = P.D(*it1);
	std::list<size_type> indices;
	std::list<double> entries;

	size_type column = 0;
#if _WAVELETTL_GALERKINUTILS_VERBOSITY >= 1
	cout << "setup_stiffness_matrix(): doing row " << row << " of " << Lambda.size()
	     << " (wavelet index " << *it1 << ")" << endl;
#endif
	for (typename std::set<Index>::const_iterator it2(Lambda.begin());
	     it2 != itend; ++it2, ++column)
	  {
// 	    if (intersect_singular_support(P.basis(), *it1, *it2)) {
	    const double entry = P.a(*it2, *it1);
#if _WAVELETTL_GALERKINUTILS_VERBOSITY >= 2
 	    if (entry != 0) {
 	      cout << " column: " << *it2 <<  ", value " << entry << endl;
 	    }
#endif
	    if (entry != 0) {
		indices.push_back(column);
		entries.push_back(entry / (d1 * P.D(*it2)));
	      }
// 	    }
	  }
	A_Lambda.set_row(row, indices, entries);
      }
  }

  template <class PROBLEM>
  void setup_righthand_side(const PROBLEM& P,
			    const std::set<typename PROBLEM::Index>& Lambda,
			    Vector<double>& F_Lambda)
  {
    F_Lambda.resize(Lambda.size());

    typedef typename SparseMatrix<double>::size_type size_type;

    size_type row = 0;
    typedef typename PROBLEM::Index Index;
    for (typename std::set<Index>::const_iterator it(Lambda.begin()), itend(Lambda.end());
	 it != itend; ++it, ++row)
      F_Lambda[row] = P.f(*it)/P.D(*it);
  }
}
