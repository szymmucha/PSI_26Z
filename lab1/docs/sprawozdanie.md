# Sprawozdanie PSI LAB 1 

## Skład zespołu
- **Szymon Mucha**
- Norbert Drabiński
- Wiktoria Parzych

## Wersja i data ostatniej edycji
- Wersja: 1.0.0
- Data ostatniej edycji: 22.11.2026

## Treść zadania
Klient TCP wysyła złożoną strukturę danych. Przykładowo: tworzymy w pamięci listę jednokierunkową lub drzewo binarne struktur zawierających (oprócz danych organizacyjnych) pewne dane dodatkowe: np. liczbę całkowitą 16-o bitową, liczbę całkowitą 32-u bitową oraz napis zmiennej i ograniczonej długości. Serwer powinien te dane odebrać, dokonać poprawnego „odpakowania” tej struktury, „odtworzyć” w pamięci odebraną strukturę i wydrukować jej pola (być może w skróconej postaci, aby uniknąć nadmiaru wyświetlanych danych).

## Opis rozwiązania

Poniżej zostało opisane w jaki sposób jest wysyłana poprzez sieć lista jednokierunkowa. Nie ma poniżej opisanego sposobu nawiązania połączenia z serwerem

### Struktura wysyłanej wiadomości:
- Pierwsze 4 bajty jest to liczba elementów wysyłanej listy(uint32). Serwer odrzuci wiadomość, jeżeli liczba elementów przekracza 1000
- Następnie wysyłane są elementy listy na element listy składa się w kolejności:
    - 4 bajty: liczba int32 - nazwane int32_data
    - 2 bajty: liczba uint16 - nazwane uint16_data 
    - 2 bajty: liczba uint16 oznaczająca długość łańcucha znaków. Uwaga jest to długość bez znaku NULL terminator. Jeżeli długość przekraczy dozwolony rozmiar 1024 bajty to cała wiadomość zostanie odrzucona - nazwane str_len
    - łańcuch znaków o długości równej str_len - nazwane str_data
- Struktura w C przechowywująca podane dane:
    ```C
    typedef struct {
        int int32_data;
        uint16_t uint16_data;
        uint16_t str_len;
        char *str_data;
    } NodeData;
    ```

### Wysłanie listy w wiadomości przez klienta
1. Zliczenie liczby elementów listy i rozmiaru potrzebnego buforu do zapisania całej listy w pamięci
    ```C
    int count = 0;
    int buffer_size = sizeof(int);
    for (Node *n = head; n; n = n->next, count++)
    {
        buffer_size += 2 + 4 + 2 + n->data.str_len;
    }
    ```
2. Utworzenie bufora i skopiowania wskaźnika na początek bufora
   ```C
    uint8_t *buf = malloc(buffer_size);
    uint8_t *buf_start = buf;
    ```
3. Zapisanie liczby elementów listy do bufora
   ```C
    const tmp_count = htonl(count);
    memcpy(buf, &tmp_count, 4);
    buf += 4;
    ```
