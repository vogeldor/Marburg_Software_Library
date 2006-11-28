// implementation for full_laplacian.h

#include <list>
#include <map>

namespace WaveletTL
{
  template <int d, int dT>
  FullLaplacian<d,dT>::FullLaplacian(const SplineBasis<d,dT>& sb,
				     const PreconditioningType precond)
    : sb_(sb), precond_(precond)
  {
    set_level(sb_.j0());
  }

  template <int d, int dT>
  inline
  const typename FullLaplacian<d,dT>::size_type
  FullLaplacian<d,dT>::row_dimension() const
  {
    return sb_.Deltasize(j_);
  }  
  
  template <int d, int dT>
  inline
  const typename FullLaplacian<d,dT>::size_type
  FullLaplacian<d,dT>::column_dimension() const
  {
    return row_dimension(); // square
  }  

  template <int d, int dT>
  void
  FullLaplacian<d,dT>::set_level(const int j) const
  {
    assert(j >= sb_.j0());
    j_ = j;
    setup_D();
  }

  template <int d, int dT>
  void
  FullLaplacian<d,dT>::setup_D() const
  {
    D_.resize(sb_.Deltasize(j_));
    if (precond_ == no_precond) {
      D_ = 1.0;
    } else {
      if (precond_ == dyadic) {
	for (int k(0); k < sb_.Deltasize(sb_.j0()); k++)
	  D_[k] = (1<<sb_.j0());
	for (int j = sb_.j0(); j < j_; j++) {
	  for (int k(sb_.Deltasize(j)); k < sb_.Deltasize(j+1); k++)
	    D_[k] = (1<<j);
	}
      } else {
	for (size_type k(0); k < D_.size(); k++)
	  D_[k] = sqrt(diagonal(k));
      }
    }
  }
  
  template <int d, int dT>
  inline
  double
  FullLaplacian<d,dT>::D(const size_type k) const {
    return D_[k];
  }
  
  template <int d, int dT>
  const double FullLaplacian<d,dT>::get_entry(const size_type row, const size_type column) const
  {
    assert(row < row_dimension() && column < column_dimension());

#if 0
    Vector<double> ecol(column_dimension()), col(row_dimension());
    ecol[column] = 1.0;
    apply(ecol, col);

    return col[row];
#else
    std::map<size_type,double> ecol, col;
    ecol[column] = 1.0;
    apply(ecol, col);

    return col[row];
#endif
  }

  template <int d, int dT>
  const double
  FullLaplacian<d,dT>::diagonal(const size_type row) const
  {
    std::map<size_type,double> e_k; e_k[row] = 1.0;
    std::map<size_type,double> y;

    // determine corresponding level of "row"
    int jrow = sb_.j0();
    if (row >= (size_type) sb_.Deltasize(sb_.j0())) {
      jrow += 1+(int)floor(log(((double)(row-sb_.Deltasize(sb_.j0())))/(1<<sb_.j0())+1)/M_LN2);
    }
    
    // apply wavelet transformation y=T_{jrow-1}e_k
    // (does nothing if jrow==j0)
    if (jrow > sb_.j0())
      sb_.apply_Tj(jrow-1, e_k, y);
    else
      y.swap(e_k);
    
    // compute Ay (see "apply")
    std::map<size_type,double> Ay;
    const double factor = ldexp(1.0, 2*jrow); // not "1<<(2*j_)" !
    if (d == 2) {
      // apply 2^{2j}*tridiag(-1,2,-1)
      for (std::map<size_type,double>::const_iterator it(y.begin());
	   it != y.end(); ++it) {
	Ay[it->first] += factor * 2*it->second;
	if (it->first > 0)
	  Ay[it->first-1] -= factor * it->second;
	if (it->first < column_dimension()-1)
	  Ay[it->first+1] -= factor * it->second;
      }
    } else {
      if (d == 3) {
	// cf. [P, Bsp. 3.26]
	for (std::map<size_type,double>::const_iterator it(y.begin());
	     it != y.end(); ++it) {
	  const size_type m = row_dimension();
	  switch(it->first) {
	  case 0:
	    Ay[0] += factor * 4*it->second/3;
	    Ay[1] -= factor * it->second/6;
	    Ay[2] -= factor * it->second/6;
	    break;
	  case 1:
	    Ay[0] -= factor * it->second/6;
	    Ay[1] += factor * it->second;
	    Ay[2] -= factor * it->second/3;
	    Ay[3] -= factor * it->second/6;
	    break;
	  default: // >= 2
	    switch(m-1-it->first) {
	    case 0: // m-1
	      Ay[m-1] += factor * 4*it->second/3;
	      Ay[m-2] -= factor * it->second/6;
	      Ay[m-3] -= factor * it->second/6;
	      break;
	    case 1: // m-2
	      Ay[m-1] -= factor * it->second/6;
	      Ay[m-2] += factor * it->second;
	      Ay[m-3] -= factor * it->second/3;
	      Ay[m-4] -= factor * it->second/6;
	      break;
	    default: // < m-2
	      Ay[it->first-2] -= factor * it->second/6;
	      Ay[it->first-1] -= factor * it->second/3;
	      Ay[it->first]   += factor * it->second;
	      Ay[it->first+1] -= factor * it->second/3;
	      Ay[it->first+2] -= factor * it->second/6;
	      break;
	    }
	    break;
	  }
	}
      }
    }

    // compute inner product <y,Ay>
    double r = 0;
    for (typename std::map<size_type,double>::const_iterator
	   ity(y.begin()),
	   ityend(y.end()),
	   itAy(Ay.begin()),
	   itAyend(Ay.end());
	 ity != ityend && itAy != itAyend; ++itAy)
      {
 	while (ity != ityend && ity->first < itAy->first) ++ity;
	if (ity != ityend)
	  if (itAy->first == ity->first)
	    r += ity->second * itAy->second;
      }
    return r;
  }

