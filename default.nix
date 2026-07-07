{ lib, stdenv, pkg-config, gtk4, gtk4-layer-shell }:

stdenv.mkDerivation {
  pname = "hitori-launcher";
  version = "0.0.1";
  src = ./.;

  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ gtk4 gtk4-layer-shell ];

  buildPhase = ''
    make CC=${stdenv.cc.targetPrefix}cc
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp hitori-launcher $out/bin/
  '';

  meta = with lib; {
    description = "A minimal Wayland launcher";
    longDescription = ''
      A small launcher that renders as a Wayland overlay via GTK4 Layer Shell
    '';
    homepage = "https://github.com/jihoo12/hitori-launcher";
    license = licenses.asl20;
    platforms = platforms.linux;
    maintainers = [ ];
  };
}
