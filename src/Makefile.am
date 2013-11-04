NULL =
bin_PROGRAMS = tcp_server_sample 
tcp_server_sample_SOURCES = \
	main.c \
	app_debug.h \
	app_debug.c \
	packet_listener.h \
	packet_listener.c \
	stub_moatapp.c \
	net_util.h \
	net_util.c \
	$(NULL)
AM_CFLAGS = -I$(top_builddir)/include
AM_LDFLAGS = -lev
