// implementation for multi_laurent_polynomial.h

namespace MathTL
{
  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::MultivariateLaurentPolynomial()
    : InfiniteVector<R, MultiIndex<int, DIMENSION> >(), Function<DIMENSION, R>()
  {
  }

  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::MultivariateLaurentPolynomial
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
    : InfiniteVector<R, MultiIndex<int, DIMENSION> >(p)
  {
  }
  
  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::MultivariateLaurentPolynomial
  (const R value)
    : InfiniteVector<R, MultiIndex<int, DIMENSION> >(), Function<DIMENSION, R>()
  {
    set_coefficient(MultiIndex<int, DIMENSION>(), value);
  }

  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::MultivariateLaurentPolynomial
  (const LaurentPolynomial<R>& p, const unsigned int n)
    : InfiniteVector<R, MultiIndex<int, DIMENSION> >(), Function<DIMENSION, R>()
  {
    for (typename LaurentPolynomial<R>::const_iterator it(p.begin());
	 it != p.end(); ++it)
      {
	MultiIndex<int, DIMENSION> index;
	for (unsigned int i(0); i < DIMENSION; i++) index[i] = 0;
	index[n] = it.index();
	set_coefficient(index, *it);
      }
  }

  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::~MultivariateLaurentPolynomial()
  {
  }

  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator =
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::operator = (p);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator = (const R c)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::clear();
    set_coefficient(MultiIndex<int, DIMENSION>(), c);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  inline
  R MultivariateLaurentPolynomial<R, DIMENSION>::get_coefficient
  (const MultiIndex<int, DIMENSION>& k) const
  {
    return InfiniteVector<R, MultiIndex<int, DIMENSION> >::get_coefficient(k);
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::set_coefficient
  (const MultiIndex<int, DIMENSION>& k, const R coeff)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::set_coefficient(k, coeff);
  }

  template <class R, unsigned int DIMENSION>
  R MultivariateLaurentPolynomial<R, DIMENSION>::value
  (const Point<DIMENSION>& p,
   const unsigned int component) const
  {
    R r(0);

    // we don't care about complexity here:
    for (typename MultivariateLaurentPolynomial<R, DIMENSION>::const_iterator it(begin());
  	 it != end(); ++it)
      {
	R help(1);
	for (unsigned int i(0); i < DIMENSION; i++)
	  {
	    assert(p[i] != 0);
	    double xi = (it.index()[i] < 0 ? 1.0/p[i] : p[i]);
	    for (int m(1); m <= abs(it.index()[i]); m++)
	      help *= xi;
	  }

	r += *it * help;
      }

    return r;
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::vector_value
  (const Point<DIMENSION> &p,
   Vector<R>& values) const
  {
    values.resize(1, false);
    values[0] = value(p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  typename MultivariateLaurentPolynomial<R, DIMENSION>::const_iterator
  MultivariateLaurentPolynomial<R, DIMENSION>::begin() const
  {
    return InfiniteVector<R, MultiIndex<int, DIMENSION> >::begin();
  }

  template <class R, unsigned int DIMENSION>
  inline
  typename MultivariateLaurentPolynomial<R, DIMENSION>::const_iterator
  MultivariateLaurentPolynomial<R, DIMENSION>::end() const
  {
    return InfiniteVector<R, MultiIndex<int, DIMENSION> >::end();
  }  

  template <class R, unsigned int DIMENSION>
  inline
  typename MultivariateLaurentPolynomial<R, DIMENSION>::const_reverse_iterator
  MultivariateLaurentPolynomial<R, DIMENSION>::rbegin() const
  {
    return InfiniteVector<R, MultiIndex<int, DIMENSION> >::rbegin();
  }
  
  template <class R, unsigned int DIMENSION>
  inline
  typename MultivariateLaurentPolynomial<R, DIMENSION>::const_reverse_iterator
  MultivariateLaurentPolynomial<R, DIMENSION>::rend() const
  {
    return InfiniteVector<R, MultiIndex<int, DIMENSION> >::rend();
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::add
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::add(p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::add
  (const R s, const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::add(s, p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::sadd
  (const R s, const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::sadd(s, p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator +=
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    add(p);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::operator +
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p) const
  {
    return (MultivariateLaurentPolynomial<R, DIMENSION>(*this) += p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  void MultivariateLaurentPolynomial<R, DIMENSION>::subtract
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::subtract(p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator -=
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    subtract(p);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::operator - () const
  {
    return (MultivariateLaurentPolynomial<R, DIMENSION>() -= *this);
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>
  MultivariateLaurentPolynomial<R, DIMENSION>::operator -
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p) const
  {
    return (MultivariateLaurentPolynomial<R, DIMENSION>(*this) -= p);
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator *= (const R c)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::scale(c);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION> operator *
  (const R c, const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    return (MultivariateLaurentPolynomial<R, DIMENSION>(p) *= c);
  }

  template <class R, unsigned int DIMENSION>
  void MultivariateLaurentPolynomial<R, DIMENSION>::multiply
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    InfiniteVector<R, MultiIndex<int, DIMENSION> > coeffs;
    MultiIndex<int, DIMENSION> index;
    for (const_iterator it(begin()), itend(end()); it != itend; ++it)
      for (const_iterator itp(p.begin()), itpend(p.end()); itp != itpend; ++itp)
	{
	  for (unsigned int i(0); i < DIMENSION; i++)
	    index[i] = it.index()[i] + itp.index()[i];

	  coeffs.set_coefficient(index, coeffs.get_coefficient(index) + (*it * *itp));
	}
    
    InfiniteVector<R, MultiIndex<int, DIMENSION> >::swap(coeffs);    
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION>&
  MultivariateLaurentPolynomial<R, DIMENSION>::operator *=
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    multiply(p);
    return *this;
  }

  template <class R, unsigned int DIMENSION>
  inline
  MultivariateLaurentPolynomial<R, DIMENSION> operator *
  (const MultivariateLaurentPolynomial<R, DIMENSION>& p,
   const MultivariateLaurentPolynomial<R, DIMENSION>& q)
  {
    return (MultivariateLaurentPolynomial<R, DIMENSION>(p) *= q);
  }

  template <class R, unsigned int DIMENSION>
  void MultivariateLaurentPolynomial<R, DIMENSION>::dump() const
  {
    cout << InfiniteVector<R, MultiIndex<int, DIMENSION> >(*this) << endl;
    for (typename MultivariateLaurentPolynomial<R, DIMENSION>::const_iterator it(begin());
	 it != end(); ++it)
      {
	cout << it.index() << ": " << *it << endl;
      }
    
    cout << *(InfiniteVector<R, MultiIndex<int, DIMENSION> >::begin()) << endl;
    cout << *(begin()) << endl;
    cout << get_coefficient(begin().index()) << endl;
  }

  template <class R, unsigned int DIMENSION>
  std::ostream& operator <<
    (std::ostream& s, const MultivariateLaurentPolynomial<R, DIMENSION>& p)
  {
    for (typename MultivariateLaurentPolynomial<R, DIMENSION>::const_iterator it(p.begin());
  	 it != p.end(); ++it)
      {
  	if (it != p.begin())
  	  s << "+";
  	s << "(" << *it << ")";
	for (unsigned int i(0); i < DIMENSION; i++)
	  {
	    if (it.index()[i] != 0)
	      {
		s << "z_" << i+1;
		if (it.index()[i] != 1)
		  {
		    s << "^";
		    if (it.index()[i] < 0)
		      s << "{" << it.index()[i] << "}";
		    else
		      s << it.index()[i];
		  }
	      }
  	  }
      }
    
    return s;
  }
}
