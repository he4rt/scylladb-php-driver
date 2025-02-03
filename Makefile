# build docs

# Start the Docs development server with hot reload
.PHONY: docs-dev
docs-dev:
	@echo "Building docs..."
	@rm -rf build
	cd docs && DOCS_ENV=local pnpm start

# Build both docs and serve it to localhost testing
## TODO: add environment variables to both builds to match production and development
.PHONY: docs-build
docs-build-dev:
	@echo "Building docs..."
	rm -rf build
	cd docs && pnpm build --out-dir=../build
	DOCS_ENV=local doctum update doctum-config.php
	@echo "Docs built successfully."

# Build both docs and serve it to localhost testing
.PHONY: docs-build
docs-build-prod:
	@echo "Building docs..."
	rm -rf build
	cd docs && pnpm build --out-dir=../build
	DOCS_ENV=production doctum update doctum-config.php
	@echo "Docs built successfully."

# Serve the built docs from Doctum and Docusaurus (after building)
.PHONY: docs-serve
docs-serve:
	@echo "Serving docs..."
	cd build && php -S 0.0.0.0:8000
	@echo "Docs served successfully."