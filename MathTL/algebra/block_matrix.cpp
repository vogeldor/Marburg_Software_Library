// implementation for block_matrix.h

#include <algebra/sparse_matrix.h>

namespace MathTL
{
  template <class C>
  BlockMatrix<C>::BlockMatrix(const size_type n)
    : blocks(n*n),
      block_rows_rows(n), block_columns_columns(n)
  {
    for (size_type i = 0; i < n; i++) {
      block_rows_rows[i] = 1;
      block_columns_columns[i] = 1;
      for (size_type j = 0; j < n; j++)
	blocks[i*n+j] = 0;
    }
    calculate_size();
  }

  template <class C>
  BlockMatrix<C>::BlockMatrix(const size_type block_rows, const size_type block_columns)
    : blocks(block_rows*block_columns),
      block_rows_rows(block_rows), block_columns_columns(block_columns)
  {
    for (size_type i = 0; i < block_rows; i++) {
      block_rows_rows[i] = 1;
      for (size_type j = 0; j < block_columns; j++)
	blocks[i*block_columns+j] = 0;
    }
    for (size_type j = 0; j < block_columns; j++)
      block_columns_columns[j] = 1;
    calculate_size();
  }

  template <class C>
  BlockMatrix<C>::BlockMatrix(const BlockMatrix<C>& M)
    : blocks(M.block_rows_rows.size()*M.block_columns_columns.size()),
      block_rows_rows(M.block_rows_rows),
      block_columns_columns(M.block_columns_columns)
  {
    for (size_type i = 0; i < block_rows(); i++)
      for (size_type j = 0; j < block_columns(); j++) {
	if (M.blocks[i*block_columns()+j])
	  blocks[i*block_columns()+j] = M.blocks[i*block_columns()+j]->clone();
	else
	  blocks[i*block_columns()+j] = 0;
      }
    calculate_size();
  }
  
  template <class C>
  BlockMatrix<C>::~BlockMatrix()
  {
    for (size_type i = 0; i < block_rows()*block_columns(); i++) {
      if (blocks[i])
	delete blocks[i];
    } 
  }

  template <class C>
  MatrixBlock<C>*
  BlockMatrix<C>::clone() const
  {
    return new BlockMatrix<C>(*this);
  }

  template <class C>
  MatrixBlock<C>*
  BlockMatrix<C>::clone_transposed() const
  {
    return new BlockMatrix<C>(transpose(*this));
  }

  template <class C>
  void
  BlockMatrix<C>::calculate_size()
  {
    rowdim_ = 0;
    for (size_type i = 0; i < block_rows(); i++)
      rowdim_ += block_rows_rows[i];
    coldim_ = 0;
    for (size_type j = 0; j < block_columns(); j++)
      coldim_ += block_columns_columns[j];
  }
  
  template <class C>
  inline
  bool BlockMatrix<C>::empty() const
  {
    return row_dimension()*column_dimension()==0;
  }

  template <class C>
  inline
  const typename BlockMatrix<C>::size_type
  BlockMatrix<C>::block_rows() const
  {
    return block_rows_rows.size();
  }

  template <class C>
  inline
  const typename BlockMatrix<C>::size_type
  BlockMatrix<C>::block_columns() const
  {
    return block_columns_columns.size();
  }

  template <class C>
  inline
  const typename BlockMatrix<C>::size_type
  BlockMatrix<C>::block_row_dimension(const size_type block_row) const
  {
    return block_rows_rows[block_row];
  }

  template <class C>
  inline
  const typename BlockMatrix<C>::size_type
  BlockMatrix<C>::block_column_dimension(const size_type block_column) const
  {
    return block_columns_columns[block_column];
  }

  template <class C>
  inline
  const MatrixBlock<C>*
  BlockMatrix<C>::get_block(const size_type block_row, const size_type block_column) const
  {
    return blocks[block_row*block_columns()+block_column];
  }

  template <class C>
  inline
  void
  BlockMatrix<C>::set_block(const size_type row, const size_type column, MatrixBlock<C>* block)
  {
    if (blocks[row*block_columns()+column])
      delete blocks[row*block_columns()+column];
    blocks[row*block_columns()+column] = block;
  }

