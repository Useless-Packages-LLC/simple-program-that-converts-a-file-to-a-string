#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>

// Compile with -DLINE_LENGTH=# to set the line length

#ifdef LINE_LENGTH
#define BUF_SIZE (LINE_LENGTH - 12)
#else
#define BUF_SIZE 116
#endif

#define USE_ESC_C(c) return (*s = (struct string) { "\\" c, 2 }, 1)
#define USE_ESC(c) USE_ESC_C(#c)

struct string {
	char *s;
	size_t n;
};

struct string to_octal(unsigned char a) {
	if (a < 8) {
		char *s = malloc(2);
		s[0] = '\\', s[1] = '0' + a;
		return (struct string) { s, 2 };
	} if (a < 64) {
		char *s = malloc(3);
		s[0] = '\\', s[1] = '0' + (a >> 3), s[2] = '0' + (a & 7);
		return (struct string) { s, 3 };
	}

	char *s = malloc(4);
	s[0] = '\\', s[1] = '0' + (a >> 6), s[2] = '0' + (a >> 3 & 7), s[3] = '0' + (a & 7);
	return (struct string) { s, 4 };
}

int handle_escape_codes(struct string *s, unsigned char a) {
	switch (a) {
		case 7: USE_ESC(a);
		case 8: USE_ESC(b);
		case 9: USE_ESC(t); // To make it more readable
		case 10: USE_ESC(n);
		case 11: USE_ESC(v);
		case 12: USE_ESC(f);
		case 13: USE_ESC(r);
		// skip \e
		case 34: USE_ESC_C("\"");
		case 39: USE_ESC_C("'");
		case 63: USE_ESC_C("?"); // ??/ logic
		case 92: USE_ESC_C("\\");
		default: return 0;
	}
}

struct write_data {
	struct string *buf;
	struct string *s;
	int *newline;
	int *offset;
};

int write_data(struct write_data *st) {
	if (*st->newline) {
		int offset = *st->offset;
		*st->newline = 0;
		char pad[offset + 5];
		memset(pad, ' ', offset + 1);
		pad[offset + 1] = '\\';
		pad[offset + 2] = '\n';
		pad[offset + 3] = '\t';
		pad[offset + 4] = '\"';
		if (write(1, pad, offset + 5) < 0) {
			perror("write");
			return 1;
		}
	}

	if (st->buf->n + st->s->n > BUF_SIZE) {
		if (write(1, st->buf->s, st->buf->n) < 0) {
			perror("write");
			return 1;
		} if (write(1, "\"", 1) < 0) {
			perror("write");
			return 1;
		}
		
		*st->newline = 1, *st->offset = BUF_SIZE - st->buf->n, st->buf->n = 0;
	}

	memcpy(st->buf->s + st->buf->n, st->s->s, st->s->n);
	st->buf->n += st->s->n;
	return 0;
}

int main() {
	unsigned char a;
	unsigned char last_octal = ~0;
	char buf_str[BUF_SIZE];
	struct string buf = { buf_str, 0 };
	int read_result, newline = 0, offset = 0;
	size_t file_size = 0;
	if (write(1, "{\t\"", 3) < 0) {
		perror("write");
		return 1;
	} while ((read_result = read(0, &a, 1)) > 0) {
		struct string s;
		int data_str = 0;
		if ((' ' > a || a > '~') && 9 != a)
			s = to_octal(a), last_octal = a, data_str = 0;
		else if (handle_escape_codes(&s, a))
			data_str = 1, last_octal = ~0;
		else {
			if ('0' <= a && a < '8' && last_octal < 64) {
				s = (struct string) { malloc(3), 3 };
				s.s[0] = '"', s.s[1] = '"', s.s[2] = a;
			} else s = (struct string) { malloc(1), 1 }, *s.s = a;
			last_octal = ~0;
		}

		struct write_data st = {
			.buf=&buf,
			.s=&s,
			.newline=&newline,
			.offset=&offset,
		};

		if (write_data(&st))
			return 1;

		if (!data_str)
			free(s.s);
		++file_size;
	}

	if (read_result < 0) {
		perror("read");
		if (write(1, buf.s, buf.n) < 0)
			perror("write");
		return 1;
	}

	if (write(1, buf.s, buf.n) < 0) {
		perror("write");
		return 1;
	}

	if (write(1, "\",\n\t" + newline, 4 - newline) < 0) {
		perror("write");
		return 1;
	}

	char szbuf[BUF_SIZE + 4];
	size_t szlen;
	if (file_size > 0x9248e25e6c) { // ~585 GiB
		if ((szlen = snprintf(szbuf, 21, "%zx", file_size)) < 0) {
			perror("sprintf");
			return 1;
		} if (write(1, "0x", 2) < 0) {
			perror("write");
			return 1;
		}
	} else if ((szlen = snprintf(szbuf, 21, "%zd", file_size)) < 0) {
		perror("sprintf");
		return 1;
	}

	szbuf[szlen] = 'u';
	memset(szbuf + szlen + 1, 'l', 2);
	memset(szbuf + szlen + 3, ' ', BUF_SIZE - szlen - 1);
	szbuf[BUF_SIZE+2] = '}';
	szbuf[BUF_SIZE+3] = ';';
	if (write(1, szbuf, BUF_SIZE + 4) < 0) {
		perror("write");
		return 1;
	}
	
	return 0;
}
