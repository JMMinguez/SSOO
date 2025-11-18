#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estructura alumno (almacena Nombre Apellido Nota)
struct alumno {
	char nombre[20];
	char apellido[20];
	float nota;
};

typedef struct alumno alumno;

// Nodo alumno (contiene alumno y un puntero al siguiente)
struct nodoAlumno {
	alumno al;
	struct nodoAlumno *siguiente;
};

typedef struct nodoAlumno nodoAlumno;

nodoAlumno *crearNodo(alumno al);

void agregarLista(nodoAlumno **lista, alumno al);
void imprimirSuspensos(nodoAlumno *lista);
void imprimirAprobados(nodoAlumno *lista);

float notaMedia(nodoAlumno *lista);

void liberarLista(nodoAlumno *lista);

#endif
