Name:		sudoku-solver
Version:	1.0.1
Release:	1
Summary:	Sudoku solver
Group:		Games/Other
License:	GPLv3+
URL:		http://dansoft.krasnokamensk.ru/more.html?id=1032
Source0:	%{name}-%{version}.tar.gz

BuildRequires:	qt5-linguist-tools
BuildRequires:	pkgconfig(Qt5Core)
BuildRequires:	pkgconfig(Qt5Gui)
BuildRequires:	pkgconfig(Qt5Widgets)


%description
Solver sudoku puzzles.

%prep
%setup -q

%build
%qmake_qt5
%make

%install
%make_install INSTALL_ROOT=%{buildroot}

%files
%doc README.md
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_iconsdir}/hicolor/*/apps/%{name}.png