4. Zapisanie elementu listy. Podczas zapisywania łańucucha znaków, przed nim jest zapisywana informacja o jego długości, aby serwer mógł bez problemu odczytać go.
    ```C
    for (Node *n = head; n; n = n->next)
    {
        fflush(stdout);
        const tmp_int32 = htonl(n->data.int32_data);
        const tmp_uint16 = htons(n->data.uint16_data);
        const tmp_str_len = htons(n->data.str_len);
        memcpy(buf, &tmp_int32, 4);
        buf += 4;
        memcpy(buf, &tmp_uint16, 2);
        buf += 2;
        memcpy(buf, &tmp_str_len, 2);
        buf += 2;
        memcpy(buf, n->data.str_data, n->data.str_len);
        buf += n->data.str_len;
    }
5. Wysłanie bufora do serwera i zwolnienie pamięci
   ```C
        int res = send_all(serverfd, buffer_size, buf_start);
        free(buf_start);
   ```
   ```C
    int send_all(int serverfd, size_t buffer_size, const void* buffer) {
        size_t current_size = 0;

        while (current_size < buffer_size) {
            ssize_t size = send(serverfd, (const char*)buffer + current_size, buffer_size - current_size, 0);
            if (size == -1) {
                if (errno == EINTR) { // Interrupt - try again
                    continue;
                }

                perror("Error sending data");
                return -1;
            }

            current_size += (size_t)size;
        }

        return 0;
    }
    ```

### Odbiór wiadomości przez serwer
1. Funkcja wczytująca dane
    ```C
    int recv_exact(int clientfd, size_t expected_size, void* out_buffer) {
        size_t current_size = 0;

        while (current_size < expected_size) {
            ssize_t size = recv(clientfd, (char*)out_buffer + current_size, expected_size - current_size, 0);
            if (size < 0) {
                if (errno == EINTR) { // Interrupt - try again
                    continue;
                }

                perror("Error receiving data");
                return -1;
            }
            else if (size == 0) {
                return -1;
            }

            current_size += (size_t)size;
        }

        return 0;
    }
    ```
2. Odczytanie liczby elementów listy
    ```C
    int res_reading_count = recv_exact(clientfd, 4, buf);
    if (res_reading_count == -1) {
        fprintf(stderr, "Error receiving count from client\n");
        fflush(stderr);
        return NULL;
    }

    uint32_t count;
    memcpy(&count, buf, 4);
    count = ntohl(count);
    printf("Receiving %d records from client\n", count);
    if (count > 1000) {
        fprintf(stderr, "Error: count %d exceeds maximum allowed\n", count);
        fflush(stderr);
        return NULL;
    }
    ```
3. Odczytywanie elementu listy w pętli 
   ```C
    for (int i = 0; i < count; i++)
    {
        Node *node = read_node(clientfd, buf);
        if (node == NULL) {
            free_list(head);
            return NULL;
        }

        if (head == NULL)
        {
            head = node;
        }
        else
        {
            tail->next = node;
        }
        tail = node;
    }
   ```
4. Odczytywanie pojedynczego elementu
    1. Na początku wczytanie pierwszych 8 batjów, ponieważ wiadomo ile można wczytać bez otrzymywania dodatkowych danych.
        ```C
        int res_reading = recv_exact(clientfd, 8, buf);
        if (res_reading == -1) {
            return NULL;
        }

        int32_t int32_data;
        uint16_t uint16_data;
        uint16_t str_len;
        memcpy(&int32_data, buf, 4);
        memcpy(&uint16_data, buf + 4, 2);
        memcpy(&str_len, buf + 6, 2);
        int32_data = ntohl(int32_data);
        uint16_data = ntohs(uint16_data);
        str_len = ntohs(str_len);
        ```
    2. Sprawdzenie dozwolonego zakresu str_len
       ```C
        if (str_len >= BUFFER_SIZE) {
            fprintf(stderr, "Error: string length %u exceeds buffer size\n", str_len);
            fflush(stderr);
            return NULL;
        }
        ```
        Z uwagą, że na początku pliku zostało zdefiniowane BUFFER_SIZE
        ```C
        #define BUFFER_SIZE 1024
        ```
    3. Odczytanie łańcucha znaków i dodanie na koniec NULL terminatora
        ```C
        res_reading = recv_exact(clientfd, str_len, buf);
        if (res_reading == -1) {
            fprintf(stderr, "Error receiving string data from client\n");
            fflush(stderr);
            return NULL;
        }

        char *str_data = malloc(str_len + 1);
        memcpy(str_data, buf, str_len);
        str_data[str_len] = '\0';
        ```
    4. Utworzenie elementu listy
        ```C
        Node *node = malloc(sizeof(Node));
        node->data.int32_data = int32_data;
        node->data.uint16_data = uint16_data;
        node->data.str_len = str_len;
        node->data.str_data = str_data;
        node->next = NULL;
        return node;
        ```


## Napotkane problemy
Nie natrafiliśmy na żadne problemy podczas wykonywania zadania

## Opis konfiguracji testowej
- Serwer nasłuchuje na 172.21.31.2:8080
- Interfejs sieciowy dla dockera: z31_network

## Opis testowania
Test został przeprowadzany poprzez uruchomienie serwera i klienta poprzez polecenie `docker compose up --build`.

- Logi klienta
    ```
    2026-11-22 21:11:22 Connected to server 172.21.31.2:8080
    2026-11-22 21:11:22 Preparing list to send to server
    2026-11-22 21:11:22 Creating element 0
    2026-11-22 21:11:22 Creating element 1
    2026-11-22 21:11:22 Creating element 2
    2026-11-22 21:11:22 Creating element 3
    2026-11-22 21:11:22 Creating element 4
    2026-11-22 21:11:22 Creating element 5
    2026-11-22 21:11:22 Creating element 6
    2026-11-22 21:11:22 Creating element 7
    2026-11-22 21:11:22 Creating element 8
    2026-11-22 21:11:22 Creating element 9
    2026-11-22 21:11:22 Preparing to send list to server
    2026-11-22 21:11:22 Sending record: int32=1804289383 uint16=9158 str_len=58 str="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    2026-11-22 21:11:22 Sending record: int32=-1714636915 uint16=56401 str_len=56 str="bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    2026-11-22 21:11:22 Sending record: int32=719885386 uint16=22764 str_len=10 str="cccccccccc"
    2026-11-22 21:11:22 Sending record: int32=-1189641421 uint16=22714 str_len=8 str="dddddddd"
    2026-11-22 21:11:22 Sending record: int32=783368690 uint16=7931 str_len=24 str="eeeeeeeeeeeeeeeeeeeeeeee"
    2026-11-22 21:11:22 Sending record: int32=-1967513926 uint16=124 str_len=7 str="fffffff"
    2026-11-22 21:11:22 Sending record: int32=304089172 uint16=10232 str_len=12 str="gggggggggggg"
    2026-11-22 21:11:22 Sending record: int32=-521595368 uint16=52711 str_len=10 str="hhhhhhhhhh"
    2026-11-22 21:11:22 Sending record: int32=336465782 uint16=9562 str_len=3 str="iii"
    2026-11-22 21:11:22 Sending record: int32=-233665123 uint16=49715 str_len=56 str="jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"
    2026-11-22 21:11:22 Sending 10 records to server. Buffer size: 328 
    2026-11-22 21:11:22 00 00 00 0A 6B 8B 45 67 23 C6 00 3A 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 61 99 CC B7 8D DC 51 00 38 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 62 2A E8 94 4A 58 EC 00 0A 63 63 63 63 63 63 63 63 63 63 B9 17 83 33 58 BA 00 08 64 64 64 64 64 64 64 64 2E B1 41 F2 1E FB 00 18 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 8A BA 1E BA 00 7C 00 07 66 66 66 66 66 66 66 12 20 08 54 27 F8 00 0C 67 67 67 67 67 67 67 67 67 67 67 67 E0 E9 16 18 CD E7 00 0A 68 68 68 68 68 68 68 68 68 68 14 0E 0F 76 25 5A 00 03 69 69 69 F2 12 8D 9D C2 33 00 38 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 6A 
    2026-11-22 21:11:22 Sent list to server
    ```
- Logi serwera
    ```
    2026-11-22 21:11:22 Accepted connection from client
    2026-11-22 21:11:22 Receiving 10 records from client
    2026-11-22 21:11:22 Received record[0]: int32=1804289383 uint16=9158 str_len=58 str="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    2026-11-22 21:11:22 Received record[1]: int32=-1714636915 uint16=56401 str_len=56 str="bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    2026-11-22 21:11:22 Received record[2]: int32=719885386 uint16=22764 str_len=10 str="cccccccccc"
    2026-11-22 21:11:22 Received record[3]: int32=-1189641421 uint16=22714 str_len=8 str="dddddddd"
    2026-11-22 21:11:22 Received record[4]: int32=783368690 uint16=7931 str_len=24 str="eeeeeeeeeeeeeeeeeeeeeeee"
    2026-11-22 21:11:22 Received record[5]: int32=-1967513926 uint16=124 str_len=7 str="fffffff"
    2026-11-22 21:11:22 Received record[6]: int32=304089172 uint16=10232 str_len=12 str="gggggggggggg"
    2026-11-22 21:11:22 Received record[7]: int32=-521595368 uint16=52711 str_len=10 str="hhhhhhhhhh"
    2026-11-22 21:11:22 Received record[8]: int32=336465782 uint16=9562 str_len=3 str="iii"
    2026-11-22 21:11:22 Received record[9]: int32=-233665123 uint16=49715 str_len=56 str="jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"
    ```

### Opis testów
Po logach widać, że serwer odczytał te same dane co klient wysłał w odpowiedniej kolejności. Przy analizowaniu wysłanego bufora przez klienta widać, że wysyła on dane w `network order`.
<br> 
Dla potwierdzenie pierwsze 4 bajty, które są odpowiedzalne za przekazanie liczbę elementów listy wyglądają następująco: `00 00 00 0A`, więc widać że liczba elementów jest wysyłana w big-endianie. Dla sprawdzanie poprawności wysyłania elementów list, sprawdzenie pierwszego elementu listy, poprzez rozkodowanie bajtów w big-endianiu:
- int32 `6B8B4567`, co daje wartość 1804289383
- int16 `23C6`, co daje wartość 9158
- int16 `003A` oznaczający długość tekstu 58
</ul>
Widać, że dane są wysyłane w `network order` i są one prawidłowo odczytywane przez serwer.
<br>
Zostało też sprawdzone, że przy uruchomieniu klienta wielokrotnie bez restartu serwera, dane są dalej odczytywane poprawnie.
