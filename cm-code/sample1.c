int fact(int x){
	/* recursive factorial function */
	if(x > 1)
		return x * fact(x-1);
	else
		return 1;
}

void main(void){
	int x;
	x = input();
	if(x > 0) output(fact(x));
}
