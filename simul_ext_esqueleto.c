#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#include <stdlib.h>

#define LONGITUD_COMANDO 100

void ImprimirByteMaps(EXT_BYTE_MAPS *ext_bytemaps);

int ComprobarComando(char *strcomando, char *orden, 
                     char *argumento1, char *argumento2,
                     EXT_ENTRADA_DIR *directorio, 
                     EXT_BLQ_INODOS *ext_blq_inodos, 
                     EXT_DATOS *memdatos, 
                     EXT_SIMPLE_SUPERBLOCK *psup,
                     EXT_BYTE_MAPS *ext_bytemaps,
                     FILE *fich);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
void Renombrar(EXT_ENTRADA_DIR *directorio, const char *nombre_actual,
               const char *nombre_nuevo);


int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
void Borrar(EXT_ENTRADA_DIR *directorio, EXT_SIMPLE_INODE *inodos,
                 unsigned char *bmap_inodos, unsigned char *bmap_bloques,
                  const char *nombre_archivo);

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
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
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Bucle de tratamiento de comandos
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);

		 } while (ComprobarComando(comando,orden,argumento1,argumento2, directorio, &ext_blq_inodos, memdatos, &ext_superblock, &ext_bytemaps, fent) !=8);
       
         //...
         // Escritura de metadatos en comandos rename, remove, copy     
         //Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
         //GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
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

//             FUNCIÓN PARA COMPROBAR QUE EL COMANDO INTRODUCIDO ES DISTINTO DE CERO
//              Funciona como una especie de switch, en el que dependiendo del valor introducido por terminal se ejecutará un comando concreto.
int ComprobarComando(char *strcomando, char *orden, 
                     char *argumento1, char *argumento2,
                     EXT_ENTRADA_DIR *directorio, 
                     EXT_BLQ_INODOS *ext_blq_inodos, 
                     EXT_DATOS *memdatos, 
                     EXT_SIMPLE_SUPERBLOCK *psup,
                     EXT_BYTE_MAPS *ext_bytemaps,
                     FILE *fich){
   int numeroComando= 0;
   //Creamos un token con el que dividiremos el comando en orden, argumento1 y argumento2
   char *token;
   token = strtok(strcomando, " ");
   strcpy(orden, token);
   
   token = strtok(NULL, " ");
  if(token!=NULL){
    strcpy(argumento1, token);
    token = strtok(NULL, " ");

    if(token!=NULL){
        strcpy(argumento2, token);
    }
  }
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
            LeeSuperBloque(psup);
            numeroComando = 2;
         }
         else if(strcmp(orden,"bytemaps\n")==0){
            ImprimirByteMaps(ext_bytemaps);
            numeroComando = 3;
         }
         else if(strcmp(orden,"rename")==0){
            Renombrar(directorio, argumento1, argumento2);
            numeroComando = 4;  
         }
         else if(strcmp(orden,"imprimir")==0){
            Imprimir(directorio, ext_blq_inodos, memdatos, argumento1);
            numeroComando = 5;
         }
         else if (strcmp(orden, "remove") == 0) {
            Borrar(directorio, ext_blq_inodos, ext_bytemaps, psup, argumento1);
            numeroComando = 6;
         }
         else if (strcmp(orden, "copy") == 0) {
            Copiar(directorio,ext_blq_inodos, ext_bytemaps, psup, memdatos, argumento1, argumento2, fich);
            numeroComando = 7;
         }
         else if (strcmp(orden, "salir\n") == 0) {
            numeroComando = 8;
        }
         else{
            printf("ERROR. Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
         }
   return numeroComando;
}

//            FUNCIÓN PARA MOSTRAR LA INFORMACIÓN DEL SUPERBLOQUE
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
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
    //Buscamos en el archivo para escribir el bloque de inodos (3)
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET); 
    if (fwrite(inodos, sizeof(EXT_BLQ_INODOS), 1, fich) != 1) {
        printf("Error al escribir la lista de inodos");
        return;
    }

    //Nos posicionamos para escribir en el bloque del directorio (4)
    fseek(fich, 512 * 4, SEEK_SET);
    if (fwrite(directorio, 20 * sizeof(EXT_ENTRADA_DIR), 1, fich) != 1) {
        printf("Error al escribir el directorio");
        return;
    }

    printf("Inodos y directorio grabados correctamente en el archivo.\n");
}

