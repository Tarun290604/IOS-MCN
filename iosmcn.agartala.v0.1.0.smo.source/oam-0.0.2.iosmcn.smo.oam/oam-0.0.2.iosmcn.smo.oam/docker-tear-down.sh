#!/bin/bash

#  ============LICENSE_START===============================================
#  Copyright (C) 2023 Nordix Foundation. All rights reserved.
#  ========================================================================
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  ============LICENSE_END=================================================
#

echo "Stop and remove all containers in the project"

docker compose -p pmrapp -f docker-compose-pmrapp_gen.yaml down
docker compose -p logger -f docker-compose-pmlog_gen.yaml down
docker compose -p grafana -f docker-compose-grafana_gen.yaml down -v
docker compose -p nonrtric -f docker-compose-nonrtric-v1_gen.yaml down
docker compose -p influx -f docker-compose-influxdb_gen.yaml down -v
docker compose -p producers -f docker-compose-producers_gen.yaml down
docker compose -p dfc -f docker-compose-dfc_gen.yaml down
docker compose -p msgbus -f docker-compose-msgbus_gen.yaml down
docker compose -p security -f docker-compose-security_gen.yaml down
docker compose -p sdn -f docker-compose-concol_gen.yaml down
docker compose -p logging -f docker-compose-logging_gen.yaml down
docker compose -p gateway -f docker-compose-common_gen.yaml down
docker compose -p influxbucket -f docker-compose-metbucket_gen.yaml down
docker compose -p telegraf -f docker-compose-telegraf_gen.yaml down

unset $(grep -v '^#' .env | awk 'BEGIN { FS = "=" } ; { print $1 }')
