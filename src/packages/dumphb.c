#include "dumphb.h"

void dumphb (const char * tfn)
{
	object_t *ob;
	FILE *f;
	const char *fn;

	fn = check_valid_path(tfn, current_object, "dumpallhb", 1);
	if (!fn){
		error("Permiso de escritura denegado sobre '%s'.\n", tfn);
		return;
	}

	f = fopen(fn, "w");
	if (!f){
		error("No se puede abrir el fichero '/%s' en modo escritura.\n", fn);
		return;
	}

	// Ya tenemos el fichero abierto, obtenemos los HB's y los recorremos.
	heart_beats = get_heart_beats();

	for(int i = 0 ; i < heart_beats->size ; i++){
		ob = heart_beats->item[i].u.ob;
		fprintf(f, "%-20s\n", ob->obname);
	}

	// Cerramos ya el fichero.
	fclose(f);
}