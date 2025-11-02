project('kzsh', ['c', 'cpp'], version: '0.2.0.alpha')

kzsh_release = meson.project_version()
kzsh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
kzsh_current_year = run_command('date', '+%Y').stdout().strip()

# uname -m (machine)
kzsh_uname_m = run_command('uname', '-m', check: false).stdout().strip().to_lower()
if kzsh_uname_m == ''
  kzsh_uname_m = 'unknown'
endif

# uname -i (hardware platform) - may be unsupported on some systems; fallback to uname -p
kzsh_uname_i = run_command('uname', '-i', check: false).stdout().strip().to_lower()
if kzsh_uname_i == ''
  kzsh_uname_i = run_command('uname', '-p', check: false).stdout().strip().to_lower()
  if kzsh_uname_i == ''
    kzsh_uname_i = 'unknown'
  endif
endif

# uname -s (kernel/OS name) lowercased
kzsh_uname_s = run_command('uname', '-s', check: false).stdout().strip().to_lower()
if kzsh_uname_s == ''
  kzsh_uname_s = 'unknown'
endif

# Detect libc via ldd --version output (best-effort)
ldd_out = run_command('ldd', '--version', check: false).stdout().strip().to_lower()
if ldd_out != ''
  if ldd_out.contains('musl')
    kzsh_libc = 'musl'
  elif ldd_out.contains('gnu') or ldd_out.contains('glibc')
    kzsh_libc = 'glibc'
  else
    kzsh_libc = 'unknown'
  endif
else
  # Try getconf for GNU_LIBC_VERSION
  getconf_out = run_command('getconf', 'GNU_LIBC_VERSION', check: false).stdout().strip().to_lower()
  if getconf_out.contains('glibc')
    kzsh_libc = 'glibc'
  else
    kzsh_libc = 'none'
  endif
endif

# Construct target string: (uname -m)-(uname -i)-(uname -s lowercased)-libc
kzsh_target = kzsh_uname_m + '-' + kzsh_uname_i + '-' + kzsh_uname_s + '-' + kzsh_libc

# Collect source files using shell (portable, no modules)
kzsh_sources = []
if run_command('test', '-d', 'src').returncode() == 0
  c_files = run_command('sh', '-c', 'echo src/*.c', check: false).stdout().strip().split()
  cpp_files = run_command('sh', '-c', 'echo src/*.cpp', check: false).stdout().strip().split()
  foreach f : c_files + cpp_files
    if f != 'src/*.c' and f != 'src/*.cpp'
      kzsh_sources += files(f)
    endif
  endforeach
else
  warning('Source directory "src" does not exist — skipping source files.')
endif

# Build executable
executable('kzsh',
  kzsh_sources,
  include_directories: include_directories('include'),
  install: true,
  cpp_args: [
    '-DKSH_RELEASE=\"' + kzsh_release + '\"',
    '-DKSH_BUILD_DATE=\"' + kzsh_build_date + '\"',
    '-DKSH_TARGET=\"' + kzsh_target + '\"',
    '-DKSH_CURRENT_YEAR=\"' + kzsh_current_year + '\"'
  ]
)

message('Building kzsh ' + kzsh_release + ' for ' + kzsh_target + ' (' + kzsh_build_date + ')')
message('Release: ' + kzsh_release + ' (current year: ' + kzsh_current_year + ')')
