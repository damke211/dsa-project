# set(COMPONENT_SRCS "user_main.c")
# set(COMPONENT_SRCS "misc.c")
# set(COMPONENT_SRCS "debug.c")
# set(COMPONENT_SRCS "wifi.c")

# register_component()

idf_component_register(SRCS "user_main.c misc.c debug.c wifi.c"
	INCLUDE_DIRS "."
	EMBED_TXTFILES howsmyssl_com_root_cert.pem)
