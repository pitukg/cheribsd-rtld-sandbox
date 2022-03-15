#include <math.h>
#include <stdlib.h>
#include <string.h>

static const char *message = "Hello, world!";
static void *p;

const char *hello_world_1(void) {
	p = malloc(1);
	return message;
}

const char *hello_world_2(void) {
	free(p);
	return message;
}

const char *hello_world_3(void) {
	const char buf1[] = "asd", buf2[] = "asd";
	(void) memcmp(buf1, buf2, 4);
	return message;
}

const char *hello_world_4(void) {
	char buf[4];
	(void) memcpy(buf, message, 4);
	return message;
}

const char *hello_world_5(void) {
	char buf[4];
	(void) memset(buf, 0, 4);
	return message;
}

const char *hello_world_6(void) {
	char buf[] = "asd";
	(void) strlen(buf);
	return message;
}

const char *hello_world_7(void) {
	(void) atof("3.14");
	return message;
}

const char *hello_world_8(void) {
	int exp;
	(void) frexp(3.14, &exp);
	return message;
}

const char *hello_world_9(void) {
	double i;
	(void) modf(3.14, &i);
	return message;
}

const char *hello_world_10(void) {
	(void) pow(3, 2);
	return message;
}
