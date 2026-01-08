**PROGETTO AGGIUNTIVO PER ARCHITETTURE DEI SISTEMI DI ELABORAZIONE**

TETRIS SVILUPPATO PER PIATTAFORMA EMBEDDED LANDTIGER 


Il progetto implementa un gioco di Tetris per piattaforma embedded Landtiger. È stato sviluppato e testato esclusivamente usando l’emulatore in debug session e non è stato vallidato su scheda reale.


Configurazione emulatore
Per eseguire correttamente il progetto, dunque, è necessario utilizzare l’emulatore nella configurazione standard di keil.


Le configurazioni del timer0 e del RIT sono definite (e modificabili) nel file TetrisASM/Source/sample.c, alle righe 19 e 20 (etichette tm0Period e RITPeriod).

