{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-21.11.tar.gz") {} }:

pkgs.mkShell {
	LOCALE_ARCHIVE_2_27 = if (pkgs.glibcLocales != null) then "${pkgs.glibcLocales}/lib/locale/locale-archive" else "";

	buildInputs = [
		pkgs.glibcLocales
		pkgs.git
		pkgs.gnumake
		pkgs.ncurses
		pkgs.gcc
		pkgs.openssl
	];
	shellHook = ''
		export LC_ALL=en_US.UTF-8
		export GIT_SSL_CAINFO=/etc/ssl/certs/ca-certificates.crt
		export SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt
	'';
}
