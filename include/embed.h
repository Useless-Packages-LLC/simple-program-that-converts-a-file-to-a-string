#ifndef USELESS_PACKAGES_EMBED_H
#define USELESS_PACKAGES_EMBED_H

#include <stddef.h>

struct embedded_string {
	const unsigned char *text;
	const size_t length;
};

#endif
