#include <iostream>
#include <map>
#include <algebra/matrix.h>
#include <algebra/vector.h>
#include <algebra/qs_matrix.h>
#include <algebra/sparse_matrix.h>
#include <utils/array1d.h>

using namespace std;
using namespace MathTL;

int main()
{
  cout << "Constructing quasi-stationary matrices..." << endl;
  
  cout << "- the default QuasiStationaryMatrix:" << endl;
  QuasiStationaryMatrix<double> Qdefault;
  cout << "Qdefault=" << endl << Qdefault;

//   Matrix<double> one(3, 3, "1 2 3 4 5 6 7 8 9");
  Matrix<double> one(4, 2, "1 2 3 4 5 6 7 8");
  Matrix<double> two(4, 2, "-4 -3 -2 -1 0 1 2 3");
  Vector<double> b1(4, "3 4 5 6");
  Vector<double> b2(3, "5 6 -7");

  QuasiStationaryMatrix<double> Q(3, 17, 9, one, two, b1, b2, 2, 1);
  cout << "Q=" << endl << Q;

  Qdefault = Q;
  cout << "after assignment, Qdefault=" << endl << Qdefault;

//   Q.set_level(4);
//   cout << "Q on next higher level:" << endl << Q;

  SparseMatrix<double> S;
  Q.to_sparse(S);
  cout << "Q as a sparse matrix:" << endl << S;

  Vector<double> x(Q.column_dimension());
  x[4] = x[5] = 1;
  Vector<double> Qx(Q.row_dimension());
  Q.apply(x, Qx);
  cout << "Q applied to x=" << x << " yields Qx=" << Qx << endl;

  Vector<double> y(Q.row_dimension());
  y[11] = y[12] = 1;
  Vector<double> Qty(Q.column_dimension());
  Q.apply_transposed(y, Qty);
  cout << "Q^T applied to y=" << y << " yields Q^Ty=" << Qty << endl;

  x.resize(Q.column_dimension()+2);
  x[5] = x[6] = 1;
  Qx.resize(Q.row_dimension()+2);
  Q.apply(x, Qx, 1, 2);
  cout << "Q applied to the part of x=" << x << " from index 1," << endl
       << "written into Qx from index 2 on yields Qx=" << Qx << endl;

  y.resize(Q.row_dimension()+3);
  y[14] = y[15] = 1;
  Qty.resize(Q.column_dimension()+1);
  Q.apply_transposed(y, Qty, 3, 1);
  cout << "Q^T applied to the part of y=" << y << " from index 3," << endl
       << "written into Qty from index 1 on yields Qty=" << Qty << endl;

  std::map<size_t, double> xsparse, Qxsparse;
  xsparse[6] = 1;
//   xsparse[Q.column_dimension()-1] = 1;
  unsigned int xoffset = 1;
  unsigned int Qxoffset = 0;
  Q.apply(xsparse, Qxsparse, xoffset, Qxoffset);
  cout << "entries of a sparse vector x:" << endl;
  for (std::map<size_t, double>::const_iterator it(xsparse.begin());
       it != xsparse.end(); ++it)
    cout << "x[" << it->first << "]=" << it->second << endl;
  cout << "Q applied to the part of x from index " << xoffset
       << ", written into Qx from index " << Qxoffset << " yields Qx=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qxsparse.begin());
       it != Qxsparse.end(); ++it)
    cout << "Qx[" << it->first << "]=" << it->second << endl;

  std::map<size_t, double> ysparse, Qtysparse;
