
include $(TOPDIR)/rules.mk

PKG_NAME:=bcml
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


define Package/bcml
        SECTION:=BenQ
        CATEGORY:=BenQ
        TITLE:=BCML Middleware Library
        SUBMENU:=Applications
endef

define Package/bcml/description
        BenQ Configuration Management Library for OpenWrt
    	Provides unified API for system configuration management
endef



# 使用 UCI 後端，避免 REST API 複雜性
CMAKE_OPTIONS += \
	-DUCI_API_ENABLE=ON \
	-DREST_API_ENABLE=OFF

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) ./src/* $(PKG_BUILD_DIR)/src/
	$(CP) ./CMakeLists.txt $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/bcml
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(1)/usr/include/bcml/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libbcml.a $(1)/usr/lib/
endef

define Package/bcml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libbcml.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,bcml))
