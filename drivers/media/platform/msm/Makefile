#
# Makefile for the QCOM specific video device drivers
# based on V4L2.
#
obj-$(CONFIG_MSM_VIDC_V4L2) += vidc/
obj-y += broadcast/
obj-$(CONFIG_DVB_MPQ) += dvb/
ifeq ($(CONFIG_SEC_J7POPLTE_USA_SPR),y)
obj-$(CONFIG_MSMB_CAMERA) += camera_v2_c7lte/
else ifeq ($(CONFIG_SEC_C7LTE_CHN),y)
obj-$(CONFIG_MSMB_CAMERA) += camera_v2_c7lte/
else ifeq ($(CONFIG_SEC_ON7XLTE_CHN),y)
obj-$(CONFIG_MSMB_CAMERA) += camera_v2_c7lte/
else
obj-$(CONFIG_MSMB_CAMERA) += camera_v2/
endif
obj-y += sde/
