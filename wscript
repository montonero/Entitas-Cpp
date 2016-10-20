#! /usr/bin/env python
import sys
import commands
import os
import os.path

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='entitas'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(ctx):
    ctx.load('compiler_c compiler_cxx')
    ctx.add_option('--boost-path', dest='boost_path', help='Boost path.')
    ctx.add_option('--wspp-path', dest='wspp_path', help='Websocket++ path.')

def configure(ctx):
    # ctx.env.ws_client_dir = os.path.abspath(os.path.join(ctx.path.parent.abspath(), 'client'))
    # ctx.recurse(ctx.env.ws_client_dir)
    ctx.load('compiler_c compiler_cxx')

def build(ctx):
    # libs = ['SDL2', 'chibi_static_lib', 'BOOST', 'ws_client_lib', 'ws_retry_client']
    libs = []

    # ctx.recurse('chibi')
    # ctx.recurse(ctx.env.ws_client_dir)

    ctx.stlib(
        source = ctx.path.ant_glob('EntitasPP/*.cpp'),
        target = 'entitas',
        cxxflags     = ['-std=c++14'],
        includes = 'EntitasPP',
        export_includes = '. EntitasPP',
    )

    s1 = ctx.path.ant_glob(['*.cpp'])
    ctx.program(
        source = s1,
        features='cxx cxxprogram',
        target='demo',
        cxxflags     = ['-std=c++14'],
        # linkflags = ['-Wl', '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl,-Bdynamic', '-lm', '-lpthread', '-lc', '-lstdc++'],
        use =  ['entitas']
    )

    s1 = ctx.path.ant_glob(['sample/*.cpp'])
    ctx.program(
        source = s1,
        features='cxx cxxprogram',
        target='s1',
        cxxflags     = ['-std=c++14'],
        # linkflags = ['-Wl', '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl,-Bdynamic', '-lm', '-lpthread', '-lc', '-lstdc++'],
        use =  ['entitas']
    )


    if ctx.cmd != 'clean':
        from waflib import Logs
        ctx.logger = Logs.make_logger('test.log', 'build') # just to get a clean output
        ctx.logger = None