  template <int d, int dT>
  template <class VECTOR>
  void FullLaplacian<d,dT>::apply(const VECTOR& x, VECTOR& Mx,
				  const bool preconditioning) const
  {
    assert(Mx.size() == row_dimension());

    VECTOR y(x);

    if (preconditioning) {
      // apply diagonal preconditioner D^{-1}
      for (size_type k(0); k < y.size(); k++)
	y[k] /= D(k);
    }

    // apply wavelet transformation T_{j-1}
    // (does nothing if j==j0)
    if (j_ > sb_.j0())
      sb_.apply_Tj(j_-1, y, Mx);
    else
      Mx.swap(y);

    // apply Laplacian w.r.t the B-Splines in V_j
    const double factor = ldexp(1.0, 2*j_); // not "1<<(2*j_)" !
    if (d == 2) {
      // apply 2^{2j}*tridiag(-1,2,-1)
      y[0] = factor * (2*Mx[0] - Mx[1]);
      const size_type m = row_dimension();
      y[m-1] = factor * (2*Mx[m-1]-Mx[m-2]);
      for (size_type i(1); i < m-1; i++)
	y[i] = factor * (2*Mx[i] - Mx[i-1] - Mx[i+1]);
    } else {
      if (d == 3) {
	// cf. [P, Bsp. 3.26]
	y[0] = factor * (4*Mx[0]/3 - Mx[1]/6 - Mx[2]/6);
	y[1] = factor * (-Mx[0]/6 + Mx[1] - Mx[2]/3 - Mx[3]/6);
	const size_type m = row_dimension();
	y[m-1] = factor * (-Mx[m-3]/6 - Mx[m-2]/6 + 4*Mx[m-1]/3);
	y[m-2] = factor * (-Mx[m-4]/6 - Mx[m-3]/3 + Mx[m-2] - Mx[m-1]/6);
	for (size_type i(2); i < m-2; i++)
	  y[i] = factor * (-Mx[i-2]/6 - Mx[i-1]/3 + Mx[i] - Mx[i+1]/3 - Mx[i+2]/6);
      }
    }
    
    // apply transposed wavelet transformation T_{j-1}^T
    // (does nothing if j==j0)
    if (j_ > sb_.j0())
      sb_.apply_Tj_transposed(j_-1, y, Mx);
    else
      Mx.swap(y);
    
    if (preconditioning) {
      // apply diagonal preconditioner D^{-1}
      for (size_type k(0); k < y.size(); k++)
	Mx[k] /= D(k);
    }
  }

