project(
  'graphio', 
  'c',
  version: '0.1.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=false'
  ]
)

args = [
  '-DWLR_USE_UNSTABLE',
  '-Wno-unused-parameter',
  '-Wno-unused-variable',
  '-Wno-unused-function'
]

add_project_arguments(
  args,
  language: 'c',
)

wayland_scanner = find_program('wayland-scanner')

wlroots = dependency('wlroots-0.19')
wayland_protos = dependency('wayland-protocols')
wayland_server = dependency('wayland-server')
wayland_client = dependency('wayland-client')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')
jansson = dependency('jansson')
cairo = dependency('cairo')

sources = [
  'src/main.c',
  'src/g_toplevel.c',
  'src/g_launcher.c',
  'src/g_layer_surface.c',
  'src/utils/g_image.c',
  'src/utils/g_image_utils.c'
]

protocols_inc = include_directories('protocols')

executable(
  'graphio',
  sources,
  include_directories: protocols_inc,
  dependencies: [
    wlroots,
    wayland_server,
    wayland_client,
    xkbcommon,
    pixman,
    jansson,
    cairo
  ]
)