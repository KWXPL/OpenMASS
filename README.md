# OpenMASS

OpenMASS – Open Maritime Autonomous Surface System.

## Środowisko C++ w Dev Container

Kontener zawiera Clang/Clang++, clangd, clang-format, clang-tidy, CMake, Ninja,
GDB oraz LLDB. Clang jest kompilatorem, a clangd zapewnia podpowiedzi,
nawigację po kodzie i diagnostykę w VS Code.

### Uruchomienie z WSL

1. Zainstaluj Docker Desktop z włączoną integracją dla używanej dystrybucji WSL.
2. Zainstaluj w VS Code rozszerzenia `WSL` oraz `Dev Containers`.
3. Dla najlepszej wydajności sklonuj repozytorium do systemu plików WSL,
   na przykład do `~/src/OpenMASS`, zamiast do `/mnt/c/...`.
4. W terminalu WSL przejdź do repozytorium i uruchom `code .`.
5. W VS Code wybierz polecenie **Dev Containers: Reopen in Container**.

Przy pierwszym uruchomieniu obraz zostanie zbudowany, a wymagane rozszerzenia
VS Code zostaną zainstalowane automatycznie.

### Budowanie projektu CMake

Konfiguracja oczekuje pliku `CMakeLists.txt` w katalogu głównym. VS Code
konfiguruje projekt automatycznie do katalogu `build` i generuje tam
`compile_commands.json`, z którego korzysta clangd.

Ręczne polecenia wewnątrz kontenera:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
./build/openmass
```
