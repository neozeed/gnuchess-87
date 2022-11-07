#
# Makefile for the scrollable text output system
#

CFLAGS = -g

LIBS = libScroll.a /users/davidh/test/libCreate/libCreate.a -lX


libScroll.a:	scrollText.o
	ar r libScroll.a scrollText.o
	ranlib libScroll.a

codeview:	codeview.o libScroll.a
	cc $(CFLAGS) -o codeview codeview.o st.o $(LIBS)
