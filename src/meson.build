binchecker_sources = [
  'main.c',
  'binchecker-application.c',
  'binchecker-window.c',
  'binchecker-diff-dialog.c'
]

binchecker_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('libbindiff')
]

binchecker_sources += gnome.compile_resources('binchecker-resources',
  'binchecker.gresource.xml',
  c_name: 'binchecker'
)

executable('binchecker', binchecker_sources,
  dependencies: binchecker_deps,
  install: true,
)
