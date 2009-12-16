SUBDIRS = sanity simple operations range
AUTOM4TE = autom4te
EXTRA_DIST = testsuite.at $(TESTSUITE) atlocal.in \
             $(srcdir)/package.m4 $(srcdir)/local.at
TESTSUITE=testsuite

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
	 $(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
	 $(SHELL) '$(TESTSUITE)' --clean

AUTOTEST = $(AUTOM4TE) --language=autotest

$(TESTSUITE): $(srcdir)/testsuite.at $(srcdir)/package.m4 $(srcdir)/local.at
	$(AUTOTEST) -I '$(srcdir)' package.m4 -o $@.tmp $@.at
	$(srcdir)/autotestfix.sed < $@.tmp > $@
	rm $@.tmp

# The `:;' works around a Bash 3.2 bug when the output is not writeable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
	echo '# Signature of the current package.' && \
	echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])' && \
	echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])' && \
	echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])' && \
	echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])' && \
	echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >'$(srcdir)/package.m4'

