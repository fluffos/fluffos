int
main(string a)
{
    object ret;
    mixed code;

    if (file_size("/tmp_eval_file.c") != -1)
	rm ("/tmp_eval_file.c");
    if (find_object("/tmp_eval_file"))
	destruct(find_object("/tmp_eval_file"));

    write_file("/tmp_eval_file.c", "mixed eval() { " + a + "; }\n");

    ret = load_object("/tmp_eval_file");

    dump_prog(ret, 1, "/CODE_TMP_FILE");
    code = read_file( "/CODE_TMP_FILE");
    rm("/CODE_TMP_FILE");
    rm("/tmp_eval_file.c");

    code = explode(code, ";;;  *** Disassembly ***\n");
    code = code[1];
    write(code[0..strlen(code)-1]);
    return 1;
}
