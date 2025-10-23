project('kzsh', ['c', 'cpp'], version: '0.1.2')

ksh_version = meson.project_version()
ksh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
ksh_target = host_machine.cpu_family() + '-' + host_machine.system()

# Gather sources safely (Meson doesn't expand wildcards automatically)
src_dir = 'src'
ksh_sources = []

if fs.exists(src_dir)
  ksh_sources += files()
  ksh_sources += [files(
    *[f for f : fs.glob('src/*.c')]
  )]
  ksh_sources += [files(
    *[f for f : fs.glob('src/*.cpp')]
  )]
else
  warning('Source directory "src" does not exist yet — skipping source files.')
endif

# Dependencies
readline_dep = dependency('readline', required: true)
glibc_dep = cc.find_library('c', required: true)

# Build executable
ksh_exe = executable('kzsh',
  ksh_sources,
  include_directories: include_directories('include'),
  dependencies: [readline_dep, glibc_dep],
  install: true,
  cpp_args: [
    '-DKSH_VERSION="' + ksh_version + '"',
    '-DKSH_BUILD_DATE="' + ksh_build_date + '"',
    '-DKSH_TARGET="' + ksh_target + '"'
  ]
)

message('Building kzsh ' + ksh_version + ' for ' + ksh_target + ' (' + ksh_build_date + ')')
