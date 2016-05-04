#!/usr/bin/env python
# encoding: utf-8
# Carlos Rafael Giani, 2006
# Thomas Nagy, 2010
# Bernhard Schuster, 2014-2016

"""
Unit testing system for C/C++/D providing test execution:

* in parallel, by using ``waf -j``
* partial (only the tests that have changed) or full (by using ``waf``)

The tests are declared by adding the **test** feature to programs::

	def options(opt):
		opt.load('compiler_c unites')
	def configure(conf):
		conf.load('compiler_c unites')
	def build(bld):
		bld(features='c cprogram unites', source='main.cpp', target='app')
		# or
		bld.program(features='unites', source='main2.cpp', target='app2')

When the build is executed, the program 'test' will be built and executed without arguments.
The success/failure is detected by looking at the return code. The status and the standard output/error
are stored on the build context.
"""

import os, sys
from waflib.TaskGen import feature, after_method
from waflib import Utils, Task, Logs, Options, Errors, Context

@feature('unites')
@after_method('apply_link')
def make_test(self):
	"""Create the unit test task. There can be only one unit test task per task generator."""
	if getattr(self, 'link_task', None):
		self.create_task('unites', self.link_task.outputs)


class unites(Task.Task):
	"""
	Execute a unit test
	"""
	color = 'PINK'
	after = ['vnum','inst']
	vars = []
	def runnable_status(self):
		"""
		Always execute the task if ``waf --no-tests`` was not used
		"""
		if getattr(Options.options, 'no_tests', False):
			return Task.SKIP_ME

		ret = super(unites, self).runnable_status()
		if ret == Task.SKIP_ME:
			return Task.RUN_ME
		return ret

	def run(self):
		"""
		Execute the test. This can fail.
		"""

		testname = str(self.inputs[0])
		filename = self.inputs[0].abspath()



		self.ut_exec = getattr(self.generator, 'ut_exec', [filename])

		try:
			fu = getattr(self.generator.bld, 'all_test_paths')
		except AttributeError:
			# this operation may be performed by at most #maxjobs
			fu = os.environ.copy()

			lst = []
			for g in self.generator.bld.groups:
				for tg in g:
					if getattr(tg, 'link_task', None):
						s = tg.link_task.outputs[0].parent.abspath()
						if s not in lst:
							lst.append(s)

			def add_path(dct, path, var):
				dct[var] = os.pathsep.join(Utils.to_list(path) + [os.environ.get(var, '')])

			if Utils.is_win32:
				add_path(fu, lst, 'PATH')
			elif Utils.unversioned_sys_platform() == 'darwin':
				add_path(fu, lst, 'DYLD_LIBRARY_PATH')
				add_path(fu, lst, 'LD_LIBRARY_PATH')
			else:
				add_path(fu, lst, 'LD_LIBRARY_PATH')
			self.generator.bld.all_test_paths = fu

		cwd = getattr(self.generator, 'ut_cwd', '') or self.inputs[0].parent.abspath()
		testcmd = getattr(Options.options, 'testcmd', False)

		if testcmd:
			self.ut_exec = (testcmd % self.ut_exec[0]).split(' ')

		#overwrite the default logger to prevent duplicate logging
		proc = Utils.subprocess.Popen(self.ut_exec,\
		                              cwd=cwd,\
		                              env=fu,\
		                              stderr=Utils.subprocess.PIPE,\
		                              stdout=Utils.subprocess.PIPE)

		(out, err) = proc.communicate()

		msg = []
		if out:
			msg.append('stdout:%s%s' % (os.linesep, out.decode('utf-8')))
		if err:
			msg.append('stderr:%s%s' % (os.linesep, err.decode('utf-8')))
		msg = os.linesep.join(msg)
		Logs.debug(msg)

		return proc.returncode


def options(opt):
	"""
	Provide the ``permissive``, ``--notests`` and ``--testcmd`` command-line options.
	"""
	opt.add_option('--permissive-tests', action='store_true', default=False, help='Do not force exit if tests fail', dest='permissive_tests')
	opt.add_option('--no-tests', action='store_true', default=False, help='Exec no unit tests', dest='no_tests')
	opt.add_option('--testcmd', action='store', default=False,
	 help = 'Run the unit tests using the test-cmd string'
	 ' example "--test-cmd="valgrind --error-exitcode=1'
	 ' %s" to run under valgrind', dest='testcmd')
