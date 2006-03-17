// implementation for p_basis.h

#include <cassert>
#include <cmath>
#include <numerics/schoenberg_splines.h>
#include <algebra/triangular_matrix.h>
#include <utils/tiny_tools.h>
#include <interval/boundary_gramian.h>

namespace WaveletTL
{
  template <int d, int dT>
  PBasis<d,dT>::PBasis(const int s0, const int s1) {
    assert(d >= 2 && d <= 3);
    assert(s0 >= d-2 && s1 >= d-2);

    this->s0 = s0;
    this->s1 = s1;

    setup();
  }


  template <int d, int dT>
  void
  PBasis<d,dT>::setup() {
    // For simplicity, we do not implement a fully generic setup here
    // but only setup the parameters for several important special cases from [P].
    // Namely, we restrict the setting to the case s0 >= d-2, where no "additional"
    // interior dual generators have to be constructed. In a later version of
    // this class, we will fix this.

#if 1
    // choose j0 s.th. the supports of the dual boundary generators do not overlap
    j0_ = (int) ceil(log(ell2T<d,dT>()-ell1T<d,dT>()+std::max(s0,s1)+1.-d)/M_LN2+1);
#else
    // choose j0 s.th. the supports of the primal boundary generators do not overlap
    // the supports of the dual boundary generators from the other endpoint
    j0_ = (int) ceil(log(ell2T<d,dT>()-ell1T<d,dT>()+2*std::max(s0,s1)+dT-d+1.)/M_LN2);
#endif
    
    // setup the refinement matrix block for all "real" primal boundary B-splines,
    // obeying the block structure (3.15)
    // (ignoring the current values of s0 (and s1))
    MathTL::SchoenbergKnotSequence<d> sknots;
    Matrix<double> ML_0;
    MathTL::compute_Bspline_refinement_matrix<d>(&sknots, ML_0);
    
    // The total number of (left) boundary generators is always exactly dT
    // (to be able reproduce all polynomials by the dual generators).
    ML_.resize(3*dT+s0-1, dT);
    for (int column = s0; column < d-1; column++)
      for (int row = s0; row < 2*(d-1); row++)
	ML_.set_entry(row-s0, column-s0, ML_0.get_entry(row,column));
    for (int k = d; k <= dT+s0; k++)
      for (int n = 2*k-d; n <= 2*k; n++)
 	ML_.set_entry(n-1-s0,k-1-s0,cdf.a().get_coefficient(MultiIndex<int,1>(-(d/2)+n+d-2*k)));
    cout << "ML=" << endl << ML_;

    MR_.resize(3*dT+s1-1, dT);
    for (int column = s1; column < d-1; column++)
      for (int row = s1; row < 2*(d-1); row++)
	MR_.set_entry(row-s1, column-s1, ML_0.get_entry(row,column));
    for (int k = d; k <= dT+s1; k++)
      for (int n = 2*k-d; n <= 2*k; n++)
 	MR_.set_entry(n-1-s1,k-1-s1,cdf.a().get_coefficient(MultiIndex<int,1>(-(d/2)+n+d-2*k)));
    cout << "MR=" << endl << MR_;

    // setup the expansion coefficients of the unbiorthogonalized dual generators
    // w.r.t. the truncated CDF generators, see [DKU, Lemma 3.1]
    Matrix<double> MLTp(3*dT+s0-1, dT);
    for (unsigned int r = 0; r < dT; r++) {
      MLTp.set_entry(r, r, ldexp(1.0, -r));
      for (int m = ellT_l(); m <= 2*ellT_l()+ell1T<d,dT>()-1; m++)
	MLTp.set_entry(dT+m-ellT_l(), r, alpha(m, r));
      for (int m = 2*ellT_l()+ell1T<d,dT>(); m <= 2*ellT_l()+ell2T<d,dT>()-2; m++)
	MLTp.set_entry(dT+m-ellT_l(), r, betaL(m, r));
    }
    cout << "MLTp=" << endl << MLTp;
        
    Matrix<double> MRTp(3*dT+s1-1, dT);
    for (unsigned int r = 0; r < dT; r++) {
      MRTp.set_entry(r, r, ldexp(1.0, -r));
      for (int m = ellT_r(); m <= 2*ellT_r()+ell1T<d,dT>()-1; m++)
	MRTp.set_entry(dT+m-ellT_r(), r, alpha(m, r));
      for (int m = 2*ellT_r()+ell1T<d,dT>(); m <= 2*ellT_r()+ell2T<d,dT>()-2; m++)
	MRTp.set_entry(dT+m-ellT_r(), r, betaR(m, r));
    }
    cout << "MRTp=" << endl << MRTp;

    // for the biorthogonalization of the generators,
    // compute the gramian matrix of the primal and dual boundary generators
    compute_biorthogonal_boundary_gramian
      <CDFMask_primal<d>,CDFMask_dual<d,dT> >(ML_, MLTp, GammaL);
    cout << "GammaL=" << endl << GammaL;

    compute_biorthogonal_boundary_gramian
      <CDFMask_primal<d>,CDFMask_dual<d,dT> >(MR_, MRTp, GammaR);
    cout << "GammaR=" << endl << GammaR;

    setup_Mj0(ML_, MR_, Mj0); // [DKU, (3.5.1)]
    Mj0.compress(1e-8);
    
    SparseMatrix<double> mj0tp; setup_Mj0Tp(MLTp, MRTp, mj0tp); // [DKU, (3.5.5)]
    mj0tp.compress(1e-8);
    
    // biorthogonalize the generators
    setup_Cj();
    Mj0T = transpose(inv_CjpT) * mj0tp * transpose(CjT); // [DKU, (2.4.3)]
    Mj0T.compress(1e-8);

    Mj0.scale(M_SQRT2);
    cout << "Mj0 (without SQRT1_2)=" << endl << Mj0;
    Mj0.scale(M_SQRT1_2);

    Mj0T.scale(M_SQRT2);
    cout << "Mj0T (without SQRT1_2)=" << endl << Mj0T;
    Mj0T.scale(M_SQRT1_2);

#if 1
    cout << "PBasis(): check biorthogonality of Mj0, Mj0T:" << endl;
//     cout << "Mj0=" << endl << Mj0 << endl << "Mj0T=" << endl << Mj0T << endl;

    SparseMatrix<double> testbio0 = transpose(Mj0) * Mj0T;
//     cout << "Mj0^T*Mj0T=" << endl << testbio0 << endl;
    for (unsigned int i = 0; i < testbio0.row_dimension(); i++)
      testbio0.set_entry(i, i, testbio0.get_entry(i, i) - 1.0);
    cout << "* ||Mj0^T*Mj0T-I||_infty: " << row_sum_norm(testbio0) << endl;

    testbio0 = transpose(Mj0T) * Mj0;
    //     cout << "Mj0T*Mj0^T=" << endl << testbio0 << endl;
    for (unsigned int i = 0; i < testbio0.row_dimension(); i++)
      testbio0.set_entry(i, i, testbio0.get_entry(i, i) - 1.0);
    cout << "* ||Mj0T^T*Mj0-I||_infty: " << row_sum_norm(testbio0) << endl;
#endif    

    // construction of the wavelet basis: initial stable completion, cf. [DKU section 4.1]

    SparseMatrix<double> FF; F(FF);           // [DKU, (4.1.14)]
    SparseMatrix<double> PP; P(ML_, MR_, PP); // [DKU, (4.1.22)]

    SparseMatrix<double> A, H, Hinv;
    GSetup(A, H, Hinv); // [DKU, (4.1.1), (4.1.13)]

#if 1
    SparseMatrix<double> Aold(A); // for the checks below
#endif

    GElim (A, H, Hinv); // elimination [DKU, (4.1.4)ff.]
    SparseMatrix<double> BB; double binv = BT(A, BB); // [DKU, (4.1.13)]
    cout << "b^{-1}=" << binv << endl;
    
#if 1
    cout << "PBasis(): check properties (4.1.15):" << endl;
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
    cout << "PBasis(): check factorization of A:" << endl;
    SparseMatrix<double> testAfact = Aold - Hinv*A;
    cout << "* in infty-norm: " << row_sum_norm(testAfact) << endl;

    cout << "PBasis(): check that H is inverse to Hinv:" << endl;
    SparseMatrix<double> testHinv = H*Hinv;
    for (unsigned int i = 0; i < testHinv.row_dimension(); i++)
      testHinv.set_entry(i, i, testHinv.get_entry(i, i) - 1.0);
    cout << "* in infty-norm: " << row_sum_norm(testHinv) << endl;
#endif

    SparseMatrix<double> mj1ih = PP * Hinv * FF; // [DKU, (4.1.23)]
    mj1ih.scale(M_SQRT1_2); // [P, Prop. 5.6]
//     cout << "PP=" << endl << PP;
//     cout << "Hinv=" << endl << Hinv;
//     cout << "mj1ih=" << endl << mj1ih;

    SparseMatrix<double> PPinv; InvertP(PP, PPinv);

    SparseMatrix<double> help = H * PPinv;
    SparseMatrix<double> gj0ih = transpose(BB) * help; gj0ih.scale(M_SQRT2); // [P, Prop. 5.6]
    SparseMatrix<double> gj1ih = transpose(FF) * help; gj1ih.scale(M_SQRT2); // [DKU, (4.1.24)], [P, Prop. 5.6]

#if 1
    cout << "PBasis(): check initial stable completion:" << endl;
    SparseMatrix<double> mj_initial(Mj0.row_dimension(),
				    Mj0.column_dimension() + mj1ih.column_dimension());
    for (unsigned int i = 0; i < Mj0.row_dimension(); i++)
      for (unsigned int j = 0; j < Mj0.column_dimension(); j++)	{
	const double help = Mj0.get_entry(i, j);
	if (help != 0)
	  mj_initial.set_entry(i, j, help);
      }
    for (unsigned int i = 0; i < mj1ih.row_dimension(); i++)
      for (unsigned int j = 0; j < mj1ih.column_dimension(); j++) {
	const double help = mj1ih.get_entry(i, j);
	if (help != 0)
	  mj_initial.set_entry(i, j+Mj0.column_dimension(), help);
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
    // (cf. DSBasis, here we have the special case Cj=Cjp=I)
    SparseMatrix<double> I; I.diagonal(Deltasize(j0()+1), 1.0);
    Mj1  = (I - (Mj0*transpose(Mj0T))) * mj1ih;
    Mj1T = transpose(gj1ih);

    Mj0 .compress(1e-8);
    Mj1 .compress(1e-8);
    Mj0T.compress(1e-8);
    Mj1T.compress(1e-8);

    // adjust scaling factor for the interior wavelets, cf. [P, p.]
    const double scaling_factor = minus1power(d+1)*2*binv;
    Mj1 .scale(scaling_factor);
    Mj1T.scale(1./scaling_factor);
    
    Mj1.scale(M_SQRT2);
    cout << "Mj1 (without SQRT1_2)=" << endl << Mj1;
    Mj1.scale(M_SQRT1_2);

    Mj1T.scale(M_SQRT2);
    cout << "Mj1T (without SQRT1_2)=" << endl << Mj1T;
    Mj1T.scale(M_SQRT1_2);

#if 1
    cout << "PBasis(): check new stable completion:" << endl;
    
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
//     cout << "Mj*Gj-I=" << endl << test_new << endl;
    cout << "* ||Mj*Gj-I||_infty: " << row_sum_norm(test_new) << endl;

    test_new = gj_new * mj_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
//     cout << "Gj*Mj-I=" << endl << test_new << endl;
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
    //     cout << "MjT*GjT-I=" << endl << test_new << endl;
    cout << "* ||MjT*GjT-I||_infty: " << row_sum_norm(test_new) << endl;

    test_new = gjt_new * mjt_new;
    for (unsigned int i = 0; i < test_new.row_dimension(); i++)
      test_new.set_entry(i, i, test_new.get_entry(i, i) - 1.0);
    //     cout << "GjT*MjT-I=" << endl << test_new << endl;
    cout << "* ||GjT*MjT-I||_infty: " << row_sum_norm(test_new) << endl;
#endif
  }

  template <int d, int dT>
  const double
  PBasis<d,dT>::alpha(const int m, const unsigned int r) const {
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
	result /= ldexp(1.0, r+1) - 2.0;
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
  PBasis<d,dT>::betaL(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2T<d,dT>())/2.0); q < ellT_l(); q++)
      result += alpha(q, r) * cdf.aT().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result;
  }

  template <int d, int dT>
  const double
  PBasis<d,dT>::betaR(const int m, const unsigned int r) const {
    // [DKU] (3.2.31)
    double result = 0;
    for (int q = (int)ceil((m-ell2T<d,dT>())/2.0); q < ellT_r(); q++)
      result += alpha(q, r) * cdf.aT().get_coefficient(MultiIndex<int,1>(m-2*q));
    return result;
  }

  template <int d, int dT>
  void
  PBasis<d,dT>::setup_Mj0(const Matrix<double>& ML, const Matrix<double>& MR, SparseMatrix<double>& Mj0) {
    // IGPMlib reference: I_Basis_Bspline_s::Mj0()
    // cf. [DKU section 3.5]

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
	Mj0.set_entry(row, col, *it);
    }

    Mj0.scale(M_SQRT1_2);
    