  template <class C>
  inline
  void
  BlockMatrix<C>::resize_block_row(const size_type block_row, const size_type rows)
  {
    block_rows_rows[block_row] = rows;
    calculate_size();
  }

  template <class C>
  inline
  void
  BlockMatrix<C>::resize_block_column(const size_type block_column, const size_type columns)
  {
    block_columns_columns[block_column] = columns;
    calculate_size();
  }

  template <class C>
  void
  BlockMatrix<C>::apply(const Vector<C>& x, Vector<C>& Mx) const
  {
    assert(Mx.size() == row_dimension());
    
    // first decompose x into the corresponding blocks
    Array1D<Vector<C> > x_blocks(block_columns());
    size_type offset = 0;
    for (size_type block_column = 0; block_column < block_columns(); block_column++) {
      x_blocks[block_column].resize(block_columns_columns[block_column]);
      for (size_type j = 0; j < block_columns_columns[block_column]; j++)
	x_blocks[block_column][j] = x[offset+j];
      offset += block_columns_columns[block_column];
    }

//     cout << "in BlockMatrix::apply(), we have x_blocks as follows:" << endl;
//     for (size_type i(0); i < block_columns(); i++)
//       cout << "x_blocks[" << i << "]=" << x_blocks[i] << endl;
    
    // calculate the blocks of Mx
    offset = 0;
    for (size_type block_row = 0; block_row < block_rows(); block_row++) {
      const size_type m = block_rows_rows[block_row]; // for readability
      for (size_type i = 0; i < m; i++)
	Mx[i+offset] = 0;
      Vector<C> Mx_block(m);
      for (size_type block_column = 0; block_column < block_columns(); block_column++) {
	if (get_block(block_row, block_column)) {
	  Mx_block.scale(0);
	  get_block(block_row, block_column)->apply(x_blocks[block_column], Mx_block);

// 	  cout << "in BlockMatrix::apply(), for block_row=" << block_row
// 	       << " and block_column=" << block_column
// 	       << ", we have x_blocks[" << block_column << "]=" << x_blocks[block_column] << endl
// 	       << ", get_block(" << block_row << "," << block_column << ")=" << endl;
// 	  get_block(block_row,block_column)->print(cout);
// 	  cout << "and Mx_block=" << Mx_block << endl;
	  
	  for (size_type i = 0; i < m; i++)
	    Mx[i+offset] += Mx_block[i];
	}
      }
      offset += Mx_block.size();
    }
  }
  
  template <class C>
  void
  BlockMatrix<C>::apply_transposed(const Vector<C>& x, Vector<C>& Mtx) const
  {
    assert(Mtx.size() == column_dimension());

    // first decompose x into the corresponding blocks
    Array1D<Vector<C> > x_blocks(block_rows());
    size_type offset = 0;
    for (size_type block_row = 0; block_row < block_rows(); block_row++) {
      x_blocks[block_row].resize(block_rows_rows[block_row]);
      for (size_type i = 0; i < block_rows_rows[block_row]; i++)
	x_blocks[block_row][i] = x[offset+i];
      offset += block_rows_rows[block_row];
    }
    
    // calculate the blocks of Mtx
    offset = 0;
    for (size_type block_column = 0; block_column < block_columns(); block_column++) {
      const size_type n = block_columns_columns[block_column]; // for readability
      for (size_type j = 0; j < n; j++)
	Mtx[j+offset] = 0;
      Vector<C> Mtx_block(n);
      for (size_type block_row = 0; block_row < block_rows(); block_row++) {
	if (get_block(block_row, block_column)) {
	  Mtx_block.scale(0);
	  get_block(block_row, block_column)->apply_transposed(x_blocks[block_row], Mtx_block);
	  for (size_type j = 0; j < n; j++)
	    Mtx[j+offset] += Mtx_block[j];
	}
      }
      offset += Mtx_block.size();
    }    
  }
  
