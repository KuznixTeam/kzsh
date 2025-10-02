Name:           kzsh
Version:        0.1.1
Release:        1%{?dist}
Summary:        Kuznix Shell - Bash-like shell
License:        GPL-3.0-or-later
URL:            https://github.com/KuznixTeam/kzsh
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc, g++, make, autoconf, automake
Requires:       bash

%description
Kuznix Shell (kzsh) is a modern bash-like shell written in C/C++ with modular features, history, aliases, and more.

%prep
%setup -q

%build
./configure --prefix=%{_prefix}
make

%install
make install DESTDIR=%{buildroot}

%files
%{_bindir}/kzsh
%doc README.md

%changelog
* Wed Oct 02 2025 Krzysztof Demir Kuzniak <krzysztofdemirkuzniak@gmail.com> - 0.1.1-1
- Initial RPM release (kzsh)
