// implementation of some (inline) Array1D<C>:: methods

#include <cassert>
#include "io/vector_io.h"

namespace MathTL
{
  template <class C>
  inline
  Array1D<C>::Array1D()
    : data_(0), size_(0)
  {
  }

  template <class C>
  inline
  Array1D<C>::Array1D(const size_type s)
    : size_(s)
  {
    if (s == 0)
      data_ = 0;
    else
      data_ = new C[s]; // calls C()
  }

  template <class C>
  inline
  Array1D<C>::~Array1D()
  {
    if (data_ != 0)
      delete [] data_;
  }
  
  template <class C>
  inline
  const typename Array1D<C>::size_type
  Array1D<C>::size() const
  {
    return size_;
  }

  template <class C>
  inline
  const C& Array1D<C>::operator [] (const size_type i) const
  {
    assert(i < size_);
    return data_[i];
  }

  template <class C>
  inline
  C& Array1D<C>::operator [] (const size_type i)
  {
    assert(i < size_);
    return data_[i];
  }

  template <class C>
  inline
  typename Array1D<C>::const_iterator
  Array1D<C>::begin() const
  {
    return &data_[0];
  }

  template <class C>
  inline
  typename Array1D<C>::iterator
  Array1D<C>::begin()
  {
    return &data_[0];
  }

  template <class C>
  inline
  typename Array1D<C>::const_iterator
  Array1D<C>::end() const
  {
    return &data_[size_];
  }

  template <class C>
  inline
  typename Array1D<C>::iterator
  Array1D<C>::end()
  {
    return &data_[size_];
  }

  template <class C>
  inline
  std::ostream& operator << (std::ostream& os, const Array1D<C>& A)
  {
    print_vector(A, os);
    return os;
  }

}
