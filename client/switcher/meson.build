project(
  'switcher', 
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

gtk = dependency('gtk+-3.0')
gtk_layer = dependency('gtk-layer-shell-0')

sources = [
  'src/main.c',
  'src/math/sw_vec2.c',
  'src/sw_switcher.c',
  'src/sw_toplevel.c',
  'src/sw_scorer.c',
  'src/sw_graph_model.c',
  'src/ui/sw_flow_layout.c',
  'src/ui/sw_polar_layout.c',
  'src/ui/sw_primitives.c',
  'src/ui/sw_border_item.c',
  'src/ui/sw_toplevel_widget.c',
  'src/ui/sw_switcher_widget.c',
  'src/ui/animation/sw_animation.c',
  'src/ui/animation/sw_animation_manager.c',
  '../../protocols/src/wlr-foreign-toplevel-management-unstable-v1-protocol.c'
]

protocols_inc = include_directories('../../protocols')

executable(
  'main',
  sources,
  include_directories: protocols_inc,
  dependencies: [
    wlroots,
    wayland_server,
    wayland_client,
    xkbcommon,
    pixman,
    jansson,
    cairo,
    gtk,
    gtk_layer
  ]
)