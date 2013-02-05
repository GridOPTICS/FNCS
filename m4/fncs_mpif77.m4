# FNCS_PROG_MPIF77
# -----------------
# If desired, replace F77 with MPIF77 while searching for a Fortran 77 compiler.
# We look for 95/90 compilers first so that we can control the INTEGER size.
# The search order changes depending on the TARGET.
#
# NOTE: We prefer "FC" and "FCFLAGS" over "F77" and "FFLAGS", respectively.
# But our Fortran source is only Fortran 77.  If FC/MPIFC is set, it is
# preferred above all.
#
AC_DEFUN([FNCS_PROG_MPIF77],
[AC_ARG_VAR([MPIF77], [MPI Fortran 77 compiler])
# If FC is set, override F77.  Similarly for MPIFC/MPIF77 and FCFLAGS/FFLAGS.
AS_IF([test "x$FC" != x],       [F77="$FC"])
AS_IF([test "x$MPIFC" != x],    [MPIF77="$MPIFC"])
AS_IF([test "x$FCFLAGS" != x],  [FFLAGS="$FCFLAGS"])
# In the case of using MPI wrappers, set F77=MPIF77 since F77 will override
# absolutely everything in our list of compilers.
# Save F77, just in case.
AS_IF([test x$with_mpi_wrappers = xyes],
    [AS_IF([test "x$F77" != "x$MPIF77"], [fncs_orig_F77="$F77"])
     AS_CASE([x$F77:x$MPIF77],
        [x:x],  [],
        [x:x*], [F77="$MPIF77"],
        [x*:x],
[AC_MSG_WARN([MPI compilers desired but F77 is set while MPIF77 is unset.])
 AC_MSG_WARN([F77 will be ignored during compiler selection, but will be])
 AC_MSG_WARN([tested first during MPI compiler unwrapping. Perhaps you])
 AC_MSG_WARN([meant to set MPIF77 instead of or in addition to F77?])
 F77=],
        [x*:x*], 
[AS_IF([test "x$F77" != "x$MPIF77"],
[AC_MSG_WARN([MPI compilers desired, MPIF77 and F77 are set, and MPIF77!=F77.])
 AC_MSG_WARN([Choosing MPIF77 over F77.])
 AC_MSG_WARN([F77 will be tested first during MPI compiler unwrapping.])])
 F77="$MPIF77"],
[AC_MSG_ERROR([F77/MPIF77 case failure])])])
AS_IF([test x$with_mpi_wrappers = xyes],
    [F77_TO_TEST="FNCS_MPIF77_COMPILERS"],
    [F77_TO_TEST="FNCS_F77_COMPILERS"])
AC_PROG_F77([$F77_TO_TEST])
])dnl
