#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

###############################################################################
# Init autoconf
###############################################################################
AC_PREREQ([2.69])
AC_INIT([Framework for Network Co-Simulation],
        [0.1],
        [jeff.daily@pnnl.gov],
        [fncs])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([integrator.h])

###############################################################################
# Init automake
###############################################################################
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])

# Don't emit "rebuild rules" for configure, Makefile.ins, etc.
AM_MAINTAINER_MODE

###############################################################################
# Misc. information and package setup.
###############################################################################
FNCS_TOP_BUILDDIR="`pwd`"
cd "$srcdir"
FNCS_TOP_SRCDIR="`pwd`"
cd "$FNCS_TOP_BUILDDIR"
AS_IF([test "$FNCS_TOP_BUILDDIR" != "$FNCS_TOP_SRCDIR"],
      [AC_MSG_NOTICE([Detected VPATH build])])

AC_LANG([C++])

# We use the MPI compiler wrappers instead of the standard compilers.
FNCS_WITH_MPI
AS_IF([test "x$with_mpi" != xno], [with_mpi_val=1], [with_mpi_val=0])
AM_CONDITIONAL([HAVE_MPI], [test "x$with_mpi" != xno])
AC_DEFINE_UNQUOTED([HAVE_MPI], [$with_mpi_val], [define to 1 to use MPI])

###############################################################################
# C++ compiler
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([C++ compiler])
AC_MSG_NOTICE

# Checks for programs.
FNCS_PROG_MPICXX
AS_IF([test "x$with_mpi_wrappers" = xyes], [FNCS_MPI_UNWRAP],
    [FNCS_ARG_PARSE([with_mpi], [MPI_LIBS], [MPI_LDFLAGS], [MPI_CPPFLAGS])])
AS_IF([test "x$with_mpi" != xno], [FNCS_MPICXX_TEST_LINK])

# Checks for libraries.
FNCS_CHECK_PACKAGE([zmq], [zmq.h], [zmq], [zmq_ctx_new])

# Checks for header files.
FNCS_CHECK_HEADERS([cstdint])
FNCS_CHECK_HEADERS([stdint.h])
FNCS_CHECK_HEADERS([sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_SIZEOF([int8_t])
AC_CHECK_SIZEOF([int16_t])
AC_CHECK_SIZEOF([int32_t])
AC_CHECK_SIZEOF([int64_t])
AC_CHECK_SIZEOF([uint8_t])
AC_CHECK_SIZEOF([uint16_t])
AC_CHECK_SIZEOF([uint32_t])
AC_CHECK_SIZEOF([uint64_t])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([unsigned char])
AC_CHECK_SIZEOF([unsigned short])
AC_CHECK_SIZEOF([unsigned int])
AC_CHECK_SIZEOF([unsigned long])
AC_CHECK_SIZEOF([unsigned long long])
AC_CHECK_SIZEOF([void*])
FNCS_CXX_NULLPTR

# Checks for library functions.
FNCS_CHECK_FUNCS([gettimeofday])

###############################################################################
# C compiler
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([C compiler])
AC_MSG_NOTICE
AC_LANG_PUSH([C])

# Checks for programs.
FNCS_PROG_MPICC
AS_IF([test "x$with_mpi_wrappers" = xyes], [FNCS_MPI_UNWRAP])
AS_IF([test "x$with_mpi" != xno], [FNCS_MPICC_TEST_LINK])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_LANG_POP([C])

###############################################################################
# Libtool setup -- no compiler/linker tests after this
###############################################################################
AC_MSG_NOTICE
AC_MSG_NOTICE([Libtool setup])
AC_MSG_NOTICE
# temporarily restore unwrapped compilers
# this works around a bug where libtool sadly relies on matching compiler
# names in order to determine features (Fortran only, I think)
# libtool doesn't recognize MPI compiler names, nor should it
AS_IF([test "x$with_mpi_wrappers" = xyes], [FNCS_MPI_UNWRAP_PUSH])
m4_ifdef([AM_PROG_AR],[AM_PROG_AR])
LT_INIT
# and now that that's over, put the MPI compilers back
# also, the above hack incorrectly sets the base compiler as the linker
AS_IF([test "x$with_mpi_wrappers" = xyes],
    [FNCS_MPI_UNWRAP_POP
     compiler="$CC"
     LTCC="$CC"
     lt_save_CC="$CC"
     compiler_DEFAULT="$CC"
     compiler_CXX="$CXX"])

###############################################################################
# Test suite setup
###############################################################################
AC_ARG_VAR([MPIEXEC],
           [how to run parallel tests if built with MPI e.g. "mpiexec -np %NP%"])
AS_IF([test "x$MPIEXEC" = x && test "x$with_mpi" != xno],
    [AC_PATH_PROGS([MPIEXEC], [mpirun mpiexec])])
AC_SUBST([MPIEXEC])

###############################################################################
# The End
###############################################################################
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Report on what we found.
AC_MSG_NOTICE([])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([ $PACKAGE_NAME configured as follows:])
AC_MSG_NOTICE([**************************************************************])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([     MPI_LIBS=$MPI_LIBS])
AC_MSG_NOTICE([  MPI_LDFLAGS=$MPI_LDFLAGS])
AC_MSG_NOTICE([ MPI_CPPFLAGS=$MPI_CPPFLAGS])
AC_MSG_NOTICE([          CXX=$CXX])
AS_IF([test "x$with_mpi_wrappers" = xyes], [
AC_MSG_NOTICE([unwrapped CXX=$ga_cv_mpicxx_naked])
])
AC_MSG_NOTICE([     CXXFLAGS=$CXXFLAGS])
AC_MSG_NOTICE([          CPP=$CPP])
AC_MSG_NOTICE([     CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([      LDFLAGS=$LDFLAGS])
AC_MSG_NOTICE([         LIBS=$LIBS])
AC_MSG_NOTICE([      MPIEXEC=$MPIEXEC])
AC_MSG_NOTICE([])

