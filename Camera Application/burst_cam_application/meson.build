project('simple-cam', 'c', 'cpp',
	default_options : [
		'werror=false',
		'warning_level=2',
		'cpp_std=c++17',
	])

# simple-cam.cpp is the fully commented application
src_files = files([
	'simple-cam.cpp',
	'event_loop.cpp',
	'file_sink.cpp',
	'frame_sink.cpp',
	'image.cpp',
])

# Point your PKG_CONFIG_PATH environment variable to the
# libcamera install path libcamera.pc file ($prefix/lib/pkgconfig/libcamera.pc)
deps = [
      dependency('libcamera', required : true),
      dependency('libevent_pthreads'),
]

cpp_arguments = [ '-Wno-unused-parameter', ]

add_project_arguments(cpp_arguments, language : 'cpp')

# simple-cam executable
simple_cam = executable('simple-cam', src_files,
                        dependencies : deps)
