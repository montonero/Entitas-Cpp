#! /usr/bin/env python
import sys
#import commands
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
    #ctx.find_program('clang++', var = 'CXX', mandatory = True)
    #ctx.find_program('clang', var = 'CC', mandatory = True)
    ctx.load('compiler_c compiler_cxx')

    ctx.check_cfg(
        path='sdl2-config',
        args='--cflags --libs',
        package='',
        uselib_store='SDL2')

    # ctx.env.append_value('CCFLAGS', '-std=c++14 -Wall ')
    ctx.check_cxx(
        #cxxflags=['-std=c++14', '-Wall'],
        cxxflags=[ '-Wall'],
        libs='SDL2',
    )

    ctx.check_cxx(lib='pthread',
        cflags='-Wall',
        uselib_store='pthread')
        # uselib_store='pthread', mandatory=True)



def build(ctx):
    # libs = ['SDL2', 'chibi_static_lib', 'BOOST', 'ws_client_lib', 'ws_retry_client']
    libs = ['entitas', 'glm']
    # ctx.recurse('chibi')
    # ctx.recurse(ctx.env.ws_client_dir)

    ctx.stlib(
        source = ctx.path.ant_glob('entitas/*.cpp'),
        target = 'entitas',
        #cxxflags     = ['-std=c++1y', '-g', '-stdlib=libc++'],
        cxxflags     = ['-std=c++1y', '-g'],
        includes = 'entitas',
        export_includes = '. entitas',
    )

    external_node = ctx.path.find_node('external')
    # chibi_node = external_node.find_node('chibi')
    glm_node = external_node.find_node('glm')

    ctx(includes=glm_node.abspath(),
        export_includes=glm_node.abspath(),
        name='glm')


    """
    ctx.stlib(
        source = ctx.path.ant_glob('external/
        target = 'Box2D',
        cxxflags     = ['-std=c++14', '-g'],
    """

    """
    s1 = ctx.path.ant_glob(['*.cpp'])
    ctx.program(
        source = s1,
        features='cxx cxxprogram',
        target='demo',
        cxxflags     = ['-std=c++1y', '-g'],
        # linkflags = ['-Wl', '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl,-Bdynamic', '-lm', '-lpthread', '-lc', '-lstdc++'],
        use =  ['entitas']
    )
    """

    s1 = ctx.path.ant_glob(['sample/sample1.cpp'])
    ctx.program(
        source = s1,
        features='cxx cxxprogram',
        target='s1',
        cxxflags     = ['-std=c++1y', '-g'],
        lib = ['m', 'c', 'pthread'],
        #linkflags = ['-Wl', '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl', '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl,-Bdynamic', '-lm', '-lpthread', '-lc', '-lstdc++'],
        use = libs
    )

    s2 = ctx.path.ant_glob(['sample/sample2.cpp'])
    ctx.program(
        source = s2,
        features='cxx cxxprogram',
        target='s2',
        cxxflags     = ['-std=c++14', '-g'],
        linkflags = [ '-lm', '-lpthread', '-lc', '-lstdc++'],
        # linkflags = ['-Wl,-Bdynamic', '-lm', '-lpthread', '-lc', '-lstdc++'],
        defines = ['_SDL2'],
        use =  libs + ['SDL2', 'pthread']
    )

    if ctx.cmd != 'clean':
        from waflib import Logs
        ctx.logger = Logs.make_logger('test.log', 'build') # just to get a clean output
        ctx.logger = None
