//**************************************************************************
// File:    integrTime.h                                                   *
// Content: classes that perform time-integration steps                    *
// Author:  Sven Gross, Joerg Peters, Volker Reichelt, IGPM RWTH Aachen    *
// Version: 0.1                                                            *
// History: begin - Nov, 19 2001                                           *
//**************************************************************************

#ifndef DROPS_STO_INTEGRTIME_H
#define DROPS_STO_INTEGRTIME_H

#include "stokes/instatstokes.h"
#include "num/stokessolver.h"

namespace DROPS
{

template <class StokesT, class SolverT>
class InstatStokesThetaSchemeCL
//**************************************************************************
//  for solving the instationary Stokes equation of type StokesT with a
//  1-step-theta-scheme: theta=1   -> impl. Euler (BDF1, backward Euler)
//                       theta=1/2 -> Crank-Nicholson (Trapezregel)
//
//  Inner stationary Stokes-type problems are solved with a SolverT-solver.
//  The matrices A, B, M and the rhs b, c of the Stokes class have to be set
//  properly! After construction, SetTimeStep has to be called once. Then
//  every DoStep performs one step in time. Changing time steps require 
//  further calls to SetTimeStep.
//**************************************************************************
{
  private:
    StokesT& _Stokes;
    SolverT& _solver;
    
    VelVecDescCL *_b, *_old_b;        // rhs + couplings with poisson matrix A
    VelVecDescCL *_cplM, *_old_cplM;  // couplings with mass matrix M
    VectorCL      _rhs;
    MatrixCL      _mat;               // M + theta*dt*A
    
    double _theta, _dt;
    
  public:
    InstatStokesThetaSchemeCL( StokesT& Stokes, SolverT& solver, double theta= 0.5)
        : _Stokes( Stokes), _solver( solver), _b( &Stokes.b), _old_b( new VelVecDescCL),
          _cplM( new VelVecDescCL), _old_cplM( new VelVecDescCL), _rhs( Stokes.b.RowIdx->NumUnknowns), 
          _theta( theta)
    { 
        _old_b->SetIdx( _b->RowIdx); _cplM->SetIdx( _b->RowIdx); _old_cplM->SetIdx( _b->RowIdx);
        _Stokes.SetupInstatRhs( _old_b, &_Stokes.c, _old_cplM, _Stokes.t, _old_b, _Stokes.t);
    }

    ~InstatStokesThetaSchemeCL()
    {
        if (_old_b == &_Stokes.b)
            delete _b;
        else
            delete _old_b; 
        delete _cplM; delete _old_cplM;
    }
    
    double GetTheta()    const { return _theta; }
    double GetTime()     const { return _Stokes.t; }
    double GetTimeStep() const { return _dt; }

    void SetTimeStep( double dt)
    {
        _dt= dt;
        _mat.LinComb( 1., _Stokes.M.Data, _theta*dt, _Stokes.A.Data);
    }
       
    void DoStep( VectorCL& v, VectorCL& p);
};


//**************************************************************************
// Preconditioner for the instationary Stokes-equations.
// cf. "Iterative Techniques For Time Dependent Stokes Problems",
//     James H. Bramble, Joseph E. Pasciak, January 1994
//
// A Poisson-problem with natural boundary-conditions for the pressure is
// solved via 1 SSOR-step, a problem with the mass-matrix aswell.
// The constant k_ has to be chosen according to h and dt, see Theorem 4.1
// of the above paper. 
// k_ = theta*dt/Re will do a good job, 
// where Re is proportional to the ratio density/viscosity.
//
// A_ is the pressure-Poisson-Matrix for natural boundary-conditions, M_ the
// pressure-mass-matrix.
//**************************************************************************
class ISPreCL
{
  private:
    MatrixCL& A_;
    MatrixCL& M_;
    double    k_;
    SSORPcCL  ssor_;

  public:
    ISPreCL( MatrixCL& A_pr, MatrixCL& M_pr,
        double k_pc= 0, double om= 1)
        : A_( A_pr), M_( M_pr), k_( k_pc), ssor_( om)  {}

