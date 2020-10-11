project('tree', 'cpp',
  default_options: [
    'cpp_std=c++17',
    'buildtype=debugoptimized',
    'warning_level=3',
    'werror=true',
  ]
)

add_global_arguments(
  '-Wno-unused-parameter',
  '-Wno-unused-function',
  '-Wno-deprecated-copy',
  language: 'cpp'
)

eigen = dependency('eigen3')

test('tree-test', executable(
  'tree-test',
  'test.cc',
  dependencies: [eigen],
  cpp_args: ['-DRUN=0'],
))

test('list-test', executable(
  'list-test',
  'test.cc',
  dependencies: [eigen],
  cpp_args: ['-DRUN=1'],
))
