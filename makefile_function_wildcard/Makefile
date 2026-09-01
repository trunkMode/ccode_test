define Get/sym
$(wildcard $(SYM_SHARE_DIR)/*.symvers)
endef

SYM_SHARE_DIR := $(CURDIR)

targetB:
	touch $(SYM_SHARE_DIR)/test.symvers

All:
	make targetAT

targetAT:
	make targetA

targetA: targetB
	@echo $(call Get/sym)
