#ifndef BUFALUMNOS_H
#define BUFALUMNOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 30

// Estructura alumno (almacena Nombre Apellido Nota)
struct alumno {
	char nombre[20];
	char apellido[20];
	float nota;
};

typedef struct alumno alumno;

struct bufferAlumno {
	alumno buffer[BUFFER_SIZE];
	int indice;
};

typedef struct bufferAlumno bufferAlumno;

bufferAlumno *crearBuffer();
void agregarAlumno(bufferAlumno *b, alumno al);
void imprimirBuffer(bufferAlumno *b);
void liberarMemoria(bufferAlumno *b);

#endif