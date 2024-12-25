#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#include <stdlib.h>

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2, EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *ext_blq_inodos, EXT_DATOS *memdatos);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
//CASI HECHA              
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
//HECHA, FALTA PULIRLA
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char comando[LONGITUD_COMANDO];
	 char orden[LONGITUD_COMANDO];
	 char argumento1[LONGITUD_COMANDO];
	 char argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     
     //Abrimos el fichero con la función fopen como lectura (r) de binario(b)
     fent = fopen("particion.bin","r+b");

     //Imprimir un error y salir en caso de que el archivo no se
     //abra correctamente
     if(fent == NULL){
      printf("Error al abrir el archivo: particion.bin");
      exit(1);
     }
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     /*POR TERMINAR
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     */
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);

		 } while (ComprobarComando(comando,orden,argumento1,argumento2, directorio, &ext_blq_inodos, memdatos) !=8);
       
         //...
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
         //GrabarByteMaps(&ext_bytemaps,fent);
         //GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(memdatos,fent);
            fclose(fent);
            return 0;
         }
     }
}

//             FUNCIÓN PARA COMPROBAR QUE EL COMANDO INTRODUCIDO ES DISTINTO DE CERO

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2, EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *ext_blq_inodos, EXT_DATOS *memdatos){
   int numeroComando= 0;
   //Creamos un token con el que dividiremos el comando en orden, argumento1 y argumento2
   char *token;
   token = strtok(strcomando, " ");
   strcpy(orden, token);
   printf("Orden: %s\n", orden);
   /*PRUEBA PARA SOLUCIONAR EL CRASHEO DEL PROGRAMA AL INTRODUCIR MAL LOS DATOS AL COMANDO
   if(strcmp(orden, "rename")==0 ||strcmp(orden, "imprimir")==0 ||strcmp(orden, "remove")==0 ||strcmp(orden, "copy")==0){
      token = strtok(NULL, " ");
      strcpy (argumento1, token);
      printf("Argumento 1: %s\n", argumento1);

      if(strcmp(orden, "rename")==0 ||strcmp(orden, "copy")==0){
         token = strtok(NULL, " ");
         strcpy (argumento2, token);
         printf("Argumento 2: %s\n", argumento2);

      }
   }
   else{
      printf("Error en la introducción de comandos\n");
   }
   */
   token = strtok(NULL, " ");
   strcpy(argumento1, token);
      printf("Argumento 1: %s\n", argumento1);
   
   token = strtok(NULL, " ");
   strcpy(argumento2, token);
      printf("Argumento 2: %s\n", argumento2);
   
/*LISTA DE COMANDOS DISPONIBLES

   dir = 1
   info = 2
   bytemaps = 3
   rename = 4
   imprimir = 5
   remove = 6
   copy = 7
   salir = 8
   comando erroneo = 0

*/
//Comparamos la orden ejecutada con todos los casos de comandos posibles
	     if (strcmp(orden,"dir\n")==0) {
            Directorio(directorio,ext_blq_inodos);
            numeroComando = 1;
            }
         else if(strcmp(orden,"info\n")==0){
            numeroComando = 2;
         }
         else if(strcmp(orden,"bytemaps\n")==0){
            numeroComando = 3;
         }
         else if(strcmp(orden,"rename\n")==0){
            numeroComando = 4;
         }
         else if(strcmp(orden,"imprimir\n")==0){
            Imprimir(directorio, ext_blq_inodos, memdatos, argumento1);
            numeroComando = 5;
         }
         else if(strcmp(orden,"remove\n")==0){
            numeroComando = 6;
         }
         else if(strcmp(orden,"copy\n")==0){
            numeroComando = 7;
         }
         else if(strcmp(orden,"salir\n")==0){
            numeroComando = 8;
         }
         else{
            printf("ERROR. Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
         }
   return numeroComando;
}
//
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
    // Mostrar la información del superbloque
    printf("Superbloque:\n");
    printf("  Número total de inodos: %u\n", psup->s_inodes_count);
    printf("  Número total de bloques: %u\n", psup->s_blocks_count);
    printf("  Bloques libres: %u\n", psup->s_free_blocks_count);
    printf("  Inodos libres: %u\n", psup->s_free_inodes_count);
    printf("  Primer bloque de datos: %u\n", psup->s_first_data_block);
    printf("  Tamaño del bloque: %u bytes\n", psup->s_block_size);
}

//             FUNCIÓN PARA GRABAR LOS INODOS Y EL DIRECTORIO

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    // Posicionarse en el archivo para escribir el bloque de inodos
    fseek(fich, 512 * 3, SEEK_SET); // Bloque 3: Lista de inodos
    if (fwrite(inodos, sizeof(EXT_BLQ_INODOS), 1, fich) != 1) {
        perror("Error al escribir la lista de inodos");
        return;
    }

    // Posicionarse para escribir el bloque del directorio
    fseek(fich, 512 * 4, SEEK_SET); // Bloque 4: Directorio
    if (fwrite(directorio, 20 * sizeof(EXT_ENTRADA_DIR), 1, fich) != 1) {
        perror("Error al escribir el directorio");
        return;
    }

    printf("Inodos y directorio grabados correctamente en el archivo.\n");
}


int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre){

   int i,j;
   unsigned int blnumber;
   EXT_DATOS datosfichero[MAX_NUMS_BLOQUE_INODO]; //Tamaño máximo posible del fichero
   i = BuscaFich(directorio, inodos, nombre);
   if(i>0){
      j = 0;
      do{
         blnumber = inodos -> blq_inodos[directorio[i].dir_inodo].i_nbloque[j];
         if(blnumber != NULL_BLOQUE){
            datosfichero[j] = memdatos[blnumber - PRIM_BLOQUE_DATOS];
         }
         j++;
      }while ((blnumber != NULL_BLOQUE) && (j<MAX_NUMS_BLOQUE_INODO));
      printf("%s\n", datosfichero->dato);
   }
   return -2; //No se encontró
}

//             ESTA FUNCIÓN SIRVE PARA LISTAR POR PANTALLA TODOS LOS FICHEROS

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
//Empezamos en 1 en vez de en cero ya que el enunciado pide que
// la entrada especial del directorio raíz no se muestre.
   for(int i=0;i<MAX_FICHEROS;i++){
      printf("%s     ", directorio[i].dir_nfich);
      printf("tamanio: %i     ", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
      printf("inodo: %i     ", directorio[i].dir_inodo);
      printf("bloques: ");
      for(int j=0;j<MAX_NUMS_BLOQUE_INODO;j++){
         printf(" %i", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
      }
      printf("\n");
   }
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre){
   int num=0;
   for(int i=0;i<MAX_FICHEROS;i++){
      if(strcmp(nombre, directorio[i].dir_nfich)==0){
         num = i;
      }
   }
   return num;
}

//             ESTA FUNCIÓN SIRVE PARA GRABAR LOS DATOS

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
    fseek(fich, 4*SIZE_BLOQUE, SEEK_SET);
    fwrite(memdatos, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
    fseek(fich, 4*SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
    fseek(fich, 4*SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_superblock, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}