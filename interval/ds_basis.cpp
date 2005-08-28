// implementation for ds_basis

#include <cassert>
#include <cmath>

#include <numerics/matrix_decomp.h>

#include <Rd/haar_mask.h>
#include <Rd/cdf_mask.h>
#include <Rd/dm_mask.h>
#include <Rd/refinable.h>
#include <Rd/r_index.h>

namespace WaveletTL
{
  template <int d, int dT>
  DSBasis<d,dT>::DSBasis(const int s0, const int s1, const int sT0, const int sT1,
			 DSBiorthogonalizationMethod bio) {
    assert(std::max(s0,s1) < d && std::max(sT0,sT1) < dT);
    
//     assert(((s0 == 0 && sT0 > 0) || (s0 > 0 && sT0 == 0))
// 	   && ((s1 == 0 && sT1 > 0) || (s1 > 0 && sT1 == 0)));
    
    this->s0 = s0;
    this->s1 = s1;
    this->sT0 = sT0;
    this->sT1 = sT1;
    this->bio = bio;

#if 0
    cout << "* some moments alpha(m,r):" << endl;
    for (unsigned int r = 1; r < d; r++)
      for (int m = -5; m <= 5; m++)
	cout << "m=" << m << ", r=" << r << ": " << alpha(m,r) << endl;
    cout << "* some moments alphaT(m,r):" << endl;
    for (unsigned int r = 1; r < dT; r++)
      for (int m = -5; m <= 5; m++)
	cout << "m=" << m << ", r=" << r << ": " << alphaT(m,r) << endl;
#endif

    setup_GammaLR();
    setup_CX_CXT();
    setup_CXA_CXAT();

    // IGPMlib reference: I_Basis_Bspline_s::Setup()

    setup_Cj();

    Matrix<double> ml = ML(); // (3.5.2)
    Matrix<double> mr = MR(); // (3.5.2)
    SparseMatrix<double> mj0;   setup_Mj0  (ml,   mr,   mj0);   // (3.5.1)

    Matrix<double> mltp = MLTp(); // (3.5.6)
    Matrix<double> mrtp = MRTp(); // (3.5.6)
    SparseMatrix<double> mj0tp; setup_Mj0Tp(mltp, mrtp, mj0tp); // (3.5.5)

    // biorthogonalize the generators
    Mj0  = transpose(inv_Cjp)  * mj0   * transpose(Cj); // (2.4.3)
    Mj0T = transpose(inv_CjpT) * mj0tp * transpose(CjT);
    
#if 1
    cout << "DSBasis(): check biorthogonality of Mj0, Mj0T:" << endl;
    cout << "Mj0=" << endl << Mj0 << endl << "Mj0T=" << endl << Mj0T << endl;

    SparseMatrix<double> testbio0 = transpose(Mj0) * Mj0T;
    cout << "Mj0^T*Mj0T=" << endl << testbio0 << endl;
    for (unsigned int i = 0; i < testbio0.row_dimension(); i++)
      testbio0.set_entry(i, i, testbio0.get_entry(i, i) - 1.0);
    cout << "* ||Mj0^T*Mj0T-I||_infty: " << row_sum_norm(testbio0) << endl;

    testbio0 = transpose(Mj0T) * Mj0;
    cout << "Mj0T*Mj0^T=" << endl << testbio0 << endl;
    for (unsigned int i = 0; i < testbio0.row_dimension(); i++)
      testbio0.set_entry(i, i, testbio0.get_entry(i, i) - 1.0);
    cout << "* ||Mj0T^T*Mj0-I||_infty: " << row_sum_norm(testbio0) << endl;
#endif    

    // construction of the wavelet basis: initial stable completion, [DKU section 4.1]

    SparseMatrix<double> FF; F(FF);         // (4.1.14)
    SparseMatrix<double> PP; P(ml, mr, PP); // (4.1.22)
 
    SparseMatrix<double> A, H, Hinv;
    GSetup(A, H, Hinv); // (4.1.1), (4.1.13)

#if 1
    SparseMatrix<double> Aold(A); // for the checks below
#endif

    GElim (A, H, Hinv); // elimination (4.1.4)ff.
    SparseMatrix<double> BB; BT(A, BB); // (4.1.13)

#if 1
    cout << "DSBasis(): check properties (4.1.15):" << endl;
    SparseMatrix<double> test4115 = transpose(BB)*A;
    for (unsigned int i = 0; i < test4115.row_dimension(); i++)
      test4115.set_entry(i, i, test4115.get_entry(i, i) - 1.0);
    cout << "* ||Bj*Ajd-I||_infty: " << row_sum_norm(test4115) << endl;

    test4115 = transpose(FF)*FF;
    for (unsigned int i = 0; i < test4115.row_dimension(); i++)
      test4115.set_entry(i, i, test4115.get_entry(i, i) - 1.0);
    cout << "* ||Fj^T*Fj-I||_infty: " << row_sum_norm(test4115) << endl;    

    test4115 = transpose(BB)*FF;
    cout << "* ||Bj*Fj||_infty: " << row_sum_norm(test4115) << endl;    

    test4115 = transpose(FF)*A;
    cout << "* ||Fj^T*A||_infty: " << row_sum_norm(test4115) << endl;    
#endif

#if 1
    cout << "DSBasis(): check factorization of A:" << endl;
    SparseMatrix<double> testAfact = Aold - Hinv*A;
    cout << "* in infty-norm: " << row_sum_norm(testAfact) << endl;

    cout << "DSBasis(): check that H is inverse to Hinv:" << endl;
    SparseMatrix<double> testHinv = H*Hinv;
    for (unsigned int i = 0; i < testHinv.row_dimension(); i++)
      testHinv.set_entry(i, i, testHinv.get_entry(i, i) - 1.0);
    cout << "* in infty-norm: " << row_sum_norm(testHinv) << endl;
#endif

    SparseMatrix<double> mj1ih = PP * Hinv * FF; // (4.1.23)
    SparseMatrix<double> PPinv; InvertP(PP, PPinv);

#if 1
    cout << "DSBasis(): check that PPinv is inverse to PP:" << endl;
    SparseMatrix<double> testPinv = PP*PPinv;
    for (unsigned int i = 0; i < testPinv.row_dimension(); i++)
      testPinv.set_entry(i, i, testPinv.get_entry(i, i) - 1.0);
    cout << "* in infty-norm: " << row_sum_norm(testPinv) << endl;
#endif

    SparseMatrix<double> help = H * PPinv;
    SparseMatrix<double> gj0ih = transpose(BB) * help;
    SparseMatrix<double> gj1ih = transpose(FF) * help; // (4.1.24)

#if 1
    cout << "DSBasis(): check initial stable completion:" << endl;
    SparseMatrix<double> mj_initial(mj0.row_dimension(),
				    mj0.column_dimension() + mj1ih.column_dimension());
    for (unsigned int i = 0; i < mj0.row_dimension(); i++)
      for (unsigned int j = 0; j < mj0.column_dimension(); j++)	{
	const double help = mj0.get_entry(i, j);
	if (help != 0)
	  mj_initial.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < mj1ih.row_dimension(); i++)
      for (unsigned int j = 0; j < mj1ih.column_dimension(); j++) {
	const double help = mj1ih.get_entry(i, j);
	if (help != 0)
	  mj_initial.set_entry(i, j+mj0.column_dimension(), help);
      }
    
    SparseMatrix<double> gj_initial(gj0ih.row_dimension() + gj1ih.row_dimension(),
 				    gj0ih.column_dimension());
    for (unsigned int i = 0; i < gj0ih.row_dimension(); i++)
      for (unsigned int j = 0; j < gj0ih.column_dimension(); j++) {
	const double help = gj0ih.get_entry(i, j);
	if (help != 0)
	  gj_initial.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < gj1ih.row_dimension(); i++)
      for (unsigned int j = 0; j < gj1ih.column_dimension(); j++) {
	const double help = gj1ih.get_entry(i, j);
	if (help != 0)
	  gj_initial.set_entry(i+gj0ih.row_dimension(), j, help);
      }
    
    SparseMatrix<double> test_initial = mj_initial * gj_initial;
    for (unsigned int i = 0; i < test_initial.row_dimension(); i++)
      test_initial.set_entry(i, i, test_initial.get_entry(i, i) - 1.0);
    cout << "* ||Mj*Gj-I||_infty: " << row_sum_norm(test_initial) << endl;

    test_initial = gj_initial * mj_initial;
    for (unsigned int i = 0; i < test_initial.row_dimension(); i++)
      test_initial.set_entry(i, i, test_initial.get_entry(i, i) - 1.0);
    cout << "* ||Gj*Mj-I||_infty: " << row_sum_norm(test_initial) << endl;
#endif

    // construction of the wavelet basis: stable completion with basis transformations
    SparseMatrix<double> I; I.diagonal(Deltasize(j0()+1), 1.0);
    Mj1  = (I - (Mj0*transpose(Mj0T))) * (transpose(inv_Cjp) * mj1ih);
    Mj1T = Cjp * transpose(gj1ih);

    Mj0 .compress(1e-8);
    Mj1 .compress(1e-8);
    Mj0T.compress(1e-8);
    Mj1T.compress(1e-8);
    
#if 1
    cout << "DSBasis(): check new stable completion:" << endl;
    
    SparseMatrix<double> mj_new(Mj0.row_dimension(),
				Mj0.column_dimension() + Mj1.column_dimension());
    for (unsigned int i = 0; i < Mj0.row_dimension(); i++)
      for (unsigned int j = 0; j < Mj0.column_dimension(); j++) {
	const double help = Mj0.get_entry(i, j);
	if (help != 0)
	  mj_new.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < Mj1.row_dimension(); i++)
      for (unsigned int j = 0; j < Mj1.column_dimension(); j++) {
	const double help = Mj1.get_entry(i, j);
	if (help != 0)
	  mj_new.set_entry(i, j+Mj0.column_dimension(), help);
      }
    
    SparseMatrix<double> gj0_new = transpose(Mj0T); gj0_new.compress();
    SparseMatrix<double> gj1_new = transpose(Mj1T); gj1_new.compress();
    SparseMatrix<double> gj_new(gj0_new.row_dimension() + gj1_new.row_dimension(),
				gj0_new.column_dimension());
    for (unsigned int i = 0; i < gj0_new.row_dimension(); i++)
      for (unsigned int j = 0; j < gj0_new.column_dimension(); j++) {
	const double help = gj0_new.get_entry(i, j);
	if (help != 0)
	  gj_new.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < gj1_new.row_dimension(); i++)
      for (unsigned int j = 0; j < gj1_new.column_dimension(); j++) {
	const double help = gj1_new.get_entry(i, j);
	if (help != 0)
	  gj_new.set_entry(i+gj0_new.row_dimension(), j, help);
      }
    
    SparseMatrix<double> test_new = mj_new * gj_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
    cout << "Mj*Gj-I=" << endl << test_new << endl;
    cout << "* ||Mj*Gj-I||_infty: " << row_sum_norm(test_new) << endl;

    test_new = gj_new * mj_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
    cout << "Gj*Mj-I=" << endl << test_new << endl;
    cout << "* ||Gj*Mj-I||_infty: " << row_sum_norm(test_new) << endl;
        
    SparseMatrix<double> mjt_new(Mj0T.row_dimension(),
				 Mj0T.column_dimension() + Mj1T.column_dimension());
    for (unsigned int i = 0; i < Mj0T.row_dimension(); i++)
      for (unsigned int j = 0; j < Mj0T.column_dimension(); j++) {
	const double help = Mj0T.get_entry(i, j);
	if (help != 0)
	  mjt_new.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < Mj1T.row_dimension(); i++)
      for (unsigned int j = 0; j < Mj1T.column_dimension(); j++) {
	const double help = Mj1T.get_entry(i, j);
	if (help != 0)
	  mjt_new.set_entry(i, j+Mj0T.column_dimension(), help);
      }
    
    SparseMatrix<double> gjt0_new = transpose(Mj0); gjt0_new.compress();
    SparseMatrix<double> gjt1_new = transpose(Mj1); gjt1_new.compress();
    SparseMatrix<double> gjt_new(gjt0_new.row_dimension() + gjt1_new.row_dimension(),
				 gjt0_new.column_dimension());
    for (unsigned int i = 0; i < gjt0_new.row_dimension(); i++)
      for (unsigned int j = 0; j < gjt0_new.column_dimension(); j++) {
	const double help = gjt0_new.get_entry(i, j);
	if (help != 0)
	  gjt_new.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < gjt1_new.row_dimension(); i++)
      for (unsigned int j = 0; j < gjt1_new.column_dimension(); j++) {
	const double help = gjt1_new.get_entry(i, j);
	if (help != 0)
	  gjt_new.set_entry(i+gjt0_new.row_dimension(), j, help);
      }
    
    test_new = mjt_new * gjt_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
    cout << "MjT*GjT-I=" << endl << test_new << endl;
    cout << "* ||MjT*GjT-I||_infty: " << row_sum_norm(test_new) << endl;

    test_new = gjt_new * mjt_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
    cout << "GjT*MjT-I=" << endl << test_new << endl;
    cout << "* ||GjT*MjT-I||_infty: " << row_sum_norm(test_new) << endl;
#endif

  }

