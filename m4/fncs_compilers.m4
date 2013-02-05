# FNCS_F77_COMPILERS
# ------------------
# Known Fortran 95 compilers:
#  bgxlf95      IBM BlueGene/P F95 cross-compiler
#  blrts_xlf95  IBM BlueGene/L F95 cross-compiler
#  efc          Intel Fortran 95 compiler for IA64
#  f95          generic compiler name
#  fort         Compaq/HP Fortran 90/95 compiler for Tru64 and Linux/Alpha
#  ftn          native Fortran 95 compiler on Cray X1,XT4,XT5
#  g95          original gcc-based f95 compiler (gfortran is a fork)
#  gfortran     GNU Fortran 95+ compiler (released in gcc 4.0)
#  ifc          Intel Fortran 95 compiler for Linux/x86 (now ifort)
#  ifort        Intel Fortran 95 compiler for Linux/x86 (was ifc)
#  lf95         Lahey-Fujitsu F95 compiler
#  pghpf/pgf95  Portland Group F95 compiler
#  xlf95        IBM (AIX) F95 compiler
#  pathf95      PathScale
#  openf95      AMD's x86 open64
#  sunf95       Sun's Studio
#  crayftn      Cray
#
# Known Fortran 90 compilers:
#  blrts_xlf90  IBM BlueGene/L F90 cross-compiler
#  epcf90       "Edinburgh Portable Compiler" F90
#  f90          generic compiler name
#  fort         Compaq/HP Fortran 90/95 compiler for Tru64 and Linux/Alpha
#  pgf90        Portland Group F90 compiler
#  xlf90        IBM (AIX) F90 compiler
#  pathf90      PathScale
#  sxf90        NEC SX Fortran 90
#  openf90      AMD's x86 open64
#  sunf90       Sun's Studio
#
# Known Fortran 77 compilers:
#  af77         Apogee F77 compiler for Intergraph hardware running CLIX
#  blrts_xlf    IBM BlueGene/L F77 cross-compiler
#  cf77         native F77 compiler under older Crays (prefer over fort77)
#  f77          generic compiler names
#  fl32         Microsoft Fortran 77 "PowerStation" compiler
#  fort77       native F77 compiler under HP-UX (and some older Crays)
#  frt          Fujitsu F77 compiler
#  g77          GNU Fortran 77 compiler
#  pgf77        Portland Group F77 compiler
#  xlf          IBM (AIX) F77 compiler
#  pathf77      PathScale
#
AC_DEFUN([FNCS_F77_COMPILERS],[xlf95 pgf95 pathf95 ifort g95 f95 fort ifc efc openf95 sunf95 crayftn gfortran lf95 ftn xlf90 f90 pgf90 pghpf pathf90 epcf90 sxf90 openf90 sunf90 xlf f77 frt pgf77 pathf77 g77 cf77 fort77 fl32 af77])dnl

# FNCS_F77_COMPILERS_R
# --------------------
# FNCS_F77_COMPILERS list with '_r' appended to each.
AC_DEFUN([FNCS_F77_COMPILERS_R],m4_combine([ ], m4_split(FNCS_F77_COMPILERS), [], [_r]))dnl

# FNCS_MPIF77_COMPILERS
# ---------------------
# Known MPI Fortran 95 compilers:
#  cmpifc       ?? not sure if this is even F95
#  ftn          native Fortran 95 compiler on Cray XT4,XT5
#  mpif95       generic compiler name
#  mpixlf95     IBM BlueGene/P Fortran 95
#  mpixlf95_r   IBM BlueGene/P Fortran 95, reentrant code
#  mpxlf95      IBM BlueGene/L Fortran 95
#  mpxlf95_r    IBM BlueGene/L Fortran 95, reentrant code
#
# Known MPI Fortran 90 compilers:
#  cmpif90c     ??
#  mpf90        ??
#  mpif90       generic compiler name
#  mpxlf90      IBM BlueGene/L Fortran 90
#  mpxlf90_r    IBM BlueGene/L Fortran 90, reentrant code
#  sxmpif90     NEC SX Fortran 90
#
# Known MPI Fortran 77 compilers:
#  hf77         ??
#  mpf77        ??
#  mpif77       generic compiler name
#  mpxlf        IBM BlueGene/L Fortran 77
#  mpxlf_r      IBM BlueGene/L Fortran 77, reentrant code
#  mpifrt       Fujitsu
#
AC_DEFUN([FNCS_MPIF77_COMPILERS],[mpif95 mpxlf95 ftn mpif90 mpxlf90 mpxlf90 mpf90 cmpif90c sxmpif90 mpif77 hf77 mpxlf mpxlf mpifrt mpf77 cmpifc])

