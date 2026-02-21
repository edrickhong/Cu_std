{
  description = "Cu_std dev env";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        clang
        cmake
        pkg-config
        bash

        dbus
	libffi
        wayland
        wayland-protocols
	wayland-scanner

        vulkan-loader
        vulkan-headers

        alsa-lib

        libX11
	libxkbcommon
      ];
    };
  };
}