  template <int d, int dT>
  const double
  DSBasis<d,dT>::alpha(const int m, const unsigned int r) const {
    double result = 0;
    if (r == 0)
      return 1; // [DKU] (5.1.1)
    else {
      if (m == 0) {
	// [DKU] (5.1.3)
	for (int k = ell1<d>(); k <= ell2<d>(); k++) {
	  double help = 0;
	  for (unsigned int s = 0; s < r; s++)
	    help += binomial(r, s) * intpower(k, r-s) * alpha(0, s);
	  result += cdf.a().get_coefficient(MultiIndex<int,1>(k)) * help;
	}
	result /= (double)((1<<(r+1))-2);
      } else {
	// [DKU] (5.1.2)
	for (unsigned int i = 0; i <= r; i++)
	  result += binomial(r, i) * intpower(m, i) * alpha(0, r-i);
      }
    }
    return result;
  }
  
  template <int d, int dT>
  const double
  DSBasis<d,dT>::alphaT(const int m, const unsigned int r) const {
    double result = 0;
    if (r == 0)
      return 1; // [DKU] (5.1.1)
    else {
      if (m == 0) {
	// [DKU] (5.1.3)
	for (int k = ell1T<d,dT>(); k <= ell2T<d,dT>(); k++) {
	  double help = 0;
	  for (unsigned int s = 0; s < r; s++)
	    help += binomial(r, s) * intpower(k, r-s) * alphaT(0, s);
	  result += cdf.aT().get_coefficient(MultiIndex<int,1>(k)) * help;
	}
	result /= (double)((1<<(r+1))-2);
      } else {
	// [DKU] (5.1.2)
	for (unsigned int i = 0; i <= r; i++)
	  result += binomial(r, i) * intpower(m, i) * alphaT(0, r-i);
      }
    }
    return result;
  }