  template <class C>
  BlockMatrix<C>
  operator * (const BlockMatrix<C>& M, const BlockMatrix<C>& N)
  {
    assert(M.block_columns() == N.block_rows());
    typedef typename BlockMatrix<C>::size_type size_type;

    BlockMatrix<C> R(M.block_rows(), N.block_columns());
    for (size_type i(0); i < M.block_rows(); i++)
      R.resize_block_row(i, M.block_row_dimension(i));
    for (size_type j(0); j < N.block_columns(); j++)
      R.resize_block_column(j, N.block_column_dimension(j));

    for (size_type i(0); i < M.block_rows(); i++)
      for (size_type j(0); j < N.block_columns(); j++)
 	{
	  // first decide whether any block has to be inserted
	  SparseMatrix<C>* B = 0;
	  for (size_type k(0); k < N.block_rows(); k++) {
	    if (M.get_block(i, k) && N.get_block(k, j)) {
	      B = new SparseMatrix<C>(M.get_block(i, k)->row_dimension(),
				      N.get_block(k, j)->column_dimension());
	      break;
	    }
	  }
	  
	  if (B) {
	    // compute the block as a dense matrix (faster)
	    Matrix<double> T(B->row_dimension(), B->column_dimension());
  	    for (size_type k(0); k < N.block_rows(); k++) {
  	      if (M.get_block(i, k) && N.get_block(k, j)) {
		// T += M(i,k)*N(k,j) columnwise
 		Vector<double> x(B->column_dimension()),
 		  y(N.get_block(k, j)->row_dimension()),
 		  z(B->row_dimension());
 		for (size_type ell(0); ell < B->column_dimension(); ell++) {
 		  x.scale(0.0); x[ell] = 1.0;
 		  N.get_block(k, j)->apply(x, y);
 		  M.get_block(i, k)->apply(y, z);
 		  for (size_type m(0); m < B->row_dimension(); m++)
 		    T.set_entry(m, ell, T.get_entry(m, ell) + z[m]);
 		}
  	      }
  	    }

	    // copy the computed block into the final matrix
	    if (row_sum_norm(T) > 1e-14) {
	      B->set_block(0, 0, T);
	      R.set_block(i, j, B);
	    }
	  }
 	}
    
    return R;
  }

  template <class C>
  BlockMatrix<C> transpose(const BlockMatrix<C>& M)
  {
    typedef typename BlockMatrix<C>::size_type size_type;

    BlockMatrix<C> R(M.block_columns(), M.block_rows());
    for (size_type block_row(0); block_row < M.block_rows(); block_row++) {
      R.resize_block_column(block_row, M.block_row_dimension(block_row));
    }
    for (size_type block_column(0); block_column < M.block_columns(); block_column++) {
      R.resize_block_row(block_column, M.block_column_dimension(block_column));
    }

    for (size_type block_row(0); block_row < M.block_rows(); block_row++)
      for (size_type block_column(0); block_column < M.block_columns(); block_column++) {
	if (M.get_block(block_row, block_column)) {
	  R.set_block(block_column, block_row, M.get_block(block_row, block_column)->clone_transposed());
	}
      }
    
    return R;
  }

  template <class C>
  void BlockMatrix<C>::print(std::ostream &os,
			     const unsigned int tabwidth,
			     const unsigned int precision) const
  {
    // quick hack, for the moment
    os << *this;
  }

  template <class C>
  inline
  std::ostream& operator << (std::ostream& os, const BlockMatrix<C>& M)
  {
    if (M.empty())
      os << "[]" << std::endl; // Matlab style
    else
      {
	os << "blocks: (" << M.block_rows() << "x" << M.block_columns() << "),"
	   << " total size: (" << M.row_dimension() << "x" << M.column_dimension() << ")"
	   << std::endl;
 	for (typename BlockMatrix<C>::size_type i(0); i < M.block_rows(); ++i)
	  for (typename BlockMatrix<C>::size_type j(0); j < M.block_columns(); ++j) {
	    os << "block (" << i << "," << j << ")";
	    if (M.get_block(i, j)==0)
	      os << ":" << std::endl << "ZERO" << std::endl;
	    else {
	      os << ", size is (" << M.get_block(i, j)->row_dimension()
		 << "x" << M.get_block(i, j)->column_dimension() << "):" << std::endl;
	      M.get_block(i, j)->print(os);
	    }
	  }
      }
    
    return os;
  }
  
}
