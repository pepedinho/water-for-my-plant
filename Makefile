# ===============================
# Makefile pour ESP32 (ESP-IDF)
# ===============================

# Chemin vers ESP-IDF
IDF_PATH ?= $(HOME)/esp/esp-idf

# Nom de ton projet
PROJECT_NAME := mon_projet

# Dossier de build
BUILD_DIR := build

# ===============================
# Règles principales
# ===============================

# Règle par défaut
.PHONY: all
all: build

# -------------------------------
# Build du projet
# -------------------------------
.PHONY: build
build:
	@echo "🔨 Compiling $(PROJECT_NAME) Project ..."
	@$(IDF_PATH)/export.sh > /dev/null 2>&1 || true && . ~/.profile || true
	@idf.py build

# -------------------------------
# Flash du projet sur l'ESP32
# -------------------------------
.PHONY: flash
flash: build
	@echo "🚀 Flashing $(PROJECT_NAME)  l'ESP32..."
	@idfx flash COM3

.PHONY: monitor
monitor:
	@idfx monitor COM3                                                                                                                            

# -------------------------------
# Nettoyage du build
# -------------------------------
.PHONY: clean
clean:
	@echo "🧹 Cleaning build files..."
	@idf.py fullclean
	@rm -rf $(BUILD_DIR)


.PHONY: re
re: clean build
	@echo "♻️ complet Rebuild for $(PROJECT_NAME)..."

.PHONY: reflash
reflash: re flash
