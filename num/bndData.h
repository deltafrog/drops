//**************************************************************************
// File:    bndData.h                                                      *
// Content: boundary data                                                  *
// Author:  Sven Gross, Joerg Peters, Volker Reichelt, IGPM RWTH Aachen    *
//**************************************************************************

#ifndef DROPS_BNDDATA_H
#define DROPS_BNDDATA_H

#include "misc/utils.h"
#include "geom/boundary.h"

namespace DROPS
{

enum BndCondT
// boundary conditions in drops. Conventions:
// - boundary condition with lower number is superior
// - odd number for boundary with unknowns
// - valid boundary conditions have numbers in the range 0..99
// - interior simplices have no boundary data and return NoBC
//   in BndDataCL::GetBC
{
    Dir0BC= 0, DirBC= 2,         // (in)hom. Dirichlet boundary conditions
    Per1BC= 11, Per2BC= 13,      // Periodic boundary condition
    Nat0BC= 21, NatBC= 23,       // (in)hom. natural   boundary conditions
    OutflowBC= 21, WallBC= 0,    // for convenience
    
    NoBC= -1,                    // interior simplices
    UndefinedBC_= 99,            // ReadMeshBuilderCL: error, unknown bc
    MaxBC_= 100                  // upper bound for valid bc's
};


template<class BndValT = double>
class BndSegDataCL
{
  public:
    typedef BndValT (*bnd_val_fun)( const Point3DCL&, double);
    
  private:
    BndCondT     bc_;
    bnd_val_fun  bnd_val_;

  public:
    BndSegDataCL( BndCondT bc= Nat0BC, bnd_val_fun f= 0)
      : bc_(bc), bnd_val_(f) {}
    
    bool        WithUnknowns() const { return bc_ & 1; }
    bool        IsDirichlet()  const { return bc_==Dir0BC || bc_==DirBC; }
    bool        IsNeumann()    const { return bc_==Nat0BC || bc_==NatBC; }
    bool	IsPeriodic()   const { return bc_==Per1BC || bc_==Per2BC ; }
    BndCondT    GetBC()        const { return bc_; }
    bnd_val_fun GetBndFun()    const { return bnd_val_; }
    BndValT     GetBndVal( const Point3DCL& p, double t) const { return bnd_val_( p, t); }
};


template<class BndValT = double>
class BndDataCL
{
  public:
    typedef BndSegDataCL<BndValT>             BndSegT;
    typedef BndValT                           bnd_type;
    typedef typename BndSegT::bnd_val_fun     bnd_val_fun;

  private:
    std::vector<BndSegT> BndData_;
    
  public:
    BndDataCL( BndIdxT numbndseg, const BndCondT* bc= 0, const bnd_val_fun* fun= 0);
    BndDataCL( BndIdxT numbndseg, const bool* isneumann, const bnd_val_fun* fun); // deprecated ctor!

    inline bool IsOnDirBnd( const VertexCL&) const;
    inline bool IsOnDirBnd( const EdgeCL&)   const;
    inline bool IsOnDirBnd( const FaceCL&)   const;
    inline bool IsOnNeuBnd( const VertexCL&) const;
    inline bool IsOnNeuBnd( const EdgeCL&)   const;
    inline bool IsOnNeuBnd( const FaceCL&)   const;
    inline bool IsOnPerBnd( const VertexCL&) const;
    inline bool IsOnPerBnd( const EdgeCL&)   const;
    inline bool IsOnPerBnd( const FaceCL&)   const;

    inline BndCondT GetBC( const VertexCL&)    const;
    inline BndCondT GetBC( const EdgeCL&)      const;
    inline BndCondT GetBC( const FaceCL&)      const;
    inline BndSegT GetBndSeg( const VertexCL&) const;
    inline BndSegT GetBndSeg( const EdgeCL&)   const;
    inline BndSegT GetBndSeg( const FaceCL&)   const;
    
    inline BndValT GetDirBndValue( const VertexCL&, double) const;
    inline BndValT GetDirBndValue( const EdgeCL&, double)   const;
    inline BndValT GetDirBndValue( const FaceCL&, double)   const;
    inline BndValT GetNeuBndValue( const VertexCL&, double) const;
    inline BndValT GetNeuBndValue( const EdgeCL&, double)   const;
    inline BndValT GetNeuBndValue( const FaceCL&, double)   const;

    inline BndValT GetDirBndValue( const VertexCL& v) const { return GetDirBndValue( v, 0); }
    inline BndValT GetDirBndValue( const EdgeCL& e)   const { return GetDirBndValue( e, 0); }
    inline BndValT GetDirBndValue( const FaceCL& f)   const { return GetDirBndValue( f, 0); }
};


template<class BndValT= double>
class NoBndDataCL
{
  public:
    typedef BndSegDataCL<BndValT>             BndSegT;
    typedef BndValT                           bnd_type;
    typedef typename BndSegT::bnd_val_fun     bnd_val_fun;

    // default ctor, dtor, whatever

