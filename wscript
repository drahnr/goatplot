#! /usr/bin/env python3
# encoding: utf-8

VERSION = '0.0.9'
LIBNAME = 'goatplot'

top = '.'
out = 'build'

import os
from waflib import Logs as logs
from waflib import Utils as utils

recurse = ['catalog','meta']

def options(opt):
	opt.recurse(recurse)
	opt.load('compiler_c gnu_dirs glib2')
	opt.load('unites', tooldir='.wafcustom')

def configure(cfg):
	cfg.env.LIBNAME = LIBNAME
	cfg.env.VERSION = VERSION

	cfg.recurse(recurse)
	cfg.load('compiler_c gnu_dirs glib2')
	cfg.load('unites', tooldir='.wafcustom')

	cfg.define('VERSION', VERSION)
	cfg.define('GETTEXT_PACKAGE', LIBNAME)


	cfg.check_cc(lib='m', uselib_store='M', mandatory=True)

	cfg.check_cfg(atleast_pkgconfig_version='0.26')
	cfg.check_cfg(package='glib-2.0', uselib_store='GLIB', args=['glib-2.0 >= 2.24', '--cflags', '--libs'], mandatory=True)
	cfg.check_cfg(package='gobject-2.0', uselib_store='GOBJECT', args=['--cflags', '--libs'], mandatory=True)
	cfg.check_cfg(package='gtk+-3.0', uselib_store='GTK3', args=['--cflags', '--libs'], mandatory=True)

	cfg.check_large_file(mandatory=False)
	cfg.check_endianness(mandatory=False)
	cfg.check_inline(mandatory=False)

	# -ggdb vs -g -- http://stackoverflow.com/questions/668962
	cfg.setenv('debug', env=cfg.env.derive())
	cfg.env.CFLAGS = ['-ggdb', '-Wall']
	cfg.define('DEBUG',1)

	cfg.setenv('release', env=cfg.env.derive())
	cfg.env.CFLAGS = ['-O2', '-Wall']
	cfg.define('RELEASE',1)


def dist(bld):
	bld.base_name = LIBNAME+'-'+VERSION
	bld.algo = 'tar.xz'
	bld.excl = ['.*', '*~','./build','*.'+bld.algo],
	bld.files = bld.path.ant_glob('**/wscript')



def pre(bld):
	if bld.cmd != 'install':
		logs.info ('Variant: \'' + bld.variant + '\'')



def build(bld):
	bld.recurse(recurse)
	bld.add_pre_fun(pre)


	objects = bld.objects(
		features = ['c', 'glib2'],
		target = 'objects',
		source = bld.path.ant_glob(['src/*.c'], excl='*/main.c'),
		includes = ['src/'],
		export_includes = ['src/'],
		uselib = 'M GOBJECT GLIB GTK3'
	)

#FIXME how to create a share lib without sources
	shlib = bld.shlib(
		features = ['c', 'glib2'],
		target = LIBNAME,
		source = bld.path.ant_glob(['src/*.c']),
		includes = ['src/'],
		export_includes = ['src/'],
		uselib = 'M GOBJECT GLIB GTK3',
		install_path = "${LIBDIR}"
	)
	bld.install_files('${INCLUDEDIR}/goatplot/', bld.path.ant_glob(['src/*.h'], excl=['src/*-internal.h']))

#	for item in shlib.includes:
#		logs.debug(item)
	test_screenshot = bld.program(
		features = ['c', 'glib2', 'unites'],
		target = 'test-screenshot',
		source = ['tests/screenshot.c'],
		includes = ['src/'],
		export_includes = ['src/'],
		use = 'objects',
		uselib = 'M GOBJECT GLIB GTK3'
	)

	test_dynamic = bld.program(
		features = ['c', 'glib2', 'unites'],
		target = 'test-dynamic',
		source = ['tests/dynamic.c'],
		includes = ['src/'],
		export_includes = ['src/'],
		use = 'objects',
		uselib = 'M GOBJECT GLIB GTK3'
	)


from waflib.Build import BuildContext

class release(BuildContext):
	"""compile release binary"""
	cmd = 'release'
	variant = 'release'

class debug(BuildContext):
	"""compile debug binary"""
	cmd = 'debug'
	variant = 'debug'
