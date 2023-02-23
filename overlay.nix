{ self }:
final: prev:
with final;
with lib;
rec {
  expidus = prev.expidus.extend (f: p: {
    defaultPackage = f.launcher;

    launcher = clang14Stdenv.mkDerivation {
      pname = "expidus-launcher";
      version = self.shortRev or "dirty";

      src = cleanSource self;

      nativeBuildInputs = [ meson ninja pkg-config expidus.sdk ];
      buildInputs = [ plymouth ];
    };
  });
}
