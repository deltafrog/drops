/// \file instatnavstokes2phase.cpp
/// \brief classes that constitute the 2-phase Navier-Stokes problem
/// \author LNM RWTH Aachen: Joerg Grande, Sven Gross, Volker Reichelt; SC RWTH Aachen: Oliver Fortmeier

/*
 * This file is part of DROPS.
 *
 * DROPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DROPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with DROPS. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Copyright 2009 LNM/SC RWTH Aachen, Germany
*/

#include "navstokes/instatnavstokes2phase.h"

namespace DROPS
{

void InstatNavierStokes2PhaseP2P1CL::SetupNonlinear_P2 (MatrixCL& N, const VelVecDescCL* vel, VelVecDescCL* cplN, const LevelsetP2CL& lset, IdxDescCL& RowIdx, double t) const
/// Couplings with dirichlet BCs are accumulated in cplN,
/// so call cplN->Clear() before if only couplings are needed.
{
    const IdxT num_unks_vel= RowIdx.NumUnknowns();

    MatrixBuilderCL mN( &N, num_unks_vel, num_unks_vel);
    if (cplN != 0) cplN->Clear();

    const Uint lvl= RowIdx.TriangLevel();
    LocalNumbP2CL n;
#ifndef _PAR
    std::cout << "entering SetupNonlinear: " << num_unks_vel << " vels. ";
#endif

    Quad5CL<Point3DCL> Grad[10], GradRef[10], u_loc, u_rho;
    Quad5CL<double> rho, Phi;

    SMatrixCL<3,3> T;

    double det, absdet;
    Point3DCL tmp;
    LevelsetP2CL::const_DiscSolCL ls= lset.GetSolution();
    const_DiscVelSolCL u( vel, &BndData_.Vel, &MG_, t);

    P2DiscCL::GetGradientsOnRef( GradRef);
    LocalP2CL<> p2Phi;
    LocalP2CL<Point3DCL> p2u;

    for (MultiGridCL::TriangTetraIteratorCL sit=MG_.GetTriangTetraBegin(lvl), send=MG_.GetTriangTetraEnd(lvl);
         sit != send; ++sit)
    {
        GetTrafoTr( T, det, *sit);
        P2DiscCL::GetGradients( Grad, GradRef, T);
        absdet= std::fabs( det);

        // collect some information about the edges and verts of the tetra
        // and save it in Numb and IsOnDirBnd
        p2Phi.assign(*sit, ls, t);
        Phi.assign( p2Phi);
        p2u.assign(*sit, u, t);
        u_loc.assign(p2u);

        n.assign( *sit, RowIdx, BndData_.Vel);

        // rho = rho( Phi)
        rho= Phi;
        rho.apply( Coeff_.rho);

        u_rho= u_loc*rho;

        for(int i= 0; i < 10; ++i)  // assemble row n.num[i]
            if (n.WithUnknowns( i))  // vert/edge i is not on a Dirichlet boundary
            {
                for(int j= 0; j < 10; ++j)
                {   // N(u)_ij = int( phi_i * rho*u * grad phi_j )
                    const double N_ij= Quad5CL<double>( dot( u_rho, Grad[j])).quadP2( i, absdet);
                    if (n.WithUnknowns( j)) // vert/edge j is not on a Dirichlet boundary
                    {
                        mN( n.num[i],   n.num[j]  )+= N_ij;
                        mN( n.num[i]+1, n.num[j]+1)+= N_ij;
                        mN( n.num[i]+2, n.num[j]+2)+= N_ij;
                    }
                    else
                        if (cplN != 0) // put coupling on rhs
                        {
                            tmp= j < 4 ? BndData_.Vel.GetDirBndValue( *sit->GetVertex( j), t)
                                    : BndData_.Vel.GetDirBndValue( *sit->GetEdge( j - 4), t);
                            cplN->Data[n.num[i]    ]-= N_ij*tmp[0];
                            cplN->Data[n.num[i] + 1]-= N_ij*tmp[1];
                            cplN->Data[n.num[i] + 2]-= N_ij*tmp[2];
                        }
                }

            }
    }

    mN.Build();
#ifndef _PAR
    std::cout << N.num_nonzeros() << " nonzeros in N!" << std::endl;
#endif
}


void InstatNavierStokes2PhaseP2P1CL::SetupNonlinear
    ( MLMatDescCL* N, const VelVecDescCL* vel, VelVecDescCL* cplN,
      const LevelsetP2CL& lset, double t) const
/// Couplings with dirichlet BCs are accumulated in cplN,
/// so call cplN->Clear() before if only couplings are needed.
{
    MLMatrixCL::iterator  itN = N->Data.begin();
    MLIdxDescCL::iterator it  = N->RowIdx->begin();
    for (size_t lvl=0; lvl < N->Data.size(); ++lvl, ++itN, ++it)
        SetupNonlinear_P2( *itN, vel, lvl == N->Data.size()-1 ? cplN : 0, lset,*it, t);
}

} // end of namespace DROPS