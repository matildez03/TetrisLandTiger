**PROGETTO AGGIUNTIVO PER ARCHITETTURE DEI SISTEMI DI ELABORAZIONE**

TETRIS SVILUPPATO PER PIATTAFORMA EMBEDDED LANDTIGER 


Il progetto implementa un gioco di Tetris per piattaforma embedded Landtiger. È stato sviluppato e testato esclusivamente usando l’emulatore in debug session e non è stato vallidato su scheda reale.


Configurazione emulatore
Per eseguire correttamente il progetto, dunque, è necessario utilizzare l’emulatore nella seguente configurazione: 
<img width="499" height="132" alt="image" src="https://github.com/user-attachments/assets/2220e8a4-08d1-4aeb-916e-cf96ec1a6d4c" />


Le configurazioni del timer0 e del RIT sono definite (e modificabili) nel file TetrisASM/Source/sample.c, alle righe 19 e 20 (etichette tm0Period e RITPeriod).
<img width="499" height="59" alt="image" src="https://github.com/user-attachments/assets/7c7d0c04-714c-4487-9c0c-e9131f018b5b" />
