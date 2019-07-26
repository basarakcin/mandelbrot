#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bmp.h"

// Methodendeklaration der Assembly Implementierung des Algorithmus (mandelbrot.S)
extern void mandelbrot(float r_start, float r_end, float i_start, float i_end, float resolution, unsigned char *img, int16_t max_iterations);

// Methodendeklaration der Referenzimplementierung des zu entwickelnden Algorithmus
void mandelbrot_c(float r_start, float r_end, float i_start, float i_end, float resolution, unsigned char *img, int16_t max_iterations);

// Methodendeklaration der Methode zur Validierung der Eingaben, Ausführen des Algorithmus und
// Verifikation der Korrektheit des Ergebnisses
int calculate_mandelbrot(char *file_name, float r_start, float r_end, float i_start, float i_end, float resolution, int16_t max_iterations);

// Methodendeklaration der Methode zum automatisierten Testen von Eingaben
int test();

// Methodeneklaration der Methode zum Testen einzelner Inputs
_Bool test_input(int index, char *input, char *expected, float r_start, float r_end,
                 float i_start, float i_end, float resolution, int16_t max_iterations);

// Methodendeklaration der statischen Methode zur Rückgabe der aktuellen Zeit
static double curtime(void);

// Main Methode: Nimmt Startparameter entgegen und ruft die Berechnungs- oder Testmethode auf
int main(int argc, char *argv[])
{

        // Initialisierung der Standardwerte zur Berechnung
        char *file_name = "mandelbrot.bmp";
        float r_start = -2;
        float r_end = 1;
        float i_start = -1;
        float i_end = 1;
        float resolution = 0.001;
        int16_t max_iterations = 255;

        // Falls Startparameter vorhanden, werden Standardwerte überschrieben
        switch (argc)
        {
        // Falls mit Standardwerten gearbeitet werden soll
        case 1:
                break;

        // Falls die automatischen Tests ausgeführt werden sollen
        case 2:
                if (!strcmp(argv[1], "test"))
                {
                        printf("Tests starten...\r\n");
                        exit(test());
                }
                else if (!strcmp(argv[1], "-h") ||
                         !strcmp(argv[1], "--help") ||
                         !strcmp(argv[1], "--hilfe"))
                {
                        printf("Format:\n[dateiname], r_start, r_end, i_start, i_end, resolution, i_max\n");
                        exit(EXIT_SUCCESS);
                }
                break;

        // Wenn Parameter und der Standarddateiname überschrieben werden sollen
        case 8:
                file_name = argv[1];
                r_start = (float)atof(argv[2]);
                r_end = (float)atof(argv[3]);
                i_start = (float)atof(argv[4]);
                i_end = (float)atof(argv[5]);
                resolution = (float)atof(argv[6]);
                max_iterations = (float)atof(argv[7]);
                strcat(file_name, ".bmp");
                break;

        // Falls nur Parameter überschrieben werden sollen
        case 7:
                r_start = (float)atof(argv[1]);
                r_end = (float)atof(argv[2]);
                i_start = (float)atof(argv[3]);
                i_end = (float)atof(argv[4]);
                resolution = (float)atof(argv[5]);
                max_iterations = (float)atof(argv[6]);
                break;

        // Falls der Nutzer ein nicht vorgesehenes Format nutzt, wird vorgesehenes
        // Format zur Hilfe ausgegeben und das Programm wird mit einem Fehler beendet.
        default:
                fprintf(stderr, "Bitte halten sie sich an das Format '[dateiname] r_start r_end i_start i_end resolution i_max'\r\n");
                fflush(stderr);
                exit(EXIT_FAILURE);
        }

        // Eigentliche Berechnung wird mit den aktuellen Parametern durchgeführt
        // Ist eine Berechnung nicht möglich, wird das Programm mit einem Fehler
        // abgebrochen
        exit(calculate_mandelbrot(file_name, r_start, r_end, i_start, i_end, resolution, max_iterations));
}

