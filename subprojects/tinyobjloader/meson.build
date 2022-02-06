project(
  'tinyobjloader',
  'cpp',
  default_options: [
    'warning_level=0'
  ]
)

inc = include_directories(
  'include',
  is_system: true
)

tinyobjloader = static_library(
  'tinyobjloader',
  'src/tiny_obj_loader.cpp',
  include_directories : inc,
)

tinyobjloader_dep = declare_dependency(
  include_directories :inc,
  link_with: tinyobjloader
)
