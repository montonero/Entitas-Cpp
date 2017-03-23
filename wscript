#! /usr/bin/env python
import sys
#import commands
import os
import os.path
from os.path import join, abspath
import inspect

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='entitas'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(ctx):
	ctx.load('compiler_c compiler_cxx')
	ctx.add_option('--emscripten', dest='emscripten', default=False, action='store_true', help='Build with emscripten.')
	ctx.add_option('--android', dest='android', default=False, action='store_true', help='Build for Android.')


def configure(ctx):
	ctx.env.emscripten = ctx.options.emscripten
	ctx.env.android = ctx.options.android


	if ctx.env.emscripten:
		waf_dir = os.path.dirname(inspect.stack()[-1][1])
		waf_extras_dir = os.path.join(waf_dir, 'waflib/extras')
		ctx.load('c_emscripten', tooldir = waf_extras_dir)
                print("Emscripten.")

	elif ctx.env.android:
		print("Android")

	else:
		ctx.find_program('clang++', var = 'CXX', mandatory = True)
		ctx.find_program('clang', var = 'CC', mandatory = True)

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
		cxxflags     = ['-std=c++14', '-g'],
		includes = 'entitas',
		export_includes = '. entitas',
	)

	external_node = ctx.path.find_node('external')
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


	cxx_flags = ['-Wall', '-Wextra', '-std=c++14', '-g']
	link_flags = ['-Wl,-rpath,.']

	if ctx.env.emscripten:
		emscripten_flags = [
			'FULL_ES2=1',
			# 'USE_ZLIB=1',
			# 'USE_LIBPNG=1',
			'DEMANGLE_SUPPORT=1',
			'OUTLINING_LIMIT=20000',
			'ASSERTIONS=1',
			'USE_SDL=2',
			'USE_SDL_TTF=2',
			'USE_SDL_IMAGE=2',
                        'DISABLE_EXCEPTION_CATCHING=0',
                        'SDL2_IMAGE_FORMATS=["png", "jpeg"]',
		]
		emscripten_flags = reduce(lambda a, v: a + ['-s', v], emscripten_flags, [])
		emscripten_flags += [
			'-Os'
		]


		s1 = ctx.path.ant_glob(['sample/sample1.cpp'])
		ctx.program(
			source = s1,
			target = 's1',
			cxxflags = cxx_flags + emscripten_flags,
			linkflags = link_flags + emscripten_flags,
			use = libs
		)

                # assets_path = ctx.path.find_dir(['assets', 'textures', 'tiles_snow'])
                assets_path = ctx.path.find_dir(['assets'])
                assets_p = assets_path.abspath() + "@/"
                print(assets_p)

                link_flags += [
                        '--preload-file', assets_p,
                ]

		s2 = ctx.path.ant_glob(['sample/sample2.cpp'])
		ctx.program(
			source = s2,
			target = 's2',
			cxxflags = cxx_flags + emscripten_flags,
			linkflags = link_flags + emscripten_flags,
			defines = ['_SDL2'],
			# lib = ['SDL2_ttf', 'SDL2_image'],
			use = libs + ['SDL2']
		)

	else:

		s1 = ctx.path.ant_glob(['sample/sample1.cpp'])
		ctx.program(
			source = s1,
			target = 's1',
			cxxflags = cxx_flags,
			linkflags = link_flags,
			lib = ['pthread'],
			use = libs
		)


		s2 = ctx.path.ant_glob(['sample/sample2.cpp'])
		ctx.program(
			source = s2,
			target = 's2',
			cxxflags = cxx_flags,
			linkflags = link_flags,
			defines = ['_SDL2'],
			lib = ['SDL2_ttf', 'SDL2_image'],
			use =  libs + ['SDL2', 'pthread']
		)

	if ctx.cmd != 'clean':
		from waflib import Logs
		ctx.logger = Logs.make_logger('test.log', 'build') # just to get a clean output
		ctx.logger = None