# FNCS_MPIF77_COMPILERS_R
# -----------------------
# FNCS_MPIF77_COMPILERS list with '_r' appended to each.
AC_DEFUN([FNCS_MPIF77_COMPILERS_R],m4_combine([ ], m4_split(FNCS_MPIF77_COMPILERS), [], [_r]))dnl

# FNCS_CC_COMPILERS
# -----------------
# Known C compilers
#  cc       generic compiler name
#  ccc      Fujitsu ?? old Cray ??
#  cl
#  ecc      Intel on IA64 ??
#  gcc      GNU
#  icc      Intel
#  bgxlc    Intel on BG/P
#  bgxlc_r  Intel on BG/P, thread safe
#  xlc      Intel
#  xlc_r    Intel, thread safe
#  pgcc     Portland Group
#  pathcc   PathScale
#  sxcc     NEC SX
#  fcc      Fujitsu
#  opencc   AMD's x86 open64
#  suncc    Sun's Studio
#  craycc   Cray
#
AC_DEFUN([FNCS_CC_COMPILERS],[bgxlc xlc pgcc pathcc icc sxcc fcc opencc suncc craycc gcc ecc cl ccc cc])dnl

# FNCS_CC_COMPILERS_R
# -------------------
# FNCS_CC_COMPILERS list with '_r' appended to each.
AC_DEFUN([FNCS_CC_COMPILERS_R],m4_combine([ ], m4_split(FNCS_CC_COMPILERS), [], [_r]))dnl

# FNCS_MPICC_COMPILERS
# --------------------
# Known MPI C compilers:
#  mpicc
#  mpixlc_r
#  mpixlc
#  hcc
#  mpxlc_r
#  mpxlc
#  sxmpicc  NEC SX
#  mpifcc   Fujitsu
#  mpgcc
#  mpcc
#  cmpicc
#  cc
#
AC_DEFUN([FNCS_MPICC_COMPILERS],[mpicc mpixlc mpixlc hcc mpxlc mpxlc sxmpicc mpifcc mpgcc mpcc cmpicc cc])dnl

# FNCS_MPICC_COMPILERS_R
# ----------------------
# FNCS_MPICC_COMPILERS list with '_r' appended to each.
AC_DEFUN([FNCS_MPICC_COMPILERS_R],m4_combine([ ], m4_split(FNCS_MPICC_COMPILERS), [], [_r]))dnl

# FNCS_CXX_COMPILERS
# ------------------
# Known C++ compilers:
#  aCC      HP-UX C++ compiler much better than `CC', so test before.
#  c++
#  cc++
#  CC
#  cl.exe
#  cxx
#  FCC      Fujitsu C++ compiler
#  g++
#  gpp
#  icpc     Intel C++ compiler
#  KCC      KAI C++ compiler
#  RCC      Rational C++
#  bgxlC    Intel
#  bgxlC_r  Intel, thread safe
#  xlC      AIX C Set++
#  xlC_r    AIX C Set++, thread safe
#  pgCC     Portland Group
#  pathCC   PathScale
#  sxc++    NEC SX
#  openCC   AMD's x86 open64
#  sunCC    Sun's Studio
#  crayc++  Cray
#
AC_DEFUN([FNCS_CXX_COMPILERS],[icpc pgCC pathCC sxc++ xlC bgxlC openCC sunCC crayc++ g++ c++ gpp aCC CC cxx cc++ cl.exe FCC KCC RCC])dnl

# FNCS_CXX_COMPILERS_R
# --------------------
AC_DEFUN([FNCS_CXX_COMPILERS_R],m4_combine([ ], m4_split(FNCS_CXX_COMPILERS), [], [_r]))dnl

# FNCS_MPICXX_COMPILERS
# ---------------------
# Known MPI C++ compilers
#  mpic++
#  mpicxx
#  mpiCC
#  sxmpic++     NEC SX
#  hcp
#  mpxlC_r
#  mpxlC
#  mpixlcxx_r
#  mpixlcxx
#  mpg++
#  mpc++
#  mpCC
#  cmpic++
#  mpiFCC       Fujitsu
#  CC
#
AC_DEFUN([FNCS_MPICXX_COMPILERS],[mpic++ mpicxx mpiCC sxmpic++ hcp mpxlC mpixlcxx mpg++ mpc++ mpCC cmpic++ mpiFCC CC])dnl

# FNCS_MPICXX_COMPILERS_R
# -----------------------
AC_DEFUN([FNCS_MPICXX_COMPILERS_R],m4_combine([ ], m4_split(FNCS_MPICXX_COMPILERS), [], [_r]))dnl