//          FUNCIÓN PARA IMPRIMIR POR PANTALLA LOS ARCHIVOS DEL DIRECTORIO
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_DATOS *memdatos, char *nombre){   
    int i,j;
   unsigned int blnumber;
   //Tamaño máximo posible que puede alcanzar el fichero
   EXT_DATOS datosFichero[MAX_NUMS_BLOQUE_INODO];
   i= BuscaFich(directorio,inodos,nombre);
    if(i>0){
      j=0;
      do{
         blnumber = inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j];
         if(blnumber != NULL_BLOQUE){
            datosFichero[j] = memdatos[blnumber - PRIM_BLOQUE_DATOS];
         }
         j++;
         }while((blnumber != NULL_BLOQUE) && (j<MAX_NUMS_BLOQUE_INODO));
         printf("%s\n", datosFichero);
    //No se encontró
   return -2;
    }
}


//             ESTA FUNCIÓN SIRVE PARA LISTAR POR PANTALLA TODOS LOS FICHEROS

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
//Empezamos en 1 en vez de en cero ya que el enunciado pide que
// la entrada especial del directorio raíz no se muestre.
   for(int i=1;i<MAX_FICHEROS;i++){
    if(directorio[i].dir_inodo!=NULL_INODO){
      printf("%s     ", directorio[i].dir_nfich);
      printf("tamanio: %i     ", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
      printf("inodo: %i     ", directorio[i].dir_inodo);
      printf("bloques: ");
    
      for(int j=0;j<MAX_NUMS_BLOQUE_INODO;j++){
        if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]!=NULL_BLOQUE)
         printf(" %i", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
      }
    
      printf("\n");
    }
   }
}
//          ESTA FUNCIÓN SIRVE PARA BUSCAR UN FICHERO DENTRO DEL DIRECTORIO
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre){
                
   int num=-1;
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
    fseek(fich, PRIM_BLOQUE_DATOS*SIZE_BLOQUE, SEEK_SET);
    fwrite(memdatos, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
    fseek(fich, PRIM_BLOQUE_DATOS*SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
    fseek(fich, PRIM_BLOQUE_DATOS*SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_superblock, MAX_BLOQUES_DATOS, SIZE_BLOQUE, fich);
}

void ImprimirByteMaps(EXT_BYTE_MAPS *bytemaps) {
    printf("Bytemaps:\n");

    // Mostrar los primeros 25 elementos del bytemap de bloques
    printf("  Bytemap de bloques:\n  ");
    for (int i = 0; i < 25; i++) {
        printf("%d ", bytemaps->bmap_bloques[i]);
    }
    printf("\n");

    // Mostrar el contenido del bytemap de inodos
    printf("  Bytemap de inodos:\n  ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}
//              FUNCIÓN PARA RENOMBRAR UN FICHERO DENTRO DEL DIRECTORIO
void Renombrar(EXT_ENTRADA_DIR *directorio, const char *nombre_actual, const char *nombre_nuevo) {
    int encontrado = 0;

    //Buscamos el nombre del archivo actual
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_actual) == 0) {
            encontrado = 1;

            //Comprobamos si existe ese fichero dentro del directorio
            for (int j = 0; j < MAX_FICHEROS; j++) {
                if (strcmp(directorio[j].dir_nfich, nombre_nuevo) == 0) {
                    printf("Error: El nuevo nombre ya existe.\n");
                    return;
                }
            }
            //En caso de no existir, renombramos el archivo
            strncpy(directorio[i].dir_nfich, nombre_nuevo, sizeof(directorio[i].dir_nfich) - 1);
            directorio[i].dir_nfich[sizeof(directorio[i].dir_nfich) - 1] = '\0';
            printf("Archivo renombrado exitosamente.\n");
            return;
        }
    }
    //En caso de que no encuentre el fichero que queremos renombrar
    if (!encontrado) {
        printf("Error: Archivo con el nombre actual no encontrado.\n");
    }
}
//          FUNCIÓN PARA ELIMINAR UN FICHERO DENTRO DEL DIRECTORIO
void Borrar(EXT_ENTRADA_DIR *directorio, EXT_SIMPLE_INODE *inodos, unsigned char *bmap_inodos, unsigned char *bmap_bloques, const char *nombre_archivo) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_archivo) == 0) {
            int inodo_num = directorio[i].dir_inodo;

            // Liberamos los bloques que tenía ocupados
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos[inodo_num].i_nbloque[j] != NULL_INODO) {
                    bmap_bloques[inodos[inodo_num].i_nbloque[j]] = 0;
                    inodos[inodo_num].i_nbloque[j] = NULL_BLOQUE;
                }
            }

            // Marcamos el inodo como disponible
            bmap_inodos[inodo_num] = 0;
            inodos[inodo_num].size_fichero = 0;

            // Eliminamos la entrada del directorio
            directorio[i].dir_nfich[0] = '\0';
            directorio[i].dir_inodo = NULL_INODO;

            printf("Archivo eliminado exitosamente.\n");
            return;
        }
    }

    printf("Error: Archivo no encontrado.\n");
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    //Creamos los índices
    int idx_origen = -1;
    int idx_destino = -1;
    int inodo_origen = -1;
    int inodo_destino = -1;
    int bloques_origen[MAX_NUMS_BLOQUE_INODO];
    int bloques_libres[MAX_NUMS_BLOQUE_INODO];  
    int i, j, k;
    int num_bloques_origen = 0;  // Contador de bloques que ocupa el archivo origen

    //Buscamos el archivo de origen en el directorio
    i = 0;
    //Si encuentra el archivo, el índice de origen es el del valor de i y 
    // el inodo guarda la dirección del inodo dentro del directorio.
    while (i < MAX_FICHEROS && idx_origen == -1) {
        if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
            idx_origen = i;
            inodo_origen = directorio[i].dir_inodo;
        }
        i++;
    }
    //En caso de que el archivo de origen no se encuentre, se muestra por pantalla un error.
    if (idx_origen == -1) {
        printf("Error: Archivo origen no encontrado.\n");
        return -1;
    }

    //Contamos los bloques que ocupa el archivo de origen
    for (i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        //Contamos solo cuando sabemos que no es 0xFFFF
        if (inodos->blq_inodos[inodo_origen].i_nbloque[i] != NULL_BLOQUE) {
            bloques_origen[num_bloques_origen] = inodos->blq_inodos[inodo_origen].i_nbloque[i];
            num_bloques_origen++;
        }
    }

    // Buscamos los bloques libres suficientes como para guardar los datos
    k = 0;
    i = PRIM_BLOQUE_DATOS;
    while (i < MAX_BLOQUES_PARTICION && k < num_bloques_origen) {
        if (ext_bytemaps->bmap_bloques[i] == 0) {  
            bloques_libres[k] = i;
            k++;
        }
        i++;
    }
    //En caso de que no haya suficientes bloques, se muestra por pantalla
    if (k < num_bloques_origen) {
        printf("Error: No hay suficientes bloques libres.\n");
        return -1;
    }

    //Buscamos un inodo libre para el archivo destino
    i = 0;
    while (i < MAX_INODOS && inodo_destino == -1) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
        //Marcamos el inodo como ocupado
            ext_bytemaps->bmap_inodos[i] = 1;
            inodo_destino = i;
        }
        i++;
    }
    //En caso de que no haya inodos libres:
    if (inodo_destino == -1) {
        printf("Error: No hay inodos libres.\n");
        return -1;
    }

    //Buscamos una entrada libre en el directorio
    i = 0;
    while (i < MAX_FICHEROS && idx_destino == -1) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            idx_destino = i;
            directorio[i].dir_inodo = inodo_destino;
            //Copiamos el nombre del archivo que queremos en el directorio
            strcpy(directorio[i].dir_nfich, nombredestino);  
        }
        i++;
    }
    //En caso de que no haya espacio en el directorio
    if (idx_destino == -1) {
        printf("Error: No hay espacio en el directorio.\n");
        return -1;
    }

    //Copiamos los bloques de datos del archivo de origen al de destino
    for (i = 0; i < num_bloques_origen; i++) {
        
        memcpy(memdatos[bloques_libres[i]].dato, memdatos[bloques_origen[i]].dato, SIZE_BLOQUE);

        //Asignamos el bloque libre al inodo del archivo de destino
        inodos->blq_inodos[inodo_destino].i_nbloque[i] = bloques_libres[i];
    }

    // Actualizamos en el inodo el tamaño del nuevo archivo
    inodos->blq_inodos[inodo_destino].size_fichero = inodos->blq_inodos[inodo_origen].size_fichero;

    //Actualizamos los mapas de bits de bloques y el superbloque
    for (i = 0; i < num_bloques_origen; i++) {
        ext_bytemaps->bmap_bloques[bloques_libres[i]] = 1;
    }

    //Todos los cambios realizados se guardan en el fichero .bin
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarByteMaps(ext_bytemaps, fich);

    printf("Archivo '%s' copiado en '%s'.\n", nombreorigen, nombredestino);
    return 0;
}



