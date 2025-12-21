.PHONY: help build upload monitor clean shell compiledb init

help:
	@echo "CrowPanel ÖV Display - Available Commands:"
	@echo "  make init        - Initialize PlatformIO project"
	@echo "  make build       - Build the project"
	@echo "  make upload      - Upload to board"
	@echo "  make monitor     - Open serial monitor"
	@echo "  make flash       - Build + Upload + Monitor"
	@echo "  make clean       - Clean build files"
	@echo "  make compiledb   - Generate compile_commands.json"
	@echo "  make shell       - Open interactive shell"

init:
	docker-compose run --rm platformio project init --board esp32-s3-devkitc-1

build:
	docker-compose run --rm platformio run

upload:
	docker-compose run --rm platformio run -t upload

uploadfs:
	docker-compose run --rm platformio run -t uploadfs

monitor:
	docker-compose run --rm platformio device monitor

flash: build upload monitor

clean:
	docker-compose run --rm platformio run -t clean

compiledb:
	docker-compose run --rm platformio run -t compiledb
	@echo "Fixing compile_commands.json for host usage..."
	@python3 scripts/fix_compile_commands_for_host.py

shell:
	docker-compose run --rm --entrypoint /bin/bash platformio
