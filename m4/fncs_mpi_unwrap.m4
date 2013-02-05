# FNCS_MPI_UNWRAP()
# -----------------
# Attempt to unwrap the MPI compiler for the current language and determine
# the underlying compiler.
#
# The strategy is to first compare the stdout of the version flags using
# a custom perl script.  Next we combine stdout and sterr for the comparison.
# Occasionally, the MPI compiler will always report a non-zero exit status.
# That is the last case checked for.
AC_DEFUN([FNCS_MPI_UNWRAP], [
# Find perl.
AC_PATH_PROG([PERL], [perl])
# Create inside.pl.
rm -f inside.pl
[cat >inside.pl <<"EOF"
#!/usr/bin/perl
use strict;
use warnings;
my $numargs = @S|@#ARGV + 1;
if ($numargs != 2) {
    print "Usage: wrapped.txt naked.txt\n";
    exit 1;
}
# Read each input file as a string (rather than a list).
local $/=undef;
open WRAPPED, "$ARGV[0]" or die "Could not open wrapped text file: $!";
my $wrapped_lines = <WRAPPED>;
close WRAPPED;
open NAKED, "$ARGV[1]" or die "Could not open naked text file: $!";
my $naked_lines = <NAKED>;
close NAKED;
# Replace newlines, + from wrapped and naked lines.
$wrapped_lines =~ tr/\n+/ /;
$naked_lines =~ tr/\n+/ /;
# Remove whitespace from beginning of wrapped and naked lines.
$wrapped_lines =~ s/^\s+//;
$naked_lines =~ s/^\s+//;
# Remove whitespace from end of wrapped and naked lines.
$wrapped_lines =~ s/\s+$//;
$naked_lines =~ s/\s+$//;
# If either wrapped_lines or naked_lines are empty, this is an error.
# It is assumed that the particular version string which created the input
# files should generate SOMETHING.
unless ($wrapped_lines) {
    exit 1;
}
unless ($naked_lines) {
    exit 1;
}
# Cray compilers append a timestamp into their version string. Remove it.
if ($wrapped_lines =~ /\QCray\E/) {
    $wrapped_lines = substr $wrapped_lines, 0, -28;
    $naked_lines = substr $naked_lines, 0, -28;
}
# Can the naked lines be found within the wrapped lines?
if ($wrapped_lines =~ /\Q$naked_lines\E/) {
    #print "Found as substring\n";
    exit 0;
}
# Are the naked lines exactly the same as the wrapped lines?
elsif ($wrapped_lines eq $naked_lines) {
    #print "Found equal\n";
    exit 0;
}
else {
    #print "Not found\n";
    exit 1;
}
EOF]
inside="$PERL inside.pl"
wrapped="$_AC_CC"
AC_LANG_CASE(
[C], [AS_CASE([$wrapped],
    [*_r],  [compilers="FNCS_C_COMPILERS_R"],
    [*],    [compilers="FNCS_C_COMPILERS"])
],
[C++], [AS_CASE([$wrapped],
    [*_r],  [compilers="FNCS_CXX_COMPILERS_R"],
    [*],    [compilers="FNCS_CXX_COMPILERS"])
],
[Fortran 77], [AS_CASE([$wrapped],
    [*_r],  [compilers="FNCS_F77_COMPILERS_R"],
    [*],    [compilers="FNCS_F77_COMPILERS"])
    ],
[Fortran], [
])
AS_VAR_PUSHDEF([fncs_save_comp], [fncs_save_[]_AC_CC[]])
AS_VAR_PUSHDEF([fncs_orig_comp], [fncs_orig_[]_AC_CC[]])
AS_VAR_PUSHDEF([fncs_cv_mpi_naked], [fncs_cv_mpi[]_AC_LANG_ABBREV[]_naked])
AC_CACHE_CHECK([for base $wrapped compiler], [fncs_cv_mpi_naked], [
base="`$wrapped -show 2>/dev/null | sed 's/@<:@ 	@:>@.*@S|@//' | head -1`"
fncs_save_comp="$_AC_CC"
_AC_CC="$base"
AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])], [fncs_cv_mpi_naked="$base"])
_AC_CC="$fncs_save_comp"
versions="--version -v -V -qversion"
found_wrapped_version=0
# Try separating stdout and stderr. Only compare stdout.
AS_IF([test "x$fncs_cv_mpi_naked" = x], [
# prepend any CC/CXX/F77 the user may have specified
compilers="$fncs_orig_comp $compilers"
echo "only comparing stdout" >&AS_MESSAGE_LOG_FD
for version in $versions
do
    echo "trying version=$version" >&AS_MESSAGE_LOG_FD
    rm -f mpi.txt mpi.err naked.txt naked.err
    AS_IF([$wrapped $version 1>mpi.txt 2>mpi.err],
        [found_wrapped_version=1
         for naked_compiler in $compilers
         do
            AS_IF([test "x$naked_compiler" != "x$wrapped"],
                [AS_IF([$naked_compiler $version 1>naked.txt 2>naked.err],
                    [AS_IF([$inside mpi.txt naked.txt >/dev/null],
                        [fncs_cv_mpi_naked=$naked_compiler; break],
                        [echo "inside.pl $wrapped $naked_compiler failed, skipping" >&AS_MESSAGE_LOG_FD])],
                    [echo "$naked_compiler $version failed, skipping" >&AS_MESSAGE_LOG_FD])])
         done],
        [echo "$wrapped $version failed, skipping" >&AS_MESSAGE_LOG_FD])
    AS_IF([test "x$fncs_cv_mpi_naked" != x], [break])
done
])
# Perhaps none of the MPI compilers had a zero exit status (this is bad).
# In this case we have to do a brute force match regardless of exit status.
AS_IF([test "x$found_wrapped_version" = x0], [
AS_IF([test "x$fncs_cv_mpi_naked" = x], [
echo "no zero exit status found for MPI compilers" >&AS_MESSAGE_LOG_FD
for version in $versions
do
    echo "trying version=$version" >&AS_MESSAGE_LOG_FD
    rm -f mpi.txt mpi.err
    $wrapped $version 1>mpi.txt 2>mpi.err
    for naked_compiler in $compilers
    do
        AS_IF([test "x$naked_compiler" != "x$wrapped"],
            [rm -f naked.txt naked.err
             AS_IF([$naked_compiler $version 1>naked.txt 2>naked.err],
                [AS_IF([$inside mpi.txt naked.txt >/dev/null],
                    [fncs_cv_mpi_naked=$naked_compiler; break],
                    [echo "inside.pl $wrapped $naked_compiler failed, skipping" >&AS_MESSAGE_LOG_FD])],
                [echo "$naked_compiler $version failed, skipping" >&AS_MESSAGE_LOG_FD])])
    done
    AS_IF([test "x$fncs_cv_mpi_naked" != x], [break])
done
])
])
# Try by combining stdout/err into one file.
AS_IF([test "x$fncs_cv_mpi_naked" = x], [
echo "try combining stdout and stderr into one file" >&AS_MESSAGE_LOG_FD
for version in $versions
do
    echo "trying version=$version" >&AS_MESSAGE_LOG_FD
    rm -f mpi.txt naked.txt
    AS_IF([$wrapped $version 1>mpi.txt 2>&1],
        [for naked_compiler in $compilers
         do
            AS_IF([test "x$naked_compiler" != "x$wrapped"],
                [AS_IF([$naked_compiler $version 1>naked.txt 2>&1],
                    [AS_IF([$inside mpi.txt naked.txt >/dev/null],
                        [fncs_cv_mpi_naked=$naked_compiler; break],
                        [echo "inside.pl $wrapped $naked_compiler failed, skipping" >&AS_MESSAGE_LOG_FD])],
                    [echo "$naked_compiler $version failed, skipping" >&AS_MESSAGE_LOG_FD])])
         done],
        [echo "$wrapped $version failed, skipping" >&AS_MESSAGE_LOG_FD])
    AS_IF([test "x$fncs_cv_mpi_naked" != x], [break])
done
])
# If we got this far, then it's likely that the MPI compiler had a zero exit
# status when it shouldn't have for one version flag, but later had a non-zero
# exit status for a flag it shouldn't have.  One false positive hid a false
# negative.  In this case, brute force compare all MPI compiler output against
# all compiler output.
AS_IF([test "x$fncs_cv_mpi_naked" = x], [
echo "we have a very badly behaving MPI compiler" >&AS_MESSAGE_LOG_FD
for version in $versions
do
    echo "trying version=$version" >&AS_MESSAGE_LOG_FD
    rm -f mpi.txt mpi.err
    $wrapped $version 1>mpi.txt 2>mpi.err
    for naked_compiler in $compilers
    do
        AS_IF([test "x$naked_compiler" != "x$wrapped"],
            [rm -f naked.txt naked.err
             AS_IF([$naked_compiler $version 1>naked.txt 2>naked.err],
                [AS_IF([$inside mpi.txt naked.txt >/dev/null],
                    [fncs_cv_mpi_naked=$naked_compiler; break],
                    [echo "inside.pl $wrapped $naked_compiler failed, skipping" >&AS_MESSAGE_LOG_FD])],
                [echo "$naked_compiler $version failed, skipping" >&AS_MESSAGE_LOG_FD])])
    done
    AS_IF([test "x$fncs_cv_mpi_naked" != x], [break])
done
])
rm -f mpi.txt mpi.err naked.txt naked.err
])
AS_IF([test "x$fncs_cv_mpi_naked" = x],
    [AC_MSG_ERROR([Could not determine the ]_AC_LANG[ compiler wrapped by MPI])],
    [AS_IF([test "x$fncs_orig_comp" != x && test "x$fncs_orig_comp" != "x$fncs_cv_mpi_naked"],
        [AC_MSG_WARN([unwrapped $wrapped ($fncs_cv_mpi_naked) does not match user-specified $fncs_orig_comp])])])
