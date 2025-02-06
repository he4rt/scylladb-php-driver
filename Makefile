# build docs

ENV ?= local

# Build the docs for the specified environment
.PHONY: docs-build
docs-build: docs-clean
	@echo "Building docs for $(ENV) environment..."
	cd docs && DOCS_ENV=$(ENV) pnpm build --out-dir=../build
	DOCS_ENV=$(ENV) ./doctum update doctum-config.php
	@echo "Docs built successfully for $(ENV) environment."

# Clean the docs build directory
.PHONY: docs-clean
docs-clean:
	@echo "Cleaning previous docs build..."
	rm -rf build
	@echo "Cleaned previous docs build."

# Start Docusaurus in development mode with HMR
.PHONY: docs-docusaurus
docs-docusaurus: docs-clean
	@echo "Building docs..."
	cd docs && DOCS_ENV=$(ENV) pnpm start

# Serve the built docs from Doctum and Docusaurus (after building)
.PHONY: docs-serve
docs-serve: docs-build
	@echo "Serving docs..."
	cd build && php -S 0.0.0.0:8000
	@echo "Docs served successfully."
