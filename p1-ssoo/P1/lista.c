#include "lista.h"

// Crear nodo tipo alumno
nodoAlumno *
crearNodo(alumno al)
{
	// Reservar memoria dinamica
	nodoAlumno *nuevoNodo = (nodoAlumno *) malloc(sizeof(nodoAlumno));

	if (nuevoNodo == NULL) {
		fprintf(stderr, "error: asignacion de memoria");
		exit(EXIT_FAILURE);
	}

	nuevoNodo->al = al;
	nuevoNodo->siguiente = NULL;
	return nuevoNodo;
}

// Doble puntero para modificar el puntero original en caso de estar vacio
void
agregarLista(nodoAlumno **lista, alumno al)
{
	nodoAlumno *nuevoNodo = crearNodo(al);

	if (*lista == NULL) {
		*lista = nuevoNodo;
	} else {
		nodoAlumno *actual = *lista;

		while (actual->siguiente != NULL) {
			actual = actual->siguiente;
		}
		actual->siguiente = nuevoNodo;
	}
}

// Recorre la lista y va imprimiendo los que tengan nota < 5
void
imprimirSuspensos(nodoAlumno *lista)
{
	nodoAlumno *actual = lista;

	while (actual != NULL) {
		if (actual->al.nota < 5) {
			printf("%s %s\n", actual->al.nombre,
			       actual->al.apellido);
		}
		actual = actual->siguiente;
	}
}

// Recorre la lista y va imprimiendo los que tengan nota >= 5
void
imprimirAprobados(nodoAlumno *lista)
{
	nodoAlumno *actual = lista;

	while (actual != NULL) {
		if (actual->al.nota >= 5) {
			printf("%s %s\n", actual->al.nombre,
			       actual->al.apellido);
		}
		actual = actual->siguiente;
	}
}

// Recorre la lista y va almaacenando la suma total de las notas y el numero de estas
float
notaMedia(nodoAlumno *lista)
{
	nodoAlumno *actual = lista;
	float suma = 0, numNotas = 0;

	while (actual != NULL) {
		suma += actual->al.nota;
		numNotas++;
		actual = actual->siguiente;
	}
	return (suma / numNotas);
}

// Libera toda la memoria asignada
void
liberarLista(nodoAlumno *lista)
{
	nodoAlumno *actual = lista;

	while (actual != NULL) {
		nodoAlumno *temp = actual;

		actual = actual->siguiente;
		free(temp);
	}
}