    template <typename Mat, typename Vec>
    void Apply(const Mat&, Vec& p, const Vec& c) const;
};


//**************************************************************************
// Preconditioner for the instationary Stokes-equations.
// Confer ISPreCL for details. This preconditioner uses a few CG-iterations
// to solve the linear systems.
// It is well suited for InexactUzawa-Solvers.
//**************************************************************************
class ISNonlinearPreCL
{
  private:
    MatrixCL&  A_;
    MatrixCL&  M_;
    double     k_;
    mutable PCG_SsorCL solver_;

  public:
    ISNonlinearPreCL( MatrixCL& A_pr, MatrixCL& M_pr,
        double k_pc= 0, double omega= 1.0)
        : A_( A_pr), M_( M_pr), k_( k_pc),
          solver_( SSORPcCL( omega), 6, 1e-30)  {}

    template <typename Mat, typename Vec>
    void Apply(const Mat&, Vec& p, const Vec& c) const;
};


//**************************************************************************
// Preconditioner for the instationary Stokes-equations.
// cf. "Iterative Techniques For Time Dependent Stokes Problems",
//     James H. Bramble, Joseph E. Pasciak, January 1994
//
// Confer ISPreCL for details. This preconditioner uses multigrid-solvers.
//**************************************************************************
class ISMGPreCL
{
  private:
    const Uint sm; // how many smoothing steps?
    const int lvl; // how many levels? (-1=all)
    const double omega; // relaxation parameter for smoother
    DROPS::SSORsmoothCL smoother; // symmetric Gauss-Seidel with over-relaxation
    DROPS::SSORPcCL directpc;
    mutable DROPS::PCG_SsorCL solver;

    DROPS::MGDataCL& A_;
    DROPS::MGDataCL& M_;
    DROPS::Uint max_iter_;
    double k_;
    std::vector<DROPS::VectorCL> ones_;

  public:
    ISMGPreCL(DROPS::MGDataCL& A_pr, DROPS::MGDataCL& M_pr,
              double k_pc, DROPS::Uint max_iter)
        :sm( 1), lvl( -1), omega( 1.0), smoother( omega), solver( directpc, 200, 1e-12),
         A_( A_pr), M_( M_pr), max_iter_( max_iter), k_( k_pc),
         ones_( M_.size()) {
        // Compute projection on constant pressure function only once.
        Uint i= 0;
        for (const_MGDataIterCL it= M_.begin(); it != M_.end(); ++it, ++i) {
            ones_[i].resize( it->Idx.NumUnknowns, 1.0/it->Idx.NumUnknowns);
        }
    }

    template <typename Mat, typename Vec>
    void Apply(const Mat&, Vec& p, const Vec& c) const;
};


//**************************************************************************
// Preconditioner for the instationary Stokes-equations.
// cf. "Iterative Techniques For Time Dependent Stokes Problems",
//     James H. Bramble, Joseph E. Pasciak, January 1994
//
// Confer ISPreCL for details regarding preconditioning of S. This
// preconditioner uses multigrid-solvers.
// It is a block-diagonal-preconditioner for Minres-solvers.
//**************************************************************************
class ISMinresMGPreCL
{
  private:
    const Uint sm; // how many smoothing steps?
    const int lvl; // how many levels? (-1=all)
    const double omega; // relaxation parameter for smoother
    SSORsmoothCL smoother;  // Symmetric-Gauss-Seidel with over-relaxation
    SSORPcCL directpc;
    mutable PCG_SsorCL solver;

    DROPS::MGDataCL& Avel_;
    DROPS::MGDataCL& Apr_;
    DROPS::MGDataCL& Mpr_;
    DROPS::Uint iter_vel_;
    DROPS::Uint iter_prA_;
    DROPS::Uint iter_prM_;
    double tol_prA_;
    double k_;
    std::vector<DROPS::VectorCL> ones_;

  public:
    ISMinresMGPreCL(DROPS::MGDataCL& A_vel,
                    DROPS::MGDataCL& A_pr, DROPS::MGDataCL& M_pr,
                    double k_pc, DROPS::Uint iter_vel, DROPS::Uint iter_prA,
                    DROPS::Uint iter_prM, double tol_prA)
        : sm( 1), lvl( -1), omega( 1.0), smoother( omega), solver( directpc, 200, 1e-12),
          Avel_( A_vel), Apr_( A_pr), Mpr_( M_pr), iter_vel_( iter_vel),
         iter_prA_( iter_prA), iter_prM_( iter_prM), tol_prA_( tol_prA), k_( k_pc),
         ones_( Mpr_.size())
    {
        // Compute projection on constant pressure function only once.
        Uint i= 0;
        for (const_MGDataIterCL it= Mpr_.begin(); it != Mpr_.end(); ++it, ++i) {
            ones_[i].resize( it->Idx.NumUnknowns, 1.0/it->Idx.NumUnknowns);
        }
    }

