//**************************************************************************
// File:    mzelle_stat.cpp                                                *
// Content: flow in drop cell                                              *
// Author:  Sven Gross, Joerg Peters, Volker Reichelt, IGPM RWTH Aachen    *
//**************************************************************************

#include "geom/multigrid.h"
#include "out/output.h"
#include "geom/builder.h"
#include "stokes/instatstokes2phase.h"
#include "stokes/integrTime.h"
#include "num/stokessolver.h"
#include "out/output.h"
#include "out/ensightOut.h"
#include "levelset/levelset.h"
#include <fstream>

// rho*du/dt - mu/Re*laplace u + Dp = f + rho*g - okn
//                          -div u = 0
//                               u = u0, t=t0

// Tropfendaten:
DROPS::Point3DCL Mitte;
double           x_Mitte= 8e-3,
                 Radius= 1.75e-3,
                 Anstroem= 4*10.241632e-3;

// Glaettungszone fuer Dichte-/Viskositaetssprung
const double sm_eps= Radius*0.05;


class ZeroFlowCL
{
// \Omega_1 = Tropfen,    \Omega_2 = umgebendes Fluid
  public:
    static DROPS::Point3DCL f(const DROPS::Point3DCL&, double)
        { DROPS::Point3DCL ret(0.0); return ret; }
    DROPS::SmoothedJumpCL rho, mu;
    const double Re, We;
    DROPS::Point3DCL g;

    ZeroFlowCL() 
      : rho( DROPS::JumpCL( 955.,   1107. ), DROPS::H_sm, sm_eps),
         mu( DROPS::JumpCL( 2.6e-3, 1.2e-3), DROPS::H_sm, sm_eps),
        Re(1.), We(1.) 
    { g[0]= 9.81; }
};


DROPS::SVectorCL<3> Null( const DROPS::Point3DCL&, double)
{ return DROPS::SVectorCL<3>(0.); }

DROPS::SVectorCL<3> Inflow( const DROPS::Point3DCL& p, double)
{ 
    DROPS::SVectorCL<3> ret(0.); 
    const double s= 0.02,    // Radius Messzelle Einlauf
                 r= std::sqrt(p[1]*p[1]+p[2]*p[2]);
    ret[0]= -(r-s)*(r+s)/s/s*Anstroem; 
    return ret; 
}

double DistanceFct( const DROPS::Point3DCL& p)
{
    const DROPS::Point3DCL d= Mitte-p;
    return d.norm()-Radius;
}


namespace DROPS // for Strategy
{

template<class Coeff>
void Strategy( InstatStokes2PhaseP2P1CL<Coeff>& Stokes, double inner_iter_tol, double sigma)
// flow control
{
    typedef InstatStokes2PhaseP2P1CL<Coeff> StokesProblemT;

    MultiGridCL& MG= Stokes.GetMG();
    LevelsetP2CL lset( MG, sigma, 0.5, 0.1);

    IdxDescCL* lidx= &lset.idx;
    IdxDescCL* vidx= &Stokes.vel_idx;
    IdxDescCL* pidx= &Stokes.pr_idx;
    MatDescCL prM;

    TimerCL time;
    Stokes.CreateNumberingVel( MG.GetLastLevel(), vidx);    
    Stokes.CreateNumberingPr(  MG.GetLastLevel(), pidx);    
    lset.CreateNumbering(      MG.GetLastLevel(), lidx);
    lset.Phi.SetIdx( lidx);
    lset.Init( DistanceFct);

    MG.SizeInfo( std::cerr);
    Stokes.b.SetIdx( vidx);
    Stokes.c.SetIdx( pidx);
    Stokes.p.SetIdx( pidx);
    Stokes.v.SetIdx( vidx);
    std::cerr << Stokes.p.Data.size() << " pressure unknowns,\n";
    std::cerr << Stokes.v.Data.size() << " velocity unknowns,\n";
    std::cerr << lset.Phi.Data.size() << " levelset unknowns.\n";
    Stokes.A.SetIdx(vidx, vidx);
    Stokes.B.SetIdx(pidx, vidx);
    Stokes.M.SetIdx(vidx, vidx);
    prM.SetIdx( pidx, pidx);
    
    Stokes.InitVel( &Stokes.v, Null);

    Stokes.SetupPrMass( &prM, lset);
    
    VelVecDescCL curv( vidx);
    Stokes.SetupSystem1( &Stokes.A, &Stokes.M, &Stokes.b, &Stokes.b, &curv, lset, Stokes.t);
    Stokes.SetupSystem2( &Stokes.B, &Stokes.c, Stokes.t);
    curv.Clear();
    lset.AccumulateBndIntegral( curv);
    
    double outer_tol;
    std::cerr << "tol = "; std::cin >> outer_tol;

    EnsightP2SolOutCL ensight( MG, lidx);
    const char datgeo[]= "ensight/mz.geo", 
               datpr[] = "ensight/mz.pr",
               datvec[]= "ensight/mz.vec",
               datscl[]= "ensight/mz.scl";
    ensight.CaseBegin( "mz.case", 2);
    ensight.DescribeGeom( "Messzelle", datgeo);
    ensight.DescribeScalar( "Levelset", datscl); 
    ensight.DescribeScalar( "Pressure", datpr,  true); 
    ensight.DescribeVector( "Velocity", datvec, true); 
    ensight.putGeom( datgeo);
    ensight.putVector( datvec, Stokes.GetVelSolution(), 0);
    ensight.putScalar( datpr,  Stokes.GetPrSolution(), 0);
    ensight.putScalar( datscl, lset.GetSolution());
    ensight.Commit();

    PSchur_GSPCG_CL schurSolver( prM.Data, 1000, outer_tol, 1000, inner_iter_tol);

    time.Reset();
    schurSolver.Solve( Stokes.A.Data, Stokes.B.Data, 
        Stokes.v.Data, Stokes.p.Data, VectorCL( Stokes.b.Data + curv.Data), Stokes.c.Data);
    time.Stop();
    std::cerr << "Solving Stokes took "<<time.GetTime()<<" sec.\n";

    ensight.putVector( datvec, Stokes.GetVelSolution(), 1);
    ensight.putScalar( datpr,  Stokes.GetPrSolution(), 1);

    ensight.CaseEnd();
    std::cerr << std::endl;
}


} // end of namespace DROPS


