cc := "gcc"
target := "sdlc.out"
src := "src/main.c"

pkgs := "sdl3 libplacebo"

default: build

build:
    {{cc}} -std=c23 {{src}} -o {{target}} \
        $(pkg-config --cflags {{pkgs}}) \
        $(pkg-config --libs {{pkgs}}) \
        -lvolk
check:
    pkg-config --exists {{pkgs}} && echo "sdl3/libplacebo found via pkg-config" || (echo "missing pkg-config dep — are you in 'nix develop'?" && exit 1)
    @echo 'int main(){return 0;}' | {{cc}} -xc - -lvolk -o /tmp/volk-check 2>&1 && echo "volk links OK" || echo "volk NOT found — check buildInputs"

clean:
    rm -f {{target}}

rebuild: clean build

run: build
    ./{{target}}
