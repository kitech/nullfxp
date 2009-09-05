#  
# spec file for package nullfxp (Version 1.7.2)  
#  
# Copyright (c) 2008 SuSE Linux AG, Nuernberg, Germany.  
# This file and all modifications and additions to the pristine  
# package are under the same license as the package itself.  
#  
# Please submit bug fixes or comments via http://www.suse.de/feedback/  
#  
# $Id$
  
# norootforbuild  
  
%define _prefix /usr  
%define rname nullfxp
%define rversion 1.7.2 
%define rrelease 1.7.2
%define releasesuffix suse%(echo "%{suse_version}" | %__sed -e 's/.$//')DSB  
  
Name:       nullfxp
Version:    %{rversion}  
Release:    %{rrelease}%{releasesuffix}  

# %if %suse_version < 1020  
# Distribution:   SUSE Linux %(echo "%{suse_version}" | %__sed -e 's/.$//' -e ':a;s/\(.$\)/\.\1/g')  
# %else  
# Distribution:   openSUSE %(echo "%{suse_version}" | %__sed -e 's/.$//' -e ':a;s/\(.$\)/\.\1/g')  
# %endif  

Summary:    Application For Easy Folder Synchronisation  
Source0:    %{rname}-%{version}-src.tar.bz2
URL:        http://www.qtchina.net  
Packager:   liugunagzhao - http://www.qtchina.net
Group:      Productivity/Other  
License:    GPL v2 or later  
BuildRoot:  %{_tmppath}/build-%{rname}-%{version}  
BuildRequires:  libqt4-devel  
  
  
%description  
NullFXP is a cross-platform sftp/ftp/secure ftp client with GUI.
It support multi host connection and mlti transformition each other at meanwhile.
  
Authors:  
-------- 
     drswinghead <liugunagzhao@users.sf.net>
  
  
%if !0%{?opensuse_bs}  
%debug_package  
%endif  
%prep  
%setup -q -n "%{rname}-%{version}-src"  
%__chmod -R -x+X *  
%__chmod -R o-w *  
  
%build  
qmake -recursive
  
%__make %{?jobs:-j%{jobs}}  
  
%install  
  
%__install -D -m 0755 "%{_builddir}/%{rname}-%{version}-src/bin/%{name}" "%{buildroot}%{_bindir}/%{name}"  

%__install -D -m 0755 "%{_builddir}/%{rname}-%{version}-src/bin/unitest" "%{buildroot}%{_bindir}/unitest"  

%__install -D -m 0644 "%{_builddir}/%{rname}-%{version}-src/src/icons/computer.png"  "%{buildroot}%{_datadir}/icons/computer.png"  
%__install -D -m 0644 "%{_builddir}/%{rname}-%{version}-src/src/icons/document-encrypt.png"  "%{buildroot}%{_datadir}/icons/document-encrypt.png"  
%__install -D -m 0644 "%{_builddir}/%{rname}-%{version}-src/src/icons/nullget-1.png"  "%{buildroot}%{_datadir}/icons/nullget-1.png"  
%__install -D -m 0644 "%{_builddir}/%{rname}-%{version}-src/src/icons/nullget-2.png"  "%{buildroot}%{_datadir}/icons/nullget-2.png"  

%__install -d -m 755 "%{buildroot}%{_datadir}/applications"  

%clean  
%__rm -rf "%{buildroot}"  
  
%files  
%defattr(-,root,root)  
# %doc gpl.txt readme.txt  
%{_bindir}/%{name}  
%{_bindir}/unitest
%{_datadir}/icons/computer.png
%{_datadir}/icons/document-encrypt.png
%{_datadir}/icons/nullget-1.png
%{_datadir}/icons/nullget-2.png
# %{_datadir}/icons
# %{_datadir}/applications

%changelog  
