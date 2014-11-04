ACLOCAL_AMFLAGS = -I m4

pkginclude_HEADERS =
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

pkginclude_HEADERS += absnetworkinterface.h
pkginclude_HEADERS += cintegrator.h
pkginclude_HEADERS += echo.h
pkginclude_HEADERS += integrator.h
pkginclude_HEADERS += message.h
pkginclude_HEADERS += objectcomminterface.h
if HAVE_ZMQ
pkginclude_HEADERS += zmqhelper.h
pkginclude_HEADERS += zmqnetworkinterface.h
endif
if HAVE_MPI
pkginclude_HEADERS += mpinetworkinterface.h
endif

pkginclude_HEADERS += commmanager/abscommmanager.h
pkginclude_HEADERS += commmanager/communicationcommanager.h
pkginclude_HEADERS += commmanager/graceperiodcommmanager.h
pkginclude_HEADERS += config_parser/factorydatabase.h
pkginclude_HEADERS += config_parser/fncsconfig.h

pkginclude_HEADERS += syncalgorithms/abssyncalgorithm.h
pkginclude_HEADERS += syncalgorithms/communicatorsimulatorsyncalgo.h
pkginclude_HEADERS += syncalgorithms/graceperiodpesimisticsyncalgo.h
pkginclude_HEADERS	+= syncalgorithms/conservativesleepingcommalgo.h
pkginclude_HEADERS	+= syncalgorithms/conservativesleepingtickalgo.h
pkginclude_HEADERS	+= syncalgorithms/optimisticticksyncalgo.h
pkginclude_HEADERS	+= syncalgorithms/optimisticcommsyncalgo.h
pkginclude_HEADERS += syncalgorithms/speculationtimecalculationstrategy.h
pkginclude_HEADERS += syncalgorithms/activesetconservativetick.h
pkginclude_HEADERS += syncalgorithms/activesetconservativecom.h
pkginclude_HEADERS += syncalgorithms/reactiveconservative.h

pkginclude_HEADERS += util/callback.h
pkginclude_HEADERS += util/simtime.h
pkginclude_HEADERS += util/csimtime.h

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
libfncs_la_SOURCES += config_parser/factorydatabase.cpp
libfncs_la_SOURCES += config_parser/fncsconfig.cpp

libfncs_la_SOURCES += syncalgorithms/abssyncalgorithm.cpp
libfncs_la_SOURCES += syncalgorithms/communicatorsimulatorsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/graceperiodpesimisticsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/conservativesleepingcommalgo.cpp
libfncs_la_SOURCES += syncalgorithms/conservativesleepingtickalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticticksyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/optimisticcommsyncalgo.cpp
libfncs_la_SOURCES += syncalgorithms/speculationtimecalculationstrategy.cpp
libfncs_la_SOURCES += syncalgorithms/activesetconservativetick.cpp
libfncs_la_SOURCES += syncalgorithms/activesetconservativecom.cpp
libfncs_la_SOURCES += syncalgorithms/reactiveconservative.cpp

libfncs_la_SOURCES += util/simtime.cpp
libfncs_la_SOURCES += util/profiler.cpp

check_PROGRAMS += tests/sanity
tests_sanity_SOURCES = tests/sanity.cpp

check_PROGRAMS += tests/messageTest
tests_messageTest_SOURCES = tests/messageTest.cpp

#check_PROGRAMS += tests/realtime_sim
#tests_realtime_sim_SOURCES = tests/realtime_sim.cpp

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
