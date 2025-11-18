#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bufalumnos.h"

int
main(int argc, char *argv[])
{
  if ((argc - 1) % 3 != 0 || (argc - 1) == 0){
    fprintf(stderr,
    "usage: ./mainbufalumnos NOMBRE APELLIDO NOTA ... \n");
    exit(EXIT_FAILURE);
  }

  bufferAlumno *buffer = crearBuffer();

  for (int i = 0; i < argc; i += 3) {
    alumno al;

    strncpy(al.nombre, argv[i], sizeof(al.nombre) - 1);
		al.nombre[sizeof(al.nombre) - 1] = '\0';
		strncpy(al.apellido, argv[i + 1], sizeof(al.apellido) - 1);
		al.apellido[sizeof(al.apellido) - 1] = '\0';
		al.nota = strtof(argv[i + 2], NULL);	// No utilizo strol ya que puede que se inserten numeros con decimales
		if (al.nota > 10 || al.nota < 0) {
			fprintf(stderr, "error: nota fuera del rango [0-10]\n");
			exit(EXIT_FAILURE);
		}

    agregarAlumno(&buffer, al);
  }

  imprimirBuffer(buffer);

  liberarMemoria(buffer);

  exit(EXIT_SUCCESS);
}