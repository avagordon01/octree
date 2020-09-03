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
  language: 'cpp'
)

test('test', executable(
  't',
  'test.cc',
))
