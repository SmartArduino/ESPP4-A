idf_component_register(SRCS "main.c" "media_sys.c" "webrtc.c" "webrtc_http_server.c"
                       EMBED_TXTFILES "ring.aac" "webrtc_test.html"
                                      "certs/servercert.pem" "certs/prvtkey.pem"
                       INCLUDE_DIRS ".")
