#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#include <stdlib.h>

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);

int ComprobarComando(char *strcomando, char *orden, 
                     char *argumento1, char *argumento2,
                     EXT_ENTRADA_DIR *directorio, 
                     EXT_BLQ_INODOS *ext_blq_inodos, 
                     EXT_DATOS *memdatos, 
                     EXT_SIMPLE_SUPERBLOCK *psup,
                     EXT_BYTE_MAPS *ext_bytemaps,
                     unsigned char *bmap_inodos, 
                     unsigned char *bmap_bloques);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);

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
void RenameFile(EXT_ENTRADA_DIR *directorio, const char *nombre_actual, const char *nombre_nuevo);

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
     
     // Buce de tratamiento de comandos
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);

		 } while (ComprobarComando(comando,orden,argumento1,argumento2, directorio, &ext_blq_inodos, memdatos, &ext_superblock, &ext_bytemaps) !=8);
       
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

//             FUNCIÓN PARA COMPROBAR QUE EL COMANDO INTRODUCIDO ES DISTINTO DE CERO

int ComprobarComando(char *strcomando, char *orden, 
                     char *argumento1, char *argumento2,
                     EXT_ENTRADA_DIR *directorio, 
                     EXT_BLQ_INODOS *ext_blq_inodos, 
                     EXT_DATOS *memdatos, 
                     EXT_SIMPLE_SUPERBLOCK *psup,
                     EXT_BYTE_MAPS *ext_bytemaps,
                     unsigned char *bmap_inodos, 
                     unsigned char *bmap_bloques){
   int numeroComando= 0;
   //Creamos un token con el que dividiremos el comando en orden, argumento1 y argumento2
   char *token;
   token = strtok(strcomando, " ");
   strcpy(orden, token);
   printf("Orden: %s\n", orden);
   
   token = strtok(NULL, " ");
  if(token!=NULL){
    strcpy(argumento1, token);
    printf("Argumento 1: %s\n", argumento1);
    token = strtok(NULL, " ");

    if(token!=NULL){
        strcpy(argumento2, token);
        printf("Argumento 2: %s\n", argumento2);
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
            Printbytemaps(ext_bytemaps);
            numeroComando = 3;
         }
         else if(strcmp(orden,"rename")==0){
            RenameFile(directorio, argumento1, argumento2);
            numeroComando = 4;  
         }
         else if(strcmp(orden,"imprimir")==0){
            Imprimir(directorio, ext_blq_inodos, memdatos, argumento1);
            numeroComando = 5;
         }
         else if (strcmp(orden, "remove") == 0) {
            RemoveFile(directorio, ext_blq_inodos, ext_bytemaps, psup, argumento1, fent);
            numeroComando = 6;
         }
         else if (strcmp(orden, "copy") == 0) {
            CopyFile(directorio, ext_blq_inodos, ext_bytemaps, psup, memdatos, argumento1, argumento2, fent);
            numeroComando = 7;
         }
         else if (strcmp(orden, "salir\n") == 0) {
            if (grabardatos) {
                GrabarDatos(memdatos, fent);
                grabardatos = 0;
            } 
            fclose(fent);
            numeroComando = 8;
        }
         else{
            printf("ERROR. Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
         }
   return numeroComando;
}
//
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
    // Muestra la información del superbloque
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


int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_DATOS *memdatos, char *nombre){   
    int i,j;
   unsigned int blnumber;
   //EXT_DATOS datosFichero[MAX_NUMS_BLOQUE_INODO];
   i= BuscaFich(directorio,inodos,nombre);
   if(i==-1){
      printf("No se encontro el archivo\n");
   }
    if(i>0){
      j=0;
      do{
         blnumber= inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j];
         if(blnumber!= NULL_BLOQUE){
            //datosFichero[j]=memdatos[blnumber-PRIM_BLOQUE_DATOS];
            printf("%s ",memdatos[blnumber-PRIM_BLOQUE_DATOS].dato);
            j++;
         }

      }while((blnumber!= NULL_BLOQUE)&& (j<MAX_NUMS_BLOQUE_INODO));
      printf("\n");
   }
   return -1;
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

void Printbytemaps(EXT_BYTE_MAPS *bytemaps) {
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

void RenameFile(EXT_ENTRADA_DIR *directorio, const char *nombre_actual, const char *nombre_nuevo) {
    int encontrado = 0;

    // Buscar el archivo con el nombre actual
    for (int i = 0; i < 20; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_actual) == 0) {
            encontrado = 1;

            // Verificar si el nuevo nombre ya existe
            for (int j = 0; j < 20; j++) {
                if (strcmp(directorio[j].dir_nfich, nombre_nuevo) == 0) {
                    printf("Error: El nuevo nombre ya existe.\n");
                    return;
                }
            }

            // Renombrar el archivo
            strncpy(directorio[i].dir_nfich, nombre_nuevo, sizeof(directorio[i].dir_nfich) - 1);
            directorio[i].dir_nfich[sizeof(directorio[i].dir_nfich) - 1] = '\0'; // Asegurar terminación
            printf("Archivo renombrado exitosamente.\n");
            return;
        }
    }

    if (!encontrado) {
        printf("Error: Archivo con el nombre actual no encontrado.\n");
    }
}

