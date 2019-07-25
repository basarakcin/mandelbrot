.intel_syntax noprefix
.global mandelbrot

#Farbschema zur Visualisierung in 4 Byte Blöcken
.data
  colorscheme:
    .align 2
    .4byte 0x1874CD
    .4byte 0x1C86EE
    .4byte 0x1E90FF
    .4byte 0xEEB422
    .4byte 0xFFC125
    .4byte 0xFFD700
    .4byte 0xCD6600
    .4byte 0xEE7600
    .4byte 0xFF7F00
    .4byte 0xEE2C2C

#Registerbelegungstabelle
#  Floating Point / Vektorregister
#    -  xmm0 - Während Initialisierung r_start, danach r_start Vektor
#    -  xmm1 - Während Initialisierung r_end, danach Iterations-Inkrementer
#    -  xmm2 - Während Initialisierung i_start, Erweiterung zum Vektor
#    -  xmm3 - Repräsentiert i_end
#    -  xmm4 - Resolution, Resolution Vektor
#    -  xmm5 - Vektor mit vierfacher Resolution für Inkrementierung von r_start
#    -  xmm6 - Temporäre Kopie von r_start Vektor zur Benutzung in der Berechnung
#    -  xmm7 - Iterationszähler Vektor
#    -  xmm8 - In der Berechnung letzter Realwert
#    -  xmm9 - In der Berechnung letzter Imaginärwert
#    - xmm10 - Vektor mit Konstanten 4
#    - xmm11 - Vektor mit Konstanten 2
#    - xmm12 - Temporäre Kopie des letzten berechneten Realwertes, allgemeines Zwischenregister
#    - xmm13 - Temporäre Kopie des letzten berechneten Imaginärwertes
#
#  Standardregister
#    - rdi - Pointer auf den für die Farbdaten allokierten Speicherbereich
#    - rsi - Maximale Anzahl pro Durchlauf durchzuführender Iterationen
#    - rax - Allgemeines Rechenregister
#    - rcx - Skalarer Iterationszähler, allgemeines Rechenregister
#    - rdx - Enthält Rest bei Divisions/Modulo Operationen
#    -  r8 - Enthält Breite des Bildes
#    -  r9 - Enthält Höhe des Bildes, gleichzeitig Zähler für Schleifendurchlauf Imaginärachse
#    - r10 - Zähler für Schleifendurchlauf auf der Realachse
#    - r11 - Konstante 10 zur Berechnung des Farboffsets
#
#Methodensignatur
#  mandelbrot(float r_start, float r_end, float i_start, float i_end, float res, char img_data, int16_t i_max)

