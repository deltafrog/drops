#############################################
#   D R O P S   local makefile              #
#############################################

# variables:

DIR = levelset

DELETE = $(EXEC)

CPP = $(wildcard *.cpp)

OBJ = $(CPP:%.cpp=%.o)

# include common rules:
include ../Make.common

SER_EXEC = reparam lsshear surfTens prJump film brick_transp twophasedrops
PAR_EXEC = twophasedrops film
EXEC = $(if $(PAR_BUILD),$(PAR_EXEC),$(SER_EXEC))

PAR_OBJ_ = ../parallel/parallel.o ../parallel/parmultigrid.o \
           ../parallel/partime.o ../parallel/addeddata.o ../parallel/partitioner.o ../parallel/loadbal.o \
           ../parallel/exchange.o ../parallel/memmgr_std.o ../parallel/parmgserialization.o \
           ../parallel/logger.o ../parallel/parddd.o 
PAR_OBJ = $(if $(PAR_BUILD),$(PAR_OBJ_),)

# rules

default: ClrScr all

reparam: \
    ../levelset/reparam.o ../levelset/fastmarch.o ../levelset/levelset.o \
    ../geom/simplex.o ../geom/multigrid.o ../geom/builder.o ../geom/topo.o ../geom/boundary.o \
    ../num/unknowns.o ../misc/utils.o ../misc/problem.o ../num/discretize.o \
    ../num/fe.o ../out/ensightOut.o ../num/interfacePatch.o ../levelset/surfacetension.o \
    ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o
	$(CXX) -o $@ $^ $(LFLAGS)

lsshear: \
    ../levelset/lsshear.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o ../levelset/fastmarch.o \
    ../num/fe.o ../out/ensightOut.o ../stokes/instatstokes2phase.o ../navstokes/instatnavstokes2phase.o \
    ../num/MGsolver.o ../num/interfacePatch.o ../levelset/surfacetension.o ../levelset/coupling.o \
    ../misc/bndmap.o ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o \
    ../geom/bndScalarFunctions.o ../num/renumber.o
	$(CXX) -o $@ $^ $(LFLAGS)

surfTens: \
    ../levelset/surfTens.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o \
    ../misc/params.o ../levelset/fastmarch.o ../stokes/instatstokes2phase.o ../navstokes/instatnavstokes2phase.o \
    ../num/fe.o ../out/ensightOut.o ../stokes/integrTime.o ../num/MGsolver.o ../misc/bndmap.o \
    ../num/interfacePatch.o ../levelset/surfacetension.o ../levelset/coupling.o ../geom/bndVelFunctions.o \
    ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o \
    ../geom/bndScalarFunctions.o ../num/renumber.o
	$(CXX) -o $@ $^ $(LFLAGS)

prJump: \
    ../levelset/prJump.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o \
    ../misc/params.o ../levelset/fastmarch.o ../stokes/instatstokes2phase.o ../num/MGsolver.o \
    ../num/fe.o  ../out/ensightOut.o ../stokes/integrTime.o ../num/interfacePatch.o \
    ../levelset/surfacetension.o ../levelset/coupling.o ../geom/bndVelFunctions.o ../misc/bndmap.o \
    ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o \
    ../geom/bndScalarFunctions.o ../num/renumber.o
	$(CXX) -o $@ $^ $(LFLAGS)

film: \
    ../levelset/film.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o ../navstokes/instatnavstokes2phase.o \
    ../misc/params.o ../levelset/fastmarch.o ../stokes/instatstokes2phase.o ../num/MGsolver.o\
    ../num/fe.o ../out/ensightOut.o ../stokes/integrTime.o ../num/interfacePatch.o \
    ../levelset/surfacetension.o ../levelset/coupling.o ../geom/bndVelFunctions.o ../misc/bndmap.o \
    ../levelset/filmCoeff.o ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o \
    ../geom/bndScalarFunctions.o ../num/renumber.o $(PAR_OBJ)
	$(CXX) -o $@ $^ $(LFLAGS)

brick_transp: \
    ../levelset/brick_transp.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o \
    ../misc/params.o ../levelset/fastmarch.o ../num/fe.o \
    ../poisson/transport2phase.o ../out/ensightOut.o ../out/vtkOut.o ../num/interfacePatch.o \
    ../levelset/surfacetension.o ../stokes/instatstokes2phase.o ../geom/bndVelFunctions.o ../misc/bndmap.o \
    ../levelset/twophaseCoeff.o ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o ../num/quadrature.o \
    ../geom/bndScalarFunctions.o
	$(CXX) -o $@ $^ $(LFLAGS)

twophasedrops: \
    ../levelset/twophasedrops.o ../geom/boundary.o ../geom/builder.o ../geom/simplex.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o ../levelset/levelset.o \
    ../misc/utils.o ../out/output.o ../num/discretize.o ../navstokes/instatnavstokes2phase.o \
    ../misc/params.o ../levelset/fastmarch.o ../stokes/instatstokes2phase.o ../num/MGsolver.o \
    ../num/fe.o ../out/ensightOut.o ../stokes/integrTime.o ../poisson/transport2phase.o \
    ../num/interfacePatch.o ../out/vtkOut.o ../surfactant/ifacetransp.o ../levelset/surfacetension.o \
    ../geom/geomselect.o  ../levelset/twophaseutils.o ../num/hypre.o ../levelset/coupling.o ../geom/bndVelFunctions.o \
    ../geom/bndScalarFunctions.o ../misc/bndmap.o ../levelset/twophaseCoeff.o \
    ../geom/principallattice.o ../geom/reftetracut.o ../geom/subtriangulation.o \
    ../num/renumber.o ../num/quadrature.o $(PAR_OBJ)
	$(CXX) -o $@ $^ $(LFLAGS)

