{ self }:
{ pkgs }:
with pkgs;
with lib;
let
  overlay = (import ./overlay.nix { inherit self; } pkgs pkgs);
  pkg = overlay.expidus.launcher;
in
mkShell rec {
  pname = "expidus-launcher";
  version = self.shortRev or "dirty";
  name = "${pname}-${version}";

  packages = with pkgs; [
    emscripten
    gdb valgrind lcov
  ] ++ pkg.buildInputs ++ pkg.nativeBuildInputs;

  inherit (pkg) mesonFlags;
}
