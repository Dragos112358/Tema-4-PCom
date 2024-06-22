Readme Pcom Tema 4

Timp de lucru temă: aproximativ 6-7 ore
Depanare: aproximativ 3 ore
Implementată ȋn C

	Partea de depanare a presupus, ȋn cazul meu, să fac checkerul să funcţioneze 
corespunzător. Am implementat destul de repede toate comenzile in client.c, ȋnsă checkerul 
tot continua să dea timeout. Problema era de la faptul că la partea de register şi la login 
ceream Username şi Password (puneam cu litere mari ȋn loc să pun cu litere mici).

	Implementare efectivă:

-toată implementarea se focusează pe crearea unui client HTTP care interacţionează cu un 
server aflat la adresa IP 34.246.184.49, pe portul 8080.
-am implementat tot ȋn client.c

	Am o funcţie simplă, numită is_valid_number, care primeşte un string ca parametru şi 
returnează 0 sau 1. (0 dacă numărul este valid, 1 ȋn caz contrar). Folosesc funcţia isdigit 
din biblioteca ctype.h pentru a vedea dacă un caracter este număr. 
	Această funcţie este:
-utila pentru a vedea daca un string este un numar valid
-utila pentru comanda get_book (se introduce un id)
-utila pentru comanda delete_book (se introduce un id si aici)
-utila pentru a transforma nr de pagini

	Ȋn partea de main, ȋmi declar 2 şiruri de caractere, cookie şi cookie_biblioteca. Cele 2 
nume sunt sugestive. (Le setez memoria pe 0 iniţial). Prima variabilă reţine tokenul primit de 
la server atunci cȃnd dau login (pe care ȋl folosesc mai departe la comanda enter_library). Cea 
de-a doua variabilă (cookie_biblioteca) o folosesc pentru a reţine tokenul primit de la server 
după ce dau request la acces ȋn bibliotecă. (Pe baza acestui token, am acces la toate comenzile 
importante: get_books, get_book <id>, add_book şi delete_book).
	După ce mi-am declarat variabilele necesare, mă conectez la server 
(open_connection) şi citesc input de la stdin folosind fgets. Abordez fiecare comandă ȋn 
parte. Compar inputul cu fiecare comandă ȋn parte folosind strncmp (buffer_citit, comandă, 
nr_caractere).

	Comanda “exit” este prima pe care am făcut-o, deoarece este foarte simplă. Dacă 
utilizatorul introduce “exit”, ȋnchid conexiunea cu serverul şi dau exit(EXIT_SUCCESS).

	Comanda “register” este şi ea destul de simplă. Citesc username-ul şi parola, la care 
adaug ‘\0’ (terminator de şir). Validez aceste date (nu accept ca userul şi parola să fie nule 
sau să fie doar un spaţiu). Dacă datele sunt valide, le adaug la un obiect json, care arată de 
forma {"username":"user1234","password":"12342"}. Lucrul cu obiectele JSON ȋl realizez folosind 
biblioteca parson.c, la care se adaugă şi parson.h. Această bibliotecă apare ȋn cerinţă ca 
recomandare pentru implementare ȋn C. Trimit către server o cerere de forma
"POST /api/v1/tema/auth/register HTTP/1.1 Host: 34.246.184.49 Connection: open Content-Type: 
application/json Content-Length: %d\r\n\r\n%s", content_length, post_data. Folosesc funcţiile 
send_to_server şi receive_from_server pentru a primi răspuns. Verfic dacă răspunsul este ok sau 
error şi afişez mesaje ca atare.

	Comanda “login” este făcută aproape identic, singurele diferenţe fiind că ȋn if fac strncmp 
cu “login”, iar mesajul trimis are o formă uşor diferită la cale:  "POST /api/v1/tema/auth/login 
HTTP/1.1 Host: 34.246.184.49 Connection: open Content-Type: application/json Content-Length: 
%d\r\n\r\n%s", content_length, post_data. O altă diferenţă este că salvez ȋn variabila cookie 
tokenul primit ȋn răspunsul de la server. Folosesc strstr şi strchr pentru a găsi primul şi 
ultimul caracter din cookie. Dacă răspunsul este ok, userul se loghează cu succes. Există 
posibilitatea ca acesta să nu se poată loga din cauza userului sau a parolei introduse incorect. 
Serverul verifică şi ca userul să fie ȋnregistrat ȋnainte de a se loga. Dacă userul nu este 
ȋnregistrat, trimite un mesaj de eroare.

	Următoarea comandă implementată este “enter_library”. Verific primul lucru daca cookie-ul de 
