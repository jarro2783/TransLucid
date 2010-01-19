EXTRA_DIST = CMakeLists.txt

lib_LTLIBRARIES = libtlinterpreter.la

libtlinterpreter_la_LDFLAGS = -version-info 0:0:0 $(GLIBMM_LIBS) \
$(BOOST_LDFLAGS)
libtlinterpreter_la_LIBADD = -lrt -lgmpxx -lgmp $(BOOST_THREAD_LIB) \
-lltdl

libtlinterpreter_la_SOURCES = interpreter.cpp interpreter_expr.cpp \
interpreter_tuple.cpp interpreter_constant.cpp \
library.cpp types.cpp \
dimtranslator.cpp \
range.cpp evaluator.cpp expr.cpp  parser.cpp \
builtin_types.cpp header_type.cpp equation.cpp \
cache.cpp utility.cpp constant.cpp function.cpp compiled_functors.cpp \
expr_compiler.cpp

libtlinterpreter_la_CPPFLAGS = $(GLIBMM_CFLAGS) -I$(top_srcdir)/src/include \
-DPKGLIBDIR=\"$(pkglibdir)\" -Wall $(BOOST_CPPFLAGS)
