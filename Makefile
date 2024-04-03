################################################################################
#
# aml_pqserver
#
################################################################################
OUT_DIR ?= .
LOCAL_PATH = $(shell pwd)
LDFLAGS += -Wl,--no-as-needed -lstdc++ -lpthread -lz -ldl -lrt -L$(STAGING_DIR)/usr/lib
CFLAGS += -Wall -Werror -Wno-unknown-pragmas -Wno-format -Wno-format-security -Wno-error=unused-result \
          -O3 -fexceptions -fnon-call-exceptions -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include

MIN_GCC_VERSION = "10"
GCC_VERSION := "`$(CC) -dumpversion | cut -f1 -d'.'`"
IS_GCC_ABOVE_MIN_VERSION := $(shell expr "$(GCC_VERSION)" ">=" "$(MIN_GCC_VERSION)")
ifeq "$(IS_GCC_ABOVE_MIN_VERSION)" "1"
CFLAGS += -std=gnu++20
endif

LIBBINDER_LDFLAGS = -lbinder -llog

LDFLAGS += $(LIBBINDER_LDFLAGS)
CFLAGS += -L $(OUT_DIR)/

################################################################################
# libpq.so - src files
################################################################################

pq_SRCS = \
  $(LOCAL_PATH)/libpq/CPQdb.cpp \
  $(LOCAL_PATH)/libpq/COverScandb.cpp \
  $(LOCAL_PATH)/libpq/CPQControl.cpp  \
  $(LOCAL_PATH)/libpq/CSqlite.cpp  \
  $(LOCAL_PATH)/libpq/SSMAction.cpp  \
  $(LOCAL_PATH)/libpq/SSMHandler.cpp  \
  $(LOCAL_PATH)/libpq/SSMHeader.cpp  \
  $(LOCAL_PATH)/libpq/CDevicePollCheckThread.cpp  \
  $(LOCAL_PATH)/libpq/CPQLog.cpp  \
  $(LOCAL_PATH)/libpq/CPQFile.cpp  \
  $(LOCAL_PATH)/libpq/CEpoll.cpp \
  $(LOCAL_PATH)/libpq/CDynamicBackLight.cpp \
  $(LOCAL_PATH)/libpq/CConfigFile.cpp \
  $(LOCAL_PATH)/libpq/UEventObserver.cpp \
  $(LOCAL_PATH)/libpq/CVdin.cpp \
  $(LOCAL_PATH)/libpq/CAmdolbyVision.cpp \
  $(LOCAL_PATH)/libpq/GammaOperation.cpp \
  $(NULL)

pq_HEADERS = \
	$(LOCAL_PATH)/libpq/include \
	$(NULL)
################################################################################
# libpqclient.so - src files
################################################################################
pqclient_SRCS  = \
	$(LOCAL_PATH)/client/PqClient.cpp \
	$(LOCAL_PATH)/client/CPqClientLog.cpp \
	$(NULL)

pqclient_HEADERS = \
	$(LOCAL_PATH)/client/include \
	$(NULL)
################################################################################
# pqservice - src files
################################################################################
pqservice_SRCS  = \
	$(LOCAL_PATH)/service/main_pqservice.cpp \
	$(LOCAL_PATH)/service/PqService.cpp \
	$(NULL)

################################################################################
# pqtest - src files
################################################################################
pqtest_SRCS  = \
	$(LOCAL_PATH)/test/main_pqtest.cpp \
	$(NULL)

################################################################################
# pqcbtest - src files
################################################################################
pqcbtest_SRCS  = \
	$(LOCAL_PATH)/test/main_callback_test.cpp \
	$(NULL)

# ---------------------------------------------------------------------
#  Build rules
BUILD_TARGETS = $(OUT_DIR)/libpqclient.so $(OUT_DIR)/libpq.so $(OUT_DIR)/pqservice $(OUT_DIR)/pqtest $(OUT_DIR)/pqcbtest

.PHONY: all install uninstall clean

$(OUT_DIR)/libpqclient.so: $(pqclient_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -I$(pqclient_HEADERS) \
	-o $@ $^ $(LDLIBS)

$(OUT_DIR)/libpq.so: $(pq_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -lsqlite3 -I$(pq_HEADERS) \
	-o $@ $^ $(LDLIBS)

$(OUT_DIR)/pqservice: $(pqservice_SRCS) $(OUT_DIR)/libpq.so
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(pq_HEADERS) \
	-L$(LOCAL_PATH) -lpq -o $@ $^ $(LDLIBS)

$(OUT_DIR)/pqtest: $(pqtest_SRCS) $(OUT_DIR)/libpqclient.so
	$(CC) $(CFLAGS) -I$(pqclient_HEADERS) -L$(LOCAL_PATH) \
	-lpqclient $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUT_DIR)/pqcbtest: $(pqcbtest_SRCS) $(OUT_DIR)/libpqclient.so
	$(CC) $(CFLAGS) -I$(pqclient_HEADERS) -L$(LOCAL_PATH) \
	-lpqclient $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: $(BUILD_TARGETS)

clean:
	rm -f $(OUT_DIR)/*.o $(BUILD_TARGETS)

install:
	install -m 0644 $(OUT_DIR)/libpqclient.so $(TARGET_DIR)/usr/lib
	install -m 0644 $(OUT_DIR)/libpq.so $(TARGET_DIR)/usr/lib/
	install -m 755 $(OUT_DIR)/pqservice $(TARGET_DIR)/usr/bin/
	install -m 755 $(OUT_DIR)/pqtest $(TARGET_DIR)/usr/bin/
	install -m 755 $(OUT_DIR)/pqcbtest $(TARGET_DIR)/usr/bin/

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/libpqclient.so
	rm -f $(TARGET_DIR)/usr/lib/libpq.so
	rm -f $(TARGET_DIR)/usr/bin/pqtest
	rm -f $(TARGET_DIR)/usr/bin/pqcbtest
	rm -f $(TARGET_DIR)/usr/bin/pqservice