logare este nenul. Ȋn cazul ȋn care nu am un cookie de logare, afişez mesajul "FAILED! You didn't 
login!\n". Dacă există cookie de logare, cer serverului acces la bibliotecă printr-un GET la calea 
/api/v1/tema/library/access. Trimit ȋn mesaj şi cookie-ul de logare. Primesc mesajul de la server. 
Folosesc un strstr pentru a delimita ȋn răspunsul serverului partea de după token (ȋl salvez ȋn 
cookie_biblioteca, deoarece am nevoie de acesta pentru a face cereri pentru restul comenzilor). Voi 
folosi acest cookie mai departe pentru a putea adauga, vizualiza şi şterge cărţi.

	Comanda “get_books” este utilă pentru a vedea titlurile şi id-urile cărţilor. Verific să fiu 
logat şi să am acces la bibliotecă. Trimit o cerere către server, ȋn care cer să văd cărţile. Mesajul 
trimis este de forma: "GET /api/v1/tema/library/books HTTP/1.1\r\nHost: 34.246.184.49\r\nConnection: 
open\r\nAuthorization: Bearer %s\r\n\r\n", cookie_biblioteca . Parsez răspunsul de la server, folosind 
strstr pentru fiecare carte ȋn parte şi afişez toate cărţile sub forma id şi titlu.

	Comanda get_book este folosită pentru a vedea o singură carte cu mai multe detalii. Utilizatorul 
introduce id-ul cărţii, iar ȋn cod este trimisă o cerere de tip GET: "GET /api/v1/tema/library/books/%d 
HTTP/1.1\r\nHost: 34.246.184.49\r\nConnection: open\r\nAuthorization: Bearer %s\r\n\r\n", id2, 
cookie_biblioteca . Verific că numărul introdus este valid (că este compus doar din cifre). Dacă este 
valid, fac atoi pe el şi ȋl convertesc la int. Parsez răspunsul de la server. Dacă apare cuvȃntul “error”, 
dau FAILED. Altfel, ȋmi creez un obiect JSON din răspuns şi afişez toate detaliile cărţii: titlu, autor, 
gen, editor şi număr de pagini.

	Comanda add_book este cam cea mai stufoasă dintre toate, deoarece trebuie să citesc 5 date şi să le şi 
validez. Folosesc un flag numit validare_date. Verific ca niciun cȃmp să nu fie gol. Verific ca numărul de 
pagini introdus chiar este un număr, folosind funcţia is_valid_number. Dacă datele nu sunt corecte, afişez 
mesajul "FAILED! Data introduced are bad formatted!\n". Dacă datele sunt corecte, fac un string de forma 
sprintf(json_string, "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"page_count\":%d,\"publisher\":\"%s\"}", 
title, author, genre, nr_pagini, publisher) . Trimit cerere de POST pentru server. Dacă răspunsul este ok, 
dau un mesaj de succes, altfel afisez: "FAILED! User didn't enter the library! Use command enter_library!\n".

	Pentru comanda delete_book, ȋntȃi verific că am acces la bibliotecă. Afişez “id=”. Las utilizatorul să 
scrie un id. Verific, folosind funcţia is_valid_number, că id-ul este valid. Dacă id-ul este valid, ȋl convertesc 
din string ȋn int. Trimit o cerere de forma sprintf(message, "DELETE /api/v1/tema/library/books/%d HTTP/1.1\r
\nHost: 34.246.184.49\r\nConnection:open\r\nAuthorization: Bearer %s\r\n\r\n",id2, cookie_biblioteca); . Parsez 
răspunsul primit de la server. Dacă afişează ceva cu “{“, comanda a eşuat şi afişez printf("FAILED! You 
introduced wrong id or no books left!\n"); .Dacă comanda este bună, afişez mesaj de succes.

	Comanda de logout are rolul de a mă deloga. Pentru a putea face acest lucru, ȋntȃi verific că sunt logat. 
Creez o cerere de logout: sprintf(message, "GET /api/v1/tema/auth/logout HTTP/1.1\r\nHost: 34.246.184.49
\r\nConnection: close\r\nAuthorization: Bearer %s\r\nCookie: %s\r\n\r\n", cookie, cookie);. Dacă serverul returnează 
un mesaj care conţine ok, atunci mă deloghez cu succes. Altfel, afişez mesaj de eroare. Setez cookie-ul de logare şi 
cel de acces la bibliotecă pe 0 (trebuie sa mă loghez din nou ca să pot să adaug,şterg sau vizualizez cărţi).