.text
mandelbrot:

  #Initialisiert r11d mit 10 für die Moduloberechnung des Farboffsets
  mov r11d, 10

  #Breite des Bildes ausrechnen, zu Integer konvertieren und
  #auf ein Vielfaches von 4 reduzieren
  #verkleinern und speichern
  movss xmm12, xmm1
  subss xmm12, xmm0
  divss xmm12, xmm4
  cvttss2si r8, xmm12
  xor dx, dx
  mov ax, r8w
  mov cx, 4
  div cx
  sub r8w, dx

  #Höhe des Bildes ausrechnen, zu Integer konvertieren und
  #auf ein Vielfaches von 4 reduzieren
  movss xmm12, xmm3
  subss xmm12, xmm2
  divss xmm12, xmm4
  cvttss2si r9, xmm12
  xor dx, dx
  mov ax, r9w
  div cx
  sub r9w, dx

  #Divisionsrest Register für spätere Farbberechnung auf 0 setzen
  xor rdx, rdx

  #Für die Berechnungen gebrauchte Kontanten 2 und 4 als
  #Single Precision Floating Point Werte in Vektorregister
  #laden
  cvtsi2ss xmm10, ecx
  pshufd xmm10, xmm10, 0x00
  shr cx, 1
  cvtsi2ss xmm11, ecx
  pshufd xmm11, xmm11, 0x00

  #Laden der ersten vier Elemente auf der Real-Achse in das r_start
  #Vektorregister
  movss xmm12, xmm0
  addss xmm12, xmm4
  insertps xmm0, xmm12, 0x10
  addss xmm12, xmm4
  insertps xmm0, xmm12, 0x20
  addss xmm12, xmm4
  insertps xmm0, xmm12, 0x30

  #Verteilen der Resolution auf dessen Vektorregister und Kopieren der
  #Vervierfachung in ein weiteres Register zur Inkrementierung der
  #Real-Achsen Elemente
  pshufd xmm4, xmm4, 0x00
  movaps xmm5, xmm4
  mulps xmm5, xmm10

  #Für die Berechnung i_start auf alle Elemente des Vektorregisters verteilen
  pshufd xmm2, xmm2, 0x00

    #Schleife über alle Werte auf der Imaginär-Achse
  .Limaginary_axis_loop:

    #Wenn die verbleibende Höhe 0 erreicht, zum Ende des Programmes springen
    cmp r9w, 0
    je .end

    #Erneutes Laden der initialen Real-Achsen Werte und Nullsetzen des Breitenzählers
    movups xmm6, xmm0
    xor r10w, r10w

    #Schleife über alle Werte auf der Real-Achse
    .Lreal_axis_loop:

      #Wenn der Breitenzähler die echte Breite erreicht, wird zum Ende des
      #Imaginär-Loops gesprungen
      cmp r10w, r8w
      jge .Limaginary_axis_loop_end

      #Iterationszähler, last_re, last_im und Iterationszählervektor auf 0 setzen
      xor cx, cx
      pxor xmm7, xmm7
      pxor xmm8, xmm8
      pxor xmm9, xmm9

      #Iterations-Inkrementer mit 1 in jedem Vektorelement laden
      mov eax, 1
      movd xmm1, eax
      pshufd xmm1, xmm1, 0x00

      #Berechnung der Anzahl der notwendigen Iterationsschritte um zu zeigen,
      #dass ein Wert nicht beschränkt ist.
      .Lcalculation_loop:

        #Wenn die maximale Anzahl von Iterationen erreicht ist, wird aus der Schleife
        #gesprungen
        cmp cx, si
        jge .Lreal_axis_loop_end

        #Sicherung des alten Real- und Imaginär-Wertes
        movaps xmm12, xmm8
        movaps xmm13, xmm9

        #Berechnung des neuen Real-Wertes
        mulps xmm8, xmm8
        mulps xmm9, xmm9
        subps xmm8, xmm9
        addps xmm8, xmm6

        #Berechnung des neuen Imaginär-Wertes
        movaps xmm9, xmm11
        mulps xmm9, xmm12
        mulps xmm9, xmm13
        addps xmm9, xmm2

        #Berechnung, ob eine der zuletzt berechneten Werte innerhalb des Kreises (s. Dokumentation)
        #liegt. Wenn nein, wird der Iterationsinkrementer an dieser Stelle auf 0
        #gesetzt, damit die Iterationsanzahl an der Stelle nicht weiter erhöht wird.
        movaps xmm12, xmm8
        mulps xmm12, xmm12
        movaps xmm13, xmm9
        mulps xmm13, xmm13
        addps xmm12, xmm13
        cmpltps xmm12, xmm10
        pand xmm1, xmm12

        #Wenn gezeigt ist, dass keiner der Werte beschränkt ist, wird aus der Schleife
        #gesprungen
        ptest xmm1, xmm1
        jz .Lreal_axis_loop_end

        #Aktualisierung des Iterationszählers und des Iterationszähler-Vektors um
        #den Iterationsinkrementer-Vektor. Rücksprung zum Schleifenanfang
        paddd xmm7, xmm1
        inc cx
        jmp .Lcalculation_loop

      #Schleifenende der Real-Schleife, in der die Farben geschrieben werden
      .Lreal_axis_loop_end:

        #Nacheinander werden die Iterationswerte aus dem Vektor in reguläre Register
        #geladen und verarbeitet. Dokumentation für ersten Schritt gilt exemplarisch
        #für die 3 verbleibenden Schritte 
        pextrd eax, xmm7, 0x00
        
        #Überprüfen ob die Iterationszahl gleich der maximalen Iterationszahl ist.
        #Wenn ja, dann überspringe folgenden Teil und fahre mit nächstem Vektorelement
        #fort.
        cmp si, ax
        je .Lskip_coloring_1
        
        #Nullsetzen des Divisionsrest Registers und Division durch 10, also Größe
        #der Farbpalette.
        xor dx, dx
        div r11w

        #Multiplikation des Farbindexes mit 4 zur korrekten Adressierung des Anfangs
        #des Farbwertes im Speicher
        sal dx, 2

        #Laden des BGR Wertes der Farbe in das allgemeine Rechenregister und Schreiben
        #an die Stelle des nächsten Pixels im Speicher
        mov eax, [colorscheme + rdx]
        mov [rdi], eax
      .Lskip_coloring_1:
        pextrd eax, xmm7, 0x01
        cmp si, ax
        je .Lskip_coloring_2
        xor dx, dx
        div r11w
        sal dx, 2
        mov eax, [colorscheme + rdx]
        mov [rdi + 3], eax
      .Lskip_coloring_2:
        pextrd eax, xmm7, 0x02
        cmp si, ax
        je .Lskip_coloring_3
        xor dx, dx
        div r11w
        sal dx, 2
        mov eax, [colorscheme + rdx]
        mov [rdi + 6], eax
      .Lskip_coloring_3:
        pextrd eax, xmm7, 0x03
        cmp si, ax
        je .Lskip_coloring_4
        xor dx, dx
        div r11w
        sal dx, 2
        mov eax, [colorscheme + rdx]
        mov [rdi + 9], eax
      .Lskip_coloring_4:
        add rdi, 12

      #Den Fortschritt auf der Real-Achse um die vierfache Resolution erhöhen,
      #Breitenfortschritt inkrementieren und zum Schleifenanfang zurückspringen
      addps xmm6, xmm5
      add r10w, 4
      jmp .Lreal_axis_loop

    .Limaginary_axis_loop_end:

      #Den Fortschritt auf der Imaginär-Achse um die Resolution erhöhen,
      #Höhe dekrementieren und zum Schleifenanfang zurückspringen
      addps xmm2, xmm4
      dec r9w
      jmp .Limaginary_axis_loop

#Beendet das Programm
.end:
  ret
