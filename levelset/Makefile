#############################################
#   D R O P S   local makefile              #
#############################################

# variables:

DIR = levelset

EXEC = lsdrops reparam lsdrivcav lsshear curvDriven risingBubble risingBubbleAdap

DELETE = $(EXEC)

CPP = $(wildcard *.cpp)

OBJ = $(CPP:%.cpp=%.o)


# rules

default: ClrScr all

lsdrops: \
    ../levelset/lsdrops.o \
    ../geom/multigrid.o ../geom/builder.o ../geom/topo.o ../geom/boundary.o \
    ../num/unknowns.o ../misc/utils.o ../misc/problem.o ../num/discretize.o
	$(CC) -o $@ $^

reparam: \
    ../levelset/reparam.o \
    ../geom/multigrid.o ../geom/builder.o ../geom/topo.o ../geom/boundary.o \
    ../num/unknowns.o ../misc/utils.o ../misc/problem.o ../num/discretize.o
	$(CC) $(CFLAGS) -o $@ $^

lsdrivcav: \
    ../levelset/lsdrivcav.o ../geom/boundary.o ../geom/builder.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o \
    ../misc/utils.o ../stokes/stokes.o ../out/output.o ../num/discretize.o
	$(CC) -o $@ $^ $(LFLAGS)

lsshear: \
    ../levelset/lsshear.o ../geom/boundary.o ../geom/builder.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o \
    ../misc/utils.o ../stokes/stokes.o ../out/output.o ../num/discretize.o
	$(CC) -o $@ $^ $(LFLAGS)

curvDriven: \
    ../levelset/curvDriven.o ../geom/boundary.o ../geom/builder.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o \
    ../misc/utils.o ../stokes/stokes.o ../out/output.o ../num/discretize.o
	$(CC) -o $@ $^ $(LFLAGS)

risingBubble: \
    ../levelset/risingBubble.o ../geom/boundary.o ../geom/builder.o ../geom/multigrid.o \
    ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o \
    ../misc/utils.o ../stokes/stokes.o ../out/output.o ../num/discretize.o
	$(CC) -o $@ $^ $(LFLAGS)

risingBubbleAdap: \
    ../levelset/risingBubbleAdap.o ../geom/boundary.o ../geom/builder.o \
    ../geom/multigrid.o ../num/unknowns.o ../geom/topo.o ../num/fe.o ../misc/problem.o \
    ../misc/utils.o ../stokes/stokes.o ../out/output.o ../num/discretize.o
	$(CC) -o $@ $^ $(LFLAGS)

# include common rules:
include ../Make.common