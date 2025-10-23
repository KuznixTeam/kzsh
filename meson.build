project('kzsh', 'c', 'cpp', version : '0.1.2')
ksh_sources = files(
	'src/*'
)
ksh_version = '0.1.2'
# Use local date at configure time for build date
ksh_build_date = run_command('date', '+%Y-%m-%d').stdout().strip()
# Compose a reasonable target string from host_machine
# host_machine.system() returns names like 'linux', cpu_family is 'x86_64' etc.
ksh_target = host_machine.cpu_family() + '-' + host_machine.system()
ksh_exe = executable('kzsh', ksh_sources,
	install : true,
	include_directories : include_directories('include'),
	cpp_args : [
		'-DKSH_VERSION="' + ksh_version + '"',
		'-DKSH_BUILD_DATE="' + ksh_build_date + '"',
		'-DKSH_TARGET="' + ksh_target + '"'
	]
)
