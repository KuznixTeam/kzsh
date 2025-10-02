project('ksh', 'c', 'cpp', version : '0.1')
ksh_sources = files(
	'src/main.c',
	'src/builtins.c',
	'src/builtins.cpp',
	'src/builtins_cpp.cpp',
	'src/shell.c',
	'src/exec.c',
	'src/utils.c',
	'src/history.c',
	'src/env.c',
	'src/alias.c'
)
ksh_version = '0.1'
# Use local date at configure time for build date
ksh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
# Compose a reasonable target string from host_machine
# host_machine.system() returns names like 'linux', cpu_family is 'x86_64' etc.
ksh_target = host_machine.cpu_family() + '-' + host_machine.system()
ksh_exe = executable('ksh', ksh_sources,
	install : true,
	include_directories : include_directories('include'),
	cpp_args : [
		'-DKSH_VERSION="' + ksh_version + '"',
		'-DKSH_BUILD_DATE="' + ksh_build_date + '"',
		'-DKSH_TARGET="' + ksh_target + '"'
	]
)
