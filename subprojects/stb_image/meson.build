project(
  'stb_image',
  'c',
  default_options: [
    'warning_level=0'
  ]
)

inc = include_directories(
  'include',
  is_system: true
)

stb_image = static_library(
  'stb_image',
  'src/stb_image.c',
  include_directories : inc,
)

stb_image_dep = declare_dependency(
  include_directories :inc,
  link_with: stb_image
)
