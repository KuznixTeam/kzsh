project('kzsh', ['c', 'cpp'], version: '0.2.0.alpha')

# -------------------------
# Project info
# -------------------------
kzsh_release = meson.project_version()
kzsh_build_date = run_command('date', '+%Y-%m-%d', check: false).stdout().strip()
kzsh_current_year = run_command('date', '+%Y', check: false).stdout().strip()

# -------------------------
# Detect target info
# -------------------------
# For cross builds, host_machine will be defined by the cross file
if host_machine
  kzsh_uname_m = host_machine.cpu_family()
  kzsh_uname_i = host_machine.cpu()
  kzsh_uname_s = host_machine.system()
  kzsh_libc = 'glibc'  # assume glibc for cross builds; can override in cross file
else
  # native build
  kzsh_uname_m = run_command('uname', '-m', check: false).stdout().strip().to_lower()
  if kzsh_uname_m == ''
    kzsh_uname_m = 'unknown'
  endif

  kzsh_uname_i = run_command('uname', '-i', check: false).stdout().strip().to_lower()
  if kzsh_uname_i == ''
    kzsh_uname_i = run_command('uname', '-p', check: false).stdout().strip().to_lower()
    if kzsh_uname_i == ''
      kzsh_uname_i = 'unknown'
    endif
  endif

  kzsh_uname_s = run_command('uname', '-s', check: false).stdout().strip().to_lower()
  if kzsh_uname_s == ''
    kzsh_uname_s = 'unknown'
  endif

  # detect libc
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
    getconf_out = run_command('getconf', 'GNU_LIBC_VERSION', check: false).stdout().strip().to_lower()
    if getconf_out.contains('glibc')
      kzsh_libc = 'glibc'
    else
      kzsh_libc = 'none'
    endif
  endif
endif

# -------------------------
# Map libc to target string
# -------------------------
if kzsh_libc == 'glibc'
  kzsh_target_libc = 'gnu'
elif kzsh_libc == 'musl'
  kzsh_target_libc = 'musl'
else
  kzsh_target_libc = 'unknown'
endif

kzsh_target = kzsh_uname_m + '-' + kzsh_uname_i + '-' + kzsh_uname_s + '-' + kzsh_target_libc

# -------------------------
# Collect source files
# -------------------------
kzsh_sources = []
if meson.is_directory('src')
  kzsh_sources = files('src/*.c', 'src/*.cpp')
else
  warning('Source directory "src" does not exist — skipping source files.')
endif

# -------------------------
# Build executable
# -------------------------
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

# -------------------------
# Build messages
# -------------------------
message('Building kzsh ' + kzsh_release + ' for ' + kzsh_target + ' (' + kzsh_build_date + ')')
message('Release: ' + kzsh_release + ' (current year: ' + kzsh_current_year + ')')