void MarkDrop (DROPS::MultiGridCL& mg, int maxLevel= -1)
{
    for (DROPS::MultiGridCL::TriangTetraIteratorCL It(mg.GetTriangTetraBegin(maxLevel)),
             ItEnd(mg.GetTriangTetraEnd(maxLevel)); It!=ItEnd; ++It)
    {
        if ( (GetBaryCenter(*It)-Mitte).norm()<=std::max(Radius,1.5*std::pow(It->GetVolume(),1.0/3.0)) )
            It->SetRegRefMark();
    }
}


int main (int argc, char** argv)
{
  try
  {
    if (argc<4)
    {
        std::cerr << "You have to specify three parameters:\n\t" 
                  << argv[0] << " <inner_iter_tol> <num_dropref> <surf.tension> [<inflow_vel>]" << std::endl;
        return 1;
    }
    double inner_iter_tol= atof(argv[1]);
    int num_dropref= atoi(argv[2]);
    double sigma= atof(argv[3]);
    if (argc>=5)
        Anstroem= atof(argv[4]);
    // bubble position
    Mitte[0]= x_Mitte;

    std::cerr << "inner iter tol:  " << inner_iter_tol << std::endl;
    std::cerr << "drop ref:   " << num_dropref << std::endl;
    std::cerr << "surface tension: " << sigma << std::endl;
    std::cerr << "inflow vel: " << Anstroem << std::endl;

    typedef DROPS::InstatStokes2PhaseP2P1CL<ZeroFlowCL>    MyStokesCL;

    std::ifstream meshfile( "gambit/messzelle.msh");
    DROPS::ReadMeshBuilderCL builder( meshfile);
    
    const DROPS::BndCondT bc[3]= 
        { DROPS::OutflowBC, DROPS::WallBC, DROPS::DirBC};
    //    bottom,           side,          top
    const DROPS::InstatStokesVelBndDataCL::bnd_val_fun bnd_fun[3]= 
        { &Null, &Null, &Inflow}; 
        
    MyStokesCL prob(builder, ZeroFlowCL(), DROPS::InstatStokesBndDataCL( 3, bc, bnd_fun));

    DROPS::MultiGridCL& mg = prob.GetMG();
    const DROPS::BoundaryCL& bnd= mg.GetBnd();
    
    for (DROPS::BndIdxT i=0, num= bnd.GetNumBndSeg(); i<num; ++i)
    {
        std::cerr << "Bnd " << i << ": "; BndCondInfo( bc[i], std::cerr);
    }
    mg.SizeInfo( std::cerr);    
    mg.ElemInfo( std::cerr);    
//    MarkAll( mg); mg.Refine();
    for (int i=0; i<num_dropref; ++i)
    {
        MarkDrop( mg);
        mg.Refine();
    }
    std::cerr << "after refinement:\n"; mg.ElemInfo( std::cerr);    
    mg.SizeInfo( std::cerr);    
    std::cerr << DROPS::SanityMGOutCL(mg) << std::endl;

    Strategy(prob, inner_iter_tol, sigma);
    double min= prob.p.Data.min(),
           max= prob.p.Data.max();
    std::cerr << "pressure min/max: "<<min<<", "<<max<<std::endl;

    return 0;
  }
  catch (DROPS::DROPSErrCL err) { err.handle(); }
}