  template <int d, int dT>
  void FullLaplacian<d,dT>::apply(const std::map<size_type,double>& x,
				  std::map<size_type,double>& Mx,
				  const bool preconditioning) const
  {
    std::map<size_type,double> y(x);

    // apply diagonal preconditioner D^{-1}
    for (std::map<size_type,double>::iterator it(y.begin()); it != y.end(); ++it)
      it->second /= D(it->first);

    // apply wavelet transformation T_{j-1}
    // (does nothing if j==j0)
    if (j_ > sb_.j0())
      sb_.apply_Tj(j_-1, y, Mx);
    else
      Mx.swap(y);

    // apply Laplacian w.r.t the B-Splines in V_j
    const double factor = ldexp(1.0, 2*j_); // not "1<<(2*j_)" !
    y.clear();
    if (d == 2) {
      // apply 2^{2j}*tridiag(-1,2,-1)
      for (std::map<size_type,double>::const_iterator it(Mx.begin());
	   it != Mx.end(); ++it) {
	y[it->first] += factor * 2*it->second;
	if (it->first > 0)
	  y[it->first-1] -= factor * it->second;
	if (it->first < column_dimension()-1)
	  y[it->first+1] -= factor * it->second;
      }
    } else {
      if (d == 3) {
	// cf. [P, Bsp. 3.26]
	for (std::map<size_type,double>::const_iterator it(Mx.begin());
	     it != Mx.end(); ++it) {
	  const size_type m = row_dimension();
	  switch(it->first) {
	  case 0:
	    y[0] += factor * 4*it->second/3;
	    y[1] -= factor * it->second/6;
	    y[2] -= factor * it->second/6;
	    break;
	  case 1:
	    y[0] -= factor * it->second/6;
	    y[1] += factor * it->second;
	    y[2] -= factor * it->second/3;
	    y[3] -= factor * it->second/6;
	    break;
	  default: // >= 2
	    switch(m-1-it->first) {
	    case 0: // m-1
	      y[m-1] += factor * 4*it->second/3;
	      y[m-2] -= factor * it->second/6;
	      y[m-3] -= factor * it->second/6;
	      break;
	    case 1: // m-2
	      y[m-1] -= factor * it->second/6;
	      y[m-2] += factor * it->second;
	      y[m-3] -= factor * it->second/3;
	      y[m-4] -= factor * it->second/6;
	      break;
	    default: // < m-2
	      y[it->first-2] -= factor * it->second/6;
	      y[it->first-1] -= factor * it->second/3;
	      y[it->first]   += factor * it->second;
	      y[it->first+1] -= factor * it->second/3;
	      y[it->first+2] -= factor * it->second/6;
	      break;
	    }
	    break;
	  }
	}
      }
    }
    
    // apply transposed wavelet transformation T_{j-1}^T
    // (does nothing if j==j0)
    if (j_ > sb_.j0())
      sb_.apply_Tj_transposed(j_-1, y, Mx);
    else
      Mx.swap(y);
    
    // apply diagonal preconditioner D^{-1}
    for (std::map<size_type,double>::iterator it(Mx.begin()); it != Mx.end(); ++it)
      it->second /= D(it->first);

    // remove unnecessary zeros
    for (typename std::map<size_type,double>::iterator it(Mx.begin()); it != Mx.end();) {
      if (it->second == 0)
	Mx.erase(it++);
      else
	++it;
    }  
  }

  template <int d, int dT>
  void
  FullLaplacian<d,dT>::to_sparse(SparseMatrix<double>& S) const
  {
    // we utilize that the stiffness matrix is symmetric
    const size_type m = row_dimension();
    S.resize(m, m);
    for (size_type row(0); row < m; row++) {
      std::map<size_type,double> x, Mx;
      x[row] = 1.0;
      apply(x, Mx);
      typename std::list<size_type> indices;
      std::list<double> entries;
      for (typename std::map<size_type,double>::const_iterator it(Mx.begin()); it != Mx.end(); ++it) {
	indices.push_back(it->first);
	entries.push_back(it->second);
      }
      S.set_row(row, indices, entries);
    }
  }
  
  template <int d, int dT>
  void FullLaplacian<d,dT>::print(std::ostream &os,
				  const unsigned int tabwidth,
				  const unsigned int precision) const
  {
    if (row_dimension() == 0)
      os << "[]" << std::endl; // Matlab style
    else
      {
	unsigned int old_precision = os.precision(precision);
	for (typename FullLaplacian<d,dT>::size_type i(0); i < row_dimension(); ++i)
	  {
	    for (typename FullLaplacian<d,dT>::size_type j(0); j < column_dimension(); ++j)
	      os << std::setw(tabwidth) << std::setprecision(precision)
		 << get_entry(i, j);
	    os << std::endl;
	  }
	os.precision(old_precision);
      }
  }

  template <int d, int dT>
  inline
  std::ostream& operator << (std::ostream& os, const FullLaplacian<d,dT>& M)
  {
    M.print(os);
    return os;
  }

}
