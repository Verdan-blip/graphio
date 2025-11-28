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

cairo = dependency('cairo')

wl_protocol_dir = wayland_protos.get_variable(pkgconfig: 'pkgdatadir')

xdg_shell_xml = join_paths(wl_protocol_dir, 'stable', 'xdg-shell', 'xdg-shell.xml')

xdg_shell_protocol_h = custom_target('xdg-shell-protocol.h',
  input: xdg_shell_xml,
  output: 'xdg-shell-protocol.h',
  command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@']
)

xdg_shell_client_protocol_h = custom_target('xdg-shell-client-protocol.h',
  input: xdg_shell_xml,
  output: 'xdg-shell-client-protocol.h',
  command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@']
)

# Исходные файлы
sources = [
  'src/main.c',
  'src/components/paddings.c',
  'src/g-output.c',
  'src/g-cursor.c',
  'src/g-server.c',
  xdg_shell_protocol_h,
  xdg_shell_client_protocol_h
]

executable(
  'graphio',
  sources,
  dependencies: [
    wlroots,
    wayland_server,
    wayland_client,
    xkbcommon,
    cairo,
    pixman
  ]
)