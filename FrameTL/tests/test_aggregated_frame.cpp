#define _DIM 2

#include <iostream>
#include <interval/ds_basis.h>
#include <interval/p_basis.h>
#include <frame_support.h>
#include <frame_index.h>




using std::cout;
using std::endl;

using FrameTL::FrameIndex;
using FrameTL::AggregatedFrame;

using namespace FrameTL;
using namespace MathTL;
using namespace WaveletTL;


int main()
{
  
  cout << "Testing class AggregatedFrame..." << endl;
  
  typedef PBasis<2,2> Basis1D;
  typedef AggregatedFrame<Basis1D,2,2> Frame;
  //typedef Frame::Index Index;

   const unsigned int DIM = 2;

  //##############################  
  Matrix<double> A(DIM,DIM);
  A(0,0) = 2.0;
  A(1,1) = 1.0;
  Point<2> b;
  b[0] = -1.0;
  b[1] = -1.0;
  AffineLinearMapping<2> affineP(A,b);
  //##############################

  //##############################
  LinearBezierMapping bezierP(Point<2>(-1.,-1),Point<2>(-1.,1),
  			      Point<2>(1.,-1.), Point<2>(1,0));
  //##############################
  Array1D<Chart<DIM,DIM>* > charts(2);
  charts[0] = &affineP;
  charts[1] = &bezierP;

  SymmetricMatrix<bool> adj(2);
  adj(0,0) = 1;
  adj(1,1) = 1;
  adj(1,0) = 1;

  //to specify primal boundary the conditions
  Array1D<FixedArray1D<int,2*DIM> > bc(2);

  //primal boundary conditions for first patch: all Dirichlet
  FixedArray1D<int,2*DIM> bound_1;
  bound_1[0] = 1;
  bound_1[1] = 1;
  bound_1[2] = 1;
  bound_1[3] = 1;

  bc[0] = bound_1;

  //primal boundary conditions for second patch: all Dirichlet
  FixedArray1D<int,2*DIM> bound_2;
  bound_2[0] = 1;
  bound_2[1] = 1;
  bound_2[2] = 1;
  bound_2[3] = 1;

  bc[1] = bound_2;

//to specify primal boundary the conditions
  Array1D<FixedArray1D<int,2*DIM> > bcT(2);

  //dual boundary conditions for first patch
  FixedArray1D<int,2*DIM> bound_3;
  bound_3[0] = 0;
  bound_3[1] = 0;
  bound_3[2] = 0;
  bound_3[3] = 0;

  bcT[0] = bound_3;

  //dual boundary conditions for second patch
  FixedArray1D<int,2*DIM> bound_4;
  bound_4[0] = 0;
  bound_4[1] = 0;
  bound_4[2] = 0;
  bound_4[3] = 0;
 
  bcT[1] = bound_4;

  Atlas<DIM,DIM> Lshaped(charts,adj);  
  cout << Lshaped << endl;

// //   typedef DSBasis<2,2> Basis1D;
// //   AggregatedFrame<Basis1D, DIM, DIM> frame(&Lshaped, bc, bcT, 5);

  const int jmax = 7;
  Frame frame(&Lshaped, bc, jmax);

//   //const MappedCubeBasis<Basis1D, DIM, DIM>* cbas = frame.get_local_basis(0);

//   //  cbas = frame.get_local_basis(1);

//   //should be caught by assert statement
//   //cbas = frame.get_local_basis(2);
 
  

  return 0;
}
