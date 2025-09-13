binchecker_sources = [
  'main.c',
  'binchecker-application.c',
  'binchecker-window.c',
]

libbindif_sources = ['libbindiff.c']
libbindif_inc = include_directories('.') # Header directory for the library

libbindif = shared_library(
    'libbindiff',              # Name of the library (e.g., libmylib.so)
    libbindif_sources,
    include_directories: libbindif_inc,
    install: true         # Install the library to the system prefix
)
binchecker_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

binchecker_sources += gnome.compile_resources('binchecker-resources',
  'binchecker.gresource.xml',
  c_name: 'binchecker'
)

executable('binchecker', binchecker_sources,
  dependencies: binchecker_deps,
  include_directories: libbindif_inc, # Include library headers for the app
  link_with: libbindif,     # Link with the shared library
  install: true,
)