//     cout << "Mj0=" << endl << Mj0 << endl;
  }

  template <int d, int dT>
  void
  PBasis<d,dT>::setup_Mj0Tp(const Matrix<double>& MLTp, const Matrix<double>& MRTp, SparseMatrix<double>& Mj0Tp) {
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
    
    int startrow = dT+ellT_l()+ell1T<d,dT>();
    for (int col = dT; col < nj-dT; col++, startrow+=2) {
      int row = startrow;
      for (MultivariateLaurentPolynomial<double, 1>::const_iterator it(cdf.aT().begin());
	   it != cdf.aT().end(); ++it, row++)
	Mj0Tp.set_entry(row, col, *it);
    }
    
    Mj0Tp.scale(M_SQRT1_2);

//     cout << "Mj0Tp=" << endl << Mj0Tp << endl;
  }

  template <int d, int dT>
  void PBasis<d,dT>::setup_Cj() {
    // IGPMlib reference: I_Basis_Bspline_s::setup_Cj(), ::put_Mat()

    // [DKU (5.2.5)]

    Matrix<double> CLGammaLInv;
    QUDecomposition<double>(GammaL).inverse(CLGammaLInv);
    Matrix<double> CLT = transpose(CLGammaLInv);
    
    Matrix<double> CRGammaRInv;
    QUDecomposition<double>(GammaR).inverse(CRGammaRInv);
    Matrix<double> CRT = transpose(CRGammaRInv);
    
    CjT.diagonal(Deltasize(j0()), 1.0);
    CjT.set_block(0, 0, CLT);
    CjT.set_block(Deltasize(j0())-CRT.row_dimension(),
 		  Deltasize(j0())-CRT.column_dimension(),
 		  CRT, true);

    Matrix<double> inv_CLT, inv_CRT;
    QUDecomposition<double>(CLT).inverse(inv_CLT);
    QUDecomposition<double>(CRT).inverse(inv_CRT);

    inv_CjT.diagonal(Deltasize(j0()), 1.0);
    inv_CjT.set_block(0, 0, inv_CLT);
    inv_CjT.set_block(Deltasize(j0())-inv_CRT.row_dimension(),
 		      Deltasize(j0())-inv_CRT.column_dimension(),
 		      inv_CRT, true);

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
    cout << "PBasis: testing setup of Cj:" << endl;

    SparseMatrix<double> test1 = CjT * inv_CjT;
    for (unsigned int i = 0; i < test1.row_dimension(); i++)
      test1.set_entry(i, i, test1.get_entry(i, i) - 1.0);
    cout << "* ||CjT*inv_CjT-I||_infty: " << row_sum_norm(test1) << endl;

    SparseMatrix<double> test3 = CjpT * inv_CjpT;
    for (unsigned int i = 0; i < test3.row_dimension(); i++)
      test3.set_entry(i, i, test3.get_entry(i, i) - 1.0);
    cout << "* ||CjpT*inv_CjpT-I||_infty: " << row_sum_norm(test3) << endl;

#endif
  }

  template <int d, int dT>
  void
  PBasis<d, dT>::F(SparseMatrix<double>& FF) {
    // IGPMlib reference: I_Basis_Bspline_s::F()
    
    const int FLow = ell2<d>()-1;      // first column of F_j in Fhat_j [P, p. 113]
    const int FUp  = (1<<j0())+ell1<d>(); // last column of F_j in Fhat_j
    
    // (4.1.14):
    
    FF.resize(Deltasize(j0()+1), 1<<j0());

    for (int r = 0; r < FLow; r++)
      FF.set_entry(r+d-1-s0, r, 1.0);
    
    int i = d-2+ell2<d>()-s0;
    for (int col = FLow; col <= FUp; col++, i+=2)
      FF.set_entry(i, col, 1.0);
    
    i = Deltasize(j0()+1)-d+s1;
    for (int col = (1<<j0())-1; col >= FUp+1; col--, i--)
      FF.set_entry(i, col, 1.0);

    cout << "F=" << endl << FF << endl;
  }

  template <int d, int dT>
  void
  PBasis<d, dT>::P(const Matrix<double>& ML, const Matrix<double>& MR, SparseMatrix<double>& PP) {
    // IGPMlib reference: I_Basis_Bspline_s::P()
    
    // (4.1.22):

    PP.diagonal(Deltasize(j0()+1), 1.0);
    
    for (unsigned int i = 0; i < ML.row_dimension(); i++)
      for (int k = 0; k < d-1-s0; k++)
 	PP.set_entry(i, k, ML.get_entry(i, k));

    for (unsigned int i = 0; i < MR.row_dimension(); i++)
      for (int k = 0; k < d-1-s1; k++)
 	PP.set_entry(Deltasize(j0()+1)-i-1, Deltasize(j0()+1)-k-1, MR.get_entry(i, k));

    cout << "P=" << endl << PP << endl;
  }

  template <int d, int dT>
  void
  PBasis<d, dT>::GSetup(SparseMatrix<double>& A, SparseMatrix<double>& H, SparseMatrix<double>& Hinv) {
    // IGPMlib reference: I_Basis_Bspline_s::GSetup()

    // (4.1.13):
    
    const int nj  = Deltasize(j0());
    const int njp = Deltasize(j0()+1);
    A.resize(njp, nj);

    for (int row = 0; row < d-1-s0; row++)
      A.set_entry(row, row, 1.0);
    
    int startrow = d-1-s0;
    for (int col = d-1-s0; col <= nj-d+s1; col++, startrow+=2) {
      int row = startrow;
      for (MultivariateLaurentPolynomial<double, 1>::const_iterator it(cdf.a().begin());
 	   it != cdf.a().end(); ++it, row++) {
  	A.set_entry(row, col, *it);
      }
    }
    
    for (int row = njp-1, col = nj-1; col >= nj-d+s1+1; row--, col--)
      A.set_entry(row, col, 1.0);

    // prepare H, Hinv for elimination process:
    H   .diagonal(njp, 1.0);
    Hinv.diagonal(njp, 1.0);

//     cout << "A=" << endl << A << endl;
  }

  template <int d, int dT>
  void
  PBasis<d, dT>::GElim(SparseMatrix<double>& A, SparseMatrix<double>& H, SparseMatrix<double>& Hinv) {
    // IGPMlib reference: I_Basis_Bspline_s::gelim()
    
    const int firstcol = d-1-s0; // first column of A_j^{(d)} in Ahat_j^{(d)}
    const int lastcol  = Deltasize(j0())-d+s1; // last column
    const int firstrow = d-1-s0; // first row of A_j^{(d)} in Ahat_j^{(d)}
    const int lastrow  = Deltasize(j0()+1)-d+s1; // last row
    
    SparseMatrix<double> help;

//     cout << "A=" << endl << A;
    
    // elimination (4.1.4)ff.:
    for (int i = 1; i <= d; i++) {
      help.diagonal(Deltasize(j0()+1), 1.0);

      // index of the entry in the first column of A_j^{(i)} (w.r.t. Ahat_j^{(i)})
      const int elimrow = i%2 ? firstrow+(i-1)/2 : lastrow-(int)floor((i-1)/2.);
//       cout << "i%2=" << i%2 << ", elimrow=" << elimrow << endl;

      // factorization [P, p. 112]      
      const int HhatLow = i%2 ? d-s0-((i+1)/2)%2 : d-s0+1-(d%2)-(i/2)%2;
      const int HhatUp  = i%2
	? Deltasize(j0()+1)-d+s1-abs((d%2)-((i+1)/2)%2)
	: Deltasize(j0()+1)-d+s1-abs((d%2)-(i/2)%2);
      
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
      
//       cout << "help=" << endl << help;

      A = help * A;
      H = help * H;
      
      A.compress(1e-14);

//       cout << "A=" << endl << A;

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
  double
  PBasis<d, dT>::BT(const SparseMatrix<double>& A, SparseMatrix<double>& BB) {
    // IGPMlib reference: I_Basis_Bspline_s::Btr()
    
    const int nj  = Deltasize(j0());
    const int njp = Deltasize(j0()+1);
    BB.resize(njp,nj);

    for (int r = 0; r < d-1-s0; r++)
      BB.set_entry(r, r, 1.0);

    const double binv = 1./A.get_entry(d-1-s0+ell2<d>(), d-1-s0);

    int i = d-1-s0+ell2<d>();
    for (int col = d-1-s0; col <= nj-d+s1; col++, i+=2)
      BB.set_entry(i, col, binv);

    for (int row = njp-1, col = nj-1; col >= nj-d+s1+1; row--, col--)
      BB.set_entry(row, col, 1.0);

//     cout << "BThat=" << endl << BB;

    return binv;
  }

  template <int d, int dT>
  void
  PBasis<d, dT>::InvertP(const SparseMatrix<double>& PP, SparseMatrix<double>& PPinv) {
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

#if 1
    cout << "PBasis(): check that PPinv is inverse to PP:" << endl;
    SparseMatrix<double> testPinv = PP*PPinv;
//     cout << "  PP*PPinv=" << endl << testPinv;
    for (unsigned int i = 0; i < testPinv.row_dimension(); i++)
      testPinv.set_entry(i, i, testPinv.get_entry(i, i) - 1.0);
    cout << "* in infty-norm: " << row_sum_norm(testPinv) << endl;
#endif
  }
  
}
