#!/bin/bash

mkdir -p ./docker/scylladb/certs

mkcert \
  -ecdsa \
  -cert-file ./docker/scylladb/certs/cert.pem \
  -key-file ./docker/scylladb/certs/key.pem \
  localhost

mkcert -install \
  -cert-file ./docker/scylladb/certs/cert.pem \
  -key-file ./docker/scylladb/certs/key.pem \

docker compose -f ./docker/docker-compose.ssl.yml up -d