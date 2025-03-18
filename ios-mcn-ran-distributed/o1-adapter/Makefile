# 
# Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The OpenAirInterface Software Alliance licenses this file to You under
# the OAI Public License, Version 1.1  (the "License"); you may not use this file
# except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.openairinterface.org/?page_id=698
# 
# Copyright: Fraunhofer Heinrich Hertz Institute
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
# For more information about the OpenAirInterface (OAI) Software Alliance:
#      contact@openairinterface.org
# 

# Top level makefile to build all

SHELL := /bin/bash

.EXPORT_ALL_VARIABLES:
HOST_IP = $(shell hostname -I|cut -d' ' -f1)
include integration/.env

## help:                Show the help.
.PHONY: help
help:
	@echo "Usage: make <target>"
	@echo ""
	@echo "Targets:"
	@fgrep "##" Makefile | fgrep -v fgrep

## build-o1-adapter:     build o1 adapter docker image
.PHONY: build-o1-adapter
build-o1-adapter: 
	(./build-adapter.sh --adapter)

## build-dev-o1-adapter:     build o1 adapter development docker image
.PHONY: build-dev-o1-adapter
build-dev-o1-adapter: 
	(./build-adapter.sh --dev)

## build-telnet-server:     build telnet test server image
.PHONY: build-telnet-server
build-telnet-server: 
	(cd integration/telnet; docker build . -t oai-telnet-server:latest )

## run-o1-oai-adapter:      Start O1-oai-adapter standalone
.PHONY: run-o1-oai-adapter
run-o1-oai-adapter: build-o1-adapter
	(cd integration; envsubst < ./config/config.json.template > ./config/config.json)
	(cd integration; docker compose up -d)

## run-o1-adapter-telnet:     Start O1-adapter and telnet simulation server
.PHONY: run-o1-adapter-telnet
run-o1-adapter-telnet: build-o1-adapter build-telnet-server
	envsubst < integration/config/config.json.template > integration/config/config.json
	(cd integration; docker compose -f docker-compose.yaml -f docker-compose-telnet.yaml up -d)

## run-smo:      Start O-RAN-SC SMO solution
.PHONY: run-smo
run-smo:
	# TODO: clone from O-RAN oam as soon as J-Release is ready: https://jira.o-ran-sc.org/projects/OAM/issues/OAM-397
	# (cd integration; [ -d oam ] || git clone -b j-release https://gerrit.o-ran-sc.org/r/oam.git)
	(cd integration; [ -d oam ] || (git clone https://gerrit.o-ran-sc.org/r/oam.git; cd oam;git fetch https://gerrit.o-ran-sc.org/r/oam refs/changes/18/12618/9 && git checkout -b change-12618 FETCH_HEAD))
	(cd integration/oam/solution/smo/common; docker compose  up zookeeper kafka messages gateway persistence -d)
	(cd integration/oam/solution/smo/oam; env;  docker compose up -d)
	

## run-o1-oai-adapter-smo:      Start O1-oai-adapter with smo
.PHONY: run-o1-oai-adapter-smo
run-o1-oai-adapter-smo: build-o1-adapter run-smo
	(cd integration; docker compose -f docker-compose.yaml  up -d)

## run-o1-oai-adapter-smo-telnet:      Start O1-oai-adapter with telnet simulation server and  smo
.PHONY: run-o1-oai-adapter-smo-telnet
run-o1-oai-adapter-smo-telnet: run-smo run-o1-adapter-telnet
	

## teardown:      stop and remove all docker containers
.PHONY: teardown
teardown:
	docker rm -f $$(docker ps -aq --filter 'label=deploy=o1-oai-adapter-deployment')

## version:             Check required versions for integration
.PHONY: versions
version:
	docker version
	docker compose version
	git --version
