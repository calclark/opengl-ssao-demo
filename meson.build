project(
  'demo',
  'cpp',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
    'cpp_args=-Wold-style-cast'
  ]
)

sources = [
  'src/main.cpp',
  'src/texture.cpp',
  'src/mesh.cpp',
  'src/shader.cpp',
  'src/scene.cpp',
  'src/manager.cpp',
  'src/loader.cpp',
  'src/camera.cpp'
]

dependencies = [
  dependency('SDL2'),
  subproject('glm').get_variable('glm_dep'),
  subproject('glad').get_variable('glad_dep'),
  subproject('tinyobjloader').get_variable('tinyobjloader_dep'),
  subproject('stb_image').get_variable('stb_image_dep'),
  subproject('fmt').get_variable('fmt_dep')
]

out = executable(
  'demo',
  sources,
  dependencies: dependencies
)

run_target('run', command: out)

