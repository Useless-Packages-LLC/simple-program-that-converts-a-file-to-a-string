#include <embed.h>

struct embedded_string a =
#include <example.txt>

#include <unistd.h>
#include <errno.h>

int main() {
	return (write(1, a.text, a.length) < 0) && errno;
}

