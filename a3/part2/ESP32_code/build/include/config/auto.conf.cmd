deps_config := \
	/home/adi/esp/esp-idf/components/app_trace/Kconfig \
	/home/adi/esp/esp-idf/components/aws_iot/Kconfig \
	/home/adi/esp/esp-idf/components/bt/Kconfig \
	/home/adi/esp/esp-idf/components/driver/Kconfig \
	/home/adi/esp/esp-idf/components/esp32/Kconfig \
	/home/adi/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/adi/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/adi/esp/esp-idf/components/ethernet/Kconfig \
	/home/adi/esp/esp-idf/components/fatfs/Kconfig \
	/home/adi/esp/esp-idf/components/freertos/Kconfig \
	/home/adi/esp/esp-idf/components/heap/Kconfig \
	/home/adi/esp/esp-idf/components/http_server/Kconfig \
	/home/adi/esp/esp-idf/components/libsodium/Kconfig \
	/home/adi/esp/esp-idf/components/log/Kconfig \
	/home/adi/esp/esp-idf/components/lwip/Kconfig \
	/home/adi/esp/esp-idf/components/mbedtls/Kconfig \
	/home/adi/esp/esp-idf/components/mdns/Kconfig \
	/home/adi/esp/esp-idf/components/mqtt/Kconfig \
	/home/adi/esp/esp-idf/components/openssl/Kconfig \
	/home/adi/esp/esp-idf/components/pthread/Kconfig \
	/home/adi/esp/esp-idf/components/spi_flash/Kconfig \
	/home/adi/esp/esp-idf/components/spiffs/Kconfig \
	/home/adi/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/adi/esp/esp-idf/components/vfs/Kconfig \
	/home/adi/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/adi/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/adi/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/adi/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/adi/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
