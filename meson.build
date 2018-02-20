project('c11', 'c')

threads = dependency('threads')
sources = ['src/pthread.c', 'src/win32.c']
incdirs = include_directories('include')

lib = static_library(
  'c11',
  sources,
  include_directories: incdirs,
  dependencies: [threads]
)

c11_dep = declare_dependency(
  include_directories: incdirs,
  link_with: lib)