// Eigentliche Methode zur Berechnung der Iterationszahlen der einzelnen komplexen
// Zahlen korrespondierend zu Pixeln. Gibt entweder Fehlercode bei illegalen Werten
// oder 0 bei Erfolg zurück
int calculate_mandelbrot(char *file_name, float r_start, float r_end, float i_start, float i_end, float resolution, int16_t max_iterations)
{
        // Die Resolution muss größer 0 sein, da sonst das Bild eine unendliche
        // oder negative Größe hätte. Gibt Fehlermeldung aus
        if (resolution <= 0)
        {
                fprintf(stderr, "   Die Resolution darf nicht negativ oder 0 sein.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Anzahl maximaler Iterationen darf nicht kleiner 0 sein. Eine ungewollt
        // negative Eingabe könnte bei unsigned Integern zu ungewollt hohen
        // Iterationsanzahlen führen. Gibt Fehlermeldung aus
        if (max_iterations < 0)
        {
                fprintf(stderr, "   Die Anzahl der maximalen Iterationen darf nicht kleiner 0 sein.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Liegt die Eingabe für r_start nicht innerhalb des Intervalls [-2;1]
        // würden zu viele sinnlose Berechnung durchgeführt werden
        // Gleiches wird für die übrigen Abmessungsparameter überprüft
        // Gibt Fehlermeldung aus.
        if (r_start < -2 || r_start > 1)
        {
                fprintf(stderr, "   Die Eingabe für r_start muss im Interval [-2;1] liegen.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        if (r_end < -2 || r_end > 1)
        {
                fprintf(stderr, "   Die Eingabe für r_end muss im Interval [-2;1] liegen.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        if (i_start < -1 || i_start > 1)
        {
                fprintf(stderr, "   Die Eingabe für i_start muss im Interval [-1;1] liegen.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        if (i_end < -1 || i_end > 1)
        {
                fprintf(stderr, "   Die Eingabe für i_end muss im Interval [-1;1] liegen.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Berechnung der Höhe und Breite des Bildes und Verkleinerung auf
        // Vielfaches von 4. Für Optimierung in Assembly Implementierung
        u_int32_t width = (r_end - r_start) / resolution;
        width = width - (width % 4);
        u_int32_t height = (i_end - i_start) / resolution;
        height = height - (height % 4);

        // Höhe und Breite des Bildes dürfen nicht negativ sein und keinen
        // Integer Overflow erzeugen, sobald sie zur Bildgröße zusammengerechnet
        // werden
        if (width <= 0 || height <= 0 || ((width * height) * 3) / height != width * 3)
        {
                fprintf(stderr, "   Mit den eingegebenen Parametern kann keine Berechnung durchgeführt werden.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Berechnung der Anzahl der benötigten Bytes der zu erzeugenden Bilddatei
        int32_t imageSize = width * height * 3;

        // Zur Sicherheit darf die erzeugte Datei nicht größer als 1.5 Gigabyte
        // und nicht kleiner als 0 Byte sein. Gibt Fehlermeldung aus
        if (imageSize > 1500000000 && imageSize < 0)
        {
                fprintf(stderr, "   Die zu erzeugende Datei ist zu groß. Bitte benutzen Sie kleinere Parameter für den Wertebereich.\r\n");
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Definition von File und Info Header für die zu erzeugende Bitmap Datei
        BITMAPFILEHEADER bmFH;
        BITMAPINFOHEADER bmIH;

        // Bitmap Dateien müssen 4 Byte aligned sein, falls diese das nicht sind,
        // wird ein Puffer an das Ende der Zeile geschrieben, der diese 4 Byte
        // auffüllt
        int32_t paddingSize = (4 - (sizeof(RGBTRIPLET) * width) % 4) % 4;

        // Berechnung der totalen Bildgröße aus der totalen Größe des Headers, der
        // Bildgröße und des benötigten Puffers
        int32_t fileSize = sizeof(bmFH) + sizeof(bmIH) + imageSize + (3 * width * paddingSize);

        // Schreiben der benötigten Daten in den File Header
        bmFH.bfType = 0x4d42;
        bmFH.bfSize = fileSize;
        bmFH.bfReserved1 = 0;
        bmFH.bfReserved2 = 0;
        bmFH.bfOffBits = sizeof(bmFH) + sizeof(bmIH);

        // Schreiben der benötigten Daten in den Info Header
        bmIH.biSize = sizeof(bmIH);
        bmIH.biWidth = width;
        bmIH.biHeight = height;
        bmIH.biPlanes = 0;
        bmIH.biBitCount = 24;
        bmIH.biCompression = 0;
        bmIH.biSizeImage = imageSize;
        bmIH.biXPelsPerMeter = 0;
        bmIH.biYPelsPerMeter = 0;
        bmIH.biClrUsed = 0;
        bmIH.biClrImportant = 0;

        // Pointer auf Anfang einer Datei wird erstellt, die bei nicht-
        // Existenz neu erstellt oder bei Existenz geleert wird.
        FILE *fp = fopen(file_name, "wb");

        // Konnte die Datei nicht geöffnet werden,
        // bricht das Programm ab.
        if (fp == NULL)
        {
                fprintf(stderr, "   Die Datei %s.bmp konnte nicht erstellt werden.\r\n", file_name);
                fflush(stderr);
                return EXIT_FAILURE;
        }

        // Speicher für die Pixel Bytes reservieren
        uint8_t *buffer = (uint8_t *)malloc(imageSize);

        // Wenn der der Speicher für den Buffer nicht allokiert
        // werden konnte, bricht das Programm ab
        if (buffer == NULL)
        {
                fprintf(stderr, "   Es konnte nicht genug Speicherplatz allokiert werden.\r\n");
                return EXIT_FAILURE;
        }

        // Messung der zur Ausführung benötigten Zeit und tatsächliche Ausführung der
        // Berechnung durch die Assembly Implementierung
        double time = curtime();
        mandelbrot(r_start, r_end, i_start, i_end, resolution, buffer, max_iterations);
        time = curtime() - time;
        printf("   Die Berechnung hat %f Sekunden gedauert.\r\n", time);
        fflush(stdout);

        // Schreibt die Header Daten in den Buffer der Bilddatei
        fwrite(&bmFH, sizeof(bmFH), 1, fp);
        fwrite(&bmIH, sizeof(bmIH), 1, fp);

        // Schreibt die generierten Pixeldaten in den Buffer der Bilddatei
        fwrite((uint8_t *)buffer, 3, imageSize, fp);

        // Wenn die Bilddatei nicht geschlossen werden konnte, wird eine
        // Fehlermeldung ausgegeben
        if (fclose(fp))
        {
                fprintf(stderr, "   Die Datei %s.bmp konnte nicht geschlossen werden.\r\n", file_name);
                fflush(stderr);
                free(buffer);
                return EXIT_FAILURE;
        }

        // Wenn noch genug Speicher reserviert werden kann, wird die Ähnlichkeit zu
        // der Ausgabe des Referenzprogrammes berechnet und ausgegeben.
        //
        // Dieser Vorgang dient der Überprüfung der Korrektheit des Programms.
        // Die Interpretation des Ähnlichkeitswertes obliegt dem Nutzer, jedoch
        // wird ein Wert über 99% als Korrekt interpretiert.
        uint8_t *comparisonBuffer = (uint8_t *)malloc(imageSize);
        if (comparisonBuffer != NULL)
        {

                // Berechnung der von Referenzprogramm benötigten Zeit und Ausführung
                // des Referenzprogrammes
                double c_time = curtime();
                mandelbrot_c(r_start, r_end, i_start, i_end, resolution,
                             comparisonBuffer, max_iterations);
                c_time = curtime() - c_time;

                // Iteration über tatsächliches und erwartetes Bild. Für jeden
                // Unterschied an beiden Bildern wird ein Zähler inkrementiert
                u_int32_t counter = 0;
                for (int i = 0; i < imageSize; i += 3)
                {
                        if ((*(buffer + i) != *(comparisonBuffer + i)))
                        {
                                counter++;
                        }
                }

                // Ausgabe der Ähnlichkeit der beiden Bilder
                printf("   Die Ähnlichkeit zur Referenzimplementierung beträgt %f Prozent.\r\n",
                       100.0 - ((float)counter / (float)(imageSize / 3.0)));
                fflush(stdout);

                // Berechnung der Zeitdifferenz von Assembly- und Referenzimplementierung und
                // Ausgabe von dieser Abhängig davon, welche Implementierung schneller war
                double time_dif = c_time - time;
                if (time_dif > 0)
                {
                        printf("   Assembly Implementierung ist um %f Sekunden schneller als das C Referenzprogramm.\r\n", time_dif);
                }
                else if (time_dif < 0)
                {
                        time_dif *= -1;
                        printf("   C Referenzprogramm ist um %f Sekunden schneller als die Assembly Implementierung.\r\n", time_dif);
                }
                else
                {
                        printf("   Beide Implementierungen sind gleich schnell.\r\n");
                }
                fflush(stdout);

                // Freigeben des für das Referenzbild allokierten Speichers zur
                // Verhinderung von Memory Leaks
                free(comparisonBuffer);
        }
        else
        {
                fprintf(stderr, "   Test kann wegen Speichermangel nicht durchgeführt werden.\r\n");
        }

        // Freigeben des für das echte Bild allokierten Speichers zur
        // Verhinderung von Memory Leaks
        free(buffer);
        printf("   Bild \"%s\" wurde erfolgreich erzeugt.\r\n", file_name);
        return EXIT_SUCCESS;
}

// test: Führt automatisch alle Tests aus, die das korrekte Reagieren auf falsche Eingaben
// des Programmes simuliert. Die Korrektheit des Programms wird durch echte Eingaben durch
// den Nutzer getestet.
int test()
{
        if (!test_input(1, "./mandelbrot -2.5 1 -1 1 0.005 255",
                        "   Die Eingabe für r_start muss im Interval [-2;1] liegen.",
                        -2.5, 1, -1, 1, 0.005, 255))
        {
                printf("Test 1) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 1) erfolgreich!\r\n\r\n");

        if (!test_input(2, "./mandelbrot -2 2 -1 1 0.005 255",
                        "   Die Eingabe für r_end muss im Interval [-2;1] liegen.",
                        -2, 2, -1, 1, 0.005, 255))
        {
                printf("Test 2) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 2) erfolgreich!\r\n\r\n");

        if (!test_input(3, "./mandelbrot -2 1 -1.75 1 0.005 255",
                        "   Die Eingabe für i_start muss im Interval [-1;1] liegen.",
                        -2, 1, -1.75, 1, 0.005, 255))
        {
                printf("Test 3) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 3) erfolgreich!\r\n\r\n");

        if (!test_input(4, "./mandelbrot -2 1 -1 1000 0.005 255",
                        "   Die Eingabe für i_end muss im Interval [-1;1] liegen.",
                        -2, 1, -1, 1000, 0.005, 255))
        {
                printf("Test 4) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 4) erfolgreich!\r\n\r\n");

        if (!test_input(5, "./mandelbrot -2 1 -1 1 -0.878 255",
                        "   Die Resolution darf nicht negativ oder 0 sein.",
                        -2, 1, -1, 1, -0.878, 255))
        {
                printf("Test 5) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 5) erfolgreich!\r\n\r\n");

        if (!test_input(6, "./mandelbrot -2 1 -1 1 0 255",
                        "   Die Resolution darf nicht negativ oder 0 sein.",
                        -2, 1, -1, 1, 0, 255))
        {
                printf("Test 6) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 6) erfolgreich!\r\n\r\n");

        if (!test_input(7, "./mandelbrot -2 1 -1 1 0.005 -107",
                        "   Die Anzahl der maximalen Iterationen darf nicht kleiner 0 sein.",
                        -2, 1, -1, 1, 0.005, -107))
        {
                printf("Test 7) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 7) erfolgreich!\r\n\r\n");

        if (!test_input(8, "./mandelbrot -2 1 -1 1 0.0000001 255",
                        "   Mit den eingegebenen Parametern kann keine Berechnung durchgeführt werden.",
                        -2, 1, -1, 1, 0.0000001, 255))
        {
                printf("Test 8) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 8) erfolgreich!\r\n\r\n");

        if (!test_input(9, "./mandelbrot -2 -2 -1 1 0.005 255",
                        "   Mit den eingegebenen Parametern kann keine Berechnung durchgeführt werden.",
                        -2, -2, -1, 1, 0.005, 255))
        {
                printf("Test 9) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 9) erfolgreich!\r\n\r\n");

        if (!test_input(10, "./mandelbrot -2 1 -1 -1 0.005 255",
                        "   Mit den eingegebenen Parametern kann keine Berechnung durchgeführt werden.",
                        -2, 1, -1, -1, 0.005, 255))
        {
                printf("Test 10) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 10) erfolgreich!\r\n\r\n");

        if (!test_input(11, "./mandelbrot 0.0 0.0 0.0 0.0 0",
                        "   Die Resolution darf nicht negativ oder 0 sein.",
                        0.0, 0.0, 0.0, 0.0, 0.0, 0))
        {
                printf("Test 11) fehlgeschlagen! Abbrechen...\r\n");
                fflush(stdout);
                return EXIT_FAILURE;
        }
        printf("Test 11) erfolgreich!\r\n\r\n");

        printf("Zur Verifikation von validen Eingaben werden unter anderem folgende Parameterübergaben empfohlen:\r\n");
        printf("   ./mandelbrot -2 1 -1 1 0.001 510\r\n");
        printf("   ./mandelbrot 0.25 0.5 0.25 0.5 0.0005 510\r\n");
        printf("   ./mandelbrot -2 -1 -1 0 0.05 127\r\n");
        printf("   ./mandelbrot -2 1 -1 1 0.005 5000\r\n");
        printf("Der Vergleich zur Referenzimplementierung erfolgt automatisch.\r\n");

        fflush(stdout);
        return EXIT_SUCCESS;
}

// test_input: Führt einen spezifischen Test aus. Es muss ein Testindex,
// ein Programmaufrufinput, eine erwartete Nachricht und die für die
// Berechnung wichtigen Parameter angegeben werden.
// Gibt einen Wahrheitswert darüber aussagend zurück, ob der Test efolgreich war
_Bool test_input(int index, char *input, char *expected, float r_start, float r_end,
                 float i_start, float i_end, float resolution, int16_t max_iterations)
{
        printf("%d) Test\r\n", index);
        printf("Input: %s\r\n", input);
        printf("Erwartet:\r\n%s\r\n", expected);
        printf("Tatsächlich:\r\n");
        fflush(stdout);
        return calculate_mandelbrot("mandelbrot", r_start, r_end, i_start, i_end, resolution, max_iterations);
}

// Statische Methode zur Rückgabe der aktuellen Zeit
static double curtime(void)
{
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        return t.tv_sec + t.tv_nsec * 1e-9f;
}

// mandelbrot_c: Referenzimplementierung des in Assembly zu implementierenden Algorithmus
// zur Berechnung und Visualisierung der Mandelbrotmenge. Funktionsweise und Dokumentation
// des Algorithmus ist der Dokumentation zu entnehmen.
void mandelbrot_c(float r_start, float r_end, float i_start, float i_end, float resolution, unsigned char *img, int16_t max_iterations)
{
        u_int8_t colorscheme[36] = {205, 116, 24, 238, 134, 28, 255, 144, 30, 34, 180, 238, 37, 193, 255, 0, 215, 255, 0, 102, 205, 0, 118, 238, 0, 127, 255, 44, 44, 238};
        int16_t iteration_counter;
        float last_Re, last_Im;
        float imaginary_progress = i_start;
        float real_progress = r_start;
        u_int32_t x, y;
        u_int32_t width = (r_end - r_start) / resolution;
        width = width - (width % 4);
        u_int32_t height = (i_end - i_start) / resolution;
        height = height - (height % 4);
        y = 0;
        while (y < height)
        {
                real_progress = (double)r_start;
                x = 0;
                while (x < width)
                {
                        iteration_counter = 0;
                        last_Re = 0;
                        last_Im = 0;
                        while (iteration_counter < max_iterations)
                        {
                                float tmp_Re = last_Re;
                                float tmp_Im = last_Im;
                                last_Re = tmp_Re * tmp_Re - tmp_Im * tmp_Im + real_progress;
                                last_Im = (2 * tmp_Re * tmp_Im) + imaginary_progress;
                                if (!((last_Re * last_Re + last_Im * last_Im) < 4))
                                        break;
                                iteration_counter++;
                        }
                        if (iteration_counter != max_iterations)
                        {
                                u_int8_t color = (iteration_counter % max_iterations) % 10;
                                *(img++) = colorscheme[3 * (color) + 0];
                                *(img++) = colorscheme[3 * (color) + 1];
                                *(img++) = colorscheme[3 * (color) + 2];
                        }
                        else
                        {
                                img += 3;
                        }
                        real_progress += resolution;
                        x++;
                }
                imaginary_progress += resolution;
                y++;
        }
}
