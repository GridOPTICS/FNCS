ACLOCAL_AMFLAGS = -I m4

include_HEADERS =
lib_LTLIBRARIES =
bin_PROGRAMS =
check_PROGRAMS =
EXTRA_DIST = README
AM_CXXFLAGS =
AM_CPPFLAGS =
AM_LDFLAGS =
LDADD =

AM_CPPFLAGS += $(MPI_CPPFLAGS)

AM_LDFLAGS += $(MPI_LDFLAGS)

AM_LDFLAGS += $(MPI_LDFLAGS)

LDADD += libfncs_api.la
LDADD += $(MPI_LIBS)

include_HEADERS += abscomminterface.h
include_HEADERS += message.h
include_HEADERS += integrator.h
include_HEADERS += mpicomminterface.h
include_HEADERS += objectcomminterface.h
include_HEADERS += util/callback.h
include_HEADERS += util/time.h

lib_LTLIBRARIES += libfncs_api.la
libfncs_api_la_SOURCES =
libfncs_api_la_SOURCES += abscomminterface.cpp
libfncs_api_la_SOURCES += message.cpp
libfncs_api_la_SOURCES += integrator.cpp
libfncs_api_la_SOURCES += mpicomminterface.cpp
libfncs_api_la_SOURCES += objectcomminterface.cpp
libfncs_api_la_SOURCES += util/time.cpp

check_PROGRAMS += tests/sanity
tests_sanity_SOURCES = tests/sanity.cpp

check_PROGRAMS += tests/messageTest
tests_messageTest_SOURCES = tests/messageTest.cpp

check_PROGRAMS += tests/simmer
tests_simmer_SOURCES = tests/simmer.cpp
