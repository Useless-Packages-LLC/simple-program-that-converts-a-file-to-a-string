
LINE_LENGTH=80

CPPFLAGS= -Iinclude "-DLINE_LENGTH=$(LINE_LENGTH)"

D= _bin _test _include
T= _bin/embed
TEST_T= _test/example

TEST_H_PFX= _include
H_PFX= include
define H
$(H_PFX)/embed.h
endef

MISC= $(TEST_H_PFX)/example.txt

_bin/%: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
_test/%: test/%.c
	$(CC) $(CPPFLAGS) -I_include $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

.PHONY: all clean check headers
all: $(D) | $(T)
clean:
	$(RM) $(T) $(TEST_T) $(MISC)
check: all | $(TEST_T)
	@echo " *** START OF TEST TEST TEST"
	@i=1 r=0; \
	for p in $(TEST_T); \
	do t="test/$${p##_test/}.sh"; \
	printf "   TEST TEST TEST [ %3d ] - %s" $$i "$$t"; \
	i=$$((i+1)); \
	test -e $$t || r=$$?; \
	bash $$t || { r=$$?; printf "\33[91m [FAILED]\033[m"; }; \
	done; \
	echo ""; \
	echo " *** END OF TEST TEST TEST"; \
	exit $$r
headers: $(H)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only $^

$(D):
	mkdir $@
	test -d $@
$(TEST_H_PFX)/example.txt: _bin/embed
	./$^ <$^ >$@

EMBED_H= $(H_PFX)/embed.h

_bin/embed: src/embed.c

_test/example: test/example.c $(EMBED_H) $(TEST_H_PFX)/example.txt


