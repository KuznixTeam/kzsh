project('kzsh', ['c', 'cpp'], version: '0.1.2')

ksh_version = meson.project_version()
ksh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
ksh_target = host_machine.cpu_family() + '-' + host_machine.system()

# ✅ safest approach — list or manually glob with `files()`
# (Meson doesn't support wildcards without modules)
ksh_sources = []
if run_command('test', '-d', 'src').returncode() == 0
  c_files = run_command('sh', '-c', 'echo src/*.c', check: false).stdout().strip().split()
  cpp_files = run_command('sh', '-c', 'echo src/*.cpp', check: false).stdout().strip().split()
  foreach f : c_files + cpp_files
    if f != 'src/*.c' and f != 'src/*.cpp'
      ksh_sources += files(f)
    endif
  endforeach
else
  warning('Source directory "src" does not exist — skipping source files.')
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
