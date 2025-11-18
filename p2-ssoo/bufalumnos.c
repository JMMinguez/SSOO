#include "bufalumnos.h"

bufferAlumno *
crearBuffer()
{
  bufferAlumno *b = (bufferAlumno *) malloc(sizeof(bufferAlumno));
  if (!b) {
    fprintf(stderr, "error: asignacion de memoria");
    exit(EXIT_FAILURE);
  }
  b->indice = 0;
  return b;
}

void
agregarAlumno(bufferAlumno *b, alumno al)
{
  if (b->indice < BUFFER_SIZE) {
    b->buffer[b->indice] = al;
    b->indice++;
  } else {
    fprintf(stderr, "error: buffer lleno");
    exit(EXIT_FAILURE);
  }
}

void
imprimirBuffer(bufferAlumno *b)
{
  if (b->indice == 0){
    printf("\nBuffer vacio");
    return;
  }
  for (int i = 0; i < b->indice; i++) {
    printf("%s %d %.2f\n",
      b->buffer[i].nombre, b->buffer[i].apellido, b->buffer[i].nota);
  }
}

void
liberarMemoria(bufferAlumno *b){
  free(b);
}