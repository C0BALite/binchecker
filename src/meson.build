binchecker_sources = [
  'main.c',
  'binchecker-application.c',
  'binchecker-window.c',
]

libbindiff_inc = include_directories('/usr/local/include/libbindiff')

libbindiff = meson.get_compiler('c').find_library('bindiff')

binchecker_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  libbindiff
]

binchecker_sources += gnome.compile_resources('binchecker-resources',
  'binchecker.gresource.xml',
  c_name: 'binchecker'
)

executable('binchecker', binchecker_sources,
  dependencies: binchecker_deps,
  include_directories:libbindiff_inc,
  install: true,
)
