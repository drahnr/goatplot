#! /usr/bin/env python3
# encoding: utf-8

VERSION = '0.0.11'
LIBNAME = 'goatplot'

top = '.'
out = 'build'

import os
from waflib import Logs as logs
from waflib import Utils as utils

recurse = ['catalog','meta','tests', 'src']

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
	bld.load('compiler_c gnu_dirs glib2')
	bld.recurse(recurse)
	bld.add_pre_fun(pre)

from waflib.Build import BuildContext

class release(BuildContext):
	"""compile release binary"""
	cmd = 'release'
	variant = 'release'

class debug(BuildContext):
	"""compile debug binary"""
	cmd = 'debug'
	variant = 'debug'

from waflib.Context import Context

def codestyle_fun(ctx):
    ctx.recurse('./src')

class codestyle(Context):
    	"""format code properly"""
    	cmd = 'codestyle'
    	fun = 'codestyle_fun'
