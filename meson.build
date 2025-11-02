project('kzsh', ['c', 'cpp'], version: '0.2.0.alpha')

ksh_release = meson.project_version()
ksh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
ksh_current_year = run_command('date', '+%Y').stdout().strip()

# Architecture (cpu family) with fallback
ksh_arch = host_machine.cpu_family()
if ksh_arch == ''
  ksh_arch = run_command('uname', '-p', check: false).stdout().strip()
  if ksh_arch == ''
    ksh_arch = 'unknown'
  endif
endif

# Machine (uname -m fallback)
ksh_machine = run_command('uname', '-m', check: false).stdout().strip()
if ksh_machine == '' or ksh_machine == 'unknown'
  ksh_machine = 'unknown'
endif

# OS
ksh_os = host_machine.system()
if ksh_os == '' or ksh_os == 'unknown'
  ksh_os = 'unknown'
endif

# Detect libc via ldd --version output (best-effort)
ldd_out = run_command('ldd', '--version', check: false).stdout().strip().to_lower()
if ldd_out != ''
  if ldd_out.contains('musl')
    ksh_libc = 'musl'
  elif ldd_out.contains('gnu') or ldd_out.contains('glibc')
    ksh_libc = 'glibc'
  else
    ksh_libc = 'unknown'
  endif
else
  # On some systems ldd may not be present; try getconf (best-effort)
  getconf_out = run_command('getconf', 'GNU_LIBC_VERSION', check: false).stdout().strip().to_lower()
  if getconf_out.contains('glibc')
    ksh_libc = 'glibc'
  else
    # If nothing found, mark as none (explicit) to match requested forms
    ksh_libc = 'none'
  endif
endif

# Compiler id (gcc/clang/msvc/etc.) - fallback to unknown-compiler
ksh_cc = meson.get_compiler('c').get_id()
if ksh_cc == '' or ksh_cc == 'unknown'
  ksh_cc = 'unknown-compiler'
endif
ksh_cc = ksh_cc.to_lower()

# Construct target string: arch-machine-os-libc-compiler
ksh_target = ksh_arch + '-' + ksh_machine + '-' + ksh_os + '-' + ksh_libc + '-' + ksh_cc

# Collect source files using shell (portable, no modules)
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

# Build executable
executable('kzsh',
  ksh_sources,
  include_directories: include_directories('include'),
  install: true,
  cpp_args: [
    '-DKSH_RELEASE=\"' + ksh_release + '\"',
    '-DKSH_BUILD_DATE=\"' + ksh_build_date + '\"',
    '-DKSH_TARGET=\"' + ksh_target + '\"',
    '-DKSH_CURRENT_YEAR=\"' + ksh_current_year + '\"'
  ]
)

message('Building kzsh ' + ksh_release + ' for ' + ksh_target + ' (' + ksh_build_date + ')')
message('Release: ' + ksh_release + ' (current year: ' + ksh_current_year + ')')
