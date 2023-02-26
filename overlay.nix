{ self }:
final: prev:
with final;
with lib;
rec {
  expidus = prev.expidus.extend (f: p: {
    defaultPackage = f.launcher;

    launcher = (p.launcher.mkPackage {
      src = self;
      rev = self.shortRev or "dirty";
    }).overrideAttrs (f: p: {
      buildInputs = p.buildInputs ++ [ expidus.libexpidus ];
    });
  });
}
