# FNCS_MPICC_TEST_PROGRAM
# -----------------------
# Create an MPI test program in C.
AC_DEFUN([FNCS_MPICC_TEST_PROGRAM], [
AC_LANG_PUSH([C])
AC_LANG_CONFTEST([AC_LANG_PROGRAM([[#include <mpi.h>]],
[[int myargc; char **myargv; MPI_Init(&myargc, &myargv); MPI_Finalize();]])])
AC_LANG_POP([C])
])dnl

# FNCS_MPICC_TEST_COMPILE
# -----------------------
# Attempt to compile a simple MPI program in C.
AC_DEFUN([FNCS_MPICC_TEST_COMPILE], [
AC_LANG_PUSH([C])
FNCS_MPICC_TEST_PROGRAM()
AC_CACHE_CHECK([whether a simple C MPI program compiles],
    [fncs_cv_c_mpi_test_compile],
    [fncs_save_CPPFLAGS="$CPPFLAGS"; CPPFLAGS="$CPPFLAGS $MPI_CPPFLAGS"
     AC_COMPILE_IFELSE([],
        [fncs_cv_c_mpi_test_compile=yes],
        [fncs_cv_c_mpi_test_compile=no])
     CPPFLAGS="$fncs_save_CPPFLAGS"])
rm -f conftest.$ac_ext
AC_LANG_POP([C])
AS_IF([test "x$fncs_cv_c_mpi_test_compile" = xno],
    [AC_MSG_FAILURE([could not compile simple C MPI program])])
])dnl

# FNCS_MPICC_TEST_LINK
# --------------------
# Attempt to link a simple MPI program in C.
AC_DEFUN([FNCS_MPICC_TEST_LINK], [
AC_LANG_PUSH([C])
FNCS_MPICC_TEST_PROGRAM()
fncs_cv_c_mpi_test_link=no
AS_IF([test "x$fncs_cv_c_mpi_test_link" = xno],
    [AC_MSG_CHECKING([whether a C MPI program links natively])
     AC_LINK_IFELSE([],
        [fncs_cv_c_mpi_test_link=yes
         MPI_LIBS=
         MPI_LDFLAGS=
         MPI_CPPFLAGS=],
        [fncs_cv_c_mpi_test_link=no])
     AC_MSG_RESULT([$fncs_cv_c_mpi_test_link])])
# That didn't work, so now let's try adding our MPI_* flags.
AS_IF([test "x$fncs_cv_c_mpi_test_link" = xno],
    [AC_MSG_CHECKING([whether a C MPI program links with additional env])
     fncs_save_LIBS="$LIBS";          LIBS="$LIBS $MPI_LIBS"
     fncs_save_LDFLAGS="$LDFLAGS";    LDFLAGS="$LDFLAGS $MPI_LDFLAGS"
     fncs_save_CPPFLAGS="$CPPFLAGS";  CPPFLAGS="$CPPFLAGS $MPI_CPPFLAGS"
     AC_LINK_IFELSE([],
        [fncs_cv_c_mpi_test_link=yes],
        [fncs_cv_c_mpi_test_link=no])
     LIBS="$fncs_save_LIBS"
     LDFLAGS="$fncs_save_LDFLAGS"
     CPPFLAGS="$fncs_save_CPPFLAGS"
     AC_MSG_RESULT([$fncs_cv_c_mpi_test_link])])
rm -f conftest.$ac_ext
AC_LANG_POP([C])
AS_IF([test "x$fncs_cv_c_mpi_test_link" = xno],
    [AC_MSG_FAILURE([could not link a C MPI program])])
])dnl