    template <typename Mat, typename Vec>
    void
    Apply(const Mat& /*A*/, const Mat& /*B*/, Vec& v, Vec& p, const Vec& b, const Vec& c) const;
};


//=================================
//     template definitions
//=================================

template <class StokesT, class SolverT>
void InstatStokesThetaSchemeCL<StokesT,SolverT>::DoStep( VectorCL& v, VectorCL& p)
{
    _Stokes.t+= _dt;
    _Stokes.SetupInstatRhs( _b, &_Stokes.c, _cplM, _Stokes.t, _b, _Stokes.t);

    _rhs=  _Stokes.A.Data * v;
    _rhs*= (_theta-1.)*_dt;
    _rhs+= _Stokes.M.Data*v + _cplM->Data - _old_cplM->Data
         + _dt*( _theta*_b->Data + (1.-_theta)*_old_b->Data);

    p*= _dt;
    _solver.Solve( _mat, _Stokes.B.Data, v, p, _rhs, _Stokes.c.Data);
    p/= _dt;

    std::swap( _b, _old_b);
    std::swap( _cplM, _old_cplM);
}


template <typename Mat, typename Vec>
void ISPreCL::Apply(const Mat&, Vec& p, const Vec& c) const
{
//    double new_res;
//    double old_res= norm( c);
    ssor_.Apply( A_, p, c);
//    std::cerr << " residual: " <<  (new_res= norm( A_*p - c)) << '\t';
//    std::cerr << " reduction: " << new_res/old_res << '\n';
//    if (c.size() != p2_.size()) p2_.resize( c.size());
//    double mnew_res;
//    double mold_res= norm( c);
    Vec p2_( c.size());
    ssor_.Apply( M_, p2_, c);
//    std::cerr << " residual: " <<  (mnew_res= norm( M_*p2_ - c)) << '\t';
//    std::cerr << " reduction: " << mnew_res/mold_res << '\n';
    axpy( k_, p2_, p); // p+= k_*p2_;
}

template <typename Mat, typename Vec>
void ISNonlinearPreCL::Apply(const Mat&, Vec& p, const Vec& c) const
{
    p= 0.0;
    solver_.Solve( A_, p, c);
    Vec p2_( c.size());
    solver_.Solve( M_, p2_, c);
    axpy( k_, p2_, p); // p+= k_*p2_;
}


template<class SmootherCL, class DirectSolverCL>
void
MGMPr(const std::vector<VectorCL>::const_iterator& ones,
      const const_MGDataIterCL& begin, const const_MGDataIterCL& fine, VectorCL& x, const VectorCL& b, 
      const SmootherCL& Smoother, const Uint smoothSteps, 
    DirectSolverCL& Solver, const int numLevel, const int numUnknDirect)
// Multigrid method, V-cycle. If numLevel==0 or #Unknowns <= numUnknDirect,
// the direct solver Solver is used.
// If one of the parameters is -1, it will be neglected.
// If MGData.begin() has been reached, the direct solver is used too.
// Concerning the stabilization see Hackbusch Multigrid-Methods and Applications;
// Basically we project on the orthogonal complement of the kernel of A before
// the coarse-grid correction.
{
    const_MGDataIterCL coarse= fine;
    --coarse;
    if(  ( numLevel==-1      ? false : numLevel==0 )
       ||( numUnknDirect==-1 ? false : x.size() <= static_cast<Uint>(numUnknDirect) ) 
       || fine==begin)
    { // use direct solver
        Solver.Solve( fine->A.Data, x, b);
        x-= dot( *ones, x);
        return;
    }
    VectorCL d(coarse->Idx.NumUnknowns), e(coarse->Idx.NumUnknowns);
    // presmoothing
    for (Uint i=0; i<smoothSteps; ++i) Smoother.Apply( fine->A.Data, x, b);
    // restriction of defect
    d= transp_mul( fine->P.Data, VectorCL( b - fine->A.Data*x));
    d-= dot( *(ones-1), d);
    // calculate coarse grid correction
    MGMPr( ones-1, begin, coarse, e, d, Smoother, smoothSteps, Solver, (numLevel==-1 ? -1 : numLevel-1), numUnknDirect);
    // add coarse grid correction
    x+= fine->P.Data * e;
    // postsmoothing
    for (Uint i=0; i<smoothSteps; ++i) Smoother.Apply( fine->A.Data, x, b);
    // This projection could probably be avoided, but it is cheap and with it,
    // we are on the safe side.
    x-= dot( *ones, x);
}


template <typename Mat, typename Vec>
void
ISMGPreCL::Apply(const Mat&, Vec& p, const Vec& c) const
{
    p= 0.0;
    const Vec c2_= c - dot( ones_.back(), c);
//    double new_res= norm( A_.back().A.Data*p - c2_);
//    double old_res;
//    std::cerr << "IsMGPcCL: "
//              << norm( cc) << " " << norm( A_.back().A.Data*c2_) << " "
//              << "\tPressure: iterations: " << max_iter_ <<'\t';
    for (DROPS::Uint i=0; i<max_iter_; ++i) {
        MGMPr( --ones_.end(), A_.begin(), --A_.end(), p, c2_, smoother, sm, solver, lvl, -1);
//        old_res= new_res;
//        std::cerr << " residual: " <<  (new_res= norm( A_.back().A.Data*p - c2_))
//                  << '\t' << norm( p)
//        std::cerr << " reduction: " << new_res/old_res << '\n';
    }
    Vec p2_( c.size());
//    double mnew_res= norm( M_.back().A.Data*p2_ - c);
//    double mold_res;
    for (DROPS::Uint i=0; i<1; ++i) {
        DROPS::MGM( M_.begin(), --M_.end(), p2_, c, smoother, sm, solver, lvl, -1);
//        mold_res= mnew_res;
//        std::cerr << "IsMGPcCL: Mass: residual: " <<  (mnew_res= norm( M_.back().A.Data*p2_ - c)) << '\t';
//        std::cerr << " reduction: " << mnew_res/mold_res << '\n';
    }
//    std::cerr << "M: iterations: " << 1 << '\t'
//              << "residual: " << norm( M_.back().A.Data*p2_ - c) << '\n';
    axpy( k_, p2_, p); // p+= k_*p2_;
}


template <typename Mat, typename Vec>
void
ISMinresMGPreCL::Apply(const Mat& /*A*/, const Mat& /*B*/, Vec& v, Vec& p, const Vec& b, const Vec& c) const
{
    for (DROPS::Uint i=0; i<iter_vel_; ++i)
        DROPS::MGM( Avel_.begin(), --Avel_.end(), v, b, smoother, sm, solver, lvl, -1);
//    std::cerr << "ISMinresMGPreCL: Velocity: iterations: " << iter_vel_ << '\t'
//              << " residual: " <<  (Avel_.back().A.Data*v - b).norm() << '\t';

    p-= dot( ones_.back(), p);
//    double new_res= (Apr_.back().A.Data*p - c).norm();
//    double old_res;
//    std::cerr << "Pressure: iterations: " << iter_prA_ <<'\t';
    for (DROPS::Uint i=0; i<iter_prA_; ++i) {
        DROPS::MGMPr( --ones_.end(), Apr_.begin(), --Apr_.end(), p, c, smoother, sm, solver, lvl, -1);
//        old_res= new_res;
//        std::cerr << " residual: " <<  (new_res= (Apr_.back().A.Data*p - c).norm()) << '\t';
//        std::cerr << " reduction: " << new_res/old_res << '\n';
    }
//    std::cerr << " residual: " <<  (Apr_.back().A.Data*p - c).norm() << '\t';

    Vec p2( p.size());
    for (DROPS::Uint i=0; i<iter_prM_; ++i)
        DROPS::MGM( Mpr_.begin(), --Mpr_.end(), p2, c, smoother, sm, solver, lvl, -1);
//    std::cerr << "Mass: iterations: " << iter_prM_ << '\t'
//              << " residual: " <<  (Mpr_.back().A.Data*p2 - c).norm() << '\n';

//    p+= k_*p2;
    axpy( k_, p2, p);
}

}    // end of namespace DROPS

#endif
