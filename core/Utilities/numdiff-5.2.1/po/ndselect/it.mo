��    #      4  /   L        (  	  1   2     d     �     �  (   �     �  ,   �     '  %   E  ,   k  -   �      �  &   �          .  ?   N  5   �  %   �  G   �  I   2     |  �   �  1   V  E   �     �  B   �     (	  )   H	  9   r	  ?   �	  P   �	     =
     D
  �  G
  T  �  %   5  /   [  %   �      �  2   �       )   #     M  .   k  )   �  *   �      �  -     )   >  )   h  <   �  6   �  (     G   /  Y   w     �  �   �  3   �  P   �      K  Q   l  $   �  #   �  ;     ;   C  V        �     �            	      
                      #   !                                                                                                              "    
For a given range of line numbers and a given step
print on the standard output all lines of a file,
starting with the first line of the range and ending within
its last line, whose line number is such that the difference
between it and the start point is an integer multiple
of the given step

 %s: Error occurred while reading from file "%s"

 %s: cannot close file "%s":
 %s: cannot open file "%s":
 %s: illegal option -- %c
 %s: invalid argument after `-%c' option
 %s: invalid option -- %c
 %s: option `%c%s' doesn't allow an argument
 %s: option `%s' is ambiguous
 %s: option `%s' requires an argument
 %s: option `--%s' doesn't allow an argument
 %s: option `-W %s' doesn't allow an argument
 %s: option `-W %s' is ambiguous
 %s: option requires an argument -- %c
 %s: unrecognized option `%c%s'
 %s: unrecognized option `--%s'
 (The default behavior is to arrive till to the end of the file) (The default behavior is to start with line number 1) (The default value for the step is 1) Exit status: 0 in case of normal termination, -1 (255) in case of error If no input file is specified, the program reads from the standard input. Ivano Primi License GPLv3+: GNU GPL version 3 or later,
see <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
 Redirect output from stdout to the indicated file Redirect warning and error messages from stderr to the indicated file Show this help message Show version number, Copyright, Distribution Terms and NO-Warranty Specify the first line to print Specify the last line that can be printed Specify the step to use when selecting the lines to print The argument after the options is the name of the file to scan. The complete path of the file should be given,
a directory name is not accepted. Usage: or Project-Id-Version: numdiff 5.2.0 (ndselect 5.2.0)
Report-Msgid-Bugs-To: ivprimi@libero.it
POT-Creation-Date: 2010-01-06 23:20+0100
PO-Revision-Date: 2010-01-07 17:00+0100
Last-Translator: Ivano Primi <ivprimi@libero.it>
Language-Team: ITALIAN <ivprimi@libero.it>
MIME-Version: 1.0
Content-Type: text/plain; charset=ISO-8859-1
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=n != 1 ? 1 : 0;
 
Dato un intervallo di numeri di linea e un certo passo,
stampa sullo schermo, cominciando dalla prima linea dello
intervallo e terminando entro l'ultima linea dello stesso,
tutte le linee di un file il cui numero di linea � tale che
la differenza tra esso e il punto di partenza dell'intervallo
� un multiplo intero del passo specificato

 %s: Errore di lettura dal file "%s"

 %s: � stato impossibile chiudere il file "%s":
 %s: impossibile aprire il file "%s":
 %s: opzione inammissibile -- %c
 %s: un argomento non valido segue l'opzione `-%c'
 %s: opzione non valida -- %c
 %s: l'opzione `%c%s' non vuole argomenti
 %s: l'opzione `%s' � ambigua
 %s: l'opzione `%s' ha bisogno di un argomento
 %s: l'opzione `--%s' non vuole argomenti
 %s: l'opzione `-W %s' non vuole argomenti
 %s: l'opzione `-W %s' � ambigua
 %s: opzione con argomento obbligatorio -- %c
 %s: l'opzione `%c%s' risulta sconosciuta
 %s: l'opzione `--%s' risulta sconosciuta
 (L'azione predefinita � di arrivare fino alla fine del file) (L'azione predefinita � di stampare dalla prima linea) (Il valore predefinito per il passo � 1) Codice di uscita: 0 in caso di conclusione regolare, -1 (255) su errore Se non si specifica il file da scorrere, il programma prende i dati
dallo standard input. Ivano Primi Licenza GPLv3+: GNU GPL version 3 o successiva,
vedi <http://gnu.org/licenses/gpl.html>.
Questo � software libero: sei libero di modificarlo e redistribuirlo.
NON c'� NESSUNA GARANZIA, per quanto consentito dalle vigenti normative.
 Reindirizza l'output dallo schermo al file indicato Reindirizza avvertimenti e messaggi di errore
    dallo schermo al file indicato Mostra questo messaggio di aiuto Mostra numero di versione, Copyright,
    termini di distribuzione e NON-Garanzia Specifica la prima linea da stampare Specifica l'ultima linea stampabile Specifica il passo con cui selezionare le linee da stampare L'argomento dopo le opzioni � il nome del file da scorrere. � bene fornire il percorso completo del file,
un nome di cartella non viene accettato. Uso: oppure 