void RemoveFile(EXT_ENTRADA_DIR *directorio, EXT_SIMPLE_INODE *inodos, unsigned char *bmap_inodos, unsigned char *bmap_bloques, const char *nombre_archivo) {
    for (int i = 0; i < 20; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_archivo) == 0) {
            unsigned short int inodo_num = directorio[i].dir_inodo;

            // Liberar los bloques asignados al inodo
            for (int j = 0; j < 7; j++) {
                if (inodos[inodo_num].i_nbloque[j] != 0xFFFF) {
                    bmap_bloques[inodos[inodo_num].i_nbloque[j]] = 0;
                    inodos[inodo_num].i_nbloque[j] = 0xFFFF;
                }
            }

            // Marcar el inodo como libre
            bmap_inodos[inodo_num] = 0;
            inodos[inodo_num].size_fichero = 0;

            // Eliminar la entrada del directorio
            directorio[i].dir_nfich[0] = '\0';
            directorio[i].dir_inodo = 0xFFFF;

            printf("Archivo eliminado exitosamente.\n");
            return;
        }
    }

    printf("Error: Archivo no encontrado.\n");
}

void CopyFile(EXT_ENTRADA_DIR *directorio, EXT_SIMPLE_INODE *inodos, unsigned char *bmap_inodos, unsigned char *bmap_bloques, const char *nombre_origen, const char *nombre_destino) {
    int origen_idx = -1, destino_idx = -1;

    // Buscar el archivo origen
    for (int i = 0; i < 20; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_origen) == 0) {
            origen_idx = i;
            break;
        }
    }
    if (origen_idx == -1) {
        printf("Error: Archivo origen no encontrado.\n");
        return;
    }

    // Verificar si el destino ya existe
    for (int i = 0; i < 20; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre_destino) == 0) {
            printf("Error: Archivo destino ya existe.\n");
            return;
        }
        if (directorio[i].dir_nfich[0] == '\0' && destino_idx == -1) {
            destino_idx = i; // Guardar la primera entrada vacía
        }
    }
    if (destino_idx == -1) {
        printf("Error: No hay espacio en el directorio para la copia.\n");
        return;
    }

    // Buscar el primer inodo libre
    int inodo_libre = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (bmap_inodos[i] == 0) {
            inodo_libre = i;
            break;
        }
    }
    if (inodo_libre == -1) {
        printf("Error: No hay inodos disponibles.\n");
        return;
    }

    // Copiar contenido del archivo
    EXT_SIMPLE_INODE *inodo_origen = &inodos[directorio[origen_idx].dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos[inodo_libre];
    *inodo_destino = *inodo_origen;

    // Asignar nuevos bloques para la copia
    for (int i = 0; i < 7 && inodo_origen->i_nbloque[i] != 0xFFFF; i++) {
        for (int j = 0; j < MAX_BLOQUES_PARTICION; j++) {
            if (bmap_bloques[j] == 0) {
                bmap_bloques[j] = 1;
                inodo_destino->i_nbloque[i] = j;
                break;
            }
        }
    }

    // Marcar el inodo como ocupado
    bmap_inodos[inodo_libre] = 1;

    // Crear la entrada en el directorio
    strncpy(directorio[destino_idx].dir_nfich, nombre_destino, sizeof(directorio[destino_idx].dir_nfich) - 1);
    directorio[destino_idx].dir_nfich[sizeof(directorio[destino_idx].dir_nfich) - 1] = '\0';
    directorio[destino_idx].dir_inodo = inodo_libre;

    printf("Archivo copiado exitosamente.\n");
}
