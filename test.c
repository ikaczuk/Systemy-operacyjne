#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>

#define MAX_BUFFER 2048
#define FIFO_PATH "test1_fifo"

int main() {
    char buffer[MAX_BUFFER];
    memset(buffer, 'A', MAX_BUFFER); // Wypełnienie bufora przykładowymi danymi
    size_t counter = 0; // Licznik zapisanych danych
    ssize_t written;

    // Utworzenie nazwanego potoku (FIFO)
    if (mkfifo(FIFO_PATH, 0777) < 0) {
        perror("Nie można utworzyć FIFO");
        return 1;
    }

    // Otwórz FIFO do zapisu w trybie nieblokującym
    int fifo_fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("Nie można otworzyć FIFO do zapisu");
        unlink(FIFO_PATH);
        return 2;
    }

    // Wypełnianie FIFO danymi
    bool flag = true;
    while (flag) {
        written = write(fifo_fd, buffer, MAX_BUFFER);
        if (written > 0) {
            counter += written;
        } else {
            flag = false;
        }
    }

    printf("Max pojemność FIFO: %zu bajtów\n", counter);

    // Otwórz FIFO do odczytu
    int fifo_read_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_read_fd < 0) {
        perror("Nie można otworzyć FIFO do odczytu");
        close(fifo_fd);
        unlink(FIFO_PATH);
        return 3;
    }

    // Odczyt danych, aby zwolnić miejsce
    size_t free_space = MAX_BUFFER * 2; // Ilość danych do zwolnienia
    size_t total_read = 0;
    ssize_t read_bytes;

    bool read_flag = true;
    while (total_read < free_space && read_flag) {
        read_bytes = read(fifo_read_fd, buffer, MAX_BUFFER);
        if (read_bytes > 0) {
            total_read += read_bytes;
        } else {
            read_flag = false;
        }
    }

    printf("Zwolniono: %zu bajtów\n", total_read);

    // Ponowna próba zapisu
    written = write(fifo_fd, buffer, MAX_BUFFER);
    if (written > 0) {
        printf("Po zwolnieniu miejsca zapisano: %zd bajtów\n", written);
    } else {
        printf("Nic nie zapisano po zwolnieniu miejsca\n");
    }

    // Zamknięcie deskryptorów i usunięcie FIFO
    close(fifo_fd);
    close(fifo_read_fd);
    unlink(FIFO_PATH);

    return 0;
}