    template<class SimplexT>
    static inline BndCondT GetBC  (const SimplexT&) { return NoBC; }
    template<class SimplexT>
    static inline bool IsOnDirBnd (const SimplexT&) { return false; }
    template<class SimplexT>
    static inline bool IsOnNeuBnd (const SimplexT&) { return false; }
    template<class SimplexT>
    static inline bool IsOnPerBnd (const SimplexT&) { return false; }
    
    static inline BndValT GetDirBndValue (const VertexCL&)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on vertex."); }
    static inline BndValT GetDirBndValue (const EdgeCL&)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on edge."); }
    static inline BndValT GetDirBndValue (const FaceCL&)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on face."); }
    static inline BndValT GetDirBndValue (const VertexCL&, double)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on vertex."); }
    static inline BndValT GetDirBndValue (const EdgeCL&, double)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on edge."); }
    static inline BndValT GetDirBndValue (const FaceCL&, double)
        { throw DROPSErrCL("NoBndDataCL::GetDirBndValue: Attempt to use Dirichlet-boundary-conditions on face."); }
};


//======================================
//        inline functions 
//======================================

template<class BndValT>
inline BndDataCL<BndValT>::BndDataCL( BndIdxT numbndseg, const BndCondT* bc, const bnd_val_fun* fun)
{
    BndData_.reserve( numbndseg);
    for (Uint i=0; i<numbndseg; ++i)
        BndData_.push_back( BndSegT( bc ? bc[i] : Nat0BC, fun ? fun[i] : 0) );
}

template<class BndValT>
inline BndDataCL<BndValT>::BndDataCL( BndIdxT numbndseg, const bool* isneumann, const bnd_val_fun* fun) // deprecated ctor!
{
    BndData_.reserve( numbndseg);
    for (Uint i=0; i<numbndseg; ++i)
        BndData_.push_back( BndSegT( isneumann[i] ? NatBC : DirBC, fun[i]) );
}


template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnDirBnd( const VertexCL& v) const
{ // v is on dir bnd, iff it is on one or more dir bnd segments
    if ( !v.IsOnBoundary() ) return false;
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
        if ( BndData_[it->GetBndIdx()].IsDirichlet() )
            return true;
    return false; 
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnNeuBnd( const VertexCL& v) const
{ // v is on neu bnd, iff it is only on neu bnd segments
    if ( !v.IsOnBoundary() ) return false;
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
        if ( !BndData_[it->GetBndIdx()].IsNeumann() )
            return false;
    return true;
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnPerBnd( const VertexCL& v) const
{ // v is on per bnd, iff it is on one or more per bnd segments and not on a dir bnd segment
    if ( !v.IsOnBoundary() ) return false;
    bool HasPer= false;
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
    {
        const BndCondT bc= BndData_[it->GetBndIdx()].GetBC();
        if (bc==DirBC || bc==Dir0BC)
            return false;
        if (bc==Per1BC || bc==Per2BC)
            HasPer= true;
    }
    return HasPer;
}        

template<class BndValT>
inline BndCondT BndDataCL<BndValT>::GetBC( const VertexCL& v) const
{ // return BC on v with lowest number (i.e. the superior BC on v)
    if ( !v.IsOnBoundary() ) return NoBC;
    BndCondT bc= MaxBC_;
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
        bc= std::min( bc, BndData_[it->GetBndIdx()].GetBC());
    return bc;
}        

template<class BndValT>
inline typename BndDataCL<BndValT>::BndSegT BndDataCL<BndValT>::GetBndSeg( const VertexCL& v) const
{ // return BC on v with lowest number (i.e. the superior BC on v)
    Assert( v.IsOnBoundary(), DROPSErrCL("BndDataCL::GetBndSeg(VertexCL): Not on boundary!"), ~0);
    BndCondT bc_min= MaxBC_;
    BndIdxT  idx= -1;
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
    {
        const BndCondT bc= BndData_[it->GetBndIdx()].GetBC();
        if (bc<bc_min) { bc_min= bc;    idx= it->GetBndIdx(); }
    }
    return BndData_[idx];
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnDirBnd( const EdgeCL& e) const
{
    if ( !e.IsOnBoundary() ) return false;
    for (const BndIdxT *it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
        if ( BndData_[*it].IsDirichlet() )
            return true;
    return false;
}

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnNeuBnd( const EdgeCL& e) const
{
    if ( !e.IsOnBoundary() ) return false;
    for (const BndIdxT *it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
        if ( !BndData_[*it].IsNeumann() )
            return false;
    return true;
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnPerBnd( const EdgeCL& e) const
{ // e is on per bnd, iff it is on one or more per bnd segments and not on a dir bnd segment
    if ( !e.IsOnBoundary() ) return false;
    bool HasPer= false;
    for (const BndIdxT *it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
    {
        const BndCondT bc= BndData_[*it].GetBC();
        if (bc==DirBC || bc==Dir0BC)
            return false;
        if (bc==Per1BC || bc==Per2BC)
            HasPer= true;
    }
    return HasPer;
}        

template<class BndValT>
inline BndCondT BndDataCL<BndValT>::GetBC( const EdgeCL& e) const
{ // return BC on e with lowest number (i.e. the superior BC on e)
    if ( !e.IsOnBoundary() ) return NoBC;
    BndCondT bc= MaxBC_;
    for (const BndIdxT *it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
        bc= std::min( bc, BndData_[*it].GetBC());
    return bc;
}        

template<class BndValT>
inline typename BndDataCL<BndValT>::BndSegT BndDataCL<BndValT>::GetBndSeg( const EdgeCL& e) const
{ // return BC on e with lowest number (i.e. the superior BC on e)
    Assert( e.IsOnBoundary(), DROPSErrCL("BndDataCL::GetBndSeg(EdgeCL): Not on boundary!"), ~0);
    BndCondT bc_min= MaxBC_;
    BndIdxT  idx= -1;
    for (const BndIdxT *it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
    {
        const BndCondT bc= BndData_[*it].GetBC();
        if (bc<bc_min) { bc_min= bc;    idx= *it; }
    }
    return BndData_[idx];
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnDirBnd( const FaceCL& f) const
{
    return f.IsOnBoundary() && BndData_[f.GetBndIdx()].IsDirichlet();
}

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnNeuBnd(const FaceCL& f) const
{
    return f.IsOnBoundary() && BndData_[f.GetBndIdx()].IsNeumann();
}        

template<class BndValT>
inline bool BndDataCL<BndValT>::IsOnPerBnd( const FaceCL& f) const
{
    return f.IsOnBoundary() && BndData_[f.GetBndIdx()].IsPeriodic();
}        

template<class BndValT>
inline BndCondT BndDataCL<BndValT>::GetBC( const FaceCL& f) const
{ 
    if ( !f.IsOnBoundary() ) return NoBC;
    return BndData_[f.GetBndIdx()].GetBC();
}        

template<class BndValT>
inline typename BndDataCL<BndValT>::BndSegT BndDataCL<BndValT>::GetBndSeg( const FaceCL& f) const
{ 
    Assert( f.IsOnBoundary(), DROPSErrCL("BndDataCL::GetBndSeg(FaceCL): Not on boundary!"), ~0);
    return BndData_[f.GetBndIdx()];
}        

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetDirBndValue( const VertexCL& v, double t) const
// Returns value of the Dirichlet boundary value. 
// Expects, that there is any Dirichlet boundary ( IsOnDirBnd(...) == true )
{
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
        if ( BndData_[it->GetBndIdx()].IsDirichlet() )
            return BndData_[it->GetBndIdx()].GetBndVal( v.GetCoord(), t);
    throw DROPSErrCL("GetDirBndValue(VertexCL): No Dirichlet Boundary Segment!");
}

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetNeuBndValue( const VertexCL& v, double t) const
// Returns value of the Neumann boundary value. 
// Expects, that there is any Neumann boundary ( IsOnNeuBnd(...) == true )
{
    for (VertexCL::const_BndVertIt it= v.GetBndVertBegin(), end= v.GetBndVertEnd(); it!=end; ++it)
        if ( BndData_[it->GetBndIdx()].IsNeumann() )
            return BndData_[it->GetBndIdx()].GetBndVal( v.GetCoord(), t);
    throw DROPSErrCL("GetNeuBndValue(VertexCL): No Neumann Boundary Segment!");
}

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetDirBndValue( const EdgeCL& e, double t) const
{
    for (const BndIdxT* it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
        if ( BndData_[*it].IsDirichlet() )
            return BndData_[*it].GetBndVal( GetBaryCenter(e), t);
    throw DROPSErrCL("GetDirBndValue(EdgeCL): No Dirichlet Boundary Segment!");
}

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetNeuBndValue( const EdgeCL& e, double t) const
// Returns value of the Neumann boundary value. 
// Expects, that there is any Neumann boundary ( IsOnNeuBnd(...) == true )
{
    for (const BndIdxT* it= e.GetBndIdxBegin(), *end= e.GetBndIdxEnd(); it!=end; ++it)
        if ( BndData_[*it].IsNeumann() )
            return BndData_[*it].GetBndVal( GetBaryCenter(e), t);
    throw DROPSErrCL("GetNeuBndValue(EdgeCL): No Neumann Boundary Segment!");
}

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetDirBndValue( const FaceCL& f, double t) const
{
    Assert( BndData_[f.GetBndIdx()].IsDirichlet(), DROPSErrCL("GetDirBndValue(FaceCL): No Dirichlet Boundary Segment!"), ~0);
    return BndData_[f.GetBndIdx()].GetBndVal( GetBaryCenter(f), t);
}

template<class BndValT>
inline BndValT BndDataCL<BndValT>::GetNeuBndValue( const FaceCL& f, double t) const
// Returns value of the Neumann boundary value. 
// Expects, that there is any Neumann boundary ( IsOnNeuBnd(...) == true )
{
    Assert( BndData_[f.GetBndIdx()].IsNeumann(), DROPSErrCL("GetNeuBndValue(FaceCL): No Neumann Boundary Segment!"), ~0);
    return BndData_[f.GetBndIdx()].GetBndVal( GetBaryCenter(f), t);
}


} //end of namespace DROPS

#endif
