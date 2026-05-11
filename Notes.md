## 1. Architettura del Programma

Il processo di trasformazione segue una pipeline logica suddivisa in quattro fasi principali:

### A. Caricamento e Decodifica

Il programma utilizza `stb_image.h` per leggere il file binario dell'immagine e convertirlo in un array lineare di byte in memoria.

* Ogni pixel viene forzato nel formato **RGBA** (4 byte: Rosso, Verde, Blu, Alfa).
* L'immagine originale viene liberata dalla memoria non appena viene creata la versione ridimensionata.

### B. Gestione dell'Aspect Ratio

Nel terminale, i caratteri non sono quadrati: solitamente sono circa due volte più alti che larghi.

* Il programma introduce una costante `CHAR_ASPECT = 0.55`.
* Senza questa correzione, l'immagine ASCII apparirebbe "schiacciata" o deformata verticalmente.

### C. Elaborazione dell'Immagine (Image Processing)

Prima della stampa, ogni pixel subisce tre trasformazioni:

1. **Downsampling**: L'immagine viene rimpicciolita per adattarsi alla larghezza del terminale (default 100 colonne) usando un campionamento *nearest-neighbor*.
2. **Correzione Colore**: Viene applicata una formula di contrasto e luminosità per rendere l'output ASCII più leggibile e vibrante.
3. **Luminosità Percettiva**: Per decidere quale carattere ASCII usare (es. `@` per il nero, `.` per il grigio chiaro), il programma calcola quanto un pixel è "chiaro" usando la formula standard ITU-R BT.601:

    $Y = 0.299R + 0.587G + 0.114B$



*Questa formula tiene conto del fatto che l'occhio umano percepisce il verde come più luminoso del blu.*

### D. Rendering nel Terminale

L'output finale avviene tramite:

* **Charset**: Una stringa di caratteri ordinati per densità visiva (`@#S%?*+;:,. `).
* **ANSI True-Color**: Il programma invia al terminale sequenze speciali del tipo `\033[38;2;R;G;Bm`. Questo permette di visualizzare milioni di colori (24-bit), a patto che il terminale sia moderno (come i terminali Linux, macOS o Windows Terminal).

---

## 2. Analisi delle Funzioni Chiave

| Funzione | Descrizione |
| --- | --- |
| `apply_contrast_channel` | Implementa un algoritmo di contrasto simile a Photoshop per espandere la gamma dinamica dei colori. |
| `luminance` | Converte un pixel a colori in un valore di scala di grigi basato sulla percezione umana. |
| `clamp` | Una funzione di utilità che impedisce ai valori dei colori di uscire dall'intervallo standard [0, 255]. |
| `stbi_load` | Funzione esterna che gestisce la complessità dei formati compressi (JPEG, PNG, BMP). |

---

## 3. Parametri

* **Larghezza**: Numero di caratteri orizzontali (modifica la risoluzione).
* **Contrassto**: Valori > 1.0 rendono le ombre più scure e le luci più chiare (migliora la resa su terminali scuri).
* **Luminosità**: Aggiunge o sottrae intensità luminosa a tutti i pixel.

---


> **Nota sulla Trasparenza**: Il programma gestisce il canale Alfa (A). Se un pixel è trasparente (valore < 128), viene automaticamente renderizzato come uno spazio vuoto, ignorando i colori di sfondo.