  template <int d, int dT>
  const double
  DSBasis<d,dT>::betaL(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2T<d,dT>())/2.0); q < ellT_l()-sT0; q++)
      result += alpha(q, r) * cdf.aT().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result * M_SQRT1_2;
  }

  template <int d, int dT>
  const double
  DSBasis<d,dT>::betaLT(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2<d>())/2.0); q < ell_l()-s0; q++)
      result += alphaT(q, r) * cdf.a().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result * M_SQRT1_2;
  }

  template <int d, int dT>
  const double
  DSBasis<d,dT>::betaR(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2T<d,dT>())/2.0); q < ellT_r()-sT1; q++)
      result += alpha(q, r) * cdf.aT().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result * M_SQRT1_2;
  }

  template <int d, int dT>
  const double
  DSBasis<d,dT>::betaRT(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2<d>())/2.0); q < ell_r()-s1; q++)
      result += alphaT(q, r) * cdf.a().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result * M_SQRT1_2;
  }

  template <int d, int dT>
  void
  DSBasis<d,dT>::setup_GammaLR() {
    // IGPMlib reference: I_Mask_Bspline::EvalGammaL(), ::EvalGammaR()

    const unsigned int GammaLsize = std::max(d-s0, dT-sT0);
    GammaL.resize(GammaLsize, GammaLsize);

    const unsigned int GammaRsize = std::max(d-s1, dT-sT1);
    GammaR.resize(GammaRsize, GammaRsize);

    // 1. compute the integrals
    //      z(s,t) = \int_0^1\phi(x-s)\tilde\phi(x-t)\,dx
    //    exactly with the [DM] trick
    MultivariateRefinableFunction<DMMask2<HaarMask, CDFMask_primal<d>, CDFMask_dual<d,dT> >,2> zmask;
    InfiniteVector<double, MultiIndex<int,2> > zvalues(zmask.evaluate());

    cout << "z=" << endl << zvalues << endl;

    // 2. compute the integrals
    //      I(nu,mu) = \int_0^\infty\phi(x-\nu)\tilde\phi(x-\mu)\,dx
    //    exactly using the z(s,t) values

    const int I1Low  = -ell2<d>()+1;
    const int I1Up   = ell_l()-d+dT-1;
    const int I1Up_r = ell_r()-d+dT-1;
    const int I2Low  = -ell2T<d,dT>()+1;
    const int I2Up   = ellT_l()-1;
    const int I2Up_r = ellT_r()-1;

    Matrix<double> I(std::max(I1Up,I1Up_r)-I1Low+1,
		     std::max(I2Up,I2Up_r)-I2Low+1);
    for (int nu = I1Low; nu < -ell1<d>(); nu++)
      for (int mu = I2Low; mu < -ell1T<d,dT>(); mu++) {
  	double help(0);
  	const int diff(mu - nu);
   	const int sLow = std::max(-ell2<d>()+1,-ell2T<d,dT>()+1-diff);
  	for (int s = sLow; s <= nu; s++)
  	  help += zvalues.get_coefficient(MultiIndex<int,2>(s, s+diff));
  	I(-I1Low+nu, -I2Low+mu) = help; // [DKU] (5.1.7)
      }
    for (int nu = I1Low; nu <= std::max(I1Up,I1Up_r); nu++)
      for (int mu = I2Low; mu <= std::max(I2Up,I2Up_r); mu++) {
 	if ((nu >= -ell1<d>()) || ((nu <= ell_l()-1) && (mu >= -ell1T<d,dT>())))
 	  I(-I1Low+nu, -I2Low+mu) = (nu == mu ? 1 : 0); // [DKU] (5.1.6)
      }

    cout << "I=" << endl << I << endl;

    // 3. finally, compute the Gramian GammaL
    for (int r = s0; r < d; r++) {
      // phiL's against phiTL's
      for (int k = sT0; k < dT; k++) {
     	double help = 0;
     	for (int nu = I1Low; nu < ell_l()-s0; nu++)
     	  for (int mu = I2Low; mu < ellT_l()-sT0; mu++)
     	    help += alphaT(nu, r) * alpha(mu, k) * I(-I1Low+nu, -I2Low+mu);
    	GammaL(r-s0, k-sT0) = help; // [DKU] (5.1.4)
      }
      // phiL's against phiT's
      for (int k = dT; k < d+sT0-s0; k++) {
	double help = 0;
	for (int nu = I1Low; nu < ell_l()-s0; nu++)
	  help += alphaT(nu, r) * I(-I1Low+nu, -I2Low+ellT_l()-dT-sT0+k);
	GammaL(r-s0, k-sT0) = help; // analogous to [DKU] (5.1.4) (!)
      }
    }
    // phi's against phiTL's
    for (int r = d; r < dT+s0-sT0; r++)
      for (int k = sT0; k < dT; k++) {
    	double help = 0;
    	for (int mu = I2Low; mu < ellT_l()-sT0; mu++)
    	  help += alpha(mu, k) * I(-I1Low+ell_l()-d-s0+r, -I2Low+mu);
    	GammaL(r-s0, k-sT0) = help; // [DKU] (5.1.5)
      }

    cout << "GammaL=" << endl << GammaL << endl;

    // The same for GammaR:

    for (int r = s1; r < d; r++) {
      // phiR's against phiTR's
      for (int k = sT1; k < dT; k++) {
     	double help = 0;
     	for (int nu = I1Low; nu < ell_r()-s1; nu++)
     	  for (int mu = I2Low; mu < ellT_r()-sT1; mu++)
     	    help += alphaT(nu, r) * alpha(mu, k) * I(-I1Low+nu, -I2Low+mu);
    	GammaR(r-s1, k-sT1) = help; // [DKU] (5.1.4)
      }
      // phiR's against phiT's
      for (int k = dT; k < d+sT1-s1; k++) {
	double help = 0;
	for (int nu = I1Low; nu < ell_r()-s1; nu++)
	  help += alphaT(nu, r) * I(-I1Low+nu, -I2Low+ellT_r()-dT-sT1+k);
	GammaR(r-s1, k-sT1) = help; // analogous to [DKU] (5.1.4) (!)
      }
    }
    // phi's against phiTR's
    for (int r = d; r < dT+s1-sT1; r++)
      for (int k = sT1; k < dT; k++) {
    	double help = 0;
    	for (int mu = I2Low; mu < ellT_r()-sT1; mu++)
    	  help += alpha(mu, k) * I(-I1Low+ell_r()-d-s1+r, -I2Low+mu);
    	GammaR(r-s1, k-sT1) = help; // [DKU] (5.1.5)
      }

    cout << "GammaR=" << endl << GammaR << endl;
  }

  template <int d, int dT>
  void
  DSBasis<d,dT>::setup_CX_CXT()
  {
    // IGPMlib reference: I_Mask_Bspline::EvalCL(), ::EvalCR()

    CL.resize(GammaL.row_dimension(), GammaL.column_dimension());
    CLT.resize(GammaL.row_dimension(), GammaL.column_dimension());
    CR.resize(GammaR.row_dimension(), GammaR.column_dimension());
    CRT.resize(GammaR.row_dimension(), GammaR.column_dimension());

    if (bio == none) {
      CL.diagonal(GammaL.row_dimension(), 1.0);
      Matrix<double> CLGammaLInv;
      QUDecomposition<double>(GammaL).inverse(CLGammaLInv);
      CLT = transpose(CLGammaLInv);

      CR.diagonal(GammaR.row_dimension(), 1.0);
      Matrix<double> CRGammaRInv;
      QUDecomposition<double>(GammaR).inverse(CRGammaRInv);
      CRT = transpose(CRGammaRInv);      
    }
    
    if (bio == SVD) {
      MathTL::SVD<double> svd(GammaL);
      Matrix<double> U, V;
      Vector<double> S;
      svd.getU(U);
      svd.getV(V);
      svd.getS(S);
      for (unsigned int i = 0; i < GammaL.row_dimension(); i++) {
 	S[i] = 1.0 / sqrt(S[i]);
 	for (unsigned int j = 0; j < GammaL.row_dimension(); j++)
 	  CL(i, j)  = S[i] * U(j, i);
      }
      CL.compress(1e-14);
      Matrix<double> CLGammaLInv;
      QUDecomposition<double>(CL * GammaL).inverse(CLGammaLInv);
      CLT = transpose(CLGammaLInv);
      CLT.compress(1e-14);
      
      MathTL::SVD<double> svd_r(GammaR);
      svd_r.getU(U);
      svd_r.getV(V);
      svd_r.getS(S);
      for (unsigned int i = 0; i < GammaR.row_dimension(); i++) {
 	S[i] = 1.0 / sqrt(S[i]);
 	for (unsigned int j = 0; j < GammaR.row_dimension(); j++)
 	  CR(i, j)  = S[i] * U(j, i);
      }
      CR.compress(1e-14);
      Matrix<double> CRGammaRInv;
      QUDecomposition<double>(CR * GammaR).inverse(CRGammaRInv);
      CRT = transpose(CRGammaRInv);      
      CRT.compress(1e-14);
    }
    
//     if (bio == Bernstein) {
//       double b(0);
//       if (d == 2)
//  	b = 0.7; // cf. [DKU]
//       else {
//  	b = 3.6;
//       }
      
//       // CL : transformation to Bernstein polynomials (triangular)
//       //   (Z_b^m)_{r,l} = (-1)^{r-1}\binom{m-1}{r}\binom{r}{l}b^{-r}, r>=l, 0 otherwise
//       for (int j(0); j < d; j++)
//  	for (int k(0); k <= j; k++)
//  	  CL_(k, j) = minus1power(j-k) * std::pow(b, -j) * binomial(d-1, j) * binomial(j, k);
//       for (int j(d); j < lupT-llowT+1; j++)
//  	CL_(j, j) = 1.0;
      
//       Matrix<double> CLGammaLInv;
//       QUDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
//       CLT_ = transpose(CLGammaLInv);
      
//       CLT_.compress(1e-14);

//       for (int j(0); j < d; j++)
// 	for (int k(0); k <= j; k++)
// 	  CR_(k, j) = minus1power(j-k) * std::pow(b, -j) * binomial(d-1, j) * binomial(j, k);
//       for (int j(d); j < rlowTh-rupTh+1; j++)
// 	CR_(j, j) = 1.0;
      
//       Matrix<double> CRGammaRInv;
//       QUDecomposition<double>(CR_ * GammaR_).inverse(CRGammaRInv);
//       CRT_ = transpose(CRGammaRInv);
      
//       CRT_.compress(1e-14);
//     }
    
    if (bio == partialSVD) {
      MathTL::SVD<double> svd(GammaL);
      Matrix<double> U, V;
      Vector<double> S;
      svd.getU(U);
      svd.getV(V);
      svd.getS(S);
      Matrix<double> GammaLInv;
      QUDecomposition<double>(GammaL).inverse(GammaLInv);
      const double a = 1.0 / GammaLInv(0, 0);
      Matrix<double> R(GammaL.row_dimension(), GammaL.column_dimension());
      for (unsigned int i = 0; i < R.row_dimension(); i++)
 	S[i] = sqrt(S[i]);
      for (unsigned int j = 0; j < R.row_dimension(); j++)
 	R(0, j) = a * V(0, j) / S[j];
      for (unsigned int i = 1; i < R.row_dimension(); i++)
 	for (unsigned int j = 0; j < R.row_dimension(); j++)
 	  R(i, j) = U(j, i) * S[j];
      for (unsigned int i = 0; i < R.row_dimension(); i++)
 	for (unsigned int j = 0; j < R.row_dimension(); j++)
 	  U(i, j) /= S[i];
      CL = R*U;
      CL.compress(1e-14);
      Matrix<double> CLGammaLInv;
      QUDecomposition<double>(CL * GammaL).inverse(CLGammaLInv);
      CLT = transpose(CLGammaLInv);
      CLT.compress(1e-14);

      MathTL::SVD<double> svd_r(GammaR);
      svd_r.getU(U);
      svd_r.getV(V);
      svd_r.getS(S);
      Matrix<double> GammaRInv;
      QUDecomposition<double>(GammaR).inverse(GammaRInv);
      const double a_r = 1.0 / GammaRInv(0, 0);
      R.resize(GammaR.row_dimension(), GammaR.column_dimension());
      for (unsigned int i = 0; i < R.row_dimension(); i++)
	S[i] = sqrt(S[i]);
      for (unsigned int j = 0; j < R.row_dimension(); j++)
 	R(0, j) = a_r * V(0, j) / S[j];
      for (unsigned int i = 1; i < R.row_dimension(); i++)
 	for (unsigned int j = 0; j < R.row_dimension(); j++)
 	  R(i, j) = U(j, i) * S[j];
      for (unsigned int i = 0; i < R.row_dimension(); i++)
 	for (unsigned int j = 0; j < R.row_dimension(); j++)
 	  U(i, j) /= S[i];
      CR = R*U;
      CR.compress(1e-14);
      Matrix<double> CRGammaRInv;
      QUDecomposition<double>(CR * GammaR).inverse(CRGammaRInv);
      CRT = transpose(CRGammaRInv);
      CRT.compress(1e-14);
    }

//     if (bio_ == BernsteinSVD) {
//       double b(0);
//       if (d == 2)
// 	b = 0.7; // cf. [DKU]
//       else {
// 	b = 3.6;
//       }
      
//       // CL_ : transformation to Bernstein polynomials (triangular)
//       //   (Z_b^m)_{r,l} = (-1)^{r-1}\binom{m-1}{r}\binom{r}{l}b^{-r}, r>=l, 0 otherwise
//       for (int j(0); j < d; j++)
// 	for (int k(0); k <= j; k++)
// 	  CL_(k, j) = minus1power(j-k) * std::pow(b, -j) * binomial(d-1, j) * binomial(j, k);
//       for (int j(d); j < lupT-llowT+1; j++)
// 	CL_(j, j) = 1.0;
      
//       Matrix<double> GammaLNew(CL_ * GammaL_);
      
//       MathTL::SVD<double> svd(GammaLNew);
//       Matrix<double> U, V;
//       Vector<double> S;
//       svd.getU(U);
//       svd.getV(V);
//       svd.getS(S);
      
//       Matrix<double> GammaLNewInv;
//       QUDecomposition<double>(GammaLNew).inverse(GammaLNewInv);
//       const double a = 1.0 / GammaLNewInv(0, 0);
      
//       Matrix<double> R(lupT-llowT+1, lupT-llowT+1);
//       for (int i(0); i < lupT-llowT+1; i++)
// 	S[i] = sqrt(S[i]);
//       for (int j(0); j < lupT-llowT+1; j++)
// 	R(0, j) = a * V(0, j) / S[j];
//       for (int i(1); i < lupT-llowT+1; i++)
// 	for (int j(0); j < lupT-llowT+1; j++)
// 	  R(i, j) = U(j, i) * S[j];
      
//       for (int i(0); i < lupT-llowT+1; i++)
// 	for (int j(0); j < lupT-llowT+1; j++)
// 	  U(i, j) /= S[i];
//       CL_ = R*U*CL_;
      
//       CL_.compress(1e-14);

//       Matrix<double> CLGammaLInv;
//       QUDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
//       CLT_ = transpose(CLGammaLInv);

//       CLT_.compress(1e-14);

//       for (int j(0); j < d; j++)
// 	for (int k(0); k <= j; k++)
// 	  CR_(k, j) = minus1power(j-k) * std::pow(b, -j) * binomial(d-1, j) * binomial(j, k);
//       for (int j(d); j < rlowTh-rupTh+1; j++)
// 	CR_(j, j) = 1.0;

//       Matrix<double> GammaRNew(CR_ * GammaR_);
      
//       MathTL::SVD<double> svd_r(GammaRNew);
//       svd_r.getU(U);
//       svd_r.getV(V);
//       svd_r.getS(S);

//       Matrix<double> GammaRNewInv;
//       QUDecomposition<double>(GammaRNew).inverse(GammaRNewInv);
//       const double a_r = 1.0 / GammaRNewInv(0, 0);
      
//       R.resize(rlowTh-rupTh+1,rlowTh-rupTh+1);
//       for (int i(0); i < rlowTh-rupTh+1; i++)
// 	S[i] = sqrt(S[i]);
//       for (int j(0); j < rlowTh-rupTh+1; j++)
// 	R(0, j) = a_r * V(0, j) / S[j];
//       for (int i(1); i < rlowTh-rupTh+1; i++)
// 	for (int j(0); j < rlowTh-rupTh+1; j++)
// 	  R(i, j) = U(j, i) * S[j];
      
//       for (int i(0); i < rlowTh-rupTh+1; i++)
// 	for (int j(0); j < rlowTh-rupTh+1; j++)
// 	  U(i, j) /= S[i];
//       CR_ = R*U*CR_;

//       CR_.compress(1e-14);

//       Matrix<double> CRGammaRInv;
//       QUDecomposition<double>(CR_ * GammaR_).inverse(CRGammaRInv);
//       CRT_ = transpose(CRGammaRInv);

//       CRT_.compress(1e-14);
//     }
    
#if 1
    // check biorthogonality of the matrix product CL * GammaL * (CLT)^T
    cout << "GammaL=" << endl << GammaL << endl;
    cout << "CL=" << endl << CL << endl;
    cout << "CLT=" << endl << CLT << endl;
    Matrix<double> check(CL * GammaL * transpose(CLT));
    for (unsigned int i(0); i < check.row_dimension(); i++)
      check(i, i) -= 1;
    cout << "error for CLT: " << row_sum_norm(check) << endl;
#endif

    QUDecomposition<double>(CL).inverse(inv_CL);
    QUDecomposition<double>(CLT).inverse(inv_CLT);

#if 1
    // check biorthogonality of the matrix product CR * GammaR * (CRT)^T
    cout << "GammaR=" << endl << GammaR << endl;
    cout << "CR=" << endl << CR << endl;
    cout << "CRT=" << endl << CRT << endl;
    Matrix<double> check2(CR * GammaR * transpose(CRT));
    for (unsigned int i(0); i < check2.row_dimension(); i++)
      check2(i, i) -= 1;
    cout << "error for CRT: " << row_sum_norm(check2) << endl;
#endif

    QUDecomposition<double>(CR).inverse(inv_CR);
    QUDecomposition<double>(CRT).inverse(inv_CRT);
  }

  template <int d, int dT>
  void DSBasis<d,dT>::setup_CXA_CXAT() {
    // IGPMlib reference: I_Mask_Bspline::EvalCL(), ::EvalCR()

    // setup CLA <-> AlphaT * (CL)^T
    CLA.resize(ell_l()+ell2<d>()-s0+std::max(0,dT-d+s0-sT0)-1, CL.row_dimension());
    for (int i = 1-ell2<d>(); i < ell_l()-s0; i++)
      for (unsigned int r = s0; r < d; r++)
	CLA(ell2<d>()-1+i, r-s0) = alphaT(i, r);
    for (unsigned int r = d; r < CL.row_dimension()+s0; r++)
      CLA(ell2<d>()-1+ell_l()-s0+r-d, r-s0) = 1.0;

    cout << "CLA before biorthogonalization:" << endl << CLA << endl;
    CLA = CLA * transpose(CL);
    CLA.compress(1e-12);
    cout << "CLA after biorthogonalization:" << endl << CLA << endl;

    // setup CLAT <-> Alpha * (CLT)^T
    CLAT.resize(ellT_l()+ell2T<d,dT>()-sT0+std::max(0,d-dT+sT0-s0)-1, CLT.row_dimension());
    for (int i = 1-ell2T<d,dT>(); i < ellT_l()-sT0; i++)
      for (unsigned int r = sT0; r < dT; r++)
	CLAT(ell2T<d,dT>()-1+i, r-sT0) = alpha(i, r);
    for (unsigned int r = dT; r < CLT.row_dimension()+sT0; r++)
      CLAT(ell2T<d,dT>()-1+ellT_l()-sT0+r-dT, r-sT0) = 1.0;

    cout << "CLAT before biorthogonalization:" << endl << CLAT << endl;
    CLAT = CLAT * transpose(CLT);
    CLAT.compress(1e-12);
    cout << "CLAT after biorthogonalization:" << endl << CLAT << endl;

    // the same for CRA, CRAT:
    CRA.resize(ell_r()+ell2<d>()-s1+std::max(0,dT-d+s1-sT1)-1, CR.row_dimension());
    for (int i = 1-ell2<d>(); i < ell_r()-s1; i++)
      for (unsigned int r = s1; r < d; r++)
	CRA(ell2<d>()-1+i, r-s1) = alphaT(i, r);
    for (unsigned int r = d; r < CR.row_dimension()+s1; r++)
      CRA(ell2<d>()-1+ell_r()-s1+r-d, r-s1) = 1.0;

    cout << "CRA before biorthogonalization:" << endl << CRA << endl;
    CRA = CRA * transpose(CR);
    CRA.compress(1e-12);
    cout << "CRA after biorthogonalization:" << endl << CRA << endl;

    CRAT.resize(ellT_r()+ell2T<d,dT>()-sT1+std::max(0,d-dT+sT1-s1)-1, CRT.row_dimension());
    for (int i = 1-ell2T<d,dT>(); i < ellT_r()-sT1; i++)
      for (unsigned int r = sT1; r < dT; r++)
	CRAT(ell2T<d,dT>()-1+i, r-sT1) = alpha(i, r);
    for (unsigned int r = dT; r < CRT.row_dimension()+sT1; r++)
      CRAT(ell2T<d,dT>()-1+ellT_r()-sT1+r-dT, r-sT1) = 1.0;

    cout << "CRAT before biorthogonalization:" << endl << CRAT << endl;
    CRAT = CRAT * transpose(CRT);
    CRAT.compress(1e-12);
    cout << "CRAT after biorthogonalization:" << endl << CRAT << endl;

#if 0
    cout << "CLA=" << endl << CLA << endl;
    cout << "CLAT=" << endl << CLAT << endl;
    cout << "CRA=" << endl << CRA << endl;
    cout << "CRAT=" << endl << CRAT << endl;
#endif
  }

  template <int d, int dT>
  void DSBasis<d,dT>::setup_Cj() {
    // IGPMlib reference: I_Basis_Bspline_s::setup_Cj(), ::put_Mat()

    // (5.2.5)
    Cj.diagonal(Deltasize(j0()), 1.0);
    Cj.set_block(0, 0, CL);
    Cj.set_block(Deltasize(j0())-CR.row_dimension(),
		 Deltasize(j0())-CR.column_dimension(),
		 CR, true);

    inv_Cj.diagonal(Deltasize(j0()), 1.0);
    inv_Cj.set_block(0, 0, inv_CL);
    inv_Cj.set_block(Deltasize(j0())-inv_CR.row_dimension(),
		     Deltasize(j0())-inv_CR.column_dimension(),
		     inv_CR, true);

    CjT.diagonal(Deltasize(j0()), 1.0);
    CjT.set_block(0, 0, CLT);
    CjT.set_block(Deltasize(j0())-CRT.row_dimension(),
		   Deltasize(j0())-CRT.column_dimension(),
		   CRT, true);

    inv_CjT.diagonal(Deltasize(j0()), 1.0);
    inv_CjT.set_block(0, 0, inv_CLT);
    inv_CjT.set_block(Deltasize(j0())-inv_CRT.row_dimension(),
		      Deltasize(j0())-inv_CRT.column_dimension(),
		      inv_CRT, true);

    Cjp.diagonal(Deltasize(j0()+1), 1.0);
    Cjp.set_block(0, 0, CL);
    Cjp.set_block(Deltasize(j0()+1)-CR.row_dimension(),
		  Deltasize(j0()+1)-CR.column_dimension(),
		  CR, true);

    inv_Cjp.diagonal(Deltasize(j0()+1), 1.0);
    inv_Cjp.set_block(0, 0, inv_CL);
    inv_Cjp.set_block(Deltasize(j0()+1)-inv_CR.row_dimension(),
		      Deltasize(j0()+1)-inv_CR.column_dimension(),
		      inv_CR, true);

    CjpT.diagonal(Deltasize(j0()+1), 1.0);
    CjpT.set_block(0, 0, CLT);
    CjpT.set_block(Deltasize(j0()+1)-CRT.row_dimension(),
		   Deltasize(j0()+1)-CRT.column_dimension(),
		   CRT, true);

    inv_CjpT.diagonal(Deltasize(j0()+1), 1.0);
    inv_CjpT.set_block(0, 0, inv_CLT);
    inv_CjpT.set_block(Deltasize(j0()+1)-inv_CRT.row_dimension(),
		       Deltasize(j0()+1)-inv_CRT.column_dimension(),
		       inv_CRT, true);

#if 1
    cout << "DSBasis: testing setup of Cj:" << endl;

    SparseMatrix<double> test1 = CjT * inv_CjT;
    for (unsigned int i = 0; i < test1.row_dimension(); i++)
      test1.set_entry(i, i, test1.get_entry(i, i) - 1.0);
    cout << "* ||CjT*inv_CjT-I||_infty: " << row_sum_norm(test1) << endl;

    SparseMatrix<double> test2 = Cj * inv_Cj;
    for (unsigned int i = 0; i < test2.row_dimension(); i++)
      test2.set_entry(i, i, test2.get_entry(i, i) - 1.0);
    cout << "* ||Cj*inv_Cj-I||_infty: " << row_sum_norm(test2) << endl;

    SparseMatrix<double> test3 = CjpT * inv_CjpT;
    for (unsigned int i = 0; i < test3.row_dimension(); i++)
      test3.set_entry(i, i, test3.get_entry(i, i) - 1.0);
    cout << "* ||CjpT*inv_CjpT-I||_infty: " << row_sum_norm(test3) << endl;

    SparseMatrix<double> test4 = Cjp * inv_Cjp;
    for (unsigned int i = 0; i < test4.row_dimension(); i++)
      test4.set_entry(i, i, test4.get_entry(i, i) - 1.0);
    cout << "* ||Cjp*inv_Cjp-I||_infty: " << row_sum_norm(test4) << endl;
#endif
  }

  template <int d, int dT>
  Matrix<double>
  DSBasis<d, dT>::ML() const {
    // IGPMlib reference: I_Basis_Bspline_s::ML()
    
    Matrix<double> ML(ell_l()+d-2*s0+ell2<d>()-1, d-s0);

    for (int row = s0; row < d; row++)
      ML(row-s0, row-s0) = 1.0 / sqrt(ldexp(1.0, 2*row+1));
    for (int m = ell_l()-s0; m <= 2*(ell_l()-s0)+ell1<d>()-1; m++)
      for (int k = s0; k < d; k++)
     	ML(-ell_l()+d+m, k-s0) = alphaT(m, k) / sqrt(ldexp(1.0, 2*k+1));
    for (int m = 2*(ell_l()-s0)+ell1<d>(); m <= 2*(ell_l()-s0)+ell2<d>()-2; m++)
      for (int k = s0; k < d; k++)
	ML(-ell_l()+d+m, k-s0) = betaLT(m, k);

    cout << "ML=" << endl << ML << endl;
    
    return ML;
  }

  template <int d, int dT>
  Matrix<double>
  DSBasis<d, dT>::MR() const {
    // IGPMlib reference: I_Basis_Bspline_s::MR()

    Matrix<double> MR(ell_r()+d-2*s1+ell2<d>()-1, d-s1);

    for (int row = s1; row < d; row++)
      MR(row-s1, row-s1) = 1.0 / sqrt(ldexp(1.0, 2*row+1));
    for (int m = ell_r()-s1; m <= 2*(ell_r()-s1)+ell1<d>()-1; m++)
      for (int k = s1; k < d; k++)
     	MR(-ell_r()+d+m, k-s1) = alphaT(m, k) / sqrt(ldexp(1.0, 2*k+1));
    for (int m = 2*(ell_r()-s1)+ell1<d>(); m <= 2*(ell_r()-s1)+ell2<d>()-2; m++)
      for (int k = s1; k < d; k++)
	MR(-ell_r()+d+m, k-s1) = betaRT(m, k);

    cout << "MR=" << endl << MR << endl;

    return MR;
  }

  template <int d, int dT>
  Matrix<double>
  DSBasis<d, dT>::MLTp() const {
    // IGPMlib reference: I_Basis_Bspline_s::MLts()

    Matrix<double> MLTp(ellT_l()+dT-2*sT0+ell2T<d,dT>()-1, dT-sT0);

    for (int row = sT0; row < dT; row++)
      MLTp(row-sT0, row-sT0) = 1.0 / sqrt(ldexp(1.0, 2*row+1));
    for (int m = ellT_l()-sT0; m <= 2*(ellT_l()-sT0)+ell1T<d,dT>()-1; m++)
      for (int k = sT0; k < dT; k++)
     	MLTp(-ellT_l()+dT+m, k-sT0) = alpha(m, k) / sqrt(ldexp(1.0, 2*k+1));
    for (int m = 2*(ellT_l()-sT0)+ell1T<d,dT>(); m <= 2*(ellT_l()-sT0)+ell2T<d,dT>()-2; m++)
      for (int k = sT0; k < dT; k++)
	MLTp(-ellT_l()+dT+m, k-sT0) = betaL(m, k);

    cout << "MLTp=" << endl << MLTp << endl;

    return MLTp;
  }

  template <int d, int dT>
  Matrix<double>
  DSBasis<d, dT>::MRTp() const {
    // IGPMlib reference: I_Basis_Bspline_s::MRts()

    Matrix<double> MRTp(ellT_r()+dT-2*sT1+ell2T<d,dT>()-1, dT-sT1);

    for (int row = sT1; row < dT; row++)
      MRTp(row-sT1, row-sT1) = 1.0 / sqrt(ldexp(1.0, 2*row+1));
    for (int m = ellT_r()-sT1; m <= 2*(ellT_r()-sT1)+ell1T<d,dT>()-1; m++)
      for (int k = sT1; k < dT; k++)
     	MRTp(-ellT_r()+dT+m, k-sT1) = alpha(m, k) / sqrt(ldexp(1.0, 2*k+1));
    for (int m = 2*(ellT_r()-sT1)+ell1T<d,dT>(); m <= 2*(ellT_r()-sT1)+ell2T<d,dT>()-2; m++)
      for (int k = sT1; k < dT; k++)
	MRTp(-ellT_r()+dT+m, k-sT1) = betaR(m, k);

    cout << "MRTp=" << endl << MRTp << endl;
 
    return MRTp;
  }

  template <int d, int dT>
  void
  DSBasis<d,dT>::setup_Mj0(const Matrix<double>& ML, const Matrix<double>& MR, SparseMatrix<double>& Mj0) {
    // IGPMlib reference: I_Basis_Bspline_s::Mj0()
    
    // TODO: enhance readability! (<-> [DKU section 3.5])

    const int nj  = Deltasize(j0());
    const int njp = Deltasize(j0()+1);
    Mj0.resize(njp, nj);

    for (unsigned int i = 0; i < ML.row_dimension(); i++)
      for (unsigned int k = 0; k < ML.column_dimension(); k++)
 	Mj0.set_entry(i, k, ML.get_entry(i, k));
    
    for (unsigned int i = 0; i < MR.row_dimension(); i++)
      for (unsigned int k = 0; k < MR.column_dimension(); k++)
 	Mj0.set_entry(njp-i-1, nj-k-1, MR.get_entry(i, k));
    
    int startrow = d+ell_l()+ell1<d>()-2*s0;
    for (int col = d-s0; col < nj-(d-s1); col++, startrow+=2) {
      int row = startrow;
      for (MultivariateLaurentPolynomial<double, 1>::const_iterator it(cdf.a().begin());
	   it != cdf.a().end(); ++it, row++)
	Mj0.set_entry(row, col, M_SQRT1_2 * *it);
    }

    cout << "Mj0=" << endl << Mj0 << endl;
  }
  
  template <int d, int dT>
  void
  DSBasis<d,dT>::setup_Mj0Tp(const Matrix<double>& MLTp, const Matrix<double>& MRTp, SparseMatrix<double>& Mj0Tp) {
    // IGPMlib reference: I_Basis_Bspline_s::Mj0ts()

    const int nj  = Deltasize(j0());
    const int njp = Deltasize(j0()+1);
    Mj0Tp.resize(njp, nj);
    
    for (unsigned int i = 0; i < MLTp.row_dimension(); i++)
      for (unsigned int k = 0; k < MLTp.column_dimension(); k++)
 	Mj0Tp.set_entry(i, k, MLTp.get_entry(i, k));
    
    for (unsigned int i = 0; i < MRTp.row_dimension(); i++)
      for (unsigned int k = 0; k < MRTp.column_dimension(); k++)
 	Mj0Tp.set_entry(njp-i-1, nj-k-1, MRTp.get_entry(i, k));
    
    int startrow = dT+ellT_l()+ell1T<d,dT>()-2*sT0;
    for (int col = dT-sT0; col < nj-(dT-sT1); col++, startrow+=2) {
      int row = startrow;
      for (MultivariateLaurentPolynomial<double, 1>::const_iterator it(cdf.aT().begin());
	   it != cdf.aT().end(); ++it, row++)
	Mj0Tp.set_entry(row, col, M_SQRT1_2 * *it);
    }
    
    cout << "Mj0Tp=" << endl << Mj0Tp << endl;
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::F(SparseMatrix<double>& FF) {
    // IGPMlib reference: I_Basis_Bspline_s::F()
    
    const int FLow = ell_l()-s0+(d%2);       // start column index for F_j in (4.1.14)
    const int FUp  = FLow+(DeltaRmin(j0())-DeltaLmax())-1; // end column index for F_j in (4.1.14)
    
    // (4.1.14):
    
    FF.resize(Deltasize(j0()+1), 1<<j0());

    for (int r = 0; r < FLow; r++)
      FF.set_entry(r+d-s0, r, 1.0);
    
    int i = d+ell_l()+(d%2)-1-2*s0;
    for (int r = FLow; r <= FUp; r++) {
      FF.set_entry(i, r-1, 1.0);
      i += 2;
    } 
    
    i = Deltasize(j0()+1)-d-1+s1;
    for (int r = (1<<j0()); r >= FUp+1; r--) {
      FF.set_entry(i, r-1, 1.0);
      i--;
    }

    cout << "F=" << endl << FF << endl;
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::P(const Matrix<double>& ML, const Matrix<double>& MR, SparseMatrix<double>& PP) {
    // IGPMlib reference: I_Basis_Bspline_s::P()
    
    // (4.1.22):

    PP.diagonal(Deltasize(j0()+1), 1.0);
    
    for (unsigned int i = 0; i < ML.row_dimension(); i++)
      for (unsigned int k = 0; k < ML.column_dimension(); k++)
	PP.set_entry(i, k, ML.get_entry(i, k));

    for (unsigned int i = 0; i < MR.row_dimension(); i++)
      for (unsigned int k = 0; k < MR.column_dimension(); k++)
	PP.set_entry(Deltasize(j0()+1)-i-1, Deltasize(j0()+1)-k-1, MR.get_entry(i, k));

    cout << "P=" << endl << PP << endl;
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::GSetup(SparseMatrix<double>& A, SparseMatrix<double>& H, SparseMatrix<double>& Hinv) {
    // IGPMlib reference: I_Basis_Bspline_s::GSetup()

    // (4.1.13):
    
    const int nj  = Deltasize(j0());
    const int njp = Deltasize(j0()+1);
    A.resize(njp, nj);

    for (int row = 0; row < d-s0; row++)
      A.set_entry(row, row, 1.0);
    
    int startrow = d+ell_l()+ell1<d>()-2*s0;
    for (int col = d-s0; col < nj-(d-s1); col++, startrow+=2) {
      int row = startrow;
      for (MultivariateLaurentPolynomial<double, 1>::const_iterator it(cdf.a().begin());
 	   it != cdf.a().end(); ++it, row++) {
 	A.set_entry(row, col, M_SQRT1_2 * *it);
      }
    }
    
    for (int row = njp-1, col = nj-1; col > nj-1-(d-s1); row--, col--)
      A.set_entry(row, col, 1.0);

    // prepare H, Hinv for elimination process:
    H   .diagonal(njp, 1.0);
    Hinv.diagonal(njp, 1.0);

    cout << "A=" << endl << A << endl;
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::GElim(SparseMatrix<double>& A, SparseMatrix<double>& H, SparseMatrix<double>& Hinv) {
    // IGPMlib reference: I_Basis_Bspline_s::gelim()
    
    // A_j=A_j^{(0)} in (4.1.1) is a q times p matrix
    const int firstcol = d-s0; // first column of A_j^{(d)} in Ahat_j^{(d)}
    const int lastcol  = (Deltasize(j0())-1)-(d-s1); // last column
    const int firstrow = d+ell_l()+ell1<d>()-2*s0; // first row of A_j^{(d)} in Ahat_j^{(d)}
    const int lastrow  = (Deltasize(j0()+1)-1)-(d+ell_r()-ell2<d>()+(d%2)-2*s1); // last row
    
    int p = lastcol-firstcol+1;
//     int q = lastrow-firstrow+1;

    SparseMatrix<double> help;
    
    // elimination (4.1.4)ff.:
    for (int i = 1; i <= d; i++) {
      help.diagonal(Deltasize(j0()+1), 1.0);

      const int elimrow = i%2 ? firstrow+(i-1)/2 : lastrow-(int)floor((i-1)/2.);

      const int HhatLow = i%2 ? elimrow : ell_l()+ell2<d>()+2-(d%2)-(i/2)-2*s0;
      const int HhatUp  = i%2 ? HhatLow + 2*p-1+(d+(d%2))/2 : elimrow;
      
      if (i%2) // i odd, elimination from above (4.1.4a)
	{
	  assert(fabs(A.get_entry(elimrow+1, firstcol)) >= 1e-10);
	  const double Uentry = -A.get_entry(elimrow, firstcol) / A.get_entry(elimrow+1, firstcol);
	  
	  // insert Uentry in Hhat
	  for (int k = HhatLow; k <= HhatUp; k += 2)
	    help.set_entry(k, k+1, Uentry);
	}
      else // i even, elimination from below (4.1.4b)
	{
	  assert(fabs(A.get_entry(elimrow-1, lastcol)) >= 1e-10);
	  const double Lentry = -A.get_entry(elimrow, lastcol) / A.get_entry(elimrow-1, lastcol);
	  
	  // insert Lentry in Hhat
	  for (int k = HhatLow; k <= HhatUp; k += 2)
	    help.set_entry(k+1, k, Lentry);
	}
      
      A = help * A;
      H = help * H;
      
      A.compress(1e-14);

      // invert help
      if (i%2) {
	for (int k = HhatLow; k <= HhatUp; k += 2)
	  help.set_entry(k, k+1, -help.get_entry(k, k+1));
      }	else {
	for (int k = HhatLow; k <= HhatUp; k += 2)
	  help.set_entry(k+1, k, -help.get_entry(k+1, k));
      }
      
      Hinv = Hinv * help;
    }
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::BT(const SparseMatrix<double>& A, SparseMatrix<double>& BB) {
    // IGPMlib reference: I_Basis_Bspline_s::Btr()
    
    const int p = (1<<j0()) - ell_l() - ell_r() - (d%2) + 1;
    const int llow = ell_l()-d;
    
    BB.resize(Deltasize(j0()+1), Deltasize(j0()));

    for (int r = 0; r < d-s0; r++)
      BB.set_entry(r, r, 1.0);

    const double help = 1./A.get_entry(d+ell_l()+ell1<d>()+ell2<d>(), d);

    for (int c = d-s0, r = d+ell_l()+ell1<d>()+ell2<d>(); c < d+p+s1; c++, r += 2)
      BB.set_entry(r-2*s0, c, help);

    for (int r = DeltaRmax(j0())-d+1+s1; r <= DeltaRmax(j0()); r++)
      BB.set_entry(-llow+r+DeltaRmax(j0()+1)-DeltaRmax(j0()), -llow+r, 1.0);
  }

  template <int d, int dT>
  void
  DSBasis<d, dT>::InvertP(const SparseMatrix<double>& PP, SparseMatrix<double>& PPinv) {
    // IGPMlib reference: I_Basis_Bspline_s::InverseP()
    
    PPinv.diagonal(PP.row_dimension(), 1.0);
    
    const int msize_l = d+ell_l()+ell2<d>()-1;
    const int msize_r = d+ell_r()+ell2<d>()-1;

    Matrix<double> ml;
    ml.diagonal(msize_l, 1.0);
    for (int i = 0; i < msize_l; i++)
      for (int k = 0; k <= d; k++)
	ml.set_entry(i, k, PP.get_entry(i, k));
    
    Matrix<double> mr;
    mr.diagonal(msize_r, 1.0);
    for (int i = 0; i < msize_r; i++)
      for (int k = 0; k <= d; k++)
	mr.set_entry(i, msize_r-d-1+k, PP.get_entry(PP.row_dimension()-msize_r+i, PP.column_dimension()-d-1+k));
    
    Matrix<double> mlinv, mrinv;
    QUDecomposition<double>(ml).inverse(mlinv);
    QUDecomposition<double>(mr).inverse(mrinv);
    
    for (int i = 0; i < msize_l; i++)
      for (int k = 0; k <= d; k++)
	PPinv.set_entry(i, k, mlinv.get_entry(i, k));
    for (int i = 0; i < msize_r; i++)
      for (int k = 0; k <= d; k++) {
	PPinv.set_entry(PP.row_dimension()-msize_r+i, PP.column_dimension()-d-1+k, mrinv.get_entry(i, msize_r-d-1+k));
      }
  }
}
