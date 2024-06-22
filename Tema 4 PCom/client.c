#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <ctype.h>
#include "parson.h"
int login = 0;

//functie utila pentru a vedea daca un string este un numar valid
//utila pentru comanda get_book (se introduce un id)
//utila pentru comanda delete_book (se introduce un id si aici)
//utila pentru a transforma nr de pagini
int is_valid_number(const char *str) {
    //verific daca am de-a face cu un string gol
    if (*str == '\0') {
        return 0; // Numar invalid
    }
    
    // Iterez prin fiecare caracter al stringului
    for(int k = 0; k < strlen(str) - 1; k++) {
        // verific daca caracterul e o cifra
        if (!isdigit(str[k])) { //biblioteca ctype
            return 0; //numar invalid
        }
        str++; // Merg la urmatorul caracter
    }
    
    return 1; //numar valid
}

int main(int argc, char *argv[]) {
    char *response;
    int sockfd;
    char cookie[1000]; //aici salvez cookie-ul primit de la server dupa logare
    memset(cookie,0,1000); //il setez pe 0
    char cookie_biblioteca[1000]; //aici salvez cookie-ul primit dupa comanda enter_library
    memset(cookie_biblioteca,0,1000); //il setez pe 0
    sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0); //ma conectez la server
    while(1)
    {
        char buf[4096];
        fgets(buf,sizeof(buf),stdin); //citesc input de la tastatura
        buf[strcspn(buf, "\n")] = '\0';
        sockfd = open_connection("34.246.184.49", 8080, AF_INET, SOCK_STREAM, 0);
        if(strncmp(buf,"exit",4)==0) //daca comanda este exit (primul caz, cel mai banal)
        {
            close_connection(sockfd); //inchid socketul
            exit(EXIT_SUCCESS); //dau exit
        }
        else if(strncmp(buf, "register", 8) == 0) { //daca comanda este register
            int flag_inregistrare = 0; //acest flag imi spune daca datele sunt valide 
            char username[4096];
            char password[4096];
            printf("username=");
            fgets(username, 4096, stdin); //citesc userul
            printf("password=");
            fgets(password, 4096, stdin); //citesc parola
            
            // Verifica daca utilizatorul si parola sunt valide
            if (password[0] == '\n' || username[0] == '\n' || strstr(username, " ") || strstr(password, " ")) {
                flag_inregistrare = 1; //invalidare date
            }
            
            // Daca exista probleme cu datele introduse, afiseaza un mesaj de eroare si reia bucla
            if (flag_inregistrare == 1) {
                fprintf(stderr, "ERROR. Username or password are too small or contain invalid characters.\n");
                continue;
            }
            username[strlen(username) - 1] = '\0';
            password[strlen(password) - 1]='\0';
			JSON_Value *user_nou = json_value_init_object(); //imi creez un obiect json
			JSON_Object *obiect_user_nou = json_value_get_object(user_nou); 
			json_object_set_string(obiect_user_nou, "username", username); //adaug userul
			json_object_set_string(obiect_user_nou, "password", password); //si parola
			char *user = json_serialize_to_string(user_nou);
            // Construieste datele pentru cererea POST de inregistrare
            int content_length = strlen(user);
            char *post_data = malloc(content_length + 1); // +1 pentru terminatorul de sir '\0'
            strcpy(post_data, user); // Copiaza datele JSON in post_data

            // Construirea mesajului HTTP pentru cererea POST de inregistrare
            char *message = malloc(2048); // Alocare suficienta pentru mesajul HTTP
            sprintf(message, "POST /api/v1/tema/auth/register HTTP/1.1\r\n"
                            "Host: 34.246.184.49\r\n"
                            "Connection: open\r\n"
                            "Content-Type: application/json\r\n" // Modificare Content-Type la application/json
                            "Content-Length: %d\r\n\r\n" // Eliminare +1 din formula originala
                            "%s", content_length, post_data);
            
            // Trimite cererea POST catre server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd); //primesc raspuns
            // Verifica raspunsul pentru a determina succesul sau esecul inregistrarii
            if (strstr(response, "ok") != NULL) { //daca apare ok, am inregistrat cu succes
                printf("201 - Created - SUCCESS\n");
            } else {
                fprintf(stderr, "ERROR Register!\n"); //userul exista deja
            }
            // Elibereaza memoria alocata
            free(post_data);
            free(message);
        }

        else if(strncmp(buf,"login",5) ==0) //comanda login
        {
            int flag_logare = 0;
            char username[100]; //citesc un user si o parola de la stdin
            char password[100];
            printf("username=");
            fgets(username,sizeof(username),stdin);
            printf("password=");
            fgets(password,sizeof(password),stdin);
            //in acest if validez datele
            if(password[0]=='\n' || username[0]=='\n' || strstr(username, " ") || strstr(password, " "))
                flag_logare  = 1; //userul si/sau parola sunt invalide
            if(flag_logare == 1) { //daca din start nu e bine, dau mesaj de eroare
                printf("ERROR. User or password are too small\n");
                continue;
            }
            username[strlen(username) - 1] = '\0'; //adaug caracter specific pentru final de rand
            password[strlen(password) - 1]='\0';
            JSON_Value *valoare_obiect = json_value_init_object();
			JSON_Object *logare = json_value_get_object(valoare_obiect);
			json_object_set_string(logare, "username", username);
			json_object_set_string(logare, "password", password);
			char *user_json = json_serialize_to_string(valoare_obiect);
            int content_length = strlen(user_json);
            char *post_data = malloc(content_length + 1); // +1 pentru terminatorul de sir '\0'
            strcpy(post_data, user_json); // Copierea datelor JSON in post_data
            // Construirea mesajului HTTP pentru cererea POST de autentificare
            char message[2048];// Alocare suficienta pentru mesajul HTTP
            memset(message,0,2048);
            sprintf(message, "POST /api/v1/tema/auth/login HTTP/1.1\r\n"
                            "Host: 34.246.184.49\r\n"
                            "Connection: open\r\n"
                            "Content-Type: application/json\r\n" // Modificare Content-Type la application/json
                            "Content-Length: %d\r\n\r\n" // Eliminarea +1 din formula originala
                            "%s", content_length, post_data);
            // Trimiterea cererii catre server si primirea raspunsului
            send_to_server(sockfd, message); //trimit mesajul la server
            response = receive_from_server(sockfd); //primesc un raspuns 
            char *cookie_start = strstr(response, "Set-Cookie: "); //extrag cookie-ul din raspunsul serverului
            if (cookie_start != NULL) {
                cookie_start += strlen("Set-Cookie: ");
                char *cookie_end = strchr(cookie_start, ';'); 
                // Cauta caracterul ';', care marcheaza sfarsitul valorii cookie-ului
                if (cookie_end != NULL) {
                    // Copiaza valoarea cookie-ului intr-un buffer separat
                    char cookie_buffer[1000]; // Poate fi ajustat in functie de lungimea maxima a valorii cookie-ului
                    strncpy(cookie_buffer, cookie_start, cookie_end - cookie_start);
                    cookie_buffer[cookie_end - cookie_start] = '\0';
                    strcpy(cookie,cookie_buffer); // Acum am valoarea cookie-ului in cookie_buffer
                }
            }
            if(strstr(response,"ok")!=NULL) //daca userul se logheaza cu succes
            {
                printf("SUCCESS! User logged in!\n");
                login = 1; //flag de logare
            }
            else {
                printf("ERROR! User and/or password incorrect!\n"); //cazul gresit
            }
        }
        if(strncmp(buf, "enter_library",13) == 0) //comanda enter_library
        {
            if (cookie[0] != ' ' && login == 1) { //daca cookie-ul de logare este nenul
                char message[1500];
                memset(message,0,1500);
                sprintf(message, "GET /api/v1/tema/library/access HTTP/1.1\r\nHost: "
                    "34.246.184.49\r\nConnection: open\r\nCookie: %s\r\n\r\n", cookie);
                send_to_server(sockfd, message); //trimit mesajul la server cu cookie-ul de logare
                response = receive_from_server(sockfd);
                //primesc un token de acces in biblioteca
                char* token_start = strstr(response, "\"token\":\"");
                if (token_start != NULL) {
                    token_start += strlen("\"token\":\"");

                    // Cauta finalul token-ului
                    char* token_end = strchr(token_start, '"');
                    if (token_end != NULL) {
                        // Extrage token-ul
                        int token_length = token_end - token_start;
                        char token[token_length + 1];
                        strncpy(token, token_start, token_length);
                        token[token_length] = '\0';
                        strcpy(cookie_biblioteca, token); //salvez tokenul in cookie_biblioteca
                    }
                }
                printf("SUCCESS! Entering the library!\n"); //afisez mesaj de succes
            }
            else
            {
                printf("ERROR! You didn't login!\n"); //daca nu am token de logare, nu sunt logat
            }

        }
        if(strncmp(buf, "get_books",9) == 0) //comanda get_books
        {
            if(cookie_biblioteca[0]!= 0 && login == 1) //daca sunt logat si am cookie de biblioteca
            {
                char message[2048]; //mesaj nou
                memset(message,0,2048); //il setez pe 0
                sprintf(message, "GET /api/v1/tema/library/books HTTP/1.1\r\nHost: 34.246.184.49"
                    "\r\nConnection: open\r\nAuthorization: Bearer %s\r\n\r\n", cookie_biblioteca);
                send_to_server(sockfd, message); //trimit mesajul la server cu token de biblioteca
                response = receive_from_server(sockfd); //primesc raspuns
                char* token = strstr(response, "{\"id\":"); //fac strstr pentru a vedea cartile
                printf("Books on this server are:\n");
                while (token != NULL) {
                    // Extrage id-ul
                    int id;
                    sscanf(token, "{\"id\":%d", &id);

                    // Extrag titlul
                    char* title_start = strstr(token, "\"title\":\"");
                    char title[100];
                    sscanf(title_start, "\"title\":\"%[^\"]\"", title);

                    // Afisez id-ul si titlul
                    printf("id: %d\n", id);
                    printf("title: %s\n", title);
                    printf("\n");
                    // Caut urmatoarea instanta a sirului "{id:"
                    token = strstr(token + 1, "{\"id\":");
                }
                printf("...............\n"); //afisez dupa ce am terminat cu cartile de afisat
            }
            else 
            {
                printf("ERROR! You can't see books. Use command enter_library before!\n"); //daca nu m-am logat
            }
        }
        if(strncmp(buf, "get_books",9) != 0 && strncmp(buf,"get_book", 8) == 0) //get_book
        {   
            if(cookie_biblioteca[0]!=0 && login == 1) { //daca am acces la biblioteca si sunt logat
                
                char id[10];
                printf("id="); //fac id
                fgets(id,sizeof(id),stdin); //citesc id-ul
                int ok = is_valid_number(id); //
                //printf("%d\n",ok);
                if(ok==0 || id[0] == '\n') //verific sa primesc un id valid
                {
                    printf("Not a valid number\n");
                }
                else 
                {
                    int id2=atoi(id); //atoi pentru a converti in numar
                    char message[2048]; //mesaj pentru acest caz
                    memset(message,0,2048);
                    sprintf(message, "GET /api/v1/tema/library/books/%d HTTP/1.1\r\nHost: 34.246.184.49"
                        "\r\nConnection: open\r\nAuthorization: Bearer %s\r\n\r\n", id2, cookie_biblioteca);
                    send_to_server(sockfd, message); //trimit la server acces la carte
                    response = receive_from_server(sockfd); //primesc raspuns
                    char *carte = strstr(response, "{");
					JSON_Value *valoare = json_parse_string(carte); //fac un obiect JSON
			        JSON_Object *obiect = json_value_get_object(valoare);
                    if(json_object_get_string(obiect,"error")!=NULL) {
                        printf("ERROR! No book was found!\n"); //nu a gasit carte cu acel id
                    }
                    else {
                        printf("\nBook with id %d:\n",id2); //afisez cartea cu id, titlu, autor, gen, publicare
                        printf("title = %s\n", json_object_get_string(obiect, "title"));
                        printf("author = %s\n", json_object_get_string(obiect, "author"));
                        printf("genre = %s\n", json_object_get_string(obiect, "genre"));
                        printf("publisher = %s\n", json_object_get_string(obiect, "publisher"));
                        printf("page_count = %.0lf\n", json_object_get_number(obiect, "page_count")); //si nr pagini
                    }

                }    
            }
            else 
            {
                printf("ERROR! User didn't enter the library! Use command enter_library!\n"); //nu a intrat in biblioteca
            }
        }
        if(strncmp("add_book",buf,8) == 0) //adaug carte
        {
            if(cookie_biblioteca[0]!=0 && login == 1) //daca am acces la biblioteca
            {
                int validare_date = 0;
                char title[1000],author[1000],genre[1000],publisher[1000],page_count[1000];
                printf("title="); 
                fgets(title,sizeof(title),stdin); //citesc titlu
                if(title[0] == '\n') //validez datele
                {
                    validare_date = 1;
                }
                title[strlen(title)-1]='\0';
                printf("author="); 
                fgets(author,sizeof(title),stdin); //citesc autor
                if(author[0] == '\n')
                {
                    validare_date = 1; //date validate
                }
                author[strlen(author)-1]='\0'; // pun terminator de sir
                printf("genre=");
                fgets(genre,sizeof(title),stdin);
                if(genre[0] == '\n')
                {
                    validare_date = 1; //valideaza date
                }
                genre[strlen(genre)-1]='\0';
                printf("publisher=");
                fgets(publisher ,sizeof(title),stdin); //citesc locul publicarii
                if(publisher[0] == '\n')
                {
                    validare_date = 1; //validez date
                }
                publisher[strlen(publisher)-1]='\0';
                printf("page_count=");
                fgets(page_count ,sizeof(title),stdin);
                if(page_count[0] == '\n')
                {
                    validare_date = 1;
                }
                page_count[strlen(page_count)-1]='\0';
                if(is_valid_number(page_count) == 0)
                {
                    validare_date = 1; //ma asigur ca numarul de pagini este valid
                }
                if(validare_date == 1) //daca datele validate nu sunt bune
                {
                    printf("ERROR! Data introduced are bad formatted!\n"); //nu adaug cartea
                }
                else
                {
                    char json_string[4780];
                    int nr_pagini = atoi(page_count); //transform nr_pagini la int
                    char message[7000];
                    memset(message,0,7000); //mesajul este setat pe 0.
                    sprintf(json_string, "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"page_count\":%d,\"publisher\":\"%s\"}", 
                        title, author, genre, nr_pagini, publisher);
                    sprintf(message, "POST /api/v1/tema/library/books HTTP/1.1\r\nHost: 34.246.184.49\r\nConnection: open\r\nAuthorization: "
                        "Bearer %s\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", cookie_biblioteca, strlen(json_string), json_string);
                    send_to_server(sockfd, message); //imi creez mesajul si il trimit la server
                    response = receive_from_server(sockfd); //primesc raspunsul
                    // Afiseaza sirul JSON rezultat
                    printf("SUCCESS! Book added to your collection!\n"); //am adaugat cartea cu succes
                }
            }
            else
            {
                printf("ERROR! User didn't enter the library! Use command enter_library!\n"); //dau fail daca nu am acces la biblioteca
            }
        }
        if(strncmp("delete_book", buf, 11) == 0)
        {
            if(cookie_biblioteca[0]!=0 && login == 1) //daca am acces la biblioteca si sunt logat
            {
                printf("id= ");
                char id[100];
                fgets(id,sizeof(id),stdin); //citesc id
                if(is_valid_number(id) == 0) //il verific
                {
                    printf("ERROR: You didn't introduce a number.\n");
                }
                else
                {
                    int id2=atoi(id); //convertesc id la int
                    char message[2000];
                    memset(message,0,2000);
                    sprintf(message, "DELETE /api/v1/tema/library/books/%d HTTP/1.1\r\nHost: 34.246.184.49\r\nConnection: "
                        "open\r\nAuthorization: Bearer %s\r\n\r\n",id2, cookie_biblioteca); //imi creez mesajul
                    send_to_server(sockfd, message);
                    response = receive_from_server(sockfd);
                    //printf("%s\n",response);
                    char *found_brace = strstr(response, "{"); //caut eroarea
                    if(found_brace != NULL)
                        printf("ERROR! You introduced wrong id or no books left!\n"); //daca da eroare, afisez ERROR
                    else
                        printf("SUCCESS! Book with id %d deleted.\n",id2); //altfel, cartea a fost stearsa
                }
            }
            else
            {
                printf("ERROR! Use command enter_library if you want to add/delete books!\n"); 
                //daca nu am acces la biblioteca, dau ERROR
            }
        }
        if(strncmp("logout",buf,6) == 0) //logout
        {
            if(cookie[0]==0 || login == 0) //daca nu sunt logat, nu pot fi delogat
            {
                printf("ERROR! You are not logged in!\n"); //dau ERROR
            }
            else
            {
                char message[2200];
                memset(message,0,2200); //setez memoria mesajului pe 0
                sprintf(message, "GET /api/v1/tema/auth/logout HTTP/1.1\r\nHost: 34.246.184.49\r\nConnection: "
                    "close\r\nAuthorization: Bearer %s\r\nCookie: %s\r\n\r\n", cookie, cookie); //creez o cerere de logout
                // Trimite cererea de logout catre server
                send_to_server(sockfd, message); 
                response = receive_from_server(sockfd); //primesc raspuns
                if(strstr(response, "ok")!=NULL) {
                    printf("SUCCESS! You are logged out!\n"); //daca am ok in mesajul de la server, SUCCCESS
                    login = 0;
                    memset(cookie,0,1000); //cookie de logare setat pe 0
                    memset(cookie_biblioteca,0,1000); //cookie de biblioteca setat pe 0
                }
                else
                {
                    printf("ERROR! Can't log out!\n"); //nu am fost logat, deci nu ma pot deloga
                }
            }
        }

    }
    //inchidem conexiunea*/
    close_connection(sockfd);
    return 0;
}
