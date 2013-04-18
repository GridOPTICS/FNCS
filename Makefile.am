ACLOCAL_AMFLAGS = -I m4

include_HEADERS =
noinst_HEADERS = 
lib_LTLIBRARIES =
bin_PROGRAMS =
check_PROGRAMS =
EXTRA_DIST = README
AM_CXXFLAGS =
AM_CPPFLAGS =
AM_LDFLAGS =
LDADD =

AM_CPPFLAGS += $(MPI_CPPFLAGS)
AM_CPPFLAGS += $(ZMQ_CPPFLAGS)

AM_LDFLAGS += $(MPI_LDFLAGS)
AM_LDFLAGS += $(ZMQ_LDFLAGS)

LDADD += libfncs_api.la
LDADD += $(MPI_LIBS)
LDADD += $(ZMQ_LIBS)

include_HEADERS += absnetworkinterface.h
include_HEADERS += cintegrator.h
include_HEADERS += echo.h
include_HEADERS += integrator.h
include_HEADERS += message.h
include_HEADERS += mpinetworkinterface.h
include_HEADERS += objectcomminterface.h
if HAVE_ZMQ
include_HEADERS += zmqnetworkinterface.h
endif

include_HEADERS += commmanager/abscommmanager.h
include_HEADERS += commmanager/communicationcommanager.h
include_HEADERS += commmanager/graceperiodcommmanager.h

include_HEADERS += syncalgorithms/abssyncalgorithm.h
include_HEADERS += syncalgorithms/communicatorsimulatorsyncalgo.h
include_HEADERS += syncalgorithms/graceperiodpesimisticsyncalgo.h
include_HEADERS	+= syncalgorithms/graceperiodspeculativesyncalgo.h
include_HEADERS	+= syncalgorithms/graceperiodnetworkdelaysyncalgo.h
include_HEADERS	+= syncalgorithms/optimisticticksyncalgo.h
include_HEADERS	+= syncalgorithms/optimisticcommsyncalgo.h

include_HEADERS += util/callback.h
include_HEADERS += util/simtime.h
include_HEADERS += util/csimtime.h

AM_CPPFLAGS += -I$(top_srcdir)/commmanager
AM_CPPFLAGS += -I$(top_srcdir)/syncalgorithms
AM_CPPFLAGS += -I$(top_srcdir)/util

lib_LTLIBRARIES += libfncs_api.la
libfncs_api_la_SOURCES =
libfncs_api_la_SOURCES += absnetworkinterface.cpp
libfncs_api_la_SOURCES += cintegrator.cpp
libfncs_api_la_SOURCES += integrator.cpp
libfncs_api_la_SOURCES += message.cpp
libfncs_api_la_SOURCES += mpinetworkinterface.cpp
libfncs_api_la_SOURCES += objectcomminterface.cpp
if HAVE_ZMQ
libfncs_api_la_SOURCES += zmqnetworkinterface.cpp
endif

libfncs_api_la_SOURCES += commmanager/abscommmanager.cpp
libfncs_api_la_SOURCES += commmanager/communicationcommanager.cpp
libfncs_api_la_SOURCES += commmanager/graceperiodcommmanager.cpp

libfncs_api_la_SOURCES += syncalgorithms/abssyncalgorithm.cpp
libfncs_api_la_SOURCES += syncalgorithms/communicatorsimulatorsyncalgo.cpp
libfncs_api_la_SOURCES += syncalgorithms/graceperiodpesimisticsyncalgo.cpp
libfncs_api_la_SOURCES += syncalgorithms/graceperiodspeculativesyncalgo.cpp
libfncs_api_la_SOURCES += syncalgorithms/graceperiodnetworkdelaysyncalgo.cpp
libfncs_api_la_SOURCES += syncalgorithms/optimisticticksyncalgo.cpp
libfncs_api_la_SOURCES += syncalgorithms/optimisticcommsyncalgo.cpp

libfncs_api_la_SOURCES += util/simtime.cpp

check_PROGRAMS += tests/sanity
tests_sanity_SOURCES = tests/sanity.cpp

check_PROGRAMS += tests/messageTest
tests_messageTest_SOURCES = tests/messageTest.cpp

check_PROGRAMS += tests/simmer
tests_simmer_SOURCES = tests/simmer.cpp

if HAVE_ZMQ
bin_PROGRAMS += fncsbroker
fncsbroker_SOURCES = fncsbroker.cpp

check_PROGRAMS += tests/zimmer
tests_zimmer_SOURCES = tests/zimmer.cpp
endif
