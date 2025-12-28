# .PHONY: devnode-global rmdev-global
# devnode-global:
# 	@MAJOR=$$(awk '/kmsgpipe_global/ {print $$1}' /proc/devices); \
# 	if [ -z "$$MAJOR" ]; then \
# 		echo "Device not registered; load module first (make insmod)"; \
# 		exit 1; \
# 	fi; \
# 	sudo mknod /dev/$(KMSGPIPE_GLOBAL) c $$MAJOR 0 || true; \
# 	sudo chmod 0666 /dev/$(KMSGPIPE_GLOBAL) || true; \
# 	echo "/dev/$(KMSGPIPE_GLOBAL) created (major=$$MAJOR, minor=0)"