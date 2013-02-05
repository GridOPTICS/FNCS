# FNCS_PROG_MPICC
# ---------------
# If desired, replace CC with MPICC while searching for a C compiler.
#
AC_DEFUN([FNCS_PROG_MPICC],
[AC_ARG_VAR([MPICC], [MPI C compiler])
# In the case of using MPI wrappers, set CC=MPICC since CC will override
# absolutely everything in our list of compilers.
# Save CC, just in case.
AS_IF([test x$with_mpi_wrappers = xyes],
    [AS_IF([test "x$CC" != "x$MPICC"], [fncs_orig_CC="$CC"])
     AS_CASE([x$CC:x$MPICC],
        [x:x],  [],
        [x:x*], [CC="$MPICC"],
        [x*:x],
[AC_MSG_WARN([MPI compilers desired but CC is set while MPICC is unset.])
 AC_MSG_WARN([CC will be ignored during compiler selection, but will be])
 AC_MSG_WARN([tested first during MPI compiler unwrapping. Perhaps you])
 AC_MSG_WARN([meant to set MPICC instead of or in addition to CC?])
 CC=],
        [x*:x*], 
[AS_IF([test "x$CC" != "x$MPICC"],
[AC_MSG_WARN([MPI compilers desired, MPICC and CC are set, and MPICC!=CC.])
 AC_MSG_WARN([Choosing MPICC over CC.])
 AC_MSG_WARN([CC will be tested first during MPI compiler unwrapping.])])
 CC="$MPICC"],
[AC_MSG_ERROR([CC/MPICC case failure])])])
AS_IF([test x$with_mpi_wrappers = xyes],
    [CC_TO_TEST="FNCS_MPICC_COMPILERS"],
    [CC_TO_TEST="FNCS_CC_COMPILERS"])
AC_PROG_CC([$CC_TO_TEST])
])dnl
