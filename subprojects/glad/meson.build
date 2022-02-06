project(
  'glad',
  'c',
  default_options: [
    'warning_level=0'
  ]
)

inc = include_directories(
  'include',
  is_system: true
)

dl_dep = meson.get_compiler('c').find_library('dl', required : false)

glad = static_library(
  'glad',
  'src/glad.c',
  include_directories : inc,
  dependencies: dl_dep
)

glad_dep = declare_dependency(
  include_directories :inc,
  link_with: glad
)