AS_VAR_POPDEF([fncs_save_comp])
AS_VAR_POPDEF([fncs_cv_mpi_naked])
rm -f inside.pl
])dnl


# FNCS_MPI_UNWRAP_PUSH()
# --------------------
# Set CC/CXX/F77/FC to their unwrapped MPI counterparts.
# Save their old values for restoring later.
AC_DEFUN([FNCS_MPI_UNWRAP_PUSH], [
fncs_mpi_unwrap_push_save_CC="$CC"
fncs_mpi_unwrap_push_save_CXX="$CXX"
fncs_mpi_unwrap_push_save_F77="$F77"
fncs_mpi_unwrap_push_save_FC="$FC"
AS_IF([test "x$fncs_cv_mpic_naked"   != x], [ CC="$fncs_cv_mpic_naked"])
AS_IF([test "x$fncs_cv_mpicxx_naked" != x], [CXX="$fncs_cv_mpicxx_naked"])
AS_IF([test "x$fncs_cv_mpif77_naked" != x], [F77="$fncs_cv_mpif77_naked"])
AS_IF([test "x$fncs_cv_mpifc_naked"  != x], [ FC="$fncs_cv_mpifc_naked"])
])dnl


# FNCS_MPI_UNWRAP_POP()
# -------------------
# Restore CC/CXX/F77/FC to their MPI counterparts.
AC_DEFUN([FNCS_MPI_UNWRAP_POP], [
 CC="$fncs_mpi_unwrap_push_save_CC"
CXX="$fncs_mpi_unwrap_push_save_CXX"
F77="$fncs_mpi_unwrap_push_save_F77"
 FC="$fncs_mpi_unwrap_push_save_FC"
])dnl