//   ysparse[9] = 1;
  ysparse[Q.row_dimension()-2] = 1;
  unsigned int yoffset = 0;
  unsigned int Qtyoffset = 0;
  Q.apply_transposed(ysparse, Qtysparse, yoffset, Qtyoffset);
  cout << "entries of a sparse vector y:" << endl;
  for (std::map<size_t, double>::const_iterator it(ysparse.begin());
       it != ysparse.end(); ++it)
    cout << "y[" << it->first << "]=" << it->second << endl;
  cout << "Q^T applied to the part of y from index " << yoffset
       << ", written into Qty from index " << Qtyoffset << " yields Qty=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qtysparse.begin());
       it != Qtysparse.end(); ++it)
    cout << "Qty[" << it->first << "]=" << it->second << endl;

  xsparse.clear();
  xsparse[6] = 1.0;
  cout << "x again:" << endl;
  for (std::map<size_t, double>::const_iterator it(xsparse.begin());
       it != xsparse.end(); ++it)
    cout << "x[" << it->first << "]=" << it->second << endl;
  Qxsparse.clear();
  Q.apply_central_block(xsparse, Qxsparse);
  cout << "central block of Q applied to x yields Qx=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qxsparse.begin());
       it != Qxsparse.end(); ++it)
    cout << "Qx[" << it->first << "]=" << it->second << endl;
    
  Qxsparse.clear();
  Q.apply_central_columns(xsparse, Qxsparse);
  cout << "central columns of Q applied to x yields Qx=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qxsparse.begin());
       it != Qxsparse.end(); ++it)
    cout << "Qx[" << it->first << "]=" << it->second << endl;

  ysparse.clear();
  ysparse[2] = 1.0;
  cout << "y again:" << endl;
  for (std::map<size_t, double>::const_iterator it(ysparse.begin());
       it != ysparse.end(); ++it)
    cout << "y[" << it->first << "]=" << it->second << endl;
  Qtysparse.clear();
  Q.apply_central_block_transposed(ysparse, Qtysparse);
  cout << "transpose of central block of Q applied to y yields Qty=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qtysparse.begin());
       it != Qtysparse.end(); ++it)
    cout << "Qty[" << it->first << "]=" << it->second << endl;
  
  ysparse.clear();
  ysparse[2] = 1.0;
  cout << "y again:" << endl;
  for (std::map<size_t, double>::const_iterator it(ysparse.begin());
       it != ysparse.end(); ++it)
    cout << "y[" << it->first << "]=" << it->second << endl;
  Qtysparse.clear();
  Q.apply_central_columns_transposed(ysparse, Qtysparse);
  cout << "transpose of central columns of Q applied to y yields Qty=" << endl;
  for (std::map<size_t, double>::const_iterator it(Qtysparse.begin());
       it != Qtysparse.end(); ++it)
    cout << "Qty[" << it->first << "]=" << it->second << endl;

  Array1D<double> periodic_band(5);
  periodic_band[0] = -1;
  periodic_band[1] = 2;
  periodic_band[2] = 3;
  periodic_band[3] = -4;
  periodic_band[4] = 5;
  PeriodicQuasiStationaryMatrix<double> Qp(2, -1, periodic_band, -2.0);
  cout << "Qp=" << endl << Qp;
  Qp.set_level(3);
  cout << "Qp(3)=" << endl << Qp; 
  PeriodicQuasiStationaryMatrix<double> Qphelp = Qp;
  cout << "after assignment, Qphelp=" << endl << Qphelp;

  SparseMatrix<double> Sp;
  Qp.to_sparse(Sp);
  cout << "Qp as a sparse matrix:" << endl << Sp;

  Vector<double> xp(Qp.column_dimension());
  xp[4] = xp[6] = 1; xp[0] = -2;
  Vector<double> Qpxp(Qp.row_dimension());
  Qp.apply(xp, Qpxp);
  cout << "Qp applied to xp=" << xp << " yields Qpxp=" << Qpxp << endl;

  Vector<double> yp(Qp.row_dimension());
  yp[0] = yp[1] = 1; yp[4] = -1;
  Vector<double> Qptyp(Qp.column_dimension());
  Qp.apply_transposed(yp, Qptyp);
  cout << "Qp^T applied to yp=" << yp << " yields Qp^Typ=" << Qptyp << endl;

  return 0;
}
