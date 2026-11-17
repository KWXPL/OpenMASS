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

### Logowanie

Przy pierwszej konfiguracji CMake pobiera bibliotekę spdlog z GitHub. Program
`gps_console` zapisuje komunikaty na stderr oraz dopisuje je do pliku
`MASSlogs/openmass_gps_status.log` obok pliku wykonywalnego, niezależnie od
katalogu uruchomienia. Do `MASSlogs/openmass_gps_data.log` dopisywane są pełne,
surowe linie `+CGNSINF` z danymi GPS, po jednej na odczyt, wraz z czasem
zapisu. Dane są rozdzielone przecinkami w kolejności zwracanej przez moduł;
puste pola oznaczają dane niepodane przez moduł. Pliki powstają automatycznie;
proces musi mieć uprawnienia do zapisu w katalogu programu. Wyznaczanie ścieżki
wymaga Linuksa i `/proc`. Pliki logów nie są obecnie rotowane. Istniejący
plik `openmass.log` nie jest przenoszony ani usuwany.

Przy uruchomieniu w terminalu program nie wypisuje binarnych danych GPS na
ekran. Aby otrzymać strumień protobuf, przekieruj stdout do pliku lub potoku,
np. `./gps_console > pozycje.pb`. Logi pozostają widoczne na stderr.

W dowolnym pliku źródłowym podłączonym do spdlog można użyć np.:

```cpp
#include <spdlog/spdlog.h>

spdlog::info("GPS device: {}", device);
spdlog::warn("No GPS response");
```
