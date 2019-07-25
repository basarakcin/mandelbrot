# mandelbrot
Die Erstellung und Implementierung eines Algorithmus in Assembler zur Generierung und Speicherung von Bilddaten der Visualisierung der Mandelbrotmenge.
Das Programm zur Berechnung der Mandelbrotmenge ist in Assembly und einem C-Rahmenprogramm implementiert. Deshalb müssen die Quelldateien vor der ersten Benutzung kompiliert werden.
```C
$ make
```
Nach der Kompilierung kann das Programm mit Standardwerten aufgerufen werden, was ein Bild im BMP Format mit dem Namen "mandelbrot.bmp" von der gesamten Mandelbrotmenge erzeugt.
Dieser Aufruf in der Konsole erfolgt so:
```C
$ ./mandelbrot
```
Um einen spezifischen Ausschnitt der Mandelbrotmenge genauer zu visualisieren, kön- nen bei Programmaufruf Übergabeparameter verwendet werden. Dabei ist es wichtig, das folgende Aufrufsmuster einzuhalten:
```C
$ ./mandelbrot dateiname r_start r_end i_start _end resolution i_max
```
Alle Parameter, bis auf i_max, sind als Fließkommazahlen anzugeben. Außerdem gelten folgende Einschränkungen:
* Die Eingabe für r_start muss im Interval [-2;1] liegen. Startwert auf Realachse.
* Die Eingabe für r_end muss im Interval [-2;1] liegen. Endwert auf Realachse.
* Die Eingabe für i_start muss im Interval [-1;1] liegen. Startwert auf Imaginärachse.
* Die Eingabe für i_end muss im Interval [-1;1] liegen. Endwert auf Imaginärachse.
* DieResolutiondarfnichtnegativoder0seinundsolltenichtgrößer1sein.Abstand zweier Samplepunkte
* Die Anzahl der maximalen Iterationen darf nicht kleiner 0 sein.

Zusätzlich dazu lassen sich durch Ausführen von
```C
$ ./mandelbrot test
```
die automatischen Tests durchführen. Diese stellen sicher, dass die Eingabevalidierung fehlerfrei ist.
Das kompilierte Programm lässt sich durch
```C
$ make clean
```
wieder entfernen
