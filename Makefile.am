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

LDADD += libfncs.la
LDADD += $(MPI_LIBS)
LDADD += $(ZMQ_LIBS)

include_HEADERS += absnetworkinterface.h
include_HEADERS += cintegrator.h
include_HEADERS += echo.h
include_HEADERS += integrator.h
include_HEADERS += message.h
include_HEADERS += objectcomminterface.h
if HAVE_ZMQ
include_HEADERS += zmqhelper.h
include_HEADERS += zmqnetworkinterface.h
endif
if HAVE_MPI
include_HEADERS += mpinetworkinterface.h
endif

include_HEADERS += commmanager/abscommmanager.h
include_HEADERS += commmanager/communicationcommanager.h
include_HEADERS += commmanager/graceperiodcommmanager.h
include_HEADERS += config_parser/factorydatabase.h

include_HEADERS += syncalgorithms/abssyncalgorithm.h
include_HEADERS += syncalgorithms/communicatorsimulatorsyncalgo.h
include_HEADERS += syncalgorithms/graceperiodpesimisticsyncalgo.h
include_HEADERS	+= syncalgorithms/conservativesleepingcommalgo.h
include_HEADERS	+= syncalgorithms/conservativesleepingtickalgo.h
include_HEADERS	+= syncalgorithms/optimisticticksyncalgo.h
include_HEADERS	+= syncalgorithms/optimisticcommsyncalgo.h
include_HEADERS	+= syncalgorithms/optimisticlowoverheadcommsyncalgo.h
include_HEADERS	+= syncalgorithms/optimisticlowoverheadticksyncalgo.h
include_HEADERS += syncalgorithms/speculationtimecalculationstrategy.h

include_HEADERS += util/callback.h
include_HEADERS += util/simtime.h
include_HEADERS += util/csimtime.h

AM_CPPFLAGS += -I$(top_srcdir)/commmanager
AM_CPPFLAGS += -I$(top_srcdir)/syncalgorithms
AM_CPPFLAGS += -I$(top_srcdir)/util
AM_CPPFLAGS += -I$(top_srcdir)/contrib/jsoncpp/dist
AM_CPPFLAGS += -I$(top_srcdir)/config_parser

lib_LTLIBRARIES += libfncs.la
libfncs_la_SOURCES =
libfncs_la_SOURCES += absnetworkinterface.cpp
libfncs_la_SOURCES += cintegrator.cpp
libfncs_la_SOURCES += integrator.cpp
libfncs_la_SOURCES += message.cpp
libfncs_la_SOURCES += objectcomminterface.cpp
if HAVE_ZMQ
libfncs_la_SOURCES += zmqhelper.cpp
libfncs_la_SOURCES += zmqnetworkinterface.cpp
endif
if HAVE_MPI
libfncs_la_SOURCES += mpinetworkinterface.cpp
endif

libfncs_la_SOURCES += commmanager/abscommmanager.cpp
libfncs_la_SOURCES += commmanager/communicationcommanager.cpp
libfncs_la_SOURCES += commmanager/graceperiodcommmanager.cpp

libfncs_la_SOURCES += contrib/jsoncpp/dist/jsoncpp.cpp
libfncs_api_la_SOURCES += config_parser/factorydatabase.cpp

libfncs_la_SOURCES += syncalgorithms/abssyncalgorithm.cpp
libfncs_la_SOURCES += syncalgorithms/communicatorsimulatorsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/graceperiodpesimisticsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/conservativesleepingcommalgo.cpp
libfncs_la_SOURCES += syncalgorithms/conservativesleepingtickalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticticksyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticcommsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticlowoverheadcommsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticlowoverheadticksyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/speculationtimecalculationstrategy.cpp

libfncs_la_SOURCES += util/simtime.cpp

check_PROGRAMS += tests/sanity
tests_sanity_SOURCES = tests/sanity.cpp

check_PROGRAMS += tests/messageTest
tests_messageTest_SOURCES = tests/messageTest.cpp

if HAVE_MPI
check_PROGRAMS += tests/simmer
tests_simmer_SOURCES = tests/simmer.cpp
endif

if HAVE_ZMQ
bin_PROGRAMS += fncsbroker
fncsbroker_SOURCES = fncsbroker.cpp

check_PROGRAMS += tests/zimmer
tests_zimmer_SOURCES = tests/zimmer.cpp

check_PROGRAMS += tests/simmerTransmission
tests_simmerTransmission_SOURCES = tests/simmerTransmission.cpp
endif
