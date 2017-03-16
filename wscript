#! /usr/bin/env python
import sys
#import commands
import os
import os.path
from os.path import join, abspath

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='entitas'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(ctx):
    ctx.load('compiler_c compiler_cxx')
    # ctx.add_option('--boost-path', dest='boost_path', help='Boost path.')
    # ctx.add_option('--wspp-path', dest='wspp_path', help='Websocket++ path.')

def configure(ctx):
    # ctx.env.ws_client_dir = os.path.abspath(os.path.join(ctx.path.parent.abspath(), 'client'))
    # ctx.recurse(ctx.env.ws_client_dir)
    #ctx.find_program('clang++', var = 'CXX', mandatory = True)
    #ctx.find_program('clang', var = 'CC', mandatory = True)
    ctx.load('compiler_c compiler_cxx')

    if sys.platform.startswith('win'):
        ctx.env.LIBPATH_SDL2   = [join("", 'stage/lib')]
        ctx.env.INCLUDES_SDL2  = [join("", 'boost')]
    else:
        ctx.check_cfg(
            path='sdl2-config',
            args='--cflags --libs',
            package='',
            uselib_store='SDL2')

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
    libs = ['entitas', 'SDLpp',
            'mathfu', 'vectorial',
    ]
    # ctx.recurse('chibi')
    # ctx.recurse(ctx.env.ws_client_dir)

    ctx.stlib(
        source = ctx.path.ant_glob('entitas/*.cpp'),
        target = 'entitas',
        #cxxflags     = ['-std=c++1y', '-g', '-stdlib=libc++'],
        cxxflags     = ['-std=c++14', '-g'],
        includes = 'entitas',
        export_includes = '. entitas',
    )

    external_node = ctx.path.find_node('external')
    # chibi_node = external_node.find_node('chibi')
    sdlpp_node = external_node.find_node('SDLpp')


    mathfu_node = external_node.find_node('mathfu')
    mathfu_include_path = join(mathfu_node.abspath(), 'include')
    ctx(includes=mathfu_include_path,
        export_includes=mathfu_include_path,
        name='mathfu')

    vectorial_node = mathfu_node.find_node('dependencies/vectorial')
    print(vectorial_node.abspath())
    vectorial_include_path = join(vectorial_node.abspath(), 'include')
    ctx(includes=vectorial_include_path,
        export_includes=vectorial_include_path,
        name='vectorial')


    ctx.recurse(sdlpp_node.abspath())

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
        cxxflags     = ['-std=c++14', '-g'],
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
        cxxflags     = ['-std=c++14', '-g'],
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
        lib = ['SDL2_ttf', 'SDL2_image'],
        use =  libs + ['SDL2', 'pthread'] 
    )

    if ctx.cmd != 'clean':
        from waflib import Logs
        ctx.logger = Logs.make_logger('test.log', 'build') # just to get a clean output
        ctx.logger = None
