# A simple test for the minimal standard C++ library
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ndcli_android

LOCAL_SRC_FILES := test-libstl.cpp \
../../src/nd_common/src/bintree.c \
../../src/nd_common/src/callstack.c \
../../src/nd_common/src/common.c \
../../src/nd_common/src/nd_cmdline.c \
../../src/nd_common/src/nd_handle.c \
../../src/nd_common/src/nd_mempool.c \
../../src/nd_common/src/nd_mutex.c \
../../src/nd_common/src/nd_static_alloc.c \
../../src/nd_common/src/nd_str.c \
../../src/nd_common/src/nd_timer.c \
../../src/nd_common/src/nd_trace.c \
../../src/nd_common/src/nd_unix.c \
../../src/nd_common/src/nd_win.c \
../../src/nd_common/src/nd_xml.c \
../../src/nd_common/src/nddir.c \
../../src/nd_common/src/node_mgr.c \
../../src/nd_common/src/recbuf.c \
../../src/nd_common/src/source_log.c \
../../src/nd_net/src/ipraw.c \
../../src/nd_net/src/nd_msgentry.c \
../../src/nd_net/src/nd_net.c \
../../src/nd_net/src/nd_netobj.c \
../../src/nd_net/src/nd_netui.c \
../../src/nd_net/src/nd_socket.c \
../../src/nd_net/src/nd_tcp.c \
../../src/nd_net/src/nd_udp.c \
../../src/nd_net/src/net_srv.c \
../../src/nd_net/src/proxy_cli.c \
../../src/nd_net/src/udt_icmp.c \
../../src/nd_net/src/udt_net.c \
../../src/nd_net/src/udt_socket.c \
../../src/nd_net/src/udt_srv.c \
../../src/nd_crypt/rsa/desc.c \
../../src/nd_crypt/rsa/digit.c \
../../src/nd_crypt/rsa/md2c.c \
../../src/nd_crypt/rsa/md5c.c \
../../src/nd_crypt/rsa/nn.c \
../../src/nd_crypt/rsa/prime.c \
../../src/nd_crypt/rsa/r_dh.c \
../../src/nd_crypt/rsa/r_encode.c \
../../src/nd_crypt/rsa/r_enhanc.c \
../../src/nd_crypt/rsa/r_keygen.c \
../../src/nd_crypt/rsa/r_random.c \
../../src/nd_crypt/rsa/r_stdlib.c \
../../src/nd_crypt/rsa/rsa.c \
../../src/nd_crypt/src/nd_cryptfile.c \
../../src/nd_crypt/src/nd_pubkey.c \
../../src/nd_crypt/src/ndcrypt.c \
../../src/nd_crypt/src/ndrsa.c \
../../src/nd_crypt/src/tea.c \
../../src/ndclient/c_api/msg_format.cpp \
../../src/ndclient/c_api/nd_connector.c \
../../src/ndclient/c_api/nd_exch_key.cpp \
../../src/ndclient/cpp_api/nd_client.cpp \
../../src/ndapplib/nd_msgpack.cpp \
../../src/ndapplib/nd_datatransfer.cpp


LOCAL_CFLAGS := --debug -DDEBUG -DND_DEBUG -D__LINUX__ -DND_UNIX -DND_ANDROID
LOCAL_CPPFLAGS:= --debug -DDEBUG -DND_DEBUG -D__LINUX__ -DND_UNIX -DND_ANDROID -frtti 

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include


include $(BUILD_SHARED_LIBRARY)
