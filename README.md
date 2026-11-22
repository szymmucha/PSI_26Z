# PSI 26Z

Repozytorium jest podzielone na podfoldery, jeden na każde laboratorium.

## Laboratorium 1

Laboratorium 1 zawiera wewnątrz siebie dwa folder: `server` i `client`, oraz plik `docker-compose.yml`. Każdy z podfolderów posiada `Dockerfile`, `Makefile` oraz pliki źródłowe.

### Sprawozdanie

Sprawozdanie do laboratorium 1 znajduje się w katalogu [lab1/docs](lab1/docs/). Jest ona zapisana w dwóch formatach [md](lab1/docs/sprawozdanie.md) i [pdf](lab1/docs/sprawozdanie.pdf). Plik pdf jest generowany z pliku md, więc zawierają one identyczną treść.

## Instrukcje

Aby uruchomić program wystarczy wejść do folderu odpowiedniego laboratorium i wpisać:

```bash
docker compose up --build
```

Przykładowo:

```bash
cd lab1 && docker compose up --build
```