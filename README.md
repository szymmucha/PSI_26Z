# PSI 26Z

Repozytorium jest podzielone na podfoldery, jeden na każde laboratorium.

## Laboratorium 1

Laboratorium 1 zawiera wewnątrz siebie dwa folder: `server` i `client`, oraz plik `docker-compose.yml`. Każdy z podfolderów posiada `Dockerfile`, `Makefile` oraz pliki źródłowe.

# Instrukcje

Aby uruchomić program wystarczy wejść do folderu odpowiedniego laboratorium i wpisać:

```bash
docker compose up --build
```

Przykładowo:

```bash
cd lab1 && docker compose up --build
```