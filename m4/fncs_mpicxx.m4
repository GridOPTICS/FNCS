# FNCS_PROG_MPICXX
# ----------------
# If desired, replace CXX with MPICXX while searching for a C++ compiler.
#
AC_DEFUN([FNCS_PROG_MPICXX],
[AC_ARG_VAR([MPICXX], [MPI C++ compiler])
# In the case of using MPI wrappers, set CXX=MPICXX since CXX will override
# absolutely everything in our list of compilers.
AS_IF([test x$with_mpi_wrappers = xyes],
    [AS_IF([test "x$CXX" != "x$MPICXX"], [fncs_orig_CXX="$CXX"])
     AS_CASE([x$CXX:x$MPICXX],
        [x:x],  [],
        [x:x*], [CXX="$MPICXX"],
        [x*:x],
[AC_MSG_WARN([MPI compilers desired but CXX is set while MPICXX is unset.])
 AC_MSG_WARN([CXX will be ignored during compiler selection, but will be])
 AC_MSG_WARN([tested first during MPI compiler unwrapping. Perhaps you])
 AC_MSG_WARN([meant to set MPICXX instead of or in addition to CXX?])
 CXX=],
        [x*:x*], 
[AS_IF([test "x$CXX" != "x$MPICXX"],
[AC_MSG_WARN([MPI compilers desired, MPICXX and CXX are set, and MPICXX!=CXX.])
 AC_MSG_WARN([Choosing MPICXX over CXX.])
 AC_MSG_WARN([CXX will be tested first during MPI compiler unwrapping.])])
 CXX="$MPICXX"],
[AC_MSG_ERROR([CXX/MPICXX case failure])])])
AS_IF([test x$with_mpi_wrappers = xyes],
    [CXX_TO_TEST="FNCS_MPICXX_COMPILERS"],
    [CXX_TO_TEST="FNCS_CXX_COMPILERS"])
AC_PROG_CXX([$CXX_TO_TEST])
])dnl
