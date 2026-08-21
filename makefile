define __build
	$(1) -shared -o libswl.dll $(2) -I. $(3)
	$(1) -o swl.o -c $(2) -I.
	ar rcs libswl.a swl.o
	rm *.o
endef

first:
	@echo "make [win/mac]"

win:
	$(call __build,gcc,swl_impl_win32.c,-lgdi32 -lopengl32 -luser32)