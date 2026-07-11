void do_tests() {
    object *obs = previous_object(-1);
    
    for (int i = 0; i < sizeof(obs); i++) {
	ASSERT(obs[i] == previous_object(i));
    }